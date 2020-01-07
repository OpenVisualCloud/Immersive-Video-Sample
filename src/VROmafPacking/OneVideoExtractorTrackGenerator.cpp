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
 */

//!
//! \file:   OneVideoExtractorTrackGenerator.cpp
//! \brief:  One video extractor track generator class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include "OneVideoExtractorTrackGenerator.h"
#include "VideoStream.h"
#include "OneVideoRegionWisePackingGenerator.h"

VCD_NS_BEGIN

OneVideoExtractorTrackGenerator::~OneVideoExtractorTrackGenerator()
{
    DELETE_ARRAY(m_videoIdxInMedia);
    DELETE_ARRAY(m_tilesInViewport);
    DELETE_MEMORY(m_viewInfo);
    DELETE_MEMORY(m_newSPSNalu);
    DELETE_MEMORY(m_newPPSNalu);
}

uint16_t OneVideoExtractorTrackGenerator::CalculateViewportNum()
{
    if (!m_videoIdxInMedia)
        return 0;

    std::map<uint8_t, MediaStream*>::iterator it;
    it = m_streams->find(m_videoIdxInMedia[0]);
    if (it == m_streams->end())
        return 0;
    VideoStream *vs = (VideoStream*)(it->second);
    uint8_t tileInRow = vs->GetTileInRow();
    uint8_t tileInCol = vs->GetTileInCol();
    uint16_t viewportNum = tileInRow * tileInCol;

    return viewportNum;
}

int32_t OneVideoExtractorTrackGenerator::FillDstRegionWisePacking(
    uint8_t viewportIdx,
    RegionWisePacking *dstRwpk)
{
    dstRwpk->projPicWidth  = m_videoWidth;
    dstRwpk->projPicHeight = m_videoHeight;

    int32_t ret = m_rwpkGen->GenerateDstRwpk(viewportIdx, dstRwpk);
    if (ret)
        return ret;

    m_packedPicWidth  = m_rwpkGen->GetPackedPicWidth();
    m_packedPicHeight = m_rwpkGen->GetPackedPicHeight();

    return ERROR_NONE;
}

int32_t OneVideoExtractorTrackGenerator::FillTilesMergeDirection(
    uint8_t viewportIdx,
    TilesMergeDirectionInCol *tilesMergeDir)
{
    if (!tilesMergeDir)
        return OMAF_ERROR_NULL_PTR;

    int32_t ret = m_rwpkGen->GenerateTilesMergeDirection(viewportIdx, tilesMergeDir);
    if (ret)
        return ret;

    return ERROR_NONE;
}

int32_t OneVideoExtractorTrackGenerator::FillDstContentCoverage(
    uint8_t viewportIdx,
    ContentCoverage *dstCovi)
{
    uint8_t tilesNumInViewRow = m_rwpkGen->GetTilesNumInViewportRow();
    uint8_t tileRowNumInView  = m_rwpkGen->GetTileRowNumInViewport();

    uint32_t projRegLeft = (viewportIdx % m_tileInRow) * m_tileWidth;
    uint32_t projRegTop  = (viewportIdx / m_tileInRow) * m_tileHeight;
    uint32_t projRegWidth  = 0;
    uint32_t projRegHeight = 0;

    uint8_t viewIdxInRow = viewportIdx % m_tileInRow;
    uint8_t viewIdxInCol = viewportIdx / m_tileInRow;

    if ((m_tileInRow - viewIdxInRow) >= tilesNumInViewRow)
    {
        for (uint8_t i = viewportIdx; i < (viewportIdx + tilesNumInViewRow); i++)
        {
            projRegWidth += m_tilesInfo[i].tileWidth;
        }
    }
    else
    {
        for (uint8_t i = viewportIdx; i < (viewportIdx + (m_tileInRow - viewIdxInRow)); i++)
        {
            projRegWidth += m_tilesInfo[i].tileWidth;
        }
        for (uint8_t i = (viewIdxInCol*m_tileInRow); i < (viewIdxInCol*m_tileInRow + (tilesNumInViewRow-(m_tileInRow-viewIdxInRow))); i++)
        {
            projRegWidth += m_tilesInfo[i].tileWidth;
        }
    }

    if ((m_tileInCol - viewIdxInCol) >= tileRowNumInView)
    {
        for (uint8_t i = viewportIdx; i < (viewportIdx+m_tileInRow*tileRowNumInView); )
        {
            projRegHeight += m_tilesInfo[i].tileHeight;
            i += m_tileInRow;
        }
    }
    else
    {
        for (uint8_t i = viewportIdx; i < (viewportIdx+(m_tileInCol-viewIdxInCol)*m_tileInRow);)
        {
            projRegHeight += m_tilesInfo[i].tileHeight;
            i += m_tileInRow;
        }
        for (uint8_t i = viewIdxInRow; i < (viewIdxInRow+(tileRowNumInView-(m_tileInCol-viewIdxInCol))*m_tileInRow); )
        {
            projRegHeight += m_tilesInfo[i].tileHeight;
            i += m_tileInRow;
        }
    }

    if (m_projType == VCD::OMAF::ProjectionFormat::PF_ERP)
    {
        dstCovi->coverageShapeType = 1;
    }
    else
    {
        dstCovi->coverageShapeType = 0;
    }

    dstCovi->numRegions          = 1;
    dstCovi->viewIdcPresenceFlag = false;
    dstCovi->defaultViewIdc      = 0;

    dstCovi->sphereRegions = new SphereRegion[dstCovi->numRegions];
    if (!dstCovi->sphereRegions)
        return OMAF_ERROR_NULL_PTR;

    SphereRegion *sphereRegion    = &(dstCovi->sphereRegions[0]);
    memset(sphereRegion, 0, sizeof(SphereRegion));
    sphereRegion->viewIdc         = 0;
    sphereRegion->centreAzimuth   = (int32_t)((((m_videoWidth / 2) - (float)(projRegLeft + projRegWidth / 2)) * 360 * 65536) / m_videoWidth);
    sphereRegion->centreElevation = (int32_t)((((m_videoHeight / 2) - (float)(projRegTop + projRegHeight / 2)) * 180 * 65536) / m_videoHeight);
    sphereRegion->centreTilt      = 0;
    sphereRegion->azimuthRange    = (uint32_t)((projRegWidth * 360.f * 65536) / m_videoWidth);
    sphereRegion->elevationRange  = (uint32_t)((projRegHeight * 180.f * 65536) / m_videoHeight);
    sphereRegion->interpolate     = 0;

    return ERROR_NONE;
}

int32_t OneVideoExtractorTrackGenerator::CheckAndFillInitInfo()
{
    if (!m_initInfo)
        return OMAF_ERROR_NULL_PTR;

    if (m_initInfo->bsNumVideo != 1)
        return OMAF_ERROR_VIDEO_NUM;

    uint8_t actualVideoNum = 0;
    uint8_t totalStreamNum = m_initInfo->bsNumVideo + m_initInfo->bsNumAudio;
    uint8_t vsIdx = 0;
    m_videoIdxInMedia = new uint8_t[totalStreamNum];
    if (!m_videoIdxInMedia)
        return OMAF_ERROR_NULL_PTR;

    for (uint8_t streamIdx = 0; streamIdx < totalStreamNum; streamIdx++)
    {
        BSBuffer *bs = &(m_initInfo->bsBuffers[streamIdx]);
        if (bs->mediaType == VIDEOTYPE)
        {
            m_videoIdxInMedia[vsIdx] = streamIdx;
            vsIdx++;
            actualVideoNum++;
        }
    }

    if (actualVideoNum != m_initInfo->bsNumVideo)
        return OMAF_ERROR_VIDEO_NUM;


    std::map<uint8_t, MediaStream*>::iterator it;
    it = m_streams->find(m_videoIdxInMedia[0]);
    if (it == m_streams->end())
        return OMAF_ERROR_STREAM_NOT_FOUND;

    VideoStream *vs1 = (VideoStream*)(it->second);
    uint16_t width1 = vs1->GetSrcWidth();
    uint16_t height1 = vs1->GetSrcHeight();

    (m_initInfo->viewportInfo)->inWidth    = width1;
    (m_initInfo->viewportInfo)->inHeight   = height1;
    (m_initInfo->viewportInfo)->tileInRow  = vs1->GetTileInRow();
    (m_initInfo->viewportInfo)->tileInCol  = vs1->GetTileInCol();
    (m_initInfo->viewportInfo)->outGeoType = 2; //viewport
    (m_initInfo->viewportInfo)->inGeoType  = vs1->GetProjType();

    m_videoWidth  = width1;
    m_videoHeight = height1;
    m_tileInRow   = vs1->GetTileInRow();
    m_tileInCol   = vs1->GetTileInCol();
    m_tilesInfo   = vs1->GetAllTilesInfo();
    m_tileWidth   = m_tilesInfo[0].tileWidth;
    m_tileHeight  = m_tilesInfo[0].tileHeight;
    m_projType    = (VCD::OMAF::ProjectionFormat)(vs1->GetProjType());

    if ((m_initInfo->segmentationInfo)->extractorTracksPerSegThread == 0)
    {
        if ((m_tileInRow * m_tileInCol) % 4 == 0)
        {
            (m_initInfo->segmentationInfo)->extractorTracksPerSegThread = 4;
        }
        else if ((m_tileInRow * m_tileInCol) % 3 == 0)
        {
            (m_initInfo->segmentationInfo)->extractorTracksPerSegThread = 3;
        }
        else if ((m_tileInRow * m_tileInCol) % 2 == 0)
        {
            (m_initInfo->segmentationInfo)->extractorTracksPerSegThread = 2;
        }
        else
        {
            (m_initInfo->segmentationInfo)->extractorTracksPerSegThread = 1;
        }
    }

    return ERROR_NONE;
}

int32_t OneVideoExtractorTrackGenerator::Initialize()
{
    if (!m_initInfo)
        return OMAF_ERROR_NULL_PTR;

    int32_t ret = CheckAndFillInitInfo();
    if (ret)
        return ret;

    std::map<uint8_t, MediaStream*>::iterator it;
    it = m_streams->find(m_videoIdxInMedia[0]); //high resolution video stream
    if (it == m_streams->end())
        return OMAF_ERROR_STREAM_NOT_FOUND;

    VideoStream *vs = (VideoStream*)(it->second);
    m_360scvpHandle = vs->Get360SCVPHandle();
    m_360scvpParam  = vs->Get360SCVPParam();
    m_origVPSNalu   = vs->GetVPSNalu();
    m_origSPSNalu   = vs->GetSPSNalu();
    m_origPPSNalu   = vs->GetPPSNalu();

    m_tilesInViewport = new TileDef[1024];
    if (!m_tilesInViewport)
        return OMAF_ERROR_NULL_PTR;

    m_viewInfo = new Param_ViewPortInfo;
    if (!m_viewInfo)
        return OMAF_ERROR_NULL_PTR;

    m_viewInfo->viewportWidth  = (m_initInfo->viewportInfo)->viewportWidth;
    m_viewInfo->viewportHeight = (m_initInfo->viewportInfo)->viewportHeight;
    m_viewInfo->viewPortPitch  = (m_initInfo->viewportInfo)->viewportPitch;
    m_viewInfo->viewPortYaw    = (m_initInfo->viewportInfo)->viewportYaw;
    m_viewInfo->viewPortFOVH   = (m_initInfo->viewportInfo)->horizontalFOVAngle;
    m_viewInfo->viewPortFOVV   = (m_initInfo->viewportInfo)->verticalFOVAngle;
    m_viewInfo->geoTypeOutput  = (EGeometryType)((m_initInfo->viewportInfo)->outGeoType);
    m_viewInfo->geoTypeInput   = (EGeometryType)((m_initInfo->viewportInfo)->inGeoType);
    m_viewInfo->faceWidth      = (m_initInfo->viewportInfo)->inWidth;
    m_viewInfo->faceHeight     = (m_initInfo->viewportInfo)->inHeight;
    m_viewInfo->tileNumRow     = (m_initInfo->viewportInfo)->tileInCol;
    m_viewInfo->tileNumCol     = (m_initInfo->viewportInfo)->tileInRow;

    ret = I360SCVP_SetParameter(m_360scvpHandle, ID_SCVP_PARAM_VIEWPORT, (void*)m_viewInfo);
    if (ret)
        return OMAF_ERROR_SCVP_SET_FAILED;

    m_360scvpParam->paramViewPort.viewportWidth  = (m_initInfo->viewportInfo)->viewportWidth;
    m_360scvpParam->paramViewPort.viewportHeight = (m_initInfo->viewportInfo)->viewportHeight;
    m_360scvpParam->paramViewPort.viewPortPitch  = (m_initInfo->viewportInfo)->viewportPitch;
    m_360scvpParam->paramViewPort.viewPortYaw    = (m_initInfo->viewportInfo)->viewportYaw;
    m_360scvpParam->paramViewPort.viewPortFOVH   = (m_initInfo->viewportInfo)->horizontalFOVAngle;
    m_360scvpParam->paramViewPort.viewPortFOVV   = (m_initInfo->viewportInfo)->verticalFOVAngle;
    m_360scvpParam->paramViewPort.geoTypeOutput  = (EGeometryType)((m_initInfo->viewportInfo)->outGeoType);
    m_360scvpParam->paramViewPort.geoTypeInput   = (EGeometryType)((m_initInfo->viewportInfo)->inGeoType);
    m_360scvpParam->paramViewPort.faceWidth      = (m_initInfo->viewportInfo)->inWidth;
    m_360scvpParam->paramViewPort.faceHeight     = (m_initInfo->viewportInfo)->inHeight;
    m_360scvpParam->paramViewPort.tileNumRow     = (m_initInfo->viewportInfo)->tileInCol;
    m_360scvpParam->paramViewPort.tileNumCol     = (m_initInfo->viewportInfo)->tileInRow;

    ret = I360SCVP_process(m_360scvpParam, m_360scvpHandle);
    if (ret)
        return OMAF_ERROR_SCVP_PROCESS_FAILED;

    Param_ViewportOutput paramViewportOutput;
    m_tilesNumInViewport = I360SCVP_getFixedNumTiles(
                    m_tilesInViewport,
                    &paramViewportOutput,
                    m_360scvpHandle);

    m_finalViewportWidth = paramViewportOutput.dstWidthAlignTile;
    m_finalViewportHeight = paramViewportOutput.dstHeightAlignTile;

    if (!m_tilesNumInViewport || m_tilesNumInViewport > 1024)
        return OMAF_ERROR_SCVP_INCORRECT_RESULT;

    m_rwpkGen = new OneVideoRegionWisePackingGenerator();
    if (!m_rwpkGen)
        return OMAF_ERROR_NULL_PTR;

    ret = m_rwpkGen->Initialize(m_streams, m_videoIdxInMedia,
         m_tilesNumInViewport, m_tilesInViewport,
         m_finalViewportWidth, m_finalViewportHeight);
    if (ret)
        return ret;

    return ERROR_NONE;
}

int32_t OneVideoExtractorTrackGenerator::GenerateExtractorTracks(std::map<uint8_t, ExtractorTrack*>& extractorTrackMap, std::map<uint8_t, MediaStream*> *streams)
{
    if (!streams)
        return OMAF_ERROR_NULL_PTR;

    m_viewportNum = CalculateViewportNum();
    if (!m_viewportNum)
        return OMAF_ERROR_VIEWPORT_NUM;

    for (uint8_t i = 0; i < m_viewportNum; i++)
    {
        ExtractorTrack *extractorTrack = new ExtractorTrack(i, streams, (m_initInfo->viewportInfo)->inGeoType);
        if (!extractorTrack)
            return OMAF_ERROR_NULL_PTR;

        int32_t retInit = extractorTrack->Initialize();
        if (retInit)
        {
            LOG(ERROR) << "Failed to initialize extractor track !" << std::endl;
            return retInit;
        }

        FillDstRegionWisePacking(i, extractorTrack->GetRwpk());

        FillTilesMergeDirection(i, extractorTrack->GetTilesMergeDir());

        FillDstContentCoverage(i, extractorTrack->GetCovi());

        extractorTrackMap.insert(std::make_pair(i, extractorTrack));
    }

    int32_t ret = GenerateNewSPS();
    if (ret)
        return ret;

    ret = GenerateNewPPS();
    if (ret)
        return ret;

    std::map<uint8_t, ExtractorTrack*>::iterator it;
    for (it = extractorTrackMap.begin(); it != extractorTrackMap.end(); it++)
    {
        ExtractorTrack *extractorTrack = it->second;
        extractorTrack->SetNalu(m_origVPSNalu, extractorTrack->GetVPS());
        extractorTrack->SetNalu(m_newSPSNalu, extractorTrack->GetSPS());
        extractorTrack->SetNalu(m_newPPSNalu, extractorTrack->GetPPS());

        PicResolution highRes = { m_videoWidth, m_videoHeight };

        std::list<PicResolution>* picResList = extractorTrack->GetPicRes();
        picResList->push_back(highRes);
    }

    return ERROR_NONE;
}

int32_t OneVideoExtractorTrackGenerator::GenerateNewSPS()
{
    if (!m_packedPicWidth || !m_packedPicHeight)
        return OMAF_ERROR_BAD_PARAM;

    if (!m_origSPSNalu || !m_360scvpParam || !m_360scvpHandle)
        return OMAF_ERROR_NULL_PTR;

    if (!(m_origSPSNalu->data) || !(m_origSPSNalu->dataSize))
        return OMAF_ERROR_INVALID_SPS;

    m_newSPSNalu = new Nalu;
    if (!m_newSPSNalu)
        return OMAF_ERROR_NULL_PTR;

    m_newSPSNalu->data = new uint8_t[1024];//include start codes
    if (!m_newSPSNalu->data)
        return OMAF_ERROR_NULL_PTR;

    m_360scvpParam->pInputBitstream   = m_origSPSNalu->data;
    m_360scvpParam->inputBitstreamLen = m_origSPSNalu->dataSize;
    m_360scvpParam->destWidth         = m_packedPicWidth;
    m_360scvpParam->destHeight        = m_packedPicHeight;
    m_360scvpParam->pOutputBitstream  = m_newSPSNalu->data;

    int32_t ret = I360SCVP_GenerateSPS(m_360scvpParam, m_360scvpHandle);
    if (ret)
        return OMAF_ERROR_SCVP_OPERATION_FAILED;

    m_newSPSNalu->dataSize       = m_360scvpParam->outputBitstreamLen;
    m_newSPSNalu->startCodesSize = HEVC_STARTCODES_LEN;
    m_newSPSNalu->naluType       = HEVC_SPS_NALU_TYPE;

    return ERROR_NONE;
}

int32_t OneVideoExtractorTrackGenerator::GenerateNewPPS()
{
    TileArrangement *tileArray = m_rwpkGen->GetMergedTilesArrange();
    if (!tileArray)
        return OMAF_ERROR_NULL_PTR;

    m_newPPSNalu = new Nalu;
    if (!m_newPPSNalu)
        return OMAF_ERROR_NULL_PTR;

    m_newPPSNalu->data     = new uint8_t[1024];//include start codes
    if (!m_newPPSNalu->data)
        return OMAF_ERROR_NULL_PTR;

    m_360scvpParam->pInputBitstream   = m_origPPSNalu->data; //includes start codes
    m_360scvpParam->inputBitstreamLen = m_origPPSNalu->dataSize;

    m_360scvpParam->pOutputBitstream  = m_newPPSNalu->data;

    int32_t ret = I360SCVP_GeneratePPS(m_360scvpParam, tileArray, m_360scvpHandle);
    if (ret)
        return OMAF_ERROR_SCVP_OPERATION_FAILED;

    m_newPPSNalu->dataSize = m_360scvpParam->outputBitstreamLen;
    m_newPPSNalu->startCodesSize = HEVC_STARTCODES_LEN;
    m_newPPSNalu->naluType = HEVC_PPS_NALU_TYPE;

    return ERROR_NONE;
}

VCD_NS_END
