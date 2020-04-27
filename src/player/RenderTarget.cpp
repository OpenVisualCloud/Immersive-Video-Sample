/*
 * Copyright (c) 2019, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.

 *
 */

//!
//! \file     RenderTarget.cpp
//! \brief    Implement class for RenderTarget.
//!

#include "RenderTarget.h"
#include "RenderContext.h"

#include <GL/glu.h>
#include <GL/glu_mangle.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <GLES3/gl3platform.h>
#include <algorithm>
#include <iostream>
#include <chrono>
#include "../trace/MtHQ_tp.h"

VCD_NS_BEGIN

RenderTarget::RenderTarget()
{
    m_fboR2THandle      = 0;
    m_fboOnScreenHandle = 0;
    m_textureOfR2S      = 0;
    m_targetWH.width    = NULL;
    m_targetWH.height   = NULL;
    m_isAllHighResoInView = true;
    m_avgChangedTime    = 0;
}

RenderTarget::~RenderTarget()
{
    RENDERBACKEND::GetInstance()->DeleteFramebuffers(1, &m_fboR2THandle);
    RENDERBACKEND::GetInstance()->DeleteFramebuffers(1, &m_fboOnScreenHandle);
    RENDERBACKEND::GetInstance()->DeleteTextures(1, &m_textureOfR2S);
    if (m_targetWH.width != NULL)
    {
        delete [] m_targetWH.width;
        m_targetWH.width = NULL;
    }
    if (m_targetWH.height != NULL)
    {
        delete [] m_targetWH.height;
        m_targetWH.height = NULL;
    }
    std::cout<<"AVG CHANGED TIME COST : "<<m_avgChangedTime<<"ms"<<std::endl;
}

RenderStatus RenderTarget::Initialize(struct MediaSourceInfo *mediaSourceInfo, uint32_t fboR2T)
{
    if (NULL == mediaSourceInfo)
    {
        return RENDER_ERROR;
    }
    uint32_t width = mediaSourceInfo->sourceWH->width[0];
    uint32_t height = mediaSourceInfo->sourceWH->height[0];
    struct SourceWH targetWH;
    targetWH.width = new uint32_t[1];
    targetWH.width[0] = width;
    targetWH.height = new uint32_t[1];
    targetWH.height[0] = height;
    SetTargetWH(targetWH);
    SetFboR2THandle(fboR2T);
    return RENDER_STATUS_OK;
}

RenderStatus RenderTarget::CreateRenderTarget(RenderBackend *renderBackend)
{
    if (NULL == renderBackend)
    {
        return RENDER_ERROR;
    }

    renderBackend->GenTextures(1, &m_textureOfR2S);
    renderBackend->BindTexture(GL_TEXTURE_2D, m_textureOfR2S);

    renderBackend->PixelStorei(GL_UNPACK_ROW_LENGTH, m_targetWH.width[0]);
    renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    renderBackend->TexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_targetWH.width[0], m_targetWH.height[0], 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    renderBackend->GenFramebuffers(1, &m_fboOnScreenHandle);
    renderBackend->BindFramebuffer(GL_FRAMEBUFFER, m_fboOnScreenHandle);
    renderBackend->FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureOfR2S, 0);

    if (renderBackend->CheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("glCheckFramebufferStatus not complete\n");
        return RENDER_ERROR;
    }
    else
    {
        printf("glCheckFramebufferStatus complete\n");
    }

    return RENDER_STATUS_OK;
}

RenderStatus RenderTarget::Update(RenderBackend *renderBackend, struct RegionInfo *regionInfo, float yaw, float pitch, float hFOV, float vFOV)
{
    if (NULL == renderBackend || NULL == regionInfo)
    {
        return RENDER_ERROR;
    }
    //1. get rwpk info and do blit.
    std::vector<uint32_t> hasHighTileIds;
    std::vector<uint32_t> hasLowTileIds;
    std::vector<std::vector<std::pair<uint32_t, std::vector<uint32_t>>>> regionInfoTransfer;
    //1.1 get high and low tile id and regionInfoTransfer.
    GetRenderMultiSource(regionInfo, hasHighTileIds, hasLowTileIds, regionInfoTransfer);

    renderBackend->BindFramebuffer(GL_READ_FRAMEBUFFER, m_fboR2THandle);
    renderBackend->BindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboOnScreenHandle);
    renderBackend->Clear(GL_COLOR_BUFFER_BIT);//clear buffer.
    //1.2 blit the region tile to FBO2.
    // glBlitFramebuffer(0, 0, FRAME_WIDTH, FRAME_HEIGHT, 0, 0, FRAME_WIDTH, FRAME_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    //the pair.second in regionInfoTransfer is : {projRegLeft, projRegTop, projRegWidth, projRegHeight, packedRegLeft, packedRegTop, packedRegWidth, packedRegHeight, packedPicWidth, packedPicHeight, tileColumn, tileRow};
    uint32_t highWidth = regionInfoTransfer[0][0].second[2];//high resolution picture width
    uint32_t highHeight = regionInfoTransfer[0][0].second[3];//high resolution picture height
    uint32_t lowWidth = regionInfoTransfer[1][0].second[2];//low resolution picture width
    uint32_t lowHeight = regionInfoTransfer[1][0].second[3];//low resolution picture height
    uint16_t highCol = regionInfoTransfer[0][0].second[10];//high resolution tile column
    uint16_t highRow = regionInfoTransfer[0][0].second[11];//high resolution tile row
    uint16_t lowCol = regionInfoTransfer[1][0].second[10];//low resolution tile column
    uint16_t lowRow = regionInfoTransfer[1][0].second[11];//low resolution tile row
    float ratioW = (float)highWidth * highCol / lowWidth / lowCol;// the scale ratio for width
    float ratioH = (float)highHeight * highRow / lowHeight / lowRow;// the sacle ratio for height

    std::vector<uint32_t> TilesInViewport;
    std::chrono::high_resolution_clock clock;
    GetTilesInViewport(yaw, pitch, hFOV, vFOV, highRow, highCol, TilesInViewport);
    bool isAllHighFlag = true;
    static uint64_t start = 0;
    static uint64_t totalChangedTime = 0;
    static uint32_t changedCount = 0;
    for (uint32_t i = 0; i < TilesInViewport.size(); i++)
    {
        if (find(hasHighTileIds.begin(), hasHighTileIds.end(), TilesInViewport[i]) == hasHighTileIds.end())
        {
            isAllHighFlag = false;
            if (m_isAllHighResoInView) // firt time to be blur
            {
                start = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
                LOG(INFO)<<"low resolution part occurs!"<<std::endl;
                //trace
                tracepoint(mthq_tp_provider, T1_change_to_lowQ, changedCount+1);
            }
            break;
        }
    }
    if (isAllHighFlag && !m_isAllHighResoInView) // first time to be clear
    {
        uint64_t end = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
        LOG(INFO)<<"T9' All high resolution part!"<<std::endl<<"cost time : "<<(end-start)<<"ms"<<std::endl;
        //trace
        tracepoint(mthq_tp_provider, T9n_change_to_highQ, changedCount+1);

        totalChangedTime += end - start;
        changedCount++;
        m_avgChangedTime = (float)totalChangedTime / changedCount;

    }
    m_isAllHighResoInView = isAllHighFlag;

    for (uint32_t j = 0; j < regionInfoTransfer[1].size(); j++) // blit order : low first.
    {
        if (find(hasLowTileIds.begin(), hasLowTileIds.end(), regionInfoTransfer[1][j].first) != hasLowTileIds.end())
        {
            uint16_t projRegLeft = regionInfoTransfer[1][j].second[0];
            uint16_t projRegTop = regionInfoTransfer[1][j].second[1];
            uint16_t projRegRight = regionInfoTransfer[1][j].second[2] + projRegLeft;
            uint16_t projRegBottom = regionInfoTransfer[1][j].second[3] + projRegTop;
            uint16_t packedRegLeft = regionInfoTransfer[1][j].second[4];
            uint16_t packedRegTop = regionInfoTransfer[1][j].second[5];
            uint16_t packedRegRight = regionInfoTransfer[1][j].second[6] + packedRegLeft;
            uint16_t packedRegBottom = regionInfoTransfer[1][j].second[7] + packedRegTop;
            glBlitFramebuffer(packedRegLeft, packedRegTop, packedRegRight, packedRegBottom, int(projRegLeft * ratioW), int(projRegTop * ratioH), int(projRegRight * ratioW), int(projRegBottom * ratioH), GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }
    }
    for (uint32_t j = 0; j < regionInfoTransfer[0].size(); j++)
    {
        uint16_t projRegLeft = regionInfoTransfer[0][j].second[0];
        uint16_t projRegTop = regionInfoTransfer[0][j].second[1];
        uint16_t projRegRight = regionInfoTransfer[0][j].second[2] + projRegLeft;
        uint16_t projRegBottom = regionInfoTransfer[0][j].second[3] + projRegTop;
        uint16_t packedRegLeft = regionInfoTransfer[0][j].second[4];
        uint16_t packedRegTop = regionInfoTransfer[0][j].second[5];
        uint16_t packedRegRight = regionInfoTransfer[0][j].second[6] + packedRegLeft;
        uint16_t packedRegBottom = regionInfoTransfer[0][j].second[7] + packedRegTop;
        glBlitFramebuffer(packedRegLeft, packedRegTop, packedRegRight, packedRegBottom, projRegLeft, projRegTop, projRegRight, projRegBottom, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return RENDER_STATUS_OK;
}

std::vector<std::vector<std::pair<uint32_t, std::vector<uint32_t>>>> RenderTarget::TransferRegionInfo(struct RegionInfo *regionInfo)
{
    std::vector<std::vector<std::pair<uint32_t, std::vector<uint32_t>>>> regionInfoTransfer(regionInfo->sourceNumber, std::vector<std::pair<uint32_t, std::vector<uint32_t>>>());
    uint16_t highTileRowNumber = regionInfo->sourceInfo[HIGHRESOTYPE].tileRowNumber;
    uint16_t highTileColumnNumber = regionInfo->sourceInfo[HIGHRESOTYPE].tileColumnNumber;
    uint16_t lowTileRowNumber = regionInfo->sourceInfo[LOWRESOTYPE].tileRowNumber;
    uint16_t lowTileColumnNumber = regionInfo->sourceInfo[LOWRESOTYPE].tileColumnNumber;
    uint16_t numRegion = regionInfo->regionWisePacking->numRegions;

    //for each tileRegion, set up a relationship between tileId and info
    TileType tileType = LOWRESOTYPE;
    for (int32_t i = numRegion-1; i >= 0; i--)
    {
        uint32_t projRegLeft = regionInfo->regionWisePacking->rectRegionPacking[i].projRegLeft;
        uint32_t projRegTop = regionInfo->regionWisePacking->rectRegionPacking[i].projRegTop;
        uint32_t projRegWidth = regionInfo->regionWisePacking->rectRegionPacking[i].projRegWidth;
        uint32_t projRegHeight = regionInfo->regionWisePacking->rectRegionPacking[i].projRegHeight;
        uint16_t packedRegLeft = regionInfo->regionWisePacking->rectRegionPacking[i].packedRegLeft;
        uint16_t packedRegTop = regionInfo->regionWisePacking->rectRegionPacking[i].packedRegTop;
        uint16_t packedRegWidth = regionInfo->regionWisePacking->rectRegionPacking[i].packedRegWidth;
        uint16_t packedRegHeight = regionInfo->regionWisePacking->rectRegionPacking[i].packedRegHeight;
        uint16_t packedPicWidth = regionInfo->regionWisePacking->packedPicWidth;
        uint16_t packedPicHeight = regionInfo->regionWisePacking->packedPicHeight;
        uint16_t tileColumn = tileType == HIGHRESOTYPE ? highTileColumnNumber : lowTileColumnNumber;
        uint16_t tileRow = tileType == HIGHRESOTYPE ? highTileRowNumber : lowTileRowNumber;

        std::pair<uint32_t, uint32_t> coord(projRegLeft / projRegWidth, projRegTop / projRegHeight);
        uint32_t tileId = (coord.first + 1) + tileColumn * coord.second;
        uint32_t tileInfo[12] = {projRegLeft, projRegTop, projRegWidth, projRegHeight, packedRegLeft, packedRegTop, packedRegWidth, packedRegHeight, packedPicWidth, packedPicHeight, tileColumn, tileRow};
        std::vector<uint32_t> tileInformation(tileInfo, tileInfo + 12);
        std::pair<uint32_t, std::vector<uint32_t>> tileInfoPair(tileId, tileInformation);
        regionInfoTransfer[tileType].push_back(tileInfoPair);
        if (i > 0 && projRegLeft == 0 && projRegTop == 0) //when (0,0) occurs and i>0
        {
            tileType = HIGHRESOTYPE;
        }
    }
    return regionInfoTransfer;
}

RenderStatus RenderTarget::GetRenderMultiSource(struct RegionInfo *regionInfo, std::vector<uint32_t> &hasHighTileIds, std::vector<uint32_t> &hasLowTileIds, std::vector<std::vector<std::pair<uint32_t, std::vector<uint32_t>>>> &regionInfoTransfer)
{
    if (NULL == regionInfo)
    {
        return RENDER_ERROR;
    }
    //1.transfer the regionInfo
    regionInfoTransfer = TransferRegionInfo(regionInfo);
    //2 get the high and low tile ids
    for (uint32_t i = 0; i < regionInfoTransfer.size(); i++)
    {
        for (uint32_t j = 0; j < regionInfoTransfer[i].size(); j++)
        {
            if (i == HIGHRESOTYPE)
            {
                hasHighTileIds.push_back(regionInfoTransfer[i][j].first);
            }
            else
            {
                hasLowTileIds.push_back(regionInfoTransfer[i][j].first);
            }
        }
    }
    //erase the similar element
    sort(hasLowTileIds.begin(), hasLowTileIds.end());
    hasLowTileIds.erase(unique(hasLowTileIds.begin(), hasLowTileIds.end()), hasLowTileIds.end());

    return RENDER_STATUS_OK;
}

std::vector<uint32_t> RenderTarget::GetRegionTileId(struct SphereRegion *sphereRegion, struct SourceInfo *sourceInfo)
{
    std::vector<uint32_t> RegionTileId;
    if (NULL == sphereRegion || NULL == sourceInfo)
    {
        return RegionTileId;
    }
    uint32_t width = sourceInfo->sourceWidth;
    uint32_t height = sourceInfo->sourceHeight;
    uint32_t tileRowNumber = sourceInfo->tileRowNumber;
    uint32_t tileColumnNumber = sourceInfo->tileColumnNumber;
    //1. process sphereregion
    uint32_t centerX = float(((sphereRegion->centreAzimuth) >> 16) + 180) / 360 * width;
    uint32_t centerY = float(((sphereRegion->centreElevation) >> 16) + 90) / 180 * height;
    uint32_t marginX = float((sphereRegion->azimuthRange) >> 16) / 360 * width;
    uint32_t marginY = float((sphereRegion->elevationRange) >> 16) / 180 * height;
    //1.1 transfer to lefttop and rightbottom
    uint32_t leftTopX = (centerX - marginX / 2 + width) % width;
    uint32_t leftTopY = (centerY - marginY / 2 + height) % height;
    uint32_t rightBottomX = (centerX + marginX / 2 + width) % width;
    uint32_t rightBottomY = (centerY + marginY / 2 + height) % height;
    u_int32_t delta = 1;
    //fix
    rightBottomX = rightBottomX % (width / tileColumnNumber) == 0 ? (rightBottomX - delta + width) % width : rightBottomX;
    rightBottomY = rightBottomY % (height / tileRowNumber) == 0 ? (rightBottomY - delta + height) % height : rightBottomY;
    //1.2 transfer to (0,0), (0,1), (1,0) …
    uint32_t s1 = leftTopX / (width / tileColumnNumber);
    uint32_t s2 = leftTopY / (height / tileRowNumber);
    uint32_t e1 = rightBottomX / (width / tileColumnNumber);
    uint32_t e2 = rightBottomY / (height / tileRowNumber);

    // need to consider the bundary change fix the problem when region only in one tile using >=
    for (uint32_t p = s1; p <= (e1 >= s1 ? e1 : e1 + tileColumnNumber); p++)
    {
        for (uint32_t q = s2; q <= (e2 >= s2 ? e2 : e2 + tileRowNumber); q++)
        {
            RegionTileId.push_back(p % tileColumnNumber + (q % tileRowNumber) * tileColumnNumber + 1); //index from 1
        }
    }
    sort(RegionTileId.begin(), RegionTileId.end());
    return RegionTileId;
}

RenderStatus RenderTarget::TransferTileIdToRegion(uint32_t tileId, SourceInfo *sourceInfo, SphereRegion *sphereRegion)
{
    if (NULL == sphereRegion || NULL == sourceInfo)
    {
        return RENDER_ERROR;
    }
    //1.transfer tileId to coord()
    uint32_t tileColumnNumber = sourceInfo->tileColumnNumber;
    uint32_t coordX = (tileId - 1) % tileColumnNumber;
    uint32_t coordY = (tileId - 1) / tileColumnNumber;
    //2.transfer to 0-width and 0-height
    sphereRegion->azimuthRange = sourceInfo->sourceWidth / sourceInfo->tileColumnNumber;
    sphereRegion->elevationRange = sourceInfo->sourceHeight / sourceInfo->tileRowNumber;
    sphereRegion->centreAzimuth = coordX * (sourceInfo->sourceWidth / sourceInfo->tileColumnNumber) + sphereRegion->azimuthRange / 2;
    sphereRegion->centreElevation = coordY * (sourceInfo->sourceHeight / sourceInfo->tileRowNumber) + sphereRegion->elevationRange / 2;
    //3.transfer to standard
    sphereRegion->azimuthRange = uint32_t(float(sphereRegion->azimuthRange) / sourceInfo->sourceWidth * 360) << 16;
    sphereRegion->elevationRange = uint32_t(float(sphereRegion->elevationRange) / sourceInfo->sourceHeight * 180) << 16;
    sphereRegion->centreAzimuth = int32_t(float(sphereRegion->centreAzimuth) / sourceInfo->sourceWidth * 360 - 180) << 16;
    sphereRegion->centreElevation = int32_t(float(sphereRegion->centreElevation) / sourceInfo->sourceHeight * 180 - 90) << 16;
    return RENDER_STATUS_OK;
}

RenderStatus RenderTarget::GetTilesInViewport(float yaw, float pitch, float hFOV, float vFOV, uint32_t row, uint32_t col, std::vector<uint32_t>& TilesInViewport)
{
    if (hFOV <= 0 || vFOV <= 0)
    {
        LOG(ERROR)<<"FOV input invalid!"<<std::endl;
        return RENDER_ERROR;
    }
    struct SphereRegion region;
    region.azimuthRange = uint32_t(hFOV) << 16;
    region.elevationRange = uint32_t(vFOV) << 16;
    region.centreAzimuth = uint32_t(yaw) << 16;
    region.centreElevation = uint32_t(pitch) << 16;
    struct SourceInfo info;
    info.sourceWidth = m_targetWH.width[0];
    info.sourceHeight = m_targetWH.height[0];
    info.tileRowNumber = row;
    info.tileColumnNumber = col;
    TilesInViewport = GetRegionTileId(&region, &info);
    return RENDER_STATUS_OK;
}

uint32_t RenderTarget::GetFboR2THandle()
{
    return m_fboR2THandle;
}

uint32_t RenderTarget::GetFboOnScreenHandle()
{
    return m_fboOnScreenHandle;
}

uint32_t RenderTarget::GetTextureOfR2S()
{
    return m_textureOfR2S;
}

struct SourceWH RenderTarget::GetTargetWH()
{
    return m_targetWH;
}

RenderStatus RenderTarget::SetFboOnScreenHandle(uint32_t handle)
{
    m_fboOnScreenHandle = handle;
    return RENDER_STATUS_OK;
}

RenderStatus RenderTarget::SetFboR2THandle(uint32_t handle)
{
    m_fboR2THandle = handle;
    return RENDER_STATUS_OK;
}

RenderStatus RenderTarget::SetTextureOfR2S(uint32_t texture)
{
    m_textureOfR2S = texture;
    return RENDER_STATUS_OK;
}

RenderStatus RenderTarget::SetTargetWH(struct SourceWH targetWH)
{
    m_targetWH = targetWH;
    return RENDER_STATUS_OK;
}

VCD_NS_END
