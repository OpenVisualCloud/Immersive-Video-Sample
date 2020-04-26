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
//! \file:   ExtractorTrackGenerator.cpp
//! \brief:  Extractor track generator class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include <set>

#include "ExtractorTrackGenerator.h"
#include "VideoStream.h"

#include "../trace/Bandwidth_tp.h"

VCD_NS_BEGIN

ExtractorTrackGenerator::~ExtractorTrackGenerator()
{
    DELETE_ARRAY(m_videoIdxInMedia);
    DELETE_ARRAY(m_tilesInViewport);
    DELETE_MEMORY(m_viewInfo);
    DELETE_MEMORY(m_newSPSNalu);
    DELETE_MEMORY(m_newPPSNalu);
}

uint16_t ExtractorTrackGenerator::CalculateViewportNum()
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

int32_t ExtractorTrackGenerator::FillDstRegionWisePacking(
    uint8_t viewportIdx,
    RegionWisePacking *dstRwpk)
{
    dstRwpk->projPicWidth  = m_origResWidth;
    dstRwpk->projPicHeight = m_origResHeight;

    int32_t ret = m_rwpkGen->GenerateDstRwpk(viewportIdx, dstRwpk);
    if (ret)
        return ret;

    m_packedPicWidth  = m_rwpkGen->GetPackedPicWidth();
    m_packedPicHeight = m_rwpkGen->GetPackedPicHeight();

    return ERROR_NONE;
}

int32_t ExtractorTrackGenerator::FillTilesMergeDirection(
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

int32_t ExtractorTrackGenerator::FillDstContentCoverage(
    uint8_t viewportIdx,
    ContentCoverage *dstCovi)
{
    uint8_t tilesNumInViewRow = m_rwpkGen->GetTilesNumInViewportRow();
    uint8_t tileRowNumInView  = m_rwpkGen->GetTileRowNumInViewport();

    uint32_t projRegLeft = (viewportIdx % m_origTileInRow) * m_origTileWidth;
    uint32_t projRegTop  = (viewportIdx / m_origTileInRow) * m_origTileHeight;
    uint32_t projRegWidth  = 0;
    uint32_t projRegHeight = 0;

    uint8_t viewIdxInRow = viewportIdx % m_origTileInRow;
    uint8_t viewIdxInCol = viewportIdx / m_origTileInRow;

    if ((m_origTileInRow - viewIdxInRow) >= tilesNumInViewRow)
    {
        for (uint8_t i = viewportIdx; i < (viewportIdx + tilesNumInViewRow); i++)
        {
            projRegWidth += m_tilesInfo[i].tileWidth;
        }
    }
    else
    {
        for (uint8_t i = viewportIdx; i < (viewportIdx + (m_origTileInRow - viewIdxInRow)); i++)
        {
            projRegWidth += m_tilesInfo[i].tileWidth;
        }
        for (uint8_t i = (viewIdxInCol*m_origTileInRow); i < (viewIdxInCol*m_origTileInRow + (tilesNumInViewRow-(m_origTileInRow-viewIdxInRow))); i++)
        {
            projRegWidth += m_tilesInfo[i].tileWidth;
        }
    }

    if ((m_origTileInCol - viewIdxInCol) >= tileRowNumInView)
    {
        for (uint8_t i = viewportIdx; i < (viewportIdx+m_origTileInRow*tileRowNumInView); )
        {
            projRegHeight += m_tilesInfo[i].tileHeight;
            i += m_origTileInRow;
        }
    }
    else
    {
        for (uint8_t i = viewportIdx; i < (viewportIdx+(m_origTileInCol-viewIdxInCol)*m_origTileInRow);)
        {
            projRegHeight += m_tilesInfo[i].tileHeight;
            i += m_origTileInRow;
        }
        for (uint8_t i = viewIdxInRow; i < (viewIdxInRow+(tileRowNumInView-(m_origTileInCol-viewIdxInCol))*m_origTileInRow); )
        {
            projRegHeight += m_tilesInfo[i].tileHeight;
            i += m_origTileInRow;
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
    sphereRegion->centreAzimuth   = (int32_t)((((m_origResWidth / 2) - (float)(projRegLeft + projRegWidth / 2)) * 360 * 65536) / m_origResWidth);
    sphereRegion->centreElevation = (int32_t)((((m_origResHeight / 2) - (float)(projRegTop + projRegHeight / 2)) * 180 * 65536) / m_origResHeight);
    sphereRegion->centreTilt      = 0;
    sphereRegion->azimuthRange    = (uint32_t)((projRegWidth * 360.f * 65536) / m_origResWidth);
    sphereRegion->elevationRange  = (uint32_t)((projRegHeight * 180.f * 65536) / m_origResHeight);
    sphereRegion->interpolate     = 0;

    return ERROR_NONE;
}

int32_t ExtractorTrackGenerator::CheckAndFillInitInfo()
{
    if (!m_initInfo)
        return OMAF_ERROR_NULL_PTR;

    uint8_t actualVideoNum = 0;
    uint8_t totalStreamNum = m_initInfo->bsNumVideo + m_initInfo->bsNumAudio;
    m_videoIdxInMedia = new uint8_t[totalStreamNum];
    if (!m_videoIdxInMedia)
        return OMAF_ERROR_NULL_PTR;

    for (uint8_t streamIdx = 0; streamIdx < totalStreamNum; streamIdx++)
    {
        BSBuffer *bs = &(m_initInfo->bsBuffers[streamIdx]);
        if (bs->mediaType == VIDEOTYPE)
        {
            actualVideoNum++;
        }
    }

    if (actualVideoNum != m_initInfo->bsNumVideo)
        return OMAF_ERROR_VIDEO_NUM;

    std::set<uint64_t> bitRateRanking;

    std::map<uint8_t, MediaStream*>::iterator it;
    for (it = m_streams->begin(); it != m_streams->end(); it++)
    {
        MediaStream *stream = it->second;
        if (stream->GetMediaType() == VIDEOTYPE)
        {
            VideoStream *vs = (VideoStream*)stream;
            uint64_t bitRate = vs->GetBitRate();
            bitRateRanking.insert(bitRate);
        }
    }

    std::set<uint64_t>::reverse_iterator rateIter = bitRateRanking.rbegin();
    uint8_t vsIdx = 0;
    for ( ; rateIter != bitRateRanking.rend(); rateIter++)
    {
        uint64_t bitRate = *rateIter;
        printf("bitRate is %ld \n", bitRate);
        for (it = m_streams->begin(); it != m_streams->end(); it++)
        {
            MediaStream *stream = it->second;
            if (stream->GetMediaType() == VIDEOTYPE)
            {
                VideoStream *vs = (VideoStream*)stream;
                uint64_t videoBitRate = vs->GetBitRate();
                if (videoBitRate == bitRate)
                {
                    m_videoIdxInMedia[vsIdx] = it->first; //rank video index from largest bitrate to smallest bitrate
                    //printf("m_videoIdxInMedia[%d] is %d \n", vsIdx, m_videoIdxInMedia[vsIdx]);
                    break;
                }
            }
        }
        vsIdx++;
    }

    uint8_t mainVSId = m_videoIdxInMedia[0];
    it = m_streams->find(mainVSId);
    if (it == m_streams->end())
        return OMAF_ERROR_STREAM_NOT_FOUND;

    VideoStream *mainVS = (VideoStream*)(it->second);
    m_origResWidth = mainVS->GetSrcWidth();
    m_origResHeight = mainVS->GetSrcHeight();
    m_origTileInRow = mainVS->GetTileInRow();
    m_origTileInCol = mainVS->GetTileInCol();
    m_tilesInfo = mainVS->GetAllTilesInfo();
    m_origTileWidth = m_tilesInfo[0].tileWidth;
    m_origTileHeight = m_tilesInfo[0].tileHeight;
    m_projType = (VCD::OMAF::ProjectionFormat)(mainVS->GetProjType());

    (m_initInfo->viewportInfo)->inWidth    = m_origResWidth;
    (m_initInfo->viewportInfo)->inHeight   = m_origResHeight;
    (m_initInfo->viewportInfo)->tileInRow  = m_origTileInRow;
    (m_initInfo->viewportInfo)->tileInCol  = m_origTileInCol;
    (m_initInfo->viewportInfo)->outGeoType = 2; //viewport
    (m_initInfo->viewportInfo)->inGeoType  = mainVS->GetProjType();

    if ((m_initInfo->segmentationInfo)->extractorTracksPerSegThread == 0)
    {
        if ((m_origTileInRow * m_origTileInCol) % 4 == 0)
        {
            (m_initInfo->segmentationInfo)->extractorTracksPerSegThread = 4;
        }
        else if ((m_origTileInRow * m_origTileInCol) % 3 == 0)
        {
            (m_initInfo->segmentationInfo)->extractorTracksPerSegThread = 3;
        }
        else if ((m_origTileInRow * m_origTileInCol) % 2 == 0)
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

int32_t ExtractorTrackGenerator::Initialize()
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

    //trace
    if ((EGeometryType)((m_initInfo->viewportInfo)->inGeoType) == EGeometryType::E_SVIDEO_EQUIRECT)
    {
        const char *projType = "ERP";
        tracepoint(bandwidth_tp_provider, initial_viewport_info,
            (m_initInfo->viewportInfo)->viewportWidth,
            (m_initInfo->viewportInfo)->viewportHeight,
            (m_initInfo->viewportInfo)->viewportPitch,
            (m_initInfo->viewportInfo)->viewportYaw,
            (m_initInfo->viewportInfo)->horizontalFOVAngle,
            (m_initInfo->viewportInfo)->verticalFOVAngle,
            projType);
    }
    else if ((EGeometryType)((m_initInfo->viewportInfo)->inGeoType) == EGeometryType::E_SVIDEO_CUBEMAP)
    {
        const char *projType = "CubeMap";
        tracepoint(bandwidth_tp_provider, initial_viewport_info,
            (m_initInfo->viewportInfo)->viewportWidth,
            (m_initInfo->viewportInfo)->viewportHeight,
            (m_initInfo->viewportInfo)->viewportPitch,
            (m_initInfo->viewportInfo)->viewportYaw,
            (m_initInfo->viewportInfo)->horizontalFOVAngle,
            (m_initInfo->viewportInfo)->verticalFOVAngle,
            projType);
    }

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
    m_viewInfo->usageType      = E_PARSER_ONENAL;

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
    m_360scvpParam->paramViewPort.usageType      = E_PARSER_ONENAL;
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

    //trace
    uint16_t highResTileWidth = (vs->GetSrcWidth()) / (vs->GetTileInRow());
    uint16_t highResTileHeight = (vs->GetSrcHeight()) / (vs->GetTileInCol());
    int32_t  selectedTileCols = paramViewportOutput.dstWidthAlignTile / (int32_t)(highResTileWidth);
    int32_t  selectedTileRows = paramViewportOutput.dstHeightAlignTile / (int32_t)(highResTileHeight);
    tracepoint(bandwidth_tp_provider, tiles_selection_redundancy,
                paramViewportOutput.dstWidthNet, paramViewportOutput.dstHeightNet,
                paramViewportOutput.dstWidthAlignTile, paramViewportOutput.dstHeightAlignTile,
                selectedTileRows, selectedTileCols);

    LOG(INFO) << "Calculated Viewport has width " << m_finalViewportWidth << " and height " << m_finalViewportHeight << " ! " << std::endl;

    if (!m_tilesNumInViewport || m_tilesNumInViewport > 1024)
        return OMAF_ERROR_SCVP_INCORRECT_RESULT;

    m_rwpkGen = new RegionWisePackingGenerator();
    if (!m_rwpkGen)
        return OMAF_ERROR_NULL_PTR;

    ret = m_rwpkGen->Initialize(
         m_initInfo->pluginPath, m_initInfo->pluginName,
         m_streams, m_videoIdxInMedia,
         m_tilesNumInViewport, m_tilesInViewport,
         m_finalViewportWidth, m_finalViewportHeight);
    if (ret)
        return ret;

    return ERROR_NONE;
}

int32_t ExtractorTrackGenerator::GenerateExtractorTracks(
    std::map<uint8_t, ExtractorTrack*>& extractorTrackMap,
    std::map<uint8_t, MediaStream*> *streams)
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
        {
            std::map<uint8_t, ExtractorTrack*>::iterator itET = extractorTrackMap.begin();
            for ( ; itET != extractorTrackMap.end(); )
            {
                ExtractorTrack *extractorTrack1 = itET->second;
                DELETE_MEMORY(extractorTrack1);
                extractorTrackMap.erase(itET++);
            }
            extractorTrackMap.clear();
            return OMAF_ERROR_NULL_PTR;
        }

        int32_t retInit = extractorTrack->Initialize();
        if (retInit)
        {
            LOG(ERROR) << "Failed to initialize extractor track !" << std::endl;

            std::map<uint8_t, ExtractorTrack*>::iterator itET = extractorTrackMap.begin();
            for ( ; itET != extractorTrackMap.end(); )
            {
                ExtractorTrack *extractorTrack1 = itET->second;
                DELETE_MEMORY(extractorTrack1);
                extractorTrackMap.erase(itET++);
            }
            extractorTrackMap.clear();
            DELETE_MEMORY(extractorTrack);
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

    std::list<PicResolution> picResolution;
    std::map<uint8_t, MediaStream*>::iterator itStr;
    uint8_t videoNum = m_initInfo->bsNumVideo;
    for (uint8_t vsIdx = 0; vsIdx < videoNum; vsIdx++)
    {
        itStr = m_streams->find(m_videoIdxInMedia[vsIdx]);
        if (itStr == m_streams->end())
            return OMAF_ERROR_STREAM_NOT_FOUND;

        VideoStream *vs = (VideoStream*)(itStr->second);

        PicResolution resolution = { vs->GetSrcWidth(), vs->GetSrcHeight() };
        //printf("one video has res %d x %d \n", resolution.width, resolution.height);
        picResolution.push_back(resolution);
    }

    std::map<uint8_t, ExtractorTrack*>::iterator it;
    for (it = extractorTrackMap.begin(); it != extractorTrackMap.end(); it++)
    {
        ExtractorTrack *extractorTrack = it->second;

        extractorTrack->SetNalu(m_origVPSNalu, extractorTrack->GetVPS());
        extractorTrack->SetNalu(m_newSPSNalu, extractorTrack->GetSPS());
        extractorTrack->SetNalu(m_newPPSNalu, extractorTrack->GetPPS());

        std::list<PicResolution>* picResList = extractorTrack->GetPicRes();
        std::list<PicResolution>::iterator itRes;
        for (itRes = picResolution.begin(); itRes != picResolution.end(); itRes++)
        {
            PicResolution picRes = *itRes;
            picResList->push_back(picRes);
        }
    }

    return ERROR_NONE;
}

int32_t ExtractorTrackGenerator::GenerateNewSPS()
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

int32_t ExtractorTrackGenerator::GenerateNewPPS()
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
