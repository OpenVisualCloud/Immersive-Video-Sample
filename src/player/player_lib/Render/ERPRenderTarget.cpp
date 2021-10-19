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
//! \file     ERPRenderTarget.cpp
//! \brief    Implement class for ERP RenderTarget.
//!
#ifdef _LINUX_OS_
#include "ERPRenderTarget.h"
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
#ifdef _USE_TRACE_
#include "../../../trace/MtHQ_tp.h"
#endif
#include "../Common/RegionData.h"
#include "../Common/DataLog.h"

VCD_NS_BEGIN

ERPRenderTarget::ERPRenderTarget()
{
}

ERPRenderTarget::~ERPRenderTarget()
{
    RenderBackend *renderBackend = RENDERBACKEND::GetInstance();
    renderBackend->DeleteFramebuffers(1, &m_fboOnScreenHandle);
    renderBackend->DeleteTextures(1, m_textureOfR2S);
    std::cout<<"AVG CHANGED TIME COST : "<<m_avgChangedTime<<"ms"<<std::endl;
}

RenderStatus ERPRenderTarget::Initialize(RenderSourceFactory* rsFactory)
{
    if (NULL == rsFactory)
    {
        return RENDER_ERROR;
    }
    this->m_rsFactory = rsFactory;

    return RENDER_STATUS_OK;
}

RenderStatus ERPRenderTarget::CreateRenderTarget()
{
    if(NULL==this->m_rsFactory){
        return RENDER_NULL_HANDLE;
    }

    uint32_t source_number = m_rsFactory->GetSourceNumber();
    SourceResolution *source_resolution = m_rsFactory->GetSourceResolution();

    RenderBackend *renderBackend = RENDERBACKEND::GetInstance();

    renderBackend->GenTextures(source_number, m_textureOfR2S);
    for (uint32_t i = 0; i < source_number; i++)
    {
        uint32_t width = source_resolution[i].width;
        uint32_t height = source_resolution[i].height;
        // bind texture according to quality ranking (1,2,3...)
        renderBackend->BindTexture(GL_TEXTURE_2D, m_textureOfR2S[source_resolution[i].qualityRanking - 1]);
        renderBackend->PixelStorei(GL_UNPACK_ROW_LENGTH, width);
        renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        renderBackend->TexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    }

    renderBackend->GenFramebuffers(1, &m_fboOnScreenHandle);
    renderBackend->BindFramebuffer(GL_FRAMEBUFFER, m_fboOnScreenHandle);

    return RENDER_STATUS_OK;
}

RenderStatus ERPRenderTarget::Update( float yaw, float pitch, float hFOV, float vFOV, uint64_t pts )
{
    if(NULL==this->m_rsFactory){
        return  RENDER_NULL_HANDLE;
    }

    RenderBackend *renderBackend = RENDERBACKEND::GetInstance();
    RenderStatus ret = RENDER_STATUS_OK;

    //1 calculate quality ranking information
    ret = CalcQualityRanking();
    if(RENDER_STATUS_OK != ret) return ret;

    //2 get high and low tile id and regionInfoTransfer.
    std::map<int32_t, std::vector<TileInformation>> regionInfoTransfer;
    ret = GetRenderMultiSource(regionInfoTransfer);
    if(RENDER_STATUS_OK != ret) return ret;

    std::vector<uint32_t> TilesInViewport;
    std::chrono::high_resolution_clock clock;

    std::map<uint32_t, RenderSource*> mapRenderSources = m_rsFactory->GetRenderSources();
    //get effective best quality area
    //FIXME: need to get main video_id and main source from packing. so far, video_id:0 with source:0 is the one.
    // RegionInfo *regionInfo = mapRenderSources[0]->GetCurrentRegionInfo();
    uint32_t highRow = m_rsFactory->GetHighTileRow();
    uint32_t highCol = m_rsFactory->GetHighTileCol();
    GetTilesInViewport(yaw, pitch, hFOV, vFOV, highRow, highCol, TilesInViewport);

    /// judge whether all best quality tiles has coverd the current viewport.
    bool isAllHighFlag = true;
    static uint64_t start = 0;
    static uint64_t totalChangedTime = 0;
    static uint32_t changedCount = 0;
    DataLog *data_log = DATALOG::GetInstance();
    for (uint32_t i = 0; i < TilesInViewport.size(); i++)
    {
        std::vector<TileInformation> listBest = mQualityRankingInfo.mapQualitySelection[mQualityRankingInfo.mainQualityRanking];
        if (!findTileID(listBest, TilesInViewport[i]))//(find(listBest.begin(), listBest.end(), TilesInViewport[i]) == listBest.end())
        {
            isAllHighFlag = false;
            if (m_isAllHighQualityInView) // firt time to be blur
            {
                start = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
                if (data_log != nullptr) {
                    data_log->SetSwitchStartTime(start);
                }
                LOG(INFO)<<"[FrameSequences][Low]: low resolution part occurs! pts is " << pts <<std::endl;
#ifdef _USE_TRACE_
                //trace
                tracepoint(mthq_tp_provider, T0_change_to_lowQ, changedCount+1, pts);
#endif
            }
            break;
        }
    }
    if (isAllHighFlag && !m_isAllHighQualityInView) // first time to be clear
    {
        uint64_t end = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
        if (data_log != nullptr) {
            data_log->SetSwitchEndTime(end);
        }
        LOG(INFO)<<"[FrameSequences][High]: T9' All high resolution part! pts is " << pts <<" cost time : "<<(end-start)<<"ms"<<std::endl;
#ifdef _USE_TRACE_
        //trace
        tracepoint(mthq_tp_provider, T12_change_to_highQ, changedCount+1, pts);
#endif
        totalChangedTime += end - start;
        changedCount++;
        m_avgChangedTime = (float)totalChangedTime / changedCount;
        LOG(INFO) << "total change time " << changedCount << std::endl;
    }
    m_isAllHighQualityInView = isAllHighFlag;

    /// blit from render source to render target
    //renderBackend->BindFramebuffer(GL_READ_FRAMEBUFFER, rs->GetFboR2THandle());
    renderBackend->BindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboOnScreenHandle);
    renderBackend->Clear(GL_COLOR_BUFFER_BIT);//clear buffer.

    //get render source through video_id which the tile is belongs to.
    for(auto it=mQualityRankingInfo.mapQualitySelection.begin(); it!=mQualityRankingInfo.mapQualitySelection.end(); it++){
        //render low quality first
        std::vector<TileInformation> vec_tile = it->second;
        if(it->first != mQualityRankingInfo.mainQualityRanking){
            for(auto itq=vec_tile.begin(); itq!=vec_tile.end(); itq++){
                TileInformation ti = *itq;
                RenderSource* rs = mapRenderSources[ti.video_id];
                renderBackend->BindFramebuffer(GL_READ_FRAMEBUFFER, rs->GetFboR2THandle());
                int32_t projFormat = m_rsFactory->GetProjectionFormat();
                // in ERP there is only highest texture to be rendered on screen
                if (projFormat == VCD::OMAF::PF_ERP){
                    m_activeTextureId = mQualityRankingInfo.mainQualityRanking - 1;
                    renderBackend->FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureOfR2S[m_activeTextureId], 0);
                }
                else{ // in Planar there is serveral textures to be rendered on screen
                    m_activeTextureId = it->first - 1; // quality ranking - 1 (quality ranking index from 1)
                    renderBackend->FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureOfR2S[m_activeTextureId], 0);
                }
                glBlitFramebuffer( ti.packedRegLeft,
                                   ti.packedRegTop,
                                   ti.packedRegLeft + ti.packedRegWidth,
                                   ti.packedRegTop + ti.packedRegHeight,
                                   ti.projRegLeft,
                                   ti.projRegTop,
                                   ti.projRegLeft + ti.projRegWidth,
                                   ti.projRegTop + ti.projRegHeight,
                                   GL_COLOR_BUFFER_BIT, GL_NEAREST);
                renderBackend->BindFramebuffer(GL_READ_FRAMEBUFFER, 0);
            }
        }
    }


    std::vector<TileInformation> vec_main = mQualityRankingInfo.mapQualitySelection[mQualityRankingInfo.mainQualityRanking];
    for(auto itm=vec_main.begin(); itm!=vec_main.end(); itm++){
        TileInformation ti = *itm;
        RenderSource* rs = mapRenderSources[ti.video_id];
        renderBackend->BindFramebuffer(GL_READ_FRAMEBUFFER, rs->GetFboR2THandle());
        m_activeTextureId = mQualityRankingInfo.mainQualityRanking - 1;
        renderBackend->FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureOfR2S[m_activeTextureId], 0);
        glBlitFramebuffer( ti.packedRegLeft,
                           ti.packedRegTop,
                           ti.packedRegLeft + ti.packedRegWidth,
                           ti.packedRegTop + ti.packedRegHeight,
                           ti.projRegLeft,
                           ti.projRegTop,
                           ti.projRegLeft + ti.projRegWidth,
                           ti.projRegTop + ti.projRegHeight,
                           GL_COLOR_BUFFER_BIT, GL_NEAREST);
        renderBackend->BindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    }

    return RENDER_STATUS_OK;
}

RenderStatus ERPRenderTarget::UpdateDisplayTex()
{
    return RENDER_STATUS_OK;
}

bool ERPRenderTarget::findTileID(std::vector<TileInformation> vecTile, uint32_t tile_id)
{
    for(auto it=vecTile.begin(); it!=vecTile.end(); it++){
        if(it->tile_id == tile_id) return true;
    }

    return false;
}

RenderStatus ERPRenderTarget::CalcQualityRanking()
{
    if(NULL==this->m_rsFactory){
        return  RENDER_NULL_HANDLE;
    }

    std::map<uint32_t, RenderSource*> mapRenderSources = m_rsFactory->GetRenderSources();
    std::list<int32_t> listQuality;
    uint32_t errorCnt = 0;
    for(auto it=mapRenderSources.begin(); it!=mapRenderSources.end(); it++){
        RegionData *regionInfo = it->second->GetCurrentRegionInfo();
        if (regionInfo == NULL || regionInfo->GetRegionWisePacking() == NULL \
             || regionInfo->GetSourceInfo() == NULL || regionInfo->GetSourceInRegion() > 2)
        {
            LOG(INFO)<<"region information is invalid!"<<endl;
            errorCnt++;
            continue;
        }
        // LOG(INFO)<<"regionInfo ptr:"<<regionInfo->GetSourceInRegion()<<" rwpk:"<<regionInfo->GetRegionWisePacking()->rectRegionPacking<<" source:"<<regionInfo->GetSourceInfo()->width<<endl;
        for(int32_t i=0; i<regionInfo->GetSourceInRegion(); i++)
            listQuality.push_back(regionInfo->GetSourceInfo()[i].qualityRanking);
    }
    if (errorCnt == mapRenderSources.size())
    {
        return RENDER_ERROR;
    }
    listQuality.sort();
    listQuality.unique();

    /// always think the high quality is with smallest quality ranking value;
    mQualityRankingInfo.mainQualityRanking = listQuality.front();
    mQualityRankingInfo.numQuality = listQuality.size();
    mQualityRankingInfo.mapQualitySelection.clear();
    std::vector<TileInformation> TileIDs[mQualityRankingInfo.numQuality];
    int i = 0;
    for(auto it=listQuality.begin(); it!=listQuality.end();it++){
        mQualityRankingInfo.mapQualitySelection[*it]=TileIDs[i];
        i++;
    }

    return RENDER_STATUS_OK;
}

int32_t ERPRenderTarget::findQuality(RegionData *regionInfo, RectangularRegionWisePacking rectRWP, int32_t& source_idx)
{
    //get the quality ranking and source index based on tile's RectangularRegionWisePacking through the tile position in the packed source.
    for(int32_t i=0; i<regionInfo->GetSourceInRegion(); i++){
        if(  (rectRWP.packedRegLeft >= regionInfo->GetSourceInfo()[i].left)
           &&(rectRWP.packedRegLeft < regionInfo->GetSourceInfo()[i].left + regionInfo->GetSourceInfo()[i].width)
           &&(rectRWP.packedRegTop >= regionInfo->GetSourceInfo()[i].top)
           &&(rectRWP.packedRegTop < regionInfo->GetSourceInfo()[i].top + regionInfo->GetSourceInfo()[i].height) ){
               source_idx = i;
               return regionInfo->GetSourceInfo()[i].qualityRanking;
           }
    }
    return 0;
}

 RenderStatus ERPRenderTarget::TransferRegionInfo(std::map<int32_t, std::vector<TileInformation>>& org_region)
{
    if(NULL==this->m_rsFactory){
        return  RENDER_NULL_HANDLE;
    }

    RenderStatus ret = RENDER_STATUS_OK;

    std::map<uint32_t, RenderSource*> mapRenderSources = m_rsFactory->GetRenderSources();

    org_region.clear();

    //add all tiles in different source into a map and organized via quality ranking
    for(auto it=mapRenderSources.begin(); it!=mapRenderSources.end(); it++){
        uint32_t video_id = it->first;
        RegionData *regionInfo = it->second->GetCurrentRegionInfo();
        if (regionInfo == NULL || regionInfo->GetRegionWisePacking() == NULL \
        || regionInfo->GetSourceInfo() == NULL || regionInfo->GetSourceInRegion() > 2 || regionInfo->GetSourceInRegion() <= 0){
            continue;
        }
        uint16_t numRegion = regionInfo->GetRegionWisePacking()->numRegions;
        for(int32_t idx=0; idx<numRegion; idx++){
            TileInformation tile_info;
            tile_info.projRegLeft     = regionInfo->GetRegionWisePacking()->rectRegionPacking[idx].projRegLeft;
            tile_info.projRegTop      = regionInfo->GetRegionWisePacking()->rectRegionPacking[idx].projRegTop;
            tile_info.projRegWidth    = regionInfo->GetRegionWisePacking()->rectRegionPacking[idx].projRegWidth;
            tile_info.projRegHeight   = regionInfo->GetRegionWisePacking()->rectRegionPacking[idx].projRegHeight;
            tile_info.packedRegLeft   = regionInfo->GetRegionWisePacking()->rectRegionPacking[idx].packedRegLeft;
            tile_info.packedRegTop    = regionInfo->GetRegionWisePacking()->rectRegionPacking[idx].packedRegTop;
            tile_info.packedRegWidth  = regionInfo->GetRegionWisePacking()->rectRegionPacking[idx].packedRegWidth;
            tile_info.packedRegHeight = regionInfo->GetRegionWisePacking()->rectRegionPacking[idx].packedRegHeight;
            tile_info.packedPicWidth  = regionInfo->GetRegionWisePacking()->packedPicWidth;
            tile_info.packedPicHeight = regionInfo->GetRegionWisePacking()->packedPicHeight;
            tile_info.video_id        = video_id;
            std::pair<uint32_t, uint32_t> coord(tile_info.projRegLeft / tile_info.projRegWidth, tile_info.projRegTop / tile_info.projRegHeight);
            int32_t source_idx = 0;
            int32_t quality = findQuality(regionInfo, regionInfo->GetRegionWisePacking()->rectRegionPacking[idx], source_idx);
            tile_info.tile_id = (coord.first + 1) + m_rsFactory->GetHighTileCol() * coord.second;

            mQualityRankingInfo.mapQualitySelection[quality].push_back(tile_info);
        }
        it->second->SafeDeleteRegionInfo();
        regionInfo = NULL;
    }
    return ret;
}

RenderStatus ERPRenderTarget::GetRenderMultiSource(std::map<int32_t, std::vector<TileInformation>> &regionInfoTransfer)
{
    //1.transfer the regionInfo
    return TransferRegionInfo(regionInfoTransfer);
}

std::vector<uint32_t> ERPRenderTarget::GetRegionTileId(struct SphereRegion *sphereRegion, struct SourceInfo *sourceInfo)
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

RenderStatus ERPRenderTarget::TransferTileIdToRegion(uint32_t tileId, struct SourceInfo *sourceInfo, SphereRegion *sphereRegion)
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

RenderStatus ERPRenderTarget::GetTilesInViewport(float yaw, float pitch, float hFOV, float vFOV, uint32_t row, uint32_t col, std::vector<uint32_t>& TilesInViewport)
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
    info.sourceWidth = m_rsFactory->GetSourceResolution()[0].width;
    info.sourceHeight = m_rsFactory->GetSourceResolution()[0].height;
    info.tileRowNumber = row;
    info.tileColumnNumber = col;
    TilesInViewport = GetRegionTileId(&region, &info);
    return RENDER_STATUS_OK;
}

VCD_NS_END
#endif