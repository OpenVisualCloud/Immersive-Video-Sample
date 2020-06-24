/*
 * Copyright (c) 2018, Intel Corporation
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
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "assert.h"
#include "360SCVPTiledstreamAPI.h"
#include "360SCVPViewportAPI.h"
#include "360SCVPMergeStreamAPI.h"
#include "360SCVPCommonDef.h"
#include "360SCVPHevcEncHdr.h"
#include "360SCVPImpl.h"
#include "360SCVPHevcTileMerge.h"


TstitchStream::TstitchStream()
{
    m_pOutTile = new TileDef[1000];
    m_pUpLeft = new point[6];
    m_pDownRight = new point[6];
    m_pNalInfo[0] = new nal_info[1000];
    m_pNalInfo[1] = new nal_info[1000];
    m_hevcState = new HEVCState;
    if (m_hevcState)
    {
        memset(m_hevcState, 0, sizeof(HEVCState));
        m_hevcState->sps_active_idx = -1;
    }
    memset(&m_pViewportParam, 0, sizeof(generateViewPortParam));
    memset(&m_mergeStreamParam, 0, sizeof(param_mergeStream));
    memset(&m_streamStitch, 0, sizeof(param_gen_tiledStream));
    m_pViewport = NULL;
    m_pMergeStream = NULL;
    m_pSteamStitch = NULL;
    m_seiFramePacking_enable = 0;
    m_seiPayloadType = 0;
    m_seiProj_enable = 0;
    m_seiSphereRot_enable = 0;
    m_seiRWPK_enable = 0;
    m_seiViewport_enable = 0;
    m_specialDataLen[0] = 0;
    m_specialDataLen[1] = 0;
    m_hrTilesInRow = 1;
    m_hrTilesInCol = 1;
    m_lrTilesInRow = 1;
    m_lrTilesInCol = 1;
    m_bSPSReady = 0;
    m_bPPSReady = 0;
    m_tileWidthCountSel[0] = 0;
    m_tileHeightCountSel[0] = 0;
    m_specialInfo[0] = new unsigned char[200];
    m_specialInfo[1] = new unsigned char[200];
    m_sliceHeaderLen = 0;
    m_dstWidthNet = 0;
    m_dstHeightNet = 0;
    m_maxSelTiles = 0;
    m_pRWPK = nullptr;
    m_pSeiViewport = nullptr;
    m_pFramePacking = nullptr;
    m_pSphereRot = nullptr;
    m_pSeiViewport = nullptr;
    m_viewportDestWidth = 0;
    m_viewportDestHeight = 0;
    m_dataSize = 0;
    m_data = nullptr;
    m_startCodesSize = 0;
    m_nalType = 0;
    m_projType = 0;
    memset(&m_sliceType, 0, sizeof(SliceType));
    m_usedType = 0;
    m_xTopLeftNet = 0;
    m_yTopLeftNet = 0;
    m_dstRwpk = RegionWisePacking();
}

TstitchStream::TstitchStream(TstitchStream& other)
{
    m_pOutTile = new TileDef[1000];
    memcpy(m_pOutTile, other.m_pOutTile, 1000 * sizeof(TileDef));
    m_pUpLeft = new point[6];
    memcpy(m_pUpLeft, other.m_pUpLeft, 6 * sizeof(point));
    m_pDownRight = new point[6];
    memcpy(m_pDownRight, other.m_pDownRight, 6 * sizeof(point));
    m_pNalInfo[0] = new nal_info[1000];
    memcpy(m_pNalInfo[0], other.m_pNalInfo[0], 1000 * sizeof(nal_info));
    m_pNalInfo[1] = new nal_info[1000];
    memcpy(m_pNalInfo[1], other.m_pNalInfo[1], 1000 * sizeof(nal_info));
    m_hevcState = new HEVCState;
    if (m_hevcState)
    {
        memcpy(m_hevcState, other.m_hevcState, sizeof(HEVCState));
    }

    memcpy(&m_pViewportParam, &(other.m_pViewportParam), sizeof(generateViewPortParam));
    memcpy(&m_mergeStreamParam, &(other.m_mergeStreamParam), sizeof(param_mergeStream));
    memcpy(&m_streamStitch, &(other.m_streamStitch), sizeof(param_gen_tiledStream));
    m_pViewport = NULL;
    m_pMergeStream = NULL;
    m_pSteamStitch = NULL;
    m_seiFramePacking_enable = other.m_seiFramePacking_enable;
    m_seiPayloadType = other.m_seiPayloadType;
    m_seiProj_enable = other.m_seiProj_enable;
    m_seiSphereRot_enable = other.m_seiSphereRot_enable;
    m_seiRWPK_enable = other.m_seiRWPK_enable;
    m_seiViewport_enable = other.m_seiViewport_enable;
    m_projType = other.m_projType;
    m_specialDataLen[0] = other.m_specialDataLen[0];
    m_specialDataLen[1] = other.m_specialDataLen[1];
    m_hrTilesInRow = other.m_hrTilesInRow;
    m_hrTilesInCol = other.m_hrTilesInCol;
    m_lrTilesInRow = other.m_lrTilesInRow;
    m_lrTilesInCol = other.m_lrTilesInCol;
    m_bSPSReady = other.m_bSPSReady;
    m_bPPSReady = other.m_bPPSReady;
    m_tileWidthCountSel[0] = other.m_tileWidthCountSel[0];
    m_tileWidthCountSel[1] = other.m_tileWidthCountSel[1];
    m_tileHeightCountSel[0] = other.m_tileHeightCountSel[0];
    m_tileHeightCountSel[1] = other.m_tileHeightCountSel[1];
    m_tileWidthCountOri[0] = other.m_tileWidthCountOri[0];
    m_tileWidthCountOri[1] = other.m_tileWidthCountOri[1];
    m_tileHeightCountOri[0] = other.m_tileHeightCountOri[0];
    m_tileHeightCountOri[1] = other.m_tileHeightCountOri[1];
    m_specialInfo[0] = new unsigned char[200];
    memcpy(m_specialInfo[0], other.m_specialInfo[0], 200 * sizeof(unsigned char));
    m_specialInfo[1] = new unsigned char[200];
    memcpy(m_specialInfo[1], other.m_specialInfo[1], 200 * sizeof(unsigned char));
    m_sliceHeaderLen = other.m_sliceHeaderLen;
    m_dstWidthNet = other.m_dstWidthNet;
    m_dstHeightNet = other.m_dstHeightNet;
    m_maxSelTiles = other.m_maxSelTiles;
    m_pRWPK = NULL;
    m_pSeiViewport = other.m_pSeiViewport;
    m_pFramePacking = other.m_pFramePacking;
    m_pSphereRot = other.m_pSphereRot;
    m_pSeiViewport = other.m_pSeiViewport;
    m_viewportDestWidth = other.m_viewportDestWidth;
    m_viewportDestHeight = other.m_viewportDestHeight;
    m_dataSize = 0;
    m_data = NULL;
    m_startCodesSize = other.m_startCodesSize;
    m_nalType = other.m_nalType;
    memcpy(&m_sliceType, &(other.m_sliceType), sizeof(SliceType));
    m_usedType = other.m_usedType;
    m_xTopLeftNet = other.m_xTopLeftNet;
    m_yTopLeftNet = other.m_yTopLeftNet;
    m_dstRwpk = RegionWisePacking();
    m_dstRwpk = other.m_dstRwpk;
}

TstitchStream::~TstitchStream()
{
    if (m_pOutTile) {
        delete []m_pOutTile;
        m_pOutTile = nullptr;
    }
    if (m_pUpLeft) {
        delete []m_pUpLeft;
        m_pUpLeft = nullptr;
    }
    if (m_pDownRight) {
        delete []m_pDownRight;
        m_pDownRight = nullptr;
    }
    if (m_pNalInfo[0]) {
        delete []m_pNalInfo[0];
        m_pNalInfo[0] = nullptr;
    }
    if (m_pNalInfo[1]) {
        delete []m_pNalInfo[1];
        m_pNalInfo[1] = nullptr;
    }
    if (m_hevcState) {
        delete m_hevcState;
        m_hevcState = nullptr;
    }
    if (m_specialInfo[0]) {
        delete []m_specialInfo[0];
        m_specialInfo[0] = nullptr;
    }
    if (m_specialInfo[1]) {
        delete []m_specialInfo[1];
        m_specialInfo[1] = nullptr;
    }
}

int32_t TstitchStream::initViewport(Param_ViewPortInfo* pViewPortInfo, int32_t tilecolCount, int32_t tilerowCount)
{
    if (pViewPortInfo == NULL)
        return -1;
    m_pViewportParam.m_pDownRight = m_pDownRight;
    m_pViewportParam.m_pUpLeft = m_pUpLeft;
    m_pViewportParam.m_iInputHeight = pViewPortInfo->faceHeight;
    m_pViewportParam.m_iInputWidth = pViewPortInfo->faceWidth;
    m_pViewportParam.m_input_geoType = pViewPortInfo->geoTypeInput;
    m_pViewportParam.m_output_geoType = pViewPortInfo->geoTypeOutput;
    m_pViewportParam.m_iViewportHeight = pViewPortInfo->viewportHeight;
    m_pViewportParam.m_iViewportWidth = pViewPortInfo->viewportWidth;
    m_pViewportParam.m_tileNumCol = tilecolCount;
    m_pViewportParam.m_tileNumRow = tilerowCount;
    m_pViewportParam.m_viewPort_fPitch = pViewPortInfo->viewPortPitch;
    m_pViewportParam.m_viewPort_fYaw = pViewPortInfo->viewPortYaw;
    m_pViewportParam.m_viewPort_hFOV = pViewPortInfo->viewPortFOVH;
    m_pViewportParam.m_viewPort_vFOV = pViewPortInfo->viewPortFOVV;
    m_pViewportParam.m_usageType = pViewPortInfo->usageType;
    if (m_pViewportParam.m_input_geoType == E_SVIDEO_EQUIRECT)
    {
        m_pViewportParam.m_paramVideoFP.cols = 1;
        m_pViewportParam.m_paramVideoFP.rows = 1;
        m_pViewportParam.m_paramVideoFP.faces[0][0].faceHeight = pViewPortInfo->faceHeight;
        m_pViewportParam.m_paramVideoFP.faces[0][0].faceWidth = pViewPortInfo->faceWidth;
        m_pViewportParam.m_paramVideoFP.faces[0][0].idFace = 1;
        m_pViewportParam.m_paramVideoFP.faces[0][0].rotFace = NO_TRANSFORM;
    }
    else
    {
        m_pViewportParam.m_paramVideoFP.cols = pViewPortInfo->paramVideoFP.cols;
        m_pViewportParam.m_paramVideoFP.rows = pViewPortInfo->paramVideoFP.rows;
    }

    for (int i = 0; i < pViewPortInfo->paramVideoFP.rows; i++)
    {
        for (int j = 0; j < pViewPortInfo->paramVideoFP.cols; j++)
        {
            m_pViewportParam.m_paramVideoFP.faces[i][j].faceHeight = pViewPortInfo->paramVideoFP.faces[i][j].faceHeight;
            m_pViewportParam.m_paramVideoFP.faces[i][j].faceWidth = pViewPortInfo->paramVideoFP.faces[i][j].faceWidth;
            m_pViewportParam.m_paramVideoFP.faces[i][j].idFace = pViewPortInfo->paramVideoFP.faces[i][j].idFace;
            m_pViewportParam.m_paramVideoFP.faces[i][j].rotFace = pViewPortInfo->paramVideoFP.faces[i][j].rotFace;
        }
    }
    m_pViewport = genViewport_Init(&m_pViewportParam);
    return 0;
}

int32_t TstitchStream::initMerge(param_360SCVP* pParamStitchStream, int32_t sliceSize)
{
    m_mergeStreamParam.pOutputBitstream = pParamStitchStream->pOutputBitstream;
    m_mergeStreamParam.inputBistreamsLen = pParamStitchStream->inputBitstreamLen;

    m_mergeStreamParam.highRes.width = pParamStitchStream->paramViewPort.faceWidth;
    m_mergeStreamParam.highRes.height = pParamStitchStream->paramViewPort.faceHeight;
    m_mergeStreamParam.highRes.num_tile_columns = m_tileWidthCountSel[0];

    m_mergeStreamParam.highRes.num_tile_rows = m_tileHeightCountSel[0];

    m_mergeStreamParam.highRes.totalTilesCount = m_tileWidthCountOri[0] * m_tileHeightCountOri[0];
    m_mergeStreamParam.highRes.selectedTilesCount = m_tileWidthCountSel[0] * m_tileHeightCountSel[0];
    m_mergeStreamParam.highRes.tile_width = m_mergeStreamParam.highRes.width / m_tileWidthCountOri[0];
    m_mergeStreamParam.highRes.tile_height = m_mergeStreamParam.highRes.height / m_tileHeightCountOri[0];

    if (pParamStitchStream->inputLowBistreamLen)
    {
        m_tileWidthCountSel[1] = m_tileWidthCountOri[1];
        m_tileHeightCountSel[1] = m_tileHeightCountOri[1];
        m_mergeStreamParam.lowRes.width = pParamStitchStream->frameWidthLow;
        m_mergeStreamParam.lowRes.height = pParamStitchStream->frameHeightLow;
        m_mergeStreamParam.lowRes.num_tile_columns = m_tileWidthCountSel[1];
        m_mergeStreamParam.lowRes.num_tile_rows = m_tileHeightCountSel[1];
        m_mergeStreamParam.lowRes.totalTilesCount = m_tileWidthCountOri[1] * m_tileHeightCountOri[1];
        m_mergeStreamParam.lowRes.selectedTilesCount = m_tileWidthCountSel[1] * m_tileHeightCountSel[1];
        m_mergeStreamParam.lowRes.tile_width = m_mergeStreamParam.lowRes.width / m_tileWidthCountOri[1];
        m_mergeStreamParam.lowRes.tile_height = m_mergeStreamParam.lowRes.height / m_tileHeightCountOri[1];
    }
    else
    {
        m_mergeStreamParam.lowRes.width = 3840;
        m_mergeStreamParam.lowRes.height = 1920;
        m_mergeStreamParam.lowRes.totalTilesCount = 0;
        m_mergeStreamParam.lowRes.selectedTilesCount = 0;
        m_mergeStreamParam.lowRes.tile_width = 640;
        m_mergeStreamParam.lowRes.tile_height = 640;

    }

    m_mergeStreamParam.highRes.bOrdered = 0;
    m_mergeStreamParam.lowRes.bOrdered = 0;
    m_mergeStreamParam.bWroteHeader = 1;

    int HR_ntile = m_tileHeightCountSel[0] * m_tileWidthCountSel[0];// m_mergeStreamParam.highRes.selectedTilesCount;
    int LR_ntile = m_mergeStreamParam.lowRes.selectedTilesCount;

    m_mergeStreamParam.highRes.pHeader = (param_oneStream_info *)malloc(sizeof(param_oneStream_info));
    m_mergeStreamParam.lowRes.pHeader = (param_oneStream_info *)malloc(sizeof(param_oneStream_info));
    m_mergeStreamParam.highRes.pTiledBitstreams = (param_oneStream_info **)malloc(HR_ntile * sizeof(param_oneStream_info *));
    if (m_mergeStreamParam.highRes.pHeader)
    {
        m_mergeStreamParam.highRes.pHeader->pTiledBitstreamBuffer = (uint8_t *)malloc(sliceSize);
    }
    if (m_mergeStreamParam.lowRes.pHeader)
    {
        m_mergeStreamParam.lowRes.pHeader->pTiledBitstreamBuffer = (uint8_t *)malloc(100);
    }

    if (!m_mergeStreamParam.highRes.pHeader || !m_mergeStreamParam.lowRes.pHeader
        || !m_mergeStreamParam.highRes.pHeader->pTiledBitstreamBuffer
        || !m_mergeStreamParam.lowRes.pHeader->pTiledBitstreamBuffer
        || !m_mergeStreamParam.highRes.pTiledBitstreams)
        return -1;
    for (int32_t i = 0; i < HR_ntile; i++)
    {
        m_mergeStreamParam.highRes.pTiledBitstreams[i] = (param_oneStream_info *)malloc(sizeof(param_oneStream_info));
        if (!m_mergeStreamParam.highRes.pTiledBitstreams[i])
            return -1;
    }
    m_mergeStreamParam.lowRes.pTiledBitstreams = (param_oneStream_info **)malloc(LR_ntile * sizeof(param_oneStream_info *));
    if (!m_mergeStreamParam.lowRes.pTiledBitstreams)
        return -1;
    for (int32_t i = 0; i < LR_ntile; i++)
    {
        m_mergeStreamParam.lowRes.pTiledBitstreams[i] = (param_oneStream_info *)malloc(sizeof(param_oneStream_info));
        if (!m_mergeStreamParam.lowRes.pTiledBitstreams[i])
            return -1;
    }

    m_pMergeStream = tile_merge_Init(&m_mergeStreamParam);
    return 0;
}

int32_t TstitchStream::init(param_360SCVP* pParamStitchStream)
{
    if (pParamStitchStream == NULL)
        return -1;
    int32_t ret = 0;
    m_specialDataLen[0] = 0;
    m_specialDataLen[1] = 0;
    m_usedType = pParamStitchStream->usedType;
    m_dstRwpk.rectRegionPacking = NULL;
    pParamStitchStream->paramViewPort.usageType = (UsageType)(pParamStitchStream->usedType);
    if (m_usedType == E_PARSER_FOR_CLIENT)
    {
        return ret;
    }
	if (pParamStitchStream->paramViewPort.geoTypeInput == E_SVIDEO_EQUIRECT)
	{
		pParamStitchStream->paramViewPort.paramVideoFP.cols = 1;
		pParamStitchStream->paramViewPort.paramVideoFP.rows = 1;
		pParamStitchStream->paramViewPort.paramVideoFP.faces[0][0].faceHeight = pParamStitchStream->paramViewPort.faceHeight;
		pParamStitchStream->paramViewPort.paramVideoFP.faces[0][0].faceWidth = pParamStitchStream->paramViewPort.faceWidth;
		pParamStitchStream->paramViewPort.paramVideoFP.faces[0][0].idFace = 1;
		pParamStitchStream->paramViewPort.paramVideoFP.faces[0][0].rotFace = NO_TRANSFORM;
	}

    if (m_usedType == E_VIEWPORT_ONLY)
    {
        // Init the viewport library
        ret = initViewport(&pParamStitchStream->paramViewPort, pParamStitchStream->paramViewPort.tileNumCol * pParamStitchStream->paramViewPort.paramVideoFP.cols,
                           pParamStitchStream->paramViewPort.tileNumRow * pParamStitchStream->paramViewPort.paramVideoFP.rows);
        return ret;
    }
    if (m_usedType == E_STREAM_STITCH_ONLY)
    {
        m_streamStitch.AUD_enable = pParamStitchStream->paramStitchInfo.AUD_enable;
        m_streamStitch.frameHeight = pParamStitchStream->paramPicInfo.picHeight;
        m_streamStitch.frameWidth = pParamStitchStream->paramPicInfo.picWidth;
        m_streamStitch.tilesHeightCount = pParamStitchStream->paramPicInfo.tileHeightNum;
        m_streamStitch.tilesUniformSpacing = pParamStitchStream->paramPicInfo.tileIsUniform;
        m_streamStitch.tilesWidthCount = pParamStitchStream->paramPicInfo.tileWidthNum;
        m_streamStitch.VUI_enable = pParamStitchStream->paramStitchInfo.VUI_enable;
        m_streamStitch.pNalInfo = m_pNalInfo[0];
        m_pSteamStitch = genTiledStream_Init(&m_streamStitch);
        if (!m_pSteamStitch)
            return -1;
        return ret;
    }
    if (parseNals(pParamStitchStream, pParamStitchStream->usedType, NULL, 0) < 0)
        return -1;

    if (pParamStitchStream->usedType == E_MERGE_AND_VIEWPORT)
    {
        parseNals(pParamStitchStream, E_MERGE_AND_VIEWPORT, NULL, 1);
        int32_t tilecolCount = m_tileWidthCountOri[0];
        int32_t tilerowCount = m_tileHeightCountOri[0];

        // Init the viewport library
        ret = initViewport(&pParamStitchStream->paramViewPort, tilecolCount* pParamStitchStream->paramViewPort.paramVideoFP.cols, tilerowCount* pParamStitchStream->paramViewPort.paramVideoFP.rows);

        int32_t sliceHeight = pParamStitchStream->paramViewPort.faceHeight / tilerowCount;
        int32_t sliceWidth = pParamStitchStream->paramViewPort.faceWidth / tilecolCount;
        int32_t sliceSize = sliceHeight * sliceWidth * 3 / 2;

        //according to the FOV information, get the tiles in the viewport area
        m_maxSelTiles = getViewPortTiles();
        genViewport_setMaxSelTiles(m_pViewport, m_maxSelTiles);

        // Init the merge library
        ret = initMerge(pParamStitchStream, sliceSize);
    }
    return ret;
}


int32_t TstitchStream::uninit()
{
    int32_t HR_ntile = m_tileHeightCountSel[0] * m_tileWidthCountSel[0];// m_mergeStreamParam.highRes.selectedTilesCount;
    int32_t LR_ntile = m_mergeStreamParam.lowRes.selectedTilesCount;
    int32_t ret = 0;

    if (m_pMergeStream)
    {
        if (m_mergeStreamParam.highRes.pHeader)
        {
            free(m_mergeStreamParam.highRes.pHeader);
            m_mergeStreamParam.highRes.pHeader = NULL;
        }
        if (m_mergeStreamParam.lowRes.pHeader)
        {
            free(m_mergeStreamParam.lowRes.pHeader);
            m_mergeStreamParam.lowRes.pHeader = NULL;
        }

        for (int32_t i = 0; i < HR_ntile; i++)
        {
            if (m_mergeStreamParam.highRes.pTiledBitstreams[i])
            {
                free(m_mergeStreamParam.highRes.pTiledBitstreams[i]);
                m_mergeStreamParam.highRes.pTiledBitstreams[i] = NULL;
            }
        }
        for (int32_t i = 0; i < LR_ntile; i++)
        {
            if (m_mergeStreamParam.lowRes.pTiledBitstreams[i])
            {
                free(m_mergeStreamParam.lowRes.pTiledBitstreams[i]);
                m_mergeStreamParam.lowRes.pTiledBitstreams[i] = NULL;
            }
        }
        ret = tile_merge_Close(m_pMergeStream);
    }
    if(m_pViewport)
        ret |= genViewport_unInit(m_pViewport);
    if (m_pSteamStitch)
        ret |= genTiledStream_unInit(m_pSteamStitch);
    if (m_pOutTile)
        delete[]m_pOutTile;
    m_pOutTile = NULL;
    if (m_pUpLeft)
        delete[]m_pUpLeft;
    m_pUpLeft = NULL;
    if (m_pDownRight)
        delete[]m_pDownRight;
    m_pDownRight = NULL;
    if(m_pNalInfo[0])
        delete[]m_pNalInfo[0];
    m_pNalInfo[0] = NULL;
    if(m_pNalInfo[1])
        delete[]m_pNalInfo[1];
    m_pNalInfo[1] = NULL;

    if (m_hevcState)
        delete m_hevcState;
    m_hevcState = NULL;
    if (m_specialInfo[0])
         delete[]m_specialInfo[0];
     m_specialInfo[0] = NULL;
    if (m_specialInfo[1])
         delete[]m_specialInfo[1];
    m_specialInfo[1] = NULL;

    if (m_dstRwpk.rectRegionPacking)
        delete[]m_dstRwpk.rectRegionPacking;
    m_dstRwpk.rectRegionPacking = NULL;

    return ret;
}

int32_t TstitchStream::parseNals(param_360SCVP* pParamStitchStream, int32_t parseType, Nalu* pNALU, int32_t streamIdx)
{
    if (!pParamStitchStream && !pNALU)
        return -1;
    param_gen_tiledStream GenStreamParam;
    param_oneStream_info  TileBitstreamList;
    param_oneStream_info  TiledBitstream;
    void* pGenStream;

    memset(&GenStreamParam, 0, sizeof(param_gen_tiledStream));
    GenStreamParam.pts = 0;
    GenStreamParam.tilesHeightCount = 1;
    GenStreamParam.tilesWidthCount = 1;
    GenStreamParam.parseType = parseType;
    GenStreamParam.pTiledBitstream = (param_oneStream_info**)&TileBitstreamList;

    if (pParamStitchStream)
    {
        if (streamIdx == 0)
        {
            TiledBitstream.pTiledBitstreamBuffer = pParamStitchStream->pInputBitstream;
            TiledBitstream.inputBufferLen = pParamStitchStream->inputBitstreamLen;
        }
        else
        {
            TiledBitstream.pTiledBitstreamBuffer = pParamStitchStream->pInputLowBitstream;
            TiledBitstream.inputBufferLen = pParamStitchStream->inputLowBistreamLen;
        }
        if (m_specialDataLen[streamIdx] > 0)
        {
            memmove(TiledBitstream.pTiledBitstreamBuffer + m_specialDataLen[streamIdx], TiledBitstream.pTiledBitstreamBuffer, pParamStitchStream->inputBitstreamLen);
            memcpy(TiledBitstream.pTiledBitstreamBuffer, m_specialInfo[streamIdx], m_specialDataLen[streamIdx]);
            TiledBitstream.inputBufferLen += m_specialDataLen[streamIdx];
        }
    }
    else if (pNALU)
    {
        TiledBitstream.pTiledBitstreamBuffer = pNALU->data;
        TiledBitstream.inputBufferLen = pNALU->dataSize;
    }

    *GenStreamParam.pTiledBitstream = &TiledBitstream;
    GenStreamParam.pNalInfo = m_pNalInfo[streamIdx];

    int32_t ret = -1;
    pGenStream = genTiledStream_Init(&GenStreamParam);
    if (pGenStream)
    {
        //save/restore the hevcState
        hevc_gen_tiledstream* pGenTilesStream = (hevc_gen_tiledstream*)pGenStream;
        if (!pGenTilesStream->pTiledBitstreams)
        {
            ret = genTiledStream_unInit(pGenStream);
            return -1;
        }
        oneStream_info * pSlice = pGenTilesStream->pTiledBitstreams[0];
        if (((pGenTilesStream->parseType == E_PARSER_ONENAL)) && m_bSPSReady && m_bPPSReady)
        {
            memcpy(pSlice->hevcSlice->sps, m_hevcState->sps,  6 * sizeof(HEVC_SPS));
            pSlice->hevcSlice->last_parsed_sps_id = m_hevcState->last_parsed_sps_id;
            memcpy(pSlice->hevcSlice->pps, m_hevcState->pps, 16 * sizeof(HEVC_PPS));
            pSlice->hevcSlice->last_parsed_pps_id = m_hevcState->last_parsed_pps_id;
        }

        genTiledStream_parseNals(&GenStreamParam, pGenStream);

        if(pGenTilesStream->parseType != E_PARSER_ONENAL)
            memcpy(m_hevcState, pSlice->hevcSlice, sizeof(HEVCState));
        else
        {
            if (GenStreamParam.nalType == GTS_HEVC_NALU_SEQ_PARAM)
            {
                memcpy(m_hevcState->sps, pSlice->hevcSlice->sps, 16 * sizeof(HEVC_SPS));
                m_hevcState->last_parsed_sps_id = pSlice->hevcSlice->last_parsed_sps_id;
                m_bSPSReady = 1;
            }
            if (GenStreamParam.nalType == GTS_HEVC_NALU_PIC_PARAM)
            {
                memcpy(m_hevcState->pps, pSlice->hevcSlice->pps, 16 * sizeof(HEVC_PPS));
                m_hevcState->last_parsed_pps_id = pSlice->hevcSlice->last_parsed_pps_id;
                m_bPPSReady = 1;
            }
        }

        m_sliceType = (SliceType)GenStreamParam.sliceType;
        m_tileHeightCountOri[streamIdx] = GenStreamParam.tilesHeightCount;
        m_tileWidthCountOri[streamIdx] = GenStreamParam.tilesWidthCount;
        if (GenStreamParam.specialLen)
        {
            m_specialDataLen[streamIdx] = GenStreamParam.specialLen;
            memcpy(m_specialInfo[streamIdx], TiledBitstream.pTiledBitstreamBuffer, m_specialDataLen[streamIdx]);
        }
        m_specialDataLen[streamIdx] = GenStreamParam.specialLen;
        m_nalType = GenStreamParam.nalType;
        m_sliceHeaderLen = GenStreamParam.sliceHeaderLen;
        m_seiPayloadType = GenStreamParam.seiPayloadType;
        m_startCodesSize = GenStreamParam.startCodesSize;
        m_dataSize = GenStreamParam.outputiledbistreamlen;
        m_data = GenStreamParam.pOutputTiledBitstream;
        ret = genTiledStream_unInit(pGenStream);
    }
    return ret;
 }

int32_t TstitchStream::feedParamToGenStream(param_360SCVP* pParamStitchStream)
{
    if (pParamStitchStream == NULL)
        return -1;
    param_oneStream_info **pTmpHigh = m_mergeStreamParam.highRes.pTiledBitstreams;
    param_oneStream_info **pTmpLow = m_mergeStreamParam.lowRes.pTiledBitstreams;
    //demux the whole frame to get each selected tile data and its length
    int32_t idx = 0;
    TileDef *pTmpTile = m_pOutTile;
    param_oneStream_info *pTmpHighHdr = m_mergeStreamParam.highRes.pHeader;
    pTmpHighHdr->pTiledBitstreamBuffer = pParamStitchStream->pInputBitstream;
    pTmpHighHdr->inputBufferLen = m_specialDataLen[0];

    param_oneStream_info *pTmpLowHdr = m_mergeStreamParam.lowRes.pHeader;
    pTmpLowHdr->pTiledBitstreamBuffer = pParamStitchStream->pInputLowBitstream;
    pTmpLowHdr->inputBufferLen = m_specialDataLen[1];
    printf("the tiled idx=");

    tile_merge_reset(m_pMergeStream);

    if (m_specialDataLen[0] == 0)
    {
        m_mergeStreamParam.bWroteHeader = 0;
    }

    for (int32_t i = 0; i < m_tileHeightCountSel[0]; i++)
    {
        for (int32_t j = 0; j < m_tileWidthCountSel[0]; j++)
        {
            pTmpHigh[idx]->tilesHeightCount = 1;
            pTmpHigh[idx]->tilesWidthCount = 1;

            pTmpHigh[idx]->inputBufferLen = (pTmpTile->idx!=0) ? m_pNalInfo[0][pTmpTile->idx].nalLen : m_pNalInfo[0][pTmpTile->idx].nalLen- m_specialDataLen[0];
            pTmpHigh[idx]->pTiledBitstreamBuffer = (pTmpTile->idx != 0) ? m_pNalInfo[0][pTmpTile->idx].pNalStream : m_pNalInfo[0][pTmpTile->idx].pNalStream + m_specialDataLen[0];
            printf(" %d ", pTmpTile->idx);
            pTmpTile++;
            idx++;
        }
    }

    idx = 0;
    for (int32_t i = 0; i < m_tileHeightCountSel[1]; i++)
    {
        for (int32_t j = 0; j < m_tileWidthCountSel[1]; j++)
        {
            pTmpLow[idx]->tilesHeightCount = 1;
            pTmpLow[idx]->tilesWidthCount = 1;

            pTmpLow[idx]->inputBufferLen = (idx != 0) ? m_pNalInfo[1][idx].nalLen : m_pNalInfo[1][idx].nalLen - m_specialDataLen[1];
            pTmpLow[idx]->pTiledBitstreamBuffer = (idx != 0) ? m_pNalInfo[1][idx].pNalStream : m_pNalInfo[1][idx].pNalStream + m_specialDataLen[1];
            printf(" %d ", idx);
            idx++;
        }
    }
    printf("\n");
    return 0;
}

int32_t TstitchStream::getViewPortTiles()
{
    if (!m_pViewport)
        return -1;
    int32_t ret = 0;

    for (int i = 0; i < 6; i++)
    {
        m_pUpLeft[i].faceId = -1;
        m_pDownRight[i].faceId = -1;
    }

    if(m_pViewportParam.m_input_geoType == E_SVIDEO_EQUIRECT)
        ret = genViewport_postprocess(&m_pViewportParam, m_pViewport);
    else
        ret = genViewport_process(&m_pViewportParam, m_pViewport);
    if (ret)
    {
        printf("gen viewport process error!\n");
        return -1;
    }
    if(m_usedType == E_MERGE_AND_VIEWPORT)
        ret = genViewport_getFixedNumTiles(m_pViewport, m_pOutTile);
    if (m_pViewportParam.m_input_geoType == SVIDEO_EQUIRECT)
    {
        int32_t widthViewport = 0;
        int32_t heightViewport = 0;
        for (int32_t i = 0; i < m_pViewportParam.m_numFaces; i++)
        {
            widthViewport += (m_pViewportParam.m_pDownRight[i].x - m_pViewportParam.m_pUpLeft[i].x);
            heightViewport = (m_pViewportParam.m_pDownRight[i].y - m_pViewportParam.m_pUpLeft[i].y);
        }

        m_tileWidthCountSel[0] = (m_tileWidthCountSel[0] != 0) ? m_tileWidthCountSel[0] : m_pViewportParam.m_viewportDestWidth / (m_pViewportParam.m_iInputWidth / m_pViewportParam.m_tileNumCol);
        m_tileHeightCountSel[0] = (m_tileHeightCountSel[0] != 0) ? m_tileHeightCountSel[0] : m_pViewportParam.m_viewportDestHeight / (m_pViewportParam.m_iInputHeight / m_pViewportParam.m_tileNumRow);

        m_dstWidthNet = widthViewport;
        m_dstHeightNet = heightViewport;
        m_xTopLeftNet = m_pViewportParam.m_pUpLeft->x;
        m_yTopLeftNet = m_pViewportParam.m_pUpLeft->y;

    }
    return ret;
}

TileDef* TstitchStream::getSelectedTile()
{
    return m_pOutTile;
}
int32_t TstitchStream::setViewPort(float yaw, float pitch)
{
    return genViewport_setViewPort(m_pViewport, yaw, pitch);
}


int32_t TstitchStream::doMerge(param_360SCVP* pParamStitchStream)
{
    int32_t ret = 0;
    if (pParamStitchStream == NULL)
        return -1;
    ret = tile_merge_Process(&m_mergeStreamParam, m_pMergeStream);
    if (ret < 0)
        return -1;
    hevc_mergeStream *mergeStream = (hevc_mergeStream *)m_pMergeStream;
    m_hrTilesInCol = mergeStream->pic_height / mergeStream->highRes.tile_height;
    m_hrTilesInRow = mergeStream->highRes.selectedTilesCount / m_hrTilesInCol;
    m_lrTilesInCol = mergeStream->pic_height / mergeStream->lowRes.tile_height;
    m_lrTilesInRow = mergeStream->lowRes.selectedTilesCount / m_lrTilesInCol;

    pParamStitchStream->outputSEILen = 0;
    m_dstRwpk.numRegions = m_tileWidthCountSel[0] * m_tileHeightCountSel[0] + m_tileWidthCountSel[1] * m_tileHeightCountSel[1];
    m_dstRwpk.numHiRegions = m_tileWidthCountSel[0] * m_tileHeightCountSel[0];
    m_dstRwpk.lowResPicWidth = mergeStream->lowRes.width;
    m_dstRwpk.lowResPicHeight = mergeStream->lowRes.height;
    m_dstRwpk.timeStamp = pParamStitchStream->timeStamp;

    if (!m_dstRwpk.rectRegionPacking)
    {
        m_dstRwpk.rectRegionPacking = new RectangularRegionWisePacking[m_dstRwpk.numRegions];
        if (!(m_dstRwpk.rectRegionPacking))
            return -1;
    }

    if(GenerateRwpkInfo(&m_dstRwpk) == 0)
        ret = EncRWPKSEI(&m_dstRwpk, pParamStitchStream->pOutputSEI, &pParamStitchStream->outputSEILen);

    pParamStitchStream->outputBitstreamLen = m_mergeStreamParam.outputiledbistreamlen;
    memcpy(pParamStitchStream->pOutputBitstream, m_mergeStreamParam.pOutputBitstream, m_mergeStreamParam.outputiledbistreamlen);

    return ret;
}

int TstitchStream::EncRWPKSEI(RegionWisePacking* pRWPK, uint8_t *pRWPKBits, uint32_t* pRWPKBitsSize)
{
    if (!pRWPK || !pRWPKBits || !pRWPKBitsSize)
        return -1;
    int32_t rwpkSize = 3000; //need to define a macro
    GTS_BitStream *bs = gts_bs_new((const int8_t *)pRWPKBits, rwpkSize, GTS_BITSTREAM_WRITE);
    int32_t sSize = 0;
    int32_t ret = -1;
    if (bs)
    {
        sSize = hevc_write_RwpkSEI(bs, pRWPK, 1);
        gts_bs_del(bs);
        ret = 0;
    }

    // insert emulation prevention byte 03
    // 0x 00 00 00 ------ > 0x00 00 03 00
    // 0x 00 00 01 ------ > 0x00 00 03 01
    // 0x 00 00 02 ------ > 0x00 00 03 02
    // 0x 00 00 03 ------ > 0x00 00 03 03
    int32_t emulateCode = 0;
/*
    int32_t pos = 4;
    int32_t Cnt = sSize - 4;
    while (Cnt)
    {
        if (!pRWPKBits[pos] && !pRWPKBits[pos + 1] && pRWPKBits[pos + 2] <= 3)
        {
            memmove(pRWPKBits + pos + 3, pRWPKBits + pos + 2, Cnt - 2);
            pRWPKBits[pos + 2] = 3;
            emulateCode++;
            pos++;
        }
        Cnt--;
        pos++;
    }
*/
    sSize = sSize + emulateCode;
    *pRWPKBitsSize = sSize;

    return ret;
}

int32_t TstitchStream::DecRWPKSEI(RegionWisePacking* pRWPK, uint8_t *pRWPKBits, uint32_t RWPKBitsSize)
{
    if (!pRWPK || !pRWPKBits)
        return -1;
    int ret = hevc_read_RwpkSEI((int8_t*)pRWPKBits, RWPKBitsSize, pRWPK);
    return ret;
}

int TstitchStream::GenerateRwpkInfo(RegionWisePacking *dstRwpk)
{
    if (!dstRwpk)
        return -1;
    TileDef* pSelectTiles = getSelectedTile();
    dstRwpk->projPicHeight = m_mergeStreamParam.highRes.height;
    dstRwpk->projPicWidth = m_mergeStreamParam.highRes.width;
    int highRes_tile_width = m_mergeStreamParam.highRes.width / m_tileWidthCountOri[0];
    int highRes_tile_height = m_mergeStreamParam.highRes.height / m_tileHeightCountOri[0];
    int lowRes_tile_width = m_mergeStreamParam.lowRes.width / m_tileWidthCountOri[1];
    int lowRes_tile_height = m_mergeStreamParam.lowRes.height / m_tileHeightCountOri[1];
    dstRwpk->constituentPicMatching = 0;

    uint8_t highTilesNum = m_tileWidthCountSel[0] * m_tileHeightCountSel[0];
    dstRwpk->packedPicWidth = highRes_tile_width * m_tileWidthCountSel[0] + lowRes_tile_width * ((dstRwpk->numRegions- highTilesNum) / m_lrTilesInCol);
    dstRwpk->packedPicHeight = highRes_tile_height * m_tileHeightCountSel[0];

    for (uint8_t regionIdx = 0; regionIdx < dstRwpk->numRegions; regionIdx++)
    {
        RectangularRegionWisePacking *rwpk = &(dstRwpk->rectRegionPacking[regionIdx]);
        rwpk->transformType = 0;
        rwpk->guardBandFlag = false;
        if (regionIdx < highTilesNum)
        {
            rwpk->projRegWidth = highRes_tile_width;
            rwpk->projRegHeight = highRes_tile_height;
            rwpk->projRegTop = pSelectTiles->y;
            rwpk->projRegLeft = pSelectTiles->x;

            rwpk->packedRegWidth = rwpk->projRegWidth;
            rwpk->packedRegHeight = rwpk->projRegHeight;
            rwpk->packedRegTop = (regionIdx % m_hrTilesInCol) * highRes_tile_height;
            rwpk->packedRegLeft = (regionIdx / m_hrTilesInCol) * highRes_tile_width;

            rwpk->leftGbWidth = 0;
            rwpk->rightGbWidth = 0;
            rwpk->topGbHeight = 0;
            rwpk->bottomGbHeight = 0;
            rwpk->gbNotUsedForPredFlag = true;
            rwpk->gbType0 = 0;
            rwpk->gbType1 = 0;
            rwpk->gbType2 = 0;
            rwpk->gbType3 = 0;
            pSelectTiles++;
        }
        else
        {
            int lowIdx = regionIdx - highTilesNum;
            rwpk->projRegWidth = lowRes_tile_width;
            rwpk->projRegHeight = lowRes_tile_height;
            rwpk->projRegTop = (lowIdx / m_tileWidthCountOri[1]) * lowRes_tile_height;
            rwpk->projRegLeft = (lowIdx % m_tileWidthCountOri[1]) * lowRes_tile_width;

            rwpk->packedRegWidth = rwpk->projRegWidth;
            rwpk->packedRegHeight = rwpk->projRegHeight;
            rwpk->packedRegTop = (lowIdx % m_lrTilesInCol) * lowRes_tile_height;
            rwpk->packedRegLeft = (lowIdx / m_lrTilesInCol) * lowRes_tile_width + highRes_tile_width * m_hrTilesInRow;

            rwpk->leftGbWidth = 0;
            rwpk->rightGbWidth = 0;
            rwpk->topGbHeight = 0;
            rwpk->bottomGbHeight = 0;
            rwpk->gbNotUsedForPredFlag = true;
            rwpk->gbType0 = 0;
            rwpk->gbType1 = 0;
            rwpk->gbType2 = 0;
            rwpk->gbType3 = 0;
        }
    }
    return 0;
}


int32_t TstitchStream::getFixedNumTiles(TileDef* pOutTile)
{
    int32_t ret = 0;
    if (pOutTile == NULL)
        return -1;
    ret = genViewport_getFixedNumTiles(m_pViewport, pOutTile);
    m_viewportDestWidth = m_pViewportParam.m_viewportDestWidth;
    m_viewportDestHeight = m_pViewportParam.m_viewportDestHeight;

    return ret;
}

int32_t TstitchStream::getTilesInViewport(TileDef* pOutTile)
{
    int32_t ret = 0;
    if (pOutTile == NULL)
        return -1;
    ret = genViewport_getTilesInViewport(m_pViewport, pOutTile);
    m_viewportDestWidth = m_pViewportParam.m_viewportDestWidth;
    m_viewportDestHeight = m_pViewportParam.m_viewportDestHeight;

    return ret;
}

int32_t  TstitchStream::doStreamStitch(param_360SCVP* pParamStitchStream)
{
    int32_t ret = 0;
    int32_t outputlen = 0;
    if (pParamStitchStream == NULL)
        return -1;

    m_streamStitch.inputBistreamsLen = pParamStitchStream->inputBitstreamLen;
    m_streamStitch.outputiledbistreamlen = pParamStitchStream->outputBitstreamLen;
    m_streamStitch.pOutputTiledBitstream = pParamStitchStream->pOutputBitstream;

    hevc_gen_tiledstream* pGenTilesStream = (hevc_gen_tiledstream*)m_pSteamStitch;
    int32_t tiled_width = pGenTilesStream->tilesWidthCount;
    int32_t tiled_height = pGenTilesStream->tilesHeightCount;
    int32_t input_count = 0;
    for (int32_t i = 0; i < tiled_height; i++)
    {
        for (int32_t j = 0; j < tiled_width; j++)
        {
            set_genHandle_params(pGenTilesStream->pTiledBitstreams[i*tiled_width + j],
            pParamStitchStream->paramStitchInfo.pTiledBitstream[input_count]);
            input_count++;
            pGenTilesStream->pTiledBitstreams[i*tiled_width + j]->curBufferLen = 0;
            pGenTilesStream->pTiledBitstreams[i*tiled_width + j]->outputBufferLen = 0;
            pGenTilesStream->pTiledBitstreams[i*tiled_width + j]->currentTileIdx = 0;
        }
    }

    pGenTilesStream->pOutputTiledBitstream = pParamStitchStream->pOutputBitstream;

    ret = merge_partstream_into1bitstream(pParamStitchStream->inputBitstreamLen);

  //  int32_t tiled_idx = 0;
    input_count = 0;
    for (int32_t i = 0; i < tiled_height; i++)
    {
        for (int32_t j = 0; j < tiled_width; j++)
        {
            param_oneStream_info *pOneStream = pParamStitchStream->paramStitchInfo.pTiledBitstream[input_count];
            int32_t outputBufferLen = pGenTilesStream->pTiledBitstreams[i*tiled_width + j]->outputBufferLen;
            int32_t curBufferLen = pGenTilesStream->pTiledBitstreams[i*tiled_width + j]->curBufferLen;
            pOneStream->outputBufferLen = outputBufferLen;
            pOneStream->curBufferLen = curBufferLen;
            outputlen += outputBufferLen;
            input_count++;
        }
    }

    oneStream_info * pSlice = pGenTilesStream->pTiledBitstreams[0];
    bool idr_flag = (pSlice->hevcSlice->s_info.nal_unit_type == GTS_HEVC_NALU_SLICE_IDR_W_DLP ||
    pSlice->hevcSlice->s_info.nal_unit_type == GTS_HEVC_NALU_SLICE_IDR_N_LP);
    pParamStitchStream->paramStitchInfo.sliceType = (slice_type)(idr_flag ? SLICE_IDR : pSlice->hevcSlice->s_info.slice_type);
    pParamStitchStream->paramStitchInfo.pts = idr_flag ? 0 : pSlice->hevcSlice->s_info.poc_lsb;
    pParamStitchStream->outputBitstreamLen = outputlen;
    return ret;
}

int32_t TstitchStream::merge_one_tile(uint8_t **pBitstream, oneStream_info* pSlice, GTS_BitStream *bs, bool bFirstTile)
{
    hevc_gen_tiledstream* pGenTilesStream = (hevc_gen_tiledstream*)m_pSteamStitch;
    if (!pGenTilesStream || !pBitstream || !pSlice || !bs)
        return GTS_BAD_PARAM;

    uint32_t nalsize[20];
    uint32_t specialLen = 0;
    int32_t framesize = 0;

    uint8_t *pBufferSliceCur = pSlice->pTiledBitstreamBuffer + pSlice->curBufferLen;
    int32_t lenSlice = pSlice->inputBufferLen - pSlice->curBufferLen;

    uint8_t *pBitstreamCur = *pBitstream;
    if (!pBitstreamCur)
        return GTS_BAD_PARAM;

    HEVCState *hevc = pSlice->hevcSlice;
    if (!hevc)
        return GTS_BAD_PARAM;

    hevc_specialInfo specialInfo;
    memset(&specialInfo, 0, sizeof(hevc_specialInfo));
    specialInfo.ptr = pBufferSliceCur;
    specialInfo.ptr_size = lenSlice;

    //set tile info as original one to make sure
    //following slices are decoded correctly
    hevc->pps[hevc->last_parsed_pps_id].tiles_enabled_flag
        = hevc->pps[hevc->last_parsed_pps_id].org_tiles_enabled_flag;

    memset(nalsize, 0, sizeof(nalsize));
    uint64_t bs_position = bs->position;
    int32_t spsCnt;
    parse_hevc_specialinfo(&specialInfo, hevc, nalsize, &specialLen, &spsCnt, 0);

    specialLen += nalsize[SLICE_HEADER];
    framesize = specialLen + nalsize[SLICE_DATA];
    lenSlice -= framesize;

    if (pGenTilesStream->AUD_enable && bFirstTile)
    {
        hevc_write_bitstream_aud(bs, hevc);
    }

    if (nalsize[SEQ_PARAM_SET])
    {
        // modify sps
        gts_media_hevc_stitch_sps(hevc, pGenTilesStream->frameWidth, pGenTilesStream->frameHeight);

        //modify pps
        gts_media_hevc_stitch_pps(hevc, pGenTilesStream);

        //merge vps, sps, pps and SEI if at first tile
        if (pSlice->address == 0)
        {
            pGenTilesStream->headerNal = (uint8_t*)bs->original + bs->position;
            pGenTilesStream->headerNalSize = (uint8_t)(bs->position);
            hevc_write_parameter_sets(bs, hevc);
            //add the sei information, rwpk, projectoin, sphere rotation, and framepacking
            if (m_seiRWPK_enable)
            {
                hevc_write_RwpkSEI(bs, m_pRWPK, 1);
            }

            if (m_seiProj_enable)
            {
                hevc_write_ProjectionSEI(bs, m_projType, 1);
            }

            if (m_seiSphereRot_enable)
            {
                hevc_write_SphereRotSEI(bs, m_pSphereRot, 1);
            }

            if (m_seiFramePacking_enable)
            {
                hevc_write_FramePackingSEI(bs, m_pFramePacking, 1);
            }

            if (m_seiViewport_enable)
            {
                hevc_write_ViewportSEI(bs, m_pSeiViewport, 1);

            }
            pGenTilesStream->headerNalSize = (uint8_t)(bs->position - pGenTilesStream->headerNalSize);
        }
    }

    hevc->pps[hevc->last_parsed_pps_id].tiles_enabled_flag = (bool)(pGenTilesStream->outTilesWidthCount > 1
        || pGenTilesStream->outTilesHeightCount > 1);

    // modify the tiled slice header , and merge slice data
    gts_media_hevc_stitch_slice_segment(hevc, pSlice, pGenTilesStream->frameWidth, (uint32_t)pSlice->currentTileIdx);
    hevc_write_slice_header(bs, hevc);

    //move to current address
    pBitstreamCur += bs->position - bs_position;
    bs_position = bs->position;

    //copy slice data
    memcpy(pBitstreamCur, pBufferSliceCur + specialLen, nalsize[SLICE_DATA]);
    pBitstreamCur += nalsize[SLICE_DATA];
    bs->position += nalsize[SLICE_DATA];
    pBufferSliceCur += specialLen + nalsize[SLICE_DATA];

    pSlice->currentTileIdx++;
    *pBitstream = pBitstreamCur;

    return framesize;
}

int32_t TstitchStream::merge_partstream_into1bitstream(int32_t totalInputLen)
{
    hevc_gen_tiledstream* pGenTilesStream = (hevc_gen_tiledstream*)m_pSteamStitch;
    if (!pGenTilesStream)
        return GTS_BAD_PARAM;

    // Set bs size larger than input, in case of additional syntax
    // need to be wrote into output bitstream.
    int32_t outputBSLen = 2 * totalInputLen;

    uint8_t* pBitstreamCur = pGenTilesStream->pOutputTiledBitstream;
    if (!pBitstreamCur)
        return GTS_BAD_PARAM;

    // define nxm tiles here, uniform type is default setting
    int32_t tilesWidthCount = pGenTilesStream->tilesWidthCount;
    int32_t tilesHeightCount = pGenTilesStream->tilesHeightCount;

    GTS_BitStream *bs = gts_bs_new((const int8_t *)pBitstreamCur, outputBSLen, GTS_BITSTREAM_WRITE);
    if (!bs)
        return GTS_OUT_OF_MEM;

    parse_tiles_info(pGenTilesStream);

    for (int32_t i = 0; i < pGenTilesStream->outTilesHeightCount; i++)
    {
        for (int32_t j = 0; j < pGenTilesStream->outTilesWidthCount; j++)
        {
            int32_t kw = 0, kh = 0;
            int32_t widthIdx = j, heightIdx = i;
            for (; kw < tilesWidthCount; kw++)
            {
                if (widthIdx <= pGenTilesStream->columnCnt[kw] - 1)
                    break;
                widthIdx -= pGenTilesStream->columnCnt[kw];
            }
            for (; kh < tilesHeightCount; kh++)
            {
                if (heightIdx <= pGenTilesStream->rowCnt[kh] - 1)
                    break;
                heightIdx -= pGenTilesStream->rowCnt[kh];
            }
            oneStream_info * pSliceCur = pGenTilesStream->pTiledBitstreams[kh*tilesWidthCount + kw];

            uint64_t bspos = 0;
            if (bs) bspos = bs->position;
            bool bFirstTile = (bool)((i == 0 && j == 0) == 1 ? 1 : 0);
            int32_t curframesize = merge_one_tile(&pBitstreamCur, pSliceCur, bs, bFirstTile);
            pSliceCur->curBufferLen += curframesize;
            pSliceCur->outputBufferLen += (uint32_t)(bs->position - bspos);
        }
    }

    if (bs) gts_bs_del(bs);
    return 0;
}


int32_t TstitchStream::getPicInfo(Param_PicInfo* pPicInfo)
{
    int32_t ret = 0;
    if (pPicInfo == NULL)
        return -1;
    HEVC_PPS *pps = &m_hevcState->pps[m_hevcState->last_parsed_pps_id];
    HEVC_SPS *sps = &m_hevcState->sps[0];
    if (!pps || !sps)
        return -1;

    pPicInfo->maxCUWidth = sps->max_CU_width;
    pPicInfo->picHeight = sps->height;
    pPicInfo->picWidth = sps->width;
    pPicInfo->tileHeightNum = pps->num_tile_rows;
    pPicInfo->tileWidthNum = pps->num_tile_columns;
    pPicInfo->tileIsUniform = pps->uniform_spacing_flag;
    return ret;
}

int32_t TstitchStream::getBSHeader(Param_BSHeader * bsHeader)
{
    int32_t ret = 0;
    if (!bsHeader)
        return -1;

    hevc_gen_tiledstream* pGenTilesStream = (hevc_gen_tiledstream*)m_pSteamStitch;

    bsHeader->data = (uint8_t *)pGenTilesStream->headerNal;
    bsHeader->size = pGenTilesStream->headerNalSize;

    return ret;
}

int32_t  TstitchStream::getRWPKInfo(RegionWisePacking *pRWPK)
{
    int32_t ret = 0;
    if (!pRWPK)
        return -1;
    memcpy(pRWPK, &m_dstRwpk, sizeof(RegionWisePacking));
    return ret;
}

int32_t TstitchStream::setViewPortInfo(Param_ViewPortInfo* pViewPortInfo)
{
    int32_t ret = 0;
    if (pViewPortInfo == NULL)
        return -1;

    // Init the viewport library
    ret = initViewport(pViewPortInfo, pViewPortInfo->tileNumCol, pViewPortInfo->tileNumRow);
    // do the process to calculate the tiles
    ret = getViewPortTiles();
    // the ret is the tile number, if there is something wrong, the ret will be less than 0
    if (ret > 0)
        ret = 0;
    return ret;
}

int32_t  TstitchStream::setSEIProjInfo(int32_t projType)
{
    int32_t ret = 0;
    m_seiProj_enable = 1;
    m_projType = projType;
    return ret;
}

int32_t  TstitchStream::setSEIRWPKInfo(RegionWisePacking* pRWPK)
{
    int32_t ret = 0;
    if (pRWPK == NULL)
        return -1;
    m_seiRWPK_enable = 1;
    m_pRWPK = pRWPK;
    return ret;
}

int32_t  TstitchStream::setSphereRot(SphereRotation* pSphereRot)
{
    int32_t ret = 0;
    if (pSphereRot == NULL)
        return -1;
    m_seiSphereRot_enable = 1;
    m_pSphereRot = pSphereRot;
    return ret;
}

int32_t  TstitchStream::setFramePacking(FramePacking* pFramePacking)
{
    int32_t ret = 0;
    if (pFramePacking == NULL)
        return -1;
    m_seiFramePacking_enable = 1;
    m_pFramePacking = pFramePacking;
    return ret;
}

int32_t  TstitchStream::setViewportSEI(OMNIViewPort* pSeiViewport)
{
    int32_t ret = 0;
    if (pSeiViewport == NULL)
        return -1;
    m_seiViewport_enable = 1;
    m_pSeiViewport = pSeiViewport;
    return ret;
}

int32_t  TstitchStream::getContentCoverage(CCDef* pOutCC)
{
    int32_t ret = 0;
    if (pOutCC == NULL)
        return -1;
    ret = genViewport_getContentCoverage(m_pViewport, pOutCC);
    return ret;
}

int32_t  TstitchStream::GeneratePPS(param_360SCVP* pParamStitchStream, TileArrangement* pTileArrange)
{
    int32_t ret = -1;
    GTS_BitStream *bs = NULL;
    GTS_BitStream *bsWrite = NULL;
    HEVCState hevcTmp;

    if (!pParamStitchStream || !pTileArrange || !pTileArrange->tileColWidth || !pTileArrange->tileRowHeight)
        return -1;

    bs = gts_bs_new((const int8_t*)pParamStitchStream->pInputBitstream, pParamStitchStream->inputBitstreamLen, GTS_BITSTREAM_READ);
    // new bs
    bsWrite = gts_bs_new((const int8_t *)pParamStitchStream->pOutputBitstream, 2 * pParamStitchStream->inputBitstreamLen, GTS_BITSTREAM_WRITE);

    if (bs && bsWrite)
    {
        // parsing the origin pps
        hevc_specialInfo specialInfo;
        memset(&specialInfo, 0, sizeof(hevc_specialInfo));
        specialInfo.ptr = pParamStitchStream->pInputBitstream;
        specialInfo.ptr_size = pParamStitchStream->inputBitstreamLen;
        uint32_t nalsize[20];
        memset(nalsize, 0, sizeof(nalsize));
        int32_t spsCnt;
        ret = hevc_import_ffextradata(&specialInfo, m_hevcState, nalsize, &spsCnt, 0);
        if (ret < 0)
        {
            gts_bs_del(bs);
            bs = NULL;
            gts_bs_del(bsWrite);
            bsWrite = NULL;
            return ret;
        }
        memcpy(&hevcTmp, m_hevcState, sizeof(HEVCState));
        if (hevcTmp.last_parsed_pps_id > 63)
        {
            gts_bs_del(bs);
            bs = NULL;
            gts_bs_del(bsWrite);
            bsWrite = NULL;
            return -1;
        }
        HEVC_PPS *pps = &hevcTmp.pps[hevcTmp.last_parsed_pps_id];
        /*
        if (!pps)
        {
            gts_bs_del(bs);
            bs = NULL;
            gts_bs_del(bsWrite);
            bsWrite = NULL;
            return -1;
        }
        */
        // modify the pps
        pps->uniform_spacing_flag = (bool)false;
        pps->num_tile_columns = pTileArrange->tileColsNum;
        for (uint32_t i = 0; i < pps->num_tile_columns; i++)
        {
            pps->column_width[i] = pTileArrange->tileColWidth[i];
        }
        pps->num_tile_rows = pTileArrange->tileRowsNum;
        for (uint32_t i = 0; i < pps->num_tile_rows; i++)
        {
            pps->row_height[i] = pTileArrange->tileRowHeight[i];
        }
        // write the new pps
        hevc_write_pps(bsWrite, &hevcTmp);
        pParamStitchStream->outputBitstreamLen = gts_bs_get_position(bsWrite);
        if (bs)
        {
            gts_bs_del(bs);
            bs = NULL;
        }
        if (bsWrite)
        {
            gts_bs_del(bsWrite);
            bsWrite = NULL;
        }
        ret = 0;
    }

    if (bs)
    {
        gts_bs_del(bs);
        bs = NULL;
    }
    if (bsWrite)
    {
        gts_bs_del(bsWrite);
        bsWrite = NULL;
    }
    return ret;
}


int32_t  TstitchStream::GenerateSPS(param_360SCVP* pParamStitchStream)
{
    int32_t ret = -1;
    GTS_BitStream *bs = NULL;
    GTS_BitStream *bsWrite = NULL;
    HEVCState hevcTmp;
    if (pParamStitchStream == NULL)
        return -1;

    bs = gts_bs_new((const int8_t*)pParamStitchStream->pInputBitstream, pParamStitchStream->inputBitstreamLen, GTS_BITSTREAM_READ);
    // new bs
    bsWrite = gts_bs_new((const int8_t *)pParamStitchStream->pOutputBitstream, 2 * pParamStitchStream->inputBitstreamLen, GTS_BITSTREAM_WRITE);

    if (bs && bsWrite)
    {
        // parsing the origin sps
        hevc_specialInfo specialInfo;
        memset(&specialInfo, 0, sizeof(hevc_specialInfo));
        specialInfo.ptr = pParamStitchStream->pInputBitstream;
        specialInfo.ptr_size = pParamStitchStream->inputBitstreamLen;
        uint32_t nalsize[20];
        memset(nalsize, 0, sizeof(nalsize));
        int32_t spsCnt;
        ret = hevc_import_ffextradata(&specialInfo, m_hevcState, nalsize, &spsCnt, 0);
        if (ret < 0)
        {
            if(bs)
            {
                //gts_bs_del(bsWrite);
                gts_bs_del(bs);
                bs = NULL;
            }
            if (bsWrite)
            {
                gts_bs_del(bsWrite);
                bsWrite = NULL;
            }

            return ret;
        }
        // modify the sps
        memcpy(&hevcTmp, m_hevcState, sizeof(HEVCState));
        HEVC_SPS *sps = &hevcTmp.sps[0];
        /*
        if (!sps)
        {
            if (bs)
            {
                gts_bs_del(bs);
                bs = NULL;
            }

            if (bsWrite)
            {
                gts_bs_del(bsWrite);
                bsWrite = NULL;
            }

            return -1;
        }
        */
        sps->width = pParamStitchStream->destWidth;
        sps->height = pParamStitchStream->destHeight;

        // write the new sps
        hevc_write_sps(bsWrite, &hevcTmp);
        pParamStitchStream->outputBitstreamLen = gts_bs_get_position(bsWrite);
        if (bsWrite)
        {
            gts_bs_del(bsWrite);
            bsWrite = NULL;
        }
        if (bs)
        {
            gts_bs_del(bs);
            bs = NULL;
        }
        ret = 0;
    }
    if (bsWrite)
    {
        gts_bs_del(bsWrite);
        bsWrite = NULL;
    }
    if (bs)
    {
        gts_bs_del(bs);
        bs = NULL;
    }
    return ret;
}

int32_t  TstitchStream::GenerateSliceHdr(param_360SCVP* pParam360SCVP, int32_t newSliceAddr)
{
    int32_t ret = -1;
    GTS_BitStream *bsWrite = NULL;
    HEVCState hevcTmp;
    if (!pParam360SCVP)
        return -1;

    // new bs
    bsWrite = gts_bs_new((const int8_t *)pParam360SCVP->pOutputBitstream, 2 * pParam360SCVP->inputBitstreamLen, GTS_BITSTREAM_WRITE);

    if (bsWrite)
    {
        // parse the old slice header
        uint32_t nalsize[20];
        hevc_specialInfo specialInfo;
        int32_t spsCnt;
        memset(&specialInfo, 0, sizeof(hevc_specialInfo));
        specialInfo.ptr = pParam360SCVP->pInputBitstream;
        specialInfo.ptr_size = pParam360SCVP->inputBitstreamLen;
        memset(nalsize, 0, sizeof(nalsize));
        ret = hevc_import_ffextradata(&specialInfo, m_hevcState, nalsize, &spsCnt, 0);
        if (ret < 0)
        {
            if(bsWrite)
            {
                gts_bs_del(bsWrite);
            }
            return ret;
        }
        // modify the sliceheader
        memcpy(&hevcTmp, m_hevcState, sizeof(HEVCState));

        HEVC_SPS *sps = &(hevcTmp.sps[0]);
        sps->width = pParam360SCVP->destWidth;
        sps->height = pParam360SCVP->destHeight;

        HEVCSliceInfo *si = &hevcTmp.s_info;
        if (!si)
            return -1;
        si->first_slice_segment_in_pic_flag = 1;
        if (newSliceAddr)
            si->first_slice_segment_in_pic_flag = 0;
        si->slice_segment_address = newSliceAddr;

        // write the new sliceheader
        hevc_write_slice_header(bsWrite, &hevcTmp);
        pParam360SCVP->outputBitstreamLen = gts_bs_get_position(bsWrite);
        gts_bs_del(bsWrite);
        ret = 0;
    }

    return ret;
}

int32_t  TstitchStream::GenerateRWPK(RegionWisePacking* pRWPK, uint8_t *pRWPKBits, int32_t* RWPKBitsSize)
{
    if (!pRWPK || !pRWPKBits || !RWPKBitsSize)
        return -1;
    int32_t rwpkSize = 200; //need to define a macro
    GTS_BitStream *bs = gts_bs_new((const int8_t *)pRWPKBits, rwpkSize, GTS_BITSTREAM_WRITE);

    int32_t ret = -1;
    if (bs)
    {
        *RWPKBitsSize = hevc_write_RwpkSEI(bs, pRWPK, 1);
        gts_bs_del(bs);
        ret = 0;
    }
    return ret;
}

int32_t  TstitchStream::GenerateProj(int32_t projType, uint8_t *pProjBits, int32_t* pProjBitsSize)
{
    if (!pProjBits || !pProjBitsSize)
        return -1;
    int32_t projSize = 200; //need to define a macro
    GTS_BitStream *bs = gts_bs_new((const int8_t *)pProjBits, projSize, GTS_BITSTREAM_WRITE);

    int32_t ret = -1;
    if (bs)
    {
        *pProjBitsSize = hevc_write_ProjectionSEI(bs, projType, 1);
        gts_bs_del(bs);
        ret = 0;
    }
    return ret;
}
