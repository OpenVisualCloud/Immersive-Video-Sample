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
//! \file:   TwoResExtractorTrackGenerator.cpp
//! \brief:  Two resolutions extractor track generator class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include "TwoResExtractorTrackGenerator.h"
#include "VideoStream.h"
#include "TwoResRegionWisePackingGenerator.h"

VCD_NS_BEGIN

TwoResExtractorTrackGenerator::~TwoResExtractorTrackGenerator()
{
    DELETE_ARRAY(m_videoIdxInMedia);
    DELETE_ARRAY(m_tilesInViewport);
    DELETE_MEMORY(m_viewInfo);
    DELETE_MEMORY(m_newSPSNalu);
    DELETE_MEMORY(m_newPPSNalu);
}

uint16_t TwoResExtractorTrackGenerator::CalculateViewportNum()
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

int32_t TwoResExtractorTrackGenerator::FillDstRegionWisePacking(
    uint8_t viewportIdx,
    RegionWisePacking *dstRwpk)
{
    dstRwpk->projPicWidth  = m_highResWidth;
    dstRwpk->projPicHeight = m_highResHeight;

    int32_t ret = m_rwpkGen->GenerateDstRwpk(viewportIdx, dstRwpk);
    if (ret)
        return ret;

    m_packedPicWidth  = m_rwpkGen->GetPackedPicWidth();
    m_packedPicHeight = m_rwpkGen->GetPackedPicHeight();

    return ERROR_NONE;
}

int32_t TwoResExtractorTrackGenerator::FillTilesMergeDirection(
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

int32_t TwoResExtractorTrackGenerator::FillDstContentCoverage(
    uint8_t viewportIdx,
    ContentCoverage *dstCovi)
{
    uint8_t tilesNumInViewRow = m_rwpkGen->GetTilesNumInViewportRow();
    uint8_t tileRowNumInView  = m_rwpkGen->GetTileRowNumInViewport();

    uint32_t projRegLeft = (viewportIdx % m_hrTileInRow) * m_hrTileWidth;
    uint32_t projRegTop  = (viewportIdx / m_hrTileInRow) * m_hrTileHeight;
    uint32_t projRegWidth  = 0;
    uint32_t projRegHeight = 0;

    uint8_t viewIdxInRow = viewportIdx % m_hrTileInRow;
    uint8_t viewIdxInCol = viewportIdx / m_hrTileInRow;

    if ((m_hrTileInRow - viewIdxInRow) >= tilesNumInViewRow)
    {
        for (uint8_t i = viewportIdx; i < (viewportIdx + tilesNumInViewRow); i++)
        {
            projRegWidth += m_tilesInfo[i].tileWidth;
        }
    }
    else
    {
        for (uint8_t i = viewportIdx; i < (viewportIdx + (m_hrTileInRow - viewIdxInRow)); i++)
        {
            projRegWidth += m_tilesInfo[i].tileWidth;
        }
        for (uint8_t i = (viewIdxInCol*m_hrTileInRow); i < (viewIdxInCol*m_hrTileInRow + (tilesNumInViewRow-(m_hrTileInRow-viewIdxInRow))); i++)
        {
            projRegWidth += m_tilesInfo[i].tileWidth;
        }
    }

    if ((m_hrTileInCol - viewIdxInCol) >= tileRowNumInView)
    {
        for (uint8_t i = viewportIdx; i < (viewportIdx+m_hrTileInRow*tileRowNumInView); )
        {
            projRegHeight += m_tilesInfo[i].tileHeight;
            i += m_hrTileInRow;
        }
    }
    else
    {
        for (uint8_t i = viewportIdx; i < (viewportIdx+(m_hrTileInCol-viewIdxInCol)*m_hrTileInRow);)
        {
            projRegHeight += m_tilesInfo[i].tileHeight;
            i += m_hrTileInRow;
        }
        for (uint8_t i = viewIdxInRow; i < (viewIdxInRow+(tileRowNumInView-(m_hrTileInCol-viewIdxInCol))*m_hrTileInRow); )
        {
            projRegHeight += m_tilesInfo[i].tileHeight;
            i += m_hrTileInRow;
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
    sphereRegion->centreAzimuth   = (int32_t)((((m_highResWidth / 2) - (float)(projRegLeft + projRegWidth / 2)) * 360 * 65536) / m_highResWidth);
    sphereRegion->centreElevation = (int32_t)((((m_highResHeight / 2) - (float)(projRegTop + projRegHeight / 2)) * 180 * 65536) / m_highResHeight);
    sphereRegion->centreTilt      = 0;
    sphereRegion->azimuthRange    = (uint32_t)((projRegWidth * 360.f * 65536) / m_highResWidth);
    sphereRegion->elevationRange  = (uint32_t)((projRegHeight * 180.f * 65536) / m_highResHeight);
    sphereRegion->interpolate     = 0;

    return ERROR_NONE;
}

int32_t TwoResExtractorTrackGenerator::CheckAndFillInitInfo()
{
    if (!m_initInfo)
        return OMAF_ERROR_NULL_PTR;

    if (m_initInfo->bsNumVideo != 2)
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
    it = m_streams->find(m_videoIdxInMedia[1]);
    if (it == m_streams->end())
        return OMAF_ERROR_STREAM_NOT_FOUND;

    VideoStream *vs2 = (VideoStream*)(it->second);
    uint16_t width2 = vs2->GetSrcWidth();
    uint16_t height2 = vs2->GetSrcHeight();

    if ((width1 == width2) && (height1 == height2))
        return OMAF_ERROR_VIDEO_RESOLUTION;

    if ((width1 * height1) == (width2 * height2))
        return OMAF_ERROR_VIDEO_RESOLUTION;

    if ((width1 * height1) > (width2 * height2))
    {
        (m_initInfo->viewportInfo)->inWidth    = width1;
        (m_initInfo->viewportInfo)->inHeight   = height1;
        (m_initInfo->viewportInfo)->tileInRow  = vs1->GetTileInRow();
        (m_initInfo->viewportInfo)->tileInCol  = vs1->GetTileInCol();
        (m_initInfo->viewportInfo)->outGeoType = 2; //viewport
        (m_initInfo->viewportInfo)->inGeoType  = vs1->GetProjType();

        m_highResWidth  = width1;
        m_highResHeight = height1;
        m_hrTileInRow   = vs1->GetTileInRow();
        m_hrTileInCol   = vs1->GetTileInCol();
        m_tilesInfo     = vs1->GetAllTilesInfo();
        m_hrTileWidth   = m_tilesInfo[0].tileWidth;
        m_hrTileHeight  = m_tilesInfo[0].tileHeight;
        m_projType      = (VCD::OMAF::ProjectionFormat)(vs1->GetProjType());
        m_lowResWidth   = width2;
        m_lowResHeight  = height2;
    } else {
        (m_initInfo->viewportInfo)->inWidth    = width2;
        (m_initInfo->viewportInfo)->inHeight   = height2;
        (m_initInfo->viewportInfo)->tileInRow  = vs2->GetTileInRow();
        (m_initInfo->viewportInfo)->tileInCol  = vs2->GetTileInCol();
        (m_initInfo->viewportInfo)->outGeoType = 2; //viewport
        (m_initInfo->viewportInfo)->inGeoType  = vs2->GetProjType();

        m_highResWidth  = width2;
        m_highResHeight = height2;
        m_hrTileInRow   = vs2->GetTileInRow();
        m_hrTileInCol   = vs2->GetTileInCol();
        m_tilesInfo     = vs2->GetAllTilesInfo();
        m_hrTileWidth   = m_tilesInfo[0].tileWidth;
        m_hrTileHeight  = m_tilesInfo[0].tileHeight;
        m_projType      = (VCD::OMAF::ProjectionFormat)(vs2->GetProjType());
        m_lowResWidth   = width1;
        m_lowResHeight  = height1;

        uint8_t tempIdx = m_videoIdxInMedia[1];
        m_videoIdxInMedia[1] = m_videoIdxInMedia[0];
        m_videoIdxInMedia[0] = tempIdx; //m_videoIdxInMedia[0] is always corresponding to the high resolution video stream
    }

    if ((m_initInfo->segmentationInfo)->extractorTracksPerSegThread == 0)
    {
        if ((m_hrTileInRow * m_hrTileInCol) % 4 == 0)
        {
            (m_initInfo->segmentationInfo)->extractorTracksPerSegThread = 4;
        }
        else if ((m_hrTileInRow * m_hrTileInCol) % 3 == 0)
        {
            (m_initInfo->segmentationInfo)->extractorTracksPerSegThread = 3;
        }
        else if ((m_hrTileInRow * m_hrTileInCol) % 2 == 0)
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

int32_t TwoResExtractorTrackGenerator::Initialize()
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

    LOG(INFO) << "Calculated Viewport has width " << m_finalViewportWidth << " and height " << m_finalViewportHeight << " ! " << std::endl;

    if (!m_tilesNumInViewport || m_tilesNumInViewport > 1024)
        return OMAF_ERROR_SCVP_INCORRECT_RESULT;

    m_rwpkGen = new TwoResRegionWisePackingGenerator();
    if (!m_rwpkGen)
        return OMAF_ERROR_NULL_PTR;

    ret = m_rwpkGen->Initialize(m_streams, m_videoIdxInMedia,
         m_tilesNumInViewport, m_tilesInViewport,
         m_finalViewportWidth, m_finalViewportHeight);
    if (ret)
        return ret;

    return ERROR_NONE;
}

int32_t TwoResExtractorTrackGenerator::GenerateExtractorTracks(std::map<uint8_t, ExtractorTrack*>& extractorTrackMap, std::map<uint8_t, MediaStream*> *streams)
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

    std::map<uint8_t, ExtractorTrack*>::iterator it;
    for (it = extractorTrackMap.begin(); it != extractorTrackMap.end(); it++)
    {
        ExtractorTrack *extractorTrack = it->second;
        //extractorTrack->SetVPS(m_origVPSNalu);
        //extractorTrack->SetSPS(m_newSPSNalu);
        //extractorTrack->SetPPS(m_newPPSNalu);
        extractorTrack->SetNalu(m_origVPSNalu, extractorTrack->GetVPS());
        extractorTrack->SetNalu(m_newSPSNalu, extractorTrack->GetSPS());
        extractorTrack->SetNalu(m_newPPSNalu, extractorTrack->GetPPS());

        PicResolution highRes = { m_highResWidth, m_highResHeight };
        PicResolution lowRes  = { m_lowResWidth, m_lowResHeight };

        std::list<PicResolution>* picResList = extractorTrack->GetPicRes();
        picResList->push_back(highRes);
        picResList->push_back(lowRes);
    }

    return ERROR_NONE;
}

int32_t TwoResExtractorTrackGenerator::GenerateNewSPS()
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

int32_t TwoResExtractorTrackGenerator::GenerateNewPPS()
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
