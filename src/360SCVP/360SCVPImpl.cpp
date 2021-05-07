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
#include <dlfcn.h>
#include "360SCVPTiledstreamAPI.h"
#include "360SCVPViewportAPI.h"
#include "360SCVPMergeStreamAPI.h"
#include "360SCVPCommonDef.h"
#include "360SCVPHevcEncHdr.h"
#include "360SCVPLog.h"
#include "TileSelectionPlugins_API.h"
#include "360SCVPImpl.h"
#include "360SCVPHevcTileMerge.h"

TstitchStream::TstitchStream()
{
    m_pOutTile = new TileDef[MAX_TILE_NUM];
    m_pUpLeft = new point[6];
    m_pDownRight = new point[6];
    m_pNalInfo[0] = new nal_info[MAX_TILE_NUM];
    m_pNalInfo[1] = new nal_info[MAX_TILE_NUM];
    m_hevcState = new HEVCState;
    if (m_hevcState)
    {
        memset_s(m_hevcState, sizeof(HEVCState), 0);
        m_hevcState->sps_active_idx = -1;
    }
    memset_s(&m_pViewportParam, sizeof(generateViewPortParam), 0);
    memset_s(&m_mergeStreamParam, sizeof(param_mergeStream), 0);
    memset_s(&m_streamStitch, sizeof(param_gen_tiledStream), 0);
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
    m_bVPSReady = 0;
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
    memset_s(&m_sliceType, sizeof(SliceType), 0);
    m_usedType = 0;
    m_xTopLeftNet = 0;
    m_yTopLeftNet = 0;
    m_dstRwpk = RegionWisePacking();
    m_pTileSelection = NULL;
    m_pluginLibHdl = NULL;
    m_createPlugin = NULL;
    m_destroyPlugin = NULL;
    m_bNeedPlugin = false;
    m_tilesInfo = new ITileInfo[MAX_TILE_NUM];
    m_mapFaceInfo = new MapFaceInfo[6];
}

TstitchStream::TstitchStream(TstitchStream& other)
{
    m_pOutTile = new TileDef[MAX_TILE_NUM];
    memcpy_s(m_pOutTile, MAX_TILE_NUM * sizeof(TileDef), other.m_pOutTile, MAX_TILE_NUM * sizeof(TileDef));
    m_pUpLeft = new point[6];
    memcpy_s(m_pUpLeft, 6 * sizeof(point), other.m_pUpLeft, 6 * sizeof(point));
    m_pDownRight = new point[6];
    memcpy_s(m_pDownRight, 6 * sizeof(point), other.m_pDownRight, 6 * sizeof(point));
    m_pNalInfo[0] = new nal_info[MAX_TILE_NUM];
    memcpy_s(m_pNalInfo[0], MAX_TILE_NUM * sizeof(nal_info), other.m_pNalInfo[0], MAX_TILE_NUM * sizeof(nal_info));
    m_pNalInfo[1] = new nal_info[MAX_TILE_NUM];
    memcpy_s(m_pNalInfo[1], MAX_TILE_NUM * sizeof(nal_info), other.m_pNalInfo[1], MAX_TILE_NUM * sizeof(nal_info));
    m_hevcState = new HEVCState;
    if (m_hevcState)
    {
        memcpy_s(m_hevcState, sizeof(HEVCState), other.m_hevcState, sizeof(HEVCState));
    }

    memcpy_s(&m_pViewportParam, sizeof(generateViewPortParam), &(other.m_pViewportParam), sizeof(generateViewPortParam));
    memcpy_s(&m_mergeStreamParam, sizeof(param_mergeStream), &(other.m_mergeStreamParam), sizeof(param_mergeStream));
    memcpy_s(&m_streamStitch, sizeof(param_gen_tiledStream), &(other.m_streamStitch), sizeof(param_gen_tiledStream));
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
    m_bVPSReady = other.m_bVPSReady;
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
    memcpy_s(m_specialInfo[0], 200 * sizeof(unsigned char), other.m_specialInfo[0], 200 * sizeof(unsigned char));
    m_specialInfo[1] = new unsigned char[200];
    memcpy_s(m_specialInfo[1], 200 * sizeof(unsigned char), other.m_specialInfo[1], 200 * sizeof(unsigned char));
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
    memcpy_s(&m_sliceType, sizeof(SliceType), &(other.m_sliceType), sizeof(SliceType));
    m_usedType = other.m_usedType;
    m_xTopLeftNet = other.m_xTopLeftNet;
    m_yTopLeftNet = other.m_yTopLeftNet;
    m_dstRwpk = RegionWisePacking();
    m_dstRwpk = other.m_dstRwpk;
    m_pTileSelection = NULL;
    m_pluginLibHdl = NULL;
    m_createPlugin = NULL;
    m_destroyPlugin = NULL;
    m_bNeedPlugin = false;
    m_tilesInfo = new ITileInfo[MAX_TILE_NUM];
    memcpy_s(m_tilesInfo, MAX_TILE_NUM * sizeof(ITileInfo), other.m_tilesInfo, MAX_TILE_NUM * sizeof(ITileInfo));
    m_mapFaceInfo = new MapFaceInfo[6];
    memcpy_s(m_mapFaceInfo, 6 * sizeof(int32_t), other.m_mapFaceInfo, 6 * sizeof(int32_t));
}

TstitchStream& TstitchStream::operator=(const TstitchStream& other)
{
    if (&other == this)
        return *this;
    SAFE_DELETE_ARRAY(m_pOutTile);
    m_pOutTile = new TileDef[MAX_TILE_NUM];
    memcpy_s(m_pOutTile, MAX_TILE_NUM * sizeof(TileDef), other.m_pOutTile, MAX_TILE_NUM * sizeof(TileDef));
    SAFE_DELETE_ARRAY(m_pUpLeft);
    m_pUpLeft = new point[6];
    memcpy_s(m_pUpLeft, 6 * sizeof(point), other.m_pUpLeft, 6 * sizeof(point));
    SAFE_DELETE_ARRAY(m_pDownRight);
    m_pDownRight = new point[6];
    memcpy_s(m_pDownRight, 6 * sizeof(point), other.m_pDownRight, 6 * sizeof(point));
    SAFE_DELETE_ARRAY(m_pNalInfo[0]);
    m_pNalInfo[0] = new nal_info[MAX_TILE_NUM];
    memcpy_s(m_pNalInfo[0], MAX_TILE_NUM * sizeof(nal_info), other.m_pNalInfo[0], MAX_TILE_NUM * sizeof(nal_info));
    SAFE_DELETE_ARRAY(m_pNalInfo[1]);
    m_pNalInfo[1] = new nal_info[MAX_TILE_NUM];
    memcpy_s(m_pNalInfo[1], MAX_TILE_NUM * sizeof(nal_info), other.m_pNalInfo[1], MAX_TILE_NUM * sizeof(nal_info));
    SAFE_DELETE(m_hevcState);
    m_hevcState = new HEVCState;
    if (m_hevcState)
    {
        memcpy_s(m_hevcState, sizeof(HEVCState), other.m_hevcState, sizeof(HEVCState));
    }

    memcpy_s(&m_pViewportParam, sizeof(generateViewPortParam), &(other.m_pViewportParam), sizeof(generateViewPortParam));
    memcpy_s(&m_mergeStreamParam, sizeof(param_mergeStream), &(other.m_mergeStreamParam), sizeof(param_mergeStream));
    memcpy_s(&m_streamStitch, sizeof(param_gen_tiledStream), &(other.m_streamStitch), sizeof(param_gen_tiledStream));
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
    m_bVPSReady = other.m_bVPSReady;
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
    SAFE_DELETE_ARRAY(m_specialInfo[0]);
    m_specialInfo[0] = new unsigned char[200];
    memcpy_s(m_specialInfo[0], 200 * sizeof(unsigned char), other.m_specialInfo[0], 200 * sizeof(unsigned char));
    SAFE_DELETE_ARRAY(m_specialInfo[1]);
    m_specialInfo[1] = new unsigned char[200];
    memcpy_s(m_specialInfo[1], 200 * sizeof(unsigned char), other.m_specialInfo[1], 200 * sizeof(unsigned char));
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
    memcpy_s(&m_sliceType, sizeof(SliceType), &(other.m_sliceType), sizeof(SliceType));
    m_usedType = other.m_usedType;
    m_xTopLeftNet = other.m_xTopLeftNet;
    m_yTopLeftNet = other.m_yTopLeftNet;
    m_dstRwpk = RegionWisePacking();
    m_dstRwpk = other.m_dstRwpk;
    m_pTileSelection = NULL;
    m_pluginLibHdl = NULL;
    m_createPlugin = NULL;
    m_destroyPlugin = NULL;
    m_bNeedPlugin = false;
    SAFE_DELETE_ARRAY(m_tilesInfo);
    m_tilesInfo = new ITileInfo[MAX_TILE_NUM];
    memcpy_s(m_tilesInfo, MAX_TILE_NUM * sizeof(ITileInfo), other.m_tilesInfo, MAX_TILE_NUM * sizeof(ITileInfo));
    SAFE_DELETE_ARRAY(m_mapFaceInfo);
    m_mapFaceInfo = new MapFaceInfo[6];
    memcpy_s(m_mapFaceInfo, 6 * sizeof(int32_t), other.m_mapFaceInfo, 6 * sizeof(int32_t));

    return *this;
}

TstitchStream::~TstitchStream()
{
    SAFE_DELETE_ARRAY(m_pOutTile);
    SAFE_DELETE_ARRAY(m_pUpLeft);
    SAFE_DELETE_ARRAY(m_pDownRight);
    SAFE_DELETE_ARRAY(m_pNalInfo[0]);
    SAFE_DELETE_ARRAY(m_pNalInfo[1]);

    SAFE_DELETE(m_hevcState);
    SAFE_DELETE_ARRAY(m_specialInfo[0]);
    SAFE_DELETE_ARRAY(m_specialInfo[1]);
    SAFE_DELETE_ARRAY(m_tilesInfo);
    SAFE_DELETE_ARRAY(m_mapFaceInfo);
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
        if (pViewPortInfo->paramVideoFP.rows != 1 || pViewPortInfo->paramVideoFP.cols != 1)
            SCVP_LOG(LOG_WARNING, "Viewport rows and cols number is illegal!!! Set them to defaul 1x1 !!!\n");

        m_pViewportParam.m_paramVideoFP.cols = 1;
        m_pViewportParam.m_paramVideoFP.rows = 1;
        m_pViewportParam.m_paramVideoFP.faces[0][0].faceHeight = pViewPortInfo->faceHeight;
        m_pViewportParam.m_paramVideoFP.faces[0][0].faceWidth = pViewPortInfo->faceWidth;
        m_pViewportParam.m_paramVideoFP.faces[0][0].idFace = 1;
        m_pViewportParam.m_paramVideoFP.faces[0][0].rotFace = NO_TRANSFORM;
    }
    else if (m_pViewportParam.m_input_geoType == E_SVIDEO_CUBEMAP)
    {
        /* Check the paramVideoFP rows / cols exceeds the maximum array size for Cubemap projections */
        if (pViewPortInfo->paramVideoFP.rows > 6 || pViewPortInfo->paramVideoFP.cols > 6
           || pViewPortInfo->paramVideoFP.rows <= 0 || pViewPortInfo->paramVideoFP.cols <= 0 ) {
            SCVP_LOG(LOG_ERROR, "Viewport rows and cols is not suitable for Cubemap: rows is %d, col is %d\n", pViewPortInfo->paramVideoFP.rows, pViewPortInfo->paramVideoFP.cols);
            return ERROR_BAD_PARAM;
        }
        else {
            m_pViewportParam.m_paramVideoFP.cols = pViewPortInfo->paramVideoFP.cols;
            m_pViewportParam.m_paramVideoFP.rows = pViewPortInfo->paramVideoFP.rows;
            for (int i = 0; i < pViewPortInfo->paramVideoFP.rows; i++) {
                for (int j = 0; j < pViewPortInfo->paramVideoFP.cols; j++) {
                     m_pViewportParam.m_paramVideoFP.faces[i][j].faceHeight = pViewPortInfo->paramVideoFP.faces[i][j].faceHeight;
                     m_pViewportParam.m_paramVideoFP.faces[i][j].faceWidth = pViewPortInfo->paramVideoFP.faces[i][j].faceWidth;
                     m_pViewportParam.m_paramVideoFP.faces[i][j].idFace = pViewPortInfo->paramVideoFP.faces[i][j].idFace;
                     m_pViewportParam.m_paramVideoFP.faces[i][j].rotFace = pViewPortInfo->paramVideoFP.faces[i][j].rotFace;
                }
            }
        }
    }
    else if (m_pViewportParam.m_input_geoType == E_SVIDEO_PLANAR)
    {
        if (pViewPortInfo->paramVideoFP.rows != 1 || pViewPortInfo->paramVideoFP.cols != 1)
            SCVP_LOG(LOG_WARNING, "Viewport rows and cols number is illegal!!! Set them to default 1x1 !!!\n");
        m_pViewportParam.m_paramVideoFP.cols = 1;
        m_pViewportParam.m_paramVideoFP.rows = 1;
        m_pViewportParam.m_paramVideoFP.faces[0][0].faceHeight = pViewPortInfo->paramVideoFP.faces[0][0].faceHeight;
        m_pViewportParam.m_paramVideoFP.faces[0][0].faceWidth = pViewPortInfo->paramVideoFP.faces[0][0].faceWidth;
        m_pViewportParam.m_paramVideoFP.faces[0][0].idFace = pViewPortInfo->paramVideoFP.faces[0][0].idFace;
        m_pViewportParam.m_paramVideoFP.faces[0][0].rotFace = NO_TRANSFORM;
    }
    else {
        SCVP_LOG(LOG_ERROR, "The Input GeoType %d is not supported by viewport implementation!\n", m_pViewportParam.m_input_geoType);
        return ERROR_BAD_PARAM;
    }

    m_pViewport = genViewport_Init(&m_pViewportParam);
    return ERROR_NONE;
}

int32_t TstitchStream::SetLogCallBack(LogFunction logFunction)
{
    if (!logFunction)
        return OMAF_ERROR_NULL_PTR;

    logCallBack = logFunction;
    return ERROR_NONE;
}

static int32_t tile_faceId_init(param_360SCVP* pParamStitchStream, int32_t* tileNumCol, int32_t* tileNumRow, ITileInfo* dstTileInfo)
{
    ITileInfo* tileInfo = dstTileInfo;
    int32_t highResNumRegions = tileNumCol[0] * tileNumRow[0];
    int32_t lowResNumRegions = tileNumCol[1] * tileNumRow[1];
    uint8_t faceRowId, faceColId;
    uint8_t regColId, regRowId;
    int32_t idxInPic = 0;
    int32_t mappedFaceId;
    if ((pParamStitchStream->paramViewPort.paramVideoFP.rows == 2) && (pParamStitchStream->paramViewPort.paramVideoFP.cols == 3)) {
        for (idxInPic = 0; idxInPic < highResNumRegions; idxInPic++) {
            regColId = idxInPic % tileNumCol[0];
            regRowId = idxInPic / tileNumCol[0];
            faceRowId = (uint8_t)(regRowId / pParamStitchStream->paramViewPort.tileNumRow);
            faceColId = (uint8_t)(regColId / pParamStitchStream->paramViewPort.tileNumCol);
            mappedFaceId = pParamStitchStream->paramViewPort.paramVideoFP.faces[faceRowId][faceColId].idFace;
            if (mappedFaceId == OMAF_FACE_PY)
                tileInfo->faceId = FACE_PY;
            else if (mappedFaceId == OMAF_FACE_PX)
                tileInfo->faceId = FACE_PX;
            else if (mappedFaceId == OMAF_FACE_NY)
                tileInfo->faceId = FACE_NY;
            else if (mappedFaceId == OMAF_FACE_NZ)
                tileInfo->faceId = FACE_NZ;
            else if (mappedFaceId == OMAF_FACE_NX)
                tileInfo->faceId = FACE_NX;
            else if (mappedFaceId == OMAF_FACE_PZ)
                tileInfo->faceId = FACE_PZ;
            tileInfo++;
        }
        int32_t tileNumColLow = pParamStitchStream->paramViewPort.tileNumCol / (pParamStitchStream->frameWidth / pParamStitchStream->frameWidthLow);
        int32_t tileNumRowLow = pParamStitchStream->paramViewPort.tileNumRow / (pParamStitchStream->frameHeight / pParamStitchStream->frameHeightLow);
        for (int32_t idxInPic = 0; idxInPic < lowResNumRegions; idxInPic++) {
            regColId = idxInPic % tileNumCol[1];
            regRowId = idxInPic / tileNumCol[1];
            faceRowId = (uint8_t)(regRowId / tileNumRowLow);
            faceColId = (uint8_t)(regColId / tileNumColLow);
            mappedFaceId = pParamStitchStream->paramViewPort.paramVideoFP.faces[faceRowId][faceColId].idFace;
            if (mappedFaceId == OMAF_FACE_PY)
                tileInfo->faceId = FACE_PY;
            else if (mappedFaceId == OMAF_FACE_PX)
                tileInfo->faceId = FACE_PX;
            else if (mappedFaceId == OMAF_FACE_NY)
                tileInfo->faceId = FACE_NY;
            else if (mappedFaceId == OMAF_FACE_NZ)
                tileInfo->faceId = FACE_NZ;
            else if (mappedFaceId == OMAF_FACE_NX)
                tileInfo->faceId = FACE_NX;
            else if (mappedFaceId == OMAF_FACE_PZ)
                tileInfo->faceId = FACE_PZ;
            tileInfo++;
        }
    }
    return ERROR_NONE;
}

static int32_t tile_localPos_init(param_360SCVP* pParamStitchStream, int32_t* tileNumCol, int32_t* tileNumRow, ITileInfo* dstTileInfo)
{
    ITileInfo* tileInfo = dstTileInfo;
    if (tileInfo == NULL) {
        SCVP_LOG(LOG_ERROR, "TilesInfo is not allocated!\n");
        return ERROR_NULL_PTR;
    }
    int32_t highResNumRegions = tileNumCol[0] * tileNumRow[0];
    int32_t lowResNumRegions = tileNumCol[1] * tileNumRow[1];

    int32_t tileWidth = pParamStitchStream->paramViewPort.faceWidth / pParamStitchStream->paramViewPort.tileNumCol;
    int32_t tileHeight = pParamStitchStream->paramViewPort.faceHeight / pParamStitchStream->paramViewPort.tileNumRow;
    int32_t regColId, regRowId;
    int32_t regColInFace, regRowInFace;
    int32_t localX, localY;
    if ((pParamStitchStream->paramViewPort.paramVideoFP.rows == 2) && (pParamStitchStream->paramViewPort.paramVideoFP.cols == 3)) {
        for (int32_t idxInPic = 0; idxInPic < highResNumRegions; idxInPic++) {
            regColId = idxInPic % tileNumCol[0];
            regRowId = idxInPic / tileNumCol[0];
            regColInFace = regColId % pParamStitchStream->paramViewPort.tileNumCol;
            regRowInFace = regRowId % pParamStitchStream->paramViewPort.tileNumRow;
            localX = tileWidth * regColInFace;
            localY = tileHeight * regRowInFace;
            if (tileInfo->faceId == FACE_PZ) {
                tileInfo->vertPos = pParamStitchStream->paramViewPort.faceWidth - tileWidth - localX;
                tileInfo->horzPos = localY;
            }
            else if (tileInfo->faceId == FACE_NZ) {
                tileInfo->vertPos = localX;
                tileInfo->horzPos = pParamStitchStream->paramViewPort.faceHeight - tileHeight - localY;
            }
            else {
                tileInfo->horzPos = localX;
                tileInfo->vertPos = localY;
            }
            tileInfo++;
        }
        int32_t faceWidthLow = pParamStitchStream->paramViewPort.faceWidth / (pParamStitchStream->frameWidth / pParamStitchStream->frameWidthLow);
        int32_t faceHeightLow = pParamStitchStream->paramViewPort.faceHeight / (pParamStitchStream->frameHeight / pParamStitchStream->frameHeightLow);
        int32_t tileWidthLow = pParamStitchStream->frameWidthLow / tileNumCol[1];
        int32_t tileHeightLow = pParamStitchStream->frameHeightLow / tileNumRow[1];
        int32_t tileNumColLow = faceWidthLow / tileWidthLow;
        int32_t tileNumRowLow = faceHeightLow / tileHeightLow;
        for (int32_t idxInPic = 0; idxInPic < lowResNumRegions; idxInPic++) {
            regColId = idxInPic % tileNumCol[1];
            regRowId = idxInPic / tileNumCol[1];
            regColInFace = regColId % tileNumColLow;
            regRowInFace = regRowId % tileNumRowLow;
            localX = tileWidthLow * regColInFace;
            localY = tileHeightLow * regRowInFace;
            if (tileInfo->faceId == FACE_PZ) {
                tileInfo->vertPos = faceWidthLow - tileWidthLow - localX;
                tileInfo->horzPos = localY;
            }
            else if (tileInfo->faceId == FACE_NZ) {
                tileInfo->vertPos = localX;
                tileInfo->horzPos = faceHeightLow - tileHeightLow - localY;
            }
            else {
                tileInfo->horzPos = localX;
                tileInfo->vertPos = localY;
            }
            tileInfo++;
        }
    }
    return ERROR_NONE;
}

int32_t TstitchStream::initTileInfo(param_360SCVP* pParamStitchStream)
{
    ITileInfo* tileInfo = m_tilesInfo;
    if (tileInfo == NULL) {
        SCVP_LOG(LOG_ERROR, "TilesInfo is not allocated!\n");
        return ERROR_NULL_PTR;
    }
    tile_faceId_init(pParamStitchStream, m_tileWidthCountOri, m_tileHeightCountOri, m_tilesInfo);

    if (pParamStitchStream->paramViewPort.usageType == E_MERGE_AND_VIEWPORT) {
        if ((pParamStitchStream->paramViewPort.paramVideoFP.rows == 2) && (pParamStitchStream->paramViewPort.paramVideoFP.cols == 3)) {
            tile_localPos_init(pParamStitchStream, m_tileWidthCountOri, m_tileHeightCountOri, m_tilesInfo);
            for (int32_t i = 0; i < 2; i++) {
                for (int32_t j = 0; j < 3; j++) {
                    m_mapFaceInfo[i * 3 + j].mappedStandardFaceId = pParamStitchStream->paramViewPort.paramVideoFP.faces[i][j].idFace;
                    m_mapFaceInfo[i * 3 + j].transformType = pParamStitchStream->paramViewPort.paramVideoFP.faces[i][j].rotFace;
                }
            }
        }
        else {
            SCVP_LOG(LOG_WARNING, "Will support tile arrangement other than 2x3 further...\n");
            return ERROR_BAD_PARAM;
        }
    }
    return ERROR_NONE;
}

int32_t TstitchStream::initMerge(param_360SCVP* pParamStitchStream, int32_t sliceSize)
{
    m_mergeStreamParam.pOutputBitstream = pParamStitchStream->pOutputBitstream;
    m_mergeStreamParam.inputBistreamsLen = pParamStitchStream->inputBitstreamLen;

    m_mergeStreamParam.highRes.width = pParamStitchStream->frameWidth;
    m_mergeStreamParam.highRes.height = pParamStitchStream->frameHeight;
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

    if (!m_mergeStreamParam.highRes.pHeader || !m_mergeStreamParam.lowRes.pHeader) {
        SCVP_LOG(LOG_ERROR, "Init Merge Failed: pHeader of highRes or lowRes is NULL\n");
        if (m_mergeStreamParam.highRes.pHeader) {
            free(m_mergeStreamParam.highRes.pHeader);
            m_mergeStreamParam.highRes.pHeader = NULL;
        }
        if (m_mergeStreamParam.lowRes.pHeader) {
            free(m_mergeStreamParam.lowRes.pHeader);
            m_mergeStreamParam.lowRes.pHeader = NULL;
        }

        return -1;
    }

    m_mergeStreamParam.highRes.pHeader->pTiledBitstreamBuffer = (uint8_t *)malloc(sliceSize);
    m_mergeStreamParam.lowRes.pHeader->pTiledBitstreamBuffer = (uint8_t *)malloc(100);

    if (!m_mergeStreamParam.highRes.pHeader->pTiledBitstreamBuffer || !m_mergeStreamParam.lowRes.pHeader->pTiledBitstreamBuffer) {
        SCVP_LOG(LOG_ERROR, "Init Merge Failed: Tiled Bitstream Buffer of highRes or lowRes is not allocated\n");

        if (m_mergeStreamParam.highRes.pHeader->pTiledBitstreamBuffer) {
            free(m_mergeStreamParam.highRes.pHeader->pTiledBitstreamBuffer);
            m_mergeStreamParam.highRes.pHeader->pTiledBitstreamBuffer = NULL;
        }
        if (m_mergeStreamParam.lowRes.pHeader->pTiledBitstreamBuffer) {
            free(m_mergeStreamParam.lowRes.pHeader->pTiledBitstreamBuffer);
            m_mergeStreamParam.lowRes.pHeader->pTiledBitstreamBuffer = NULL;
        }
        if (m_mergeStreamParam.highRes.pHeader) {
            free(m_mergeStreamParam.highRes.pHeader);
            m_mergeStreamParam.highRes.pHeader = NULL;
        }
        if (m_mergeStreamParam.lowRes.pHeader) {
            free(m_mergeStreamParam.lowRes.pHeader);
            m_mergeStreamParam.lowRes.pHeader = NULL;
        }
        return -1;
    }

    m_mergeStreamParam.highRes.pTiledBitstreams = (param_oneStream_info **)malloc(HR_ntile * sizeof(param_oneStream_info *));
    if (!m_mergeStreamParam.highRes.pTiledBitstreams) {
        SCVP_LOG(LOG_ERROR, "Init Merge Failed: Tiled Bitstreams of highRes is not allocated\n");

        if (m_mergeStreamParam.highRes.pHeader->pTiledBitstreamBuffer) {
            free(m_mergeStreamParam.highRes.pHeader->pTiledBitstreamBuffer);
            m_mergeStreamParam.highRes.pHeader->pTiledBitstreamBuffer = NULL;
        }
        if (m_mergeStreamParam.lowRes.pHeader->pTiledBitstreamBuffer) {
            free(m_mergeStreamParam.lowRes.pHeader->pTiledBitstreamBuffer);
            m_mergeStreamParam.lowRes.pHeader->pTiledBitstreamBuffer = NULL;
        }
        if (m_mergeStreamParam.highRes.pHeader) {
            free(m_mergeStreamParam.highRes.pHeader);
            m_mergeStreamParam.highRes.pHeader = NULL;
        }
        if (m_mergeStreamParam.lowRes.pHeader) {
            free(m_mergeStreamParam.lowRes.pHeader);
            m_mergeStreamParam.lowRes.pHeader = NULL;
        }
        return -1;
    }

    for (int32_t i = 0; i < HR_ntile; i++)
    {
        m_mergeStreamParam.highRes.pTiledBitstreams[i] = (param_oneStream_info *)malloc(sizeof(param_oneStream_info));
        if (!m_mergeStreamParam.highRes.pTiledBitstreams[i]) {
            for (int32_t j = 0; j < i; j++) {
                if (m_mergeStreamParam.highRes.pTiledBitstreams[j]) {
                    free(m_mergeStreamParam.highRes.pTiledBitstreams[j]);
                    m_mergeStreamParam.highRes.pTiledBitstreams[j] = NULL;
                }
            }

            free(m_mergeStreamParam.highRes.pTiledBitstreams);
            m_mergeStreamParam.highRes.pTiledBitstreams = NULL;

            if (m_mergeStreamParam.highRes.pHeader->pTiledBitstreamBuffer) {
                free(m_mergeStreamParam.highRes.pHeader->pTiledBitstreamBuffer);
                m_mergeStreamParam.highRes.pHeader->pTiledBitstreamBuffer = NULL;
            }
            if (m_mergeStreamParam.lowRes.pHeader->pTiledBitstreamBuffer) {
                free(m_mergeStreamParam.lowRes.pHeader->pTiledBitstreamBuffer);
                m_mergeStreamParam.lowRes.pHeader->pTiledBitstreamBuffer = NULL;
            }
            if (m_mergeStreamParam.highRes.pHeader) {
                free(m_mergeStreamParam.highRes.pHeader);
                m_mergeStreamParam.highRes.pHeader = NULL;
            }
            if (m_mergeStreamParam.lowRes.pHeader) {
                free(m_mergeStreamParam.lowRes.pHeader);
                m_mergeStreamParam.lowRes.pHeader = NULL;
            }
            return -1;
        }
    }
    m_mergeStreamParam.lowRes.pTiledBitstreams = (param_oneStream_info **)malloc(LR_ntile * sizeof(param_oneStream_info *));
    if (!m_mergeStreamParam.lowRes.pTiledBitstreams)
    {
        if (m_mergeStreamParam.highRes.pTiledBitstreams)
        {
            for (int32_t i = 0; i < HR_ntile; i++)
            {
                if (m_mergeStreamParam.highRes.pTiledBitstreams[i])
                {
                    free(m_mergeStreamParam.highRes.pTiledBitstreams[i]);
                    m_mergeStreamParam.highRes.pTiledBitstreams[i] = NULL;
                }
            }
            free(m_mergeStreamParam.highRes.pTiledBitstreams);
            m_mergeStreamParam.highRes.pTiledBitstreams = NULL;
        }

        if (m_mergeStreamParam.highRes.pHeader->pTiledBitstreamBuffer) {
            free(m_mergeStreamParam.highRes.pHeader->pTiledBitstreamBuffer);
            m_mergeStreamParam.highRes.pHeader->pTiledBitstreamBuffer = NULL;
        }
        if (m_mergeStreamParam.lowRes.pHeader->pTiledBitstreamBuffer) {
            free(m_mergeStreamParam.lowRes.pHeader->pTiledBitstreamBuffer);
            m_mergeStreamParam.lowRes.pHeader->pTiledBitstreamBuffer = NULL;
        }
        if (m_mergeStreamParam.highRes.pHeader) {
            free(m_mergeStreamParam.highRes.pHeader);
            m_mergeStreamParam.highRes.pHeader = NULL;
        }
        if (m_mergeStreamParam.lowRes.pHeader) {
            free(m_mergeStreamParam.lowRes.pHeader);
            m_mergeStreamParam.lowRes.pHeader = NULL;
        }
        return -1;
    }
    for (int32_t i = 0; i < LR_ntile; i++)
    {
        m_mergeStreamParam.lowRes.pTiledBitstreams[i] = (param_oneStream_info *)malloc(sizeof(param_oneStream_info));
        if (!m_mergeStreamParam.lowRes.pTiledBitstreams[i])
        {
            for (int32_t j = 0; j < i; j++)
            {
                if (m_mergeStreamParam.lowRes.pTiledBitstreams[j])
                {
                    free(m_mergeStreamParam.lowRes.pTiledBitstreams[j]);
                    m_mergeStreamParam.lowRes.pTiledBitstreams[j] = NULL;
                }
            }

            free(m_mergeStreamParam.lowRes.pTiledBitstreams);
            m_mergeStreamParam.lowRes.pTiledBitstreams = NULL;

            if (m_mergeStreamParam.highRes.pTiledBitstreams)
            {
                for (int32_t i = 0; i < HR_ntile; i++)
                {
                    if (m_mergeStreamParam.highRes.pTiledBitstreams[i])
                    {
                        free(m_mergeStreamParam.highRes.pTiledBitstreams[i]);
                        m_mergeStreamParam.highRes.pTiledBitstreams[i] = NULL;
                    }
                }
                free(m_mergeStreamParam.highRes.pTiledBitstreams);
                m_mergeStreamParam.highRes.pTiledBitstreams = NULL;
            }

            if (m_mergeStreamParam.highRes.pHeader->pTiledBitstreamBuffer) {
                free(m_mergeStreamParam.highRes.pHeader->pTiledBitstreamBuffer);
                m_mergeStreamParam.highRes.pHeader->pTiledBitstreamBuffer = NULL;
            }
            if (m_mergeStreamParam.lowRes.pHeader->pTiledBitstreamBuffer) {
                free(m_mergeStreamParam.lowRes.pHeader->pTiledBitstreamBuffer);
                m_mergeStreamParam.lowRes.pHeader->pTiledBitstreamBuffer = NULL;
            }
            if (m_mergeStreamParam.highRes.pHeader) {
                free(m_mergeStreamParam.highRes.pHeader);
                m_mergeStreamParam.highRes.pHeader = NULL;
            }
            if (m_mergeStreamParam.lowRes.pHeader) {
                free(m_mergeStreamParam.lowRes.pHeader);
                m_mergeStreamParam.lowRes.pHeader = NULL;
            }
            return -1;
        }
    }

    m_pMergeStream = tile_merge_Init(&m_mergeStreamParam);
    if (pParamStitchStream->paramViewPort.geoTypeInput == E_SVIDEO_CUBEMAP)
        initTileInfo(pParamStitchStream);

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
    m_pViewportParam.m_input_geoType = pParamStitchStream->paramViewPort.geoTypeInput;

    if (pParamStitchStream->logFunction)
        logCallBack = (LogFunction)(pParamStitchStream->logFunction);
    else
        logCallBack = GlogFunction;

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
    else if (pParamStitchStream->paramViewPort.geoTypeInput == E_SVIDEO_PLANAR)
    {
        char* pluginLibPath = pParamStitchStream->pluginDef.pluginLibPath;
        m_bNeedPlugin = true;
        if (!pluginLibPath) {
            SCVP_LOG(LOG_ERROR, "The plugin library file path is NULL!\n");
            return OMAF_INVALID_PLUGIN_PARAM;
        }
        void* libHandler = dlopen(pluginLibPath, RTLD_LAZY);
        const char *dlsymErr = dlerror();
        if (!libHandler)
        {
            SCVP_LOG(LOG_ERROR,"failed to open tile selection library path!\n");
            return OMAF_ERROR_DLOPEN;
        }

        if (dlsymErr) {
            SCVP_LOG(LOG_ERROR, "Get error msg when load the plugin lib file: %s\n", dlsymErr);
            return OMAF_ERROR_DLSYM;
        }

        CreateTileSelection *createTS = NULL;
        createTS = (CreateTileSelection*)dlsym(libHandler, "Create");
        dlsymErr = dlerror();
        if (dlsymErr) {
            SCVP_LOG(LOG_ERROR, "Failed to load symbol Create: %s\n", dlsymErr);
            return OMAF_ERROR_DLSYM;
        }

        if (!createTS) {
            SCVP_LOG(LOG_ERROR, "NULL Tile Selection Creator !\n");
            return ERROR_NULL_PTR;
        }
        TileSelection *tileSelection = createTS();
        if (!tileSelection) {
            SCVP_LOG(LOG_ERROR,"failed to Create TileSelection Handler!\n");
            dlclose(libHandler);
            libHandler = NULL;
            return ERROR_NULL_PTR;
        }

        DestroyTileSelection *destroyTS = NULL;
        destroyTS = (DestroyTileSelection*)dlsym(libHandler, "Destroy");
        dlsymErr = dlerror();
        if (dlsymErr)
        {
            SCVP_LOG(LOG_ERROR, "Failed to load symbol Destroy for TileSelection!\n");
            return OMAF_ERROR_DLSYM;
        }
        if (!destroyTS) {
            SCVP_LOG(LOG_ERROR, "NULL Destroy TileSelection!\n");
            return ERROR_NULL_PTR;
        }
        m_pTileSelection = tileSelection;
        m_pluginLibHdl = libHandler;
        m_createPlugin = (void*)createTS;
        m_destroyPlugin = (void*)destroyTS;
        ret = m_pTileSelection->Initialize(pParamStitchStream);
        if (ret) {
            SCVP_LOG(LOG_ERROR, "Failed to Initialize Tile Selection Plugin with error code %d\n", ret);
        }
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
        ret = initViewport(&pParamStitchStream->paramViewPort, tilecolCount, tilerowCount);

        int32_t sliceHeight = pParamStitchStream->paramViewPort.faceHeight / (tilerowCount / pParamStitchStream->paramViewPort.paramVideoFP.rows);
        int32_t sliceWidth = pParamStitchStream->paramViewPort.faceWidth / (tilecolCount / pParamStitchStream->paramViewPort.paramVideoFP.cols);
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

        if (m_mergeStreamParam.highRes.pTiledBitstreams)
        {
            for (int32_t i = 0; i < HR_ntile; i++)
            {
                if (m_mergeStreamParam.highRes.pTiledBitstreams[i])
                {
                    free(m_mergeStreamParam.highRes.pTiledBitstreams[i]);
                    m_mergeStreamParam.highRes.pTiledBitstreams[i] = NULL;
                }
            }
            free(m_mergeStreamParam.highRes.pTiledBitstreams);
            m_mergeStreamParam.highRes.pTiledBitstreams = NULL;
        }
        if (m_mergeStreamParam.lowRes.pTiledBitstreams)
        {
            for (int32_t i = 0; i < LR_ntile; i++)
            {
                if (m_mergeStreamParam.lowRes.pTiledBitstreams[i])
                {
                    free(m_mergeStreamParam.lowRes.pTiledBitstreams[i]);
                    m_mergeStreamParam.lowRes.pTiledBitstreams[i] = NULL;
                }
            }
            free(m_mergeStreamParam.lowRes.pTiledBitstreams);
            m_mergeStreamParam.lowRes.pTiledBitstreams = NULL;
        }

        ret = tile_merge_Close(m_pMergeStream);
    }
    if(m_pViewport)
        ret |= genViewport_unInit(m_pViewport);
    if (m_pSteamStitch)
        ret |= genTiledStream_unInit(m_pSteamStitch);
    SAFE_DELETE_ARRAY(m_pOutTile);
    SAFE_DELETE_ARRAY(m_pUpLeft);
    SAFE_DELETE_ARRAY(m_pDownRight);
    SAFE_DELETE_ARRAY(m_pNalInfo[0]);
    SAFE_DELETE_ARRAY(m_pNalInfo[1]);

    SAFE_DELETE(m_hevcState);
    SAFE_DELETE_ARRAY(m_specialInfo[0]);
    SAFE_DELETE_ARRAY(m_specialInfo[1]);

    SAFE_DELETE_ARRAY(m_dstRwpk.rectRegionPacking);

    if (m_pTileSelection) {
        ret = m_pTileSelection->UnInit();
        if (ret != ERROR_NONE) {
            SCVP_LOG(LOG_ERROR, "Tile Selection Uninitialization is Failed with error code %d\n", ret);
            return ret;
        }
        DestroyTileSelection *destroyTS = NULL;

        destroyTS = (DestroyTileSelection*)m_destroyPlugin;
        if (!destroyTS)
        {
            SCVP_LOG(LOG_ERROR, "NULL Destroy TileSelection!\n");
            return ERROR_NULL_PTR;
        }
        void* pluginHdl = m_pluginLibHdl;
        (*destroyTS)(m_pTileSelection);
        if (pluginHdl != NULL)
            dlclose(pluginHdl);
        m_createPlugin = NULL;
        m_destroyPlugin = NULL;
        m_pluginLibHdl = NULL;
        ret = ERROR_NONE;
    }

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

    memset_s(&GenStreamParam, sizeof(param_gen_tiledStream), 0);
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
            memmove_s(TiledBitstream.pTiledBitstreamBuffer + m_specialDataLen[streamIdx], pParamStitchStream->inputBitstreamLen, TiledBitstream.pTiledBitstreamBuffer, pParamStitchStream->inputBitstreamLen);
            memcpy_s(TiledBitstream.pTiledBitstreamBuffer, m_specialDataLen[streamIdx], m_specialInfo[streamIdx], m_specialDataLen[streamIdx]);
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
        if (((pGenTilesStream->parseType == E_PARSER_ONENAL)) && m_bVPSReady && m_bSPSReady && m_bPPSReady)
        {
            memcpy_s(pSlice->hevcSlice->vps, 16 * sizeof(HEVC_VPS), m_hevcState->vps, 16 * sizeof(HEVC_VPS));
            pSlice->hevcSlice->last_parsed_vps_id = m_hevcState->last_parsed_vps_id;
            memcpy_s(pSlice->hevcSlice->sps, 16 * sizeof(HEVC_SPS), m_hevcState->sps, 16 * sizeof(HEVC_SPS));
            pSlice->hevcSlice->last_parsed_sps_id = m_hevcState->last_parsed_sps_id;
            memcpy_s(pSlice->hevcSlice->pps, 64 * sizeof(HEVC_PPS), m_hevcState->pps, 64 * sizeof(HEVC_PPS));
            pSlice->hevcSlice->last_parsed_pps_id = m_hevcState->last_parsed_pps_id;
        }

        genTiledStream_parseNals(&GenStreamParam, pGenStream);

        if(pGenTilesStream->parseType != E_PARSER_ONENAL)
            memcpy_s(m_hevcState, sizeof(HEVCState), pSlice->hevcSlice, sizeof(HEVCState));
        else
        {
            if (GenStreamParam.nalType == GTS_HEVC_NALU_VID_PARAM)
            {
                memcpy_s(m_hevcState->vps, 16 * sizeof(HEVC_VPS), pSlice->hevcSlice->vps, 16 * sizeof(HEVC_VPS));
                m_hevcState->last_parsed_vps_id = pSlice->hevcSlice->last_parsed_vps_id;
                m_bVPSReady = 1;
            }

            if (GenStreamParam.nalType == GTS_HEVC_NALU_SEQ_PARAM)
            {
                //memcpy_s(m_hevcState->vps, 16 * sizeof(HEVC_VPS), pSlice->hevcSlice->vps, 16 * sizeof(HEVC_VPS));
                //m_hevcState->last_parsed_vps_id = pSlice->hevcSlice->last_parsed_vps_id;
                memcpy_s(m_hevcState->sps, 16 * sizeof(HEVC_SPS), pSlice->hevcSlice->sps, 16 * sizeof(HEVC_SPS));
                m_hevcState->last_parsed_sps_id = pSlice->hevcSlice->last_parsed_sps_id;
                m_bSPSReady = 1;
            }
            if (GenStreamParam.nalType == GTS_HEVC_NALU_PIC_PARAM)
            {
                memcpy_s(m_hevcState->pps, 64 * sizeof(HEVC_SPS), pSlice->hevcSlice->pps, 64 * sizeof(HEVC_PPS));
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
            memcpy_s(m_specialInfo[streamIdx], m_specialDataLen[streamIdx], TiledBitstream.pTiledBitstreamBuffer, m_specialDataLen[streamIdx]);
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

int32_t TstitchStream::ConvertTilesIdx(uint16_t tilesNum)
 {
     if (!m_pOutTile)
         return ERROR_NULL_PTR;
     TileDef* pSelectedTile = m_pOutTile;

     for (uint16_t idx = 0; idx < tilesNum; idx++)
     {
         for (uint8_t regIdx = 0; regIdx < (m_tileWidthCountOri[0] * m_tileHeightCountOri[0]); regIdx++)
         {
             ITileInfo* tileInfo = &(m_tilesInfo[regIdx]);
             if (((int32_t)(tileInfo->horzPos) == pSelectedTile->x) &&
                 ((int32_t)(tileInfo->vertPos) == pSelectedTile->y) &&
                 (tileInfo->faceId == pSelectedTile->faceId))
             {
                 pSelectedTile->idx = regIdx;
                 break;
             }
         }
         pSelectedTile++;
     }

     return ERROR_NONE;
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

    tile_merge_reset(m_pMergeStream);

    if (m_specialDataLen[0] == 0)
    {
        m_mergeStreamParam.bWroteHeader = 0;
    }

    if (m_pViewportParam.m_input_geoType == E_SVIDEO_CUBEMAP)
        ConvertTilesIdx(m_tileHeightCountSel[0] * m_tileWidthCountSel[0]);
    for (int32_t i = 0; i < m_tileHeightCountSel[0]; i++)
    {
        for (int32_t j = 0; j < m_tileWidthCountSel[0]; j++)
        {
            pTmpHigh[idx]->tilesHeightCount = 1;
            pTmpHigh[idx]->tilesWidthCount = 1;

            pTmpHigh[idx]->inputBufferLen = (pTmpTile->idx!=0) ? m_pNalInfo[0][pTmpTile->idx].nalLen : m_pNalInfo[0][pTmpTile->idx].nalLen- m_specialDataLen[0];
            pTmpHigh[idx]->pTiledBitstreamBuffer = (pTmpTile->idx != 0) ? m_pNalInfo[0][pTmpTile->idx].pNalStream : m_pNalInfo[0][pTmpTile->idx].pNalStream + m_specialDataLen[0];
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
            idx++;
        }
    }
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

    ret = genViewport_postprocess(&m_pViewportParam, m_pViewport);
    if (ret)
    {
        SCVP_LOG(LOG_ERROR, "gen viewport process error!\n");
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
    else if (m_pViewportParam.m_input_geoType == SVIDEO_CUBEMAP)
    {
        int32_t widthViewport = 0;
        int32_t heightViewport = 0;
        for (int32_t i = 0; i < m_pViewportParam.m_numFaces; i++)
        {
            widthViewport += (m_pViewportParam.m_pDownRight[i].x - m_pViewportParam.m_pUpLeft[i].x);
            heightViewport = (m_pViewportParam.m_pDownRight[i].y - m_pViewportParam.m_pUpLeft[i].y);
        }
        m_tileWidthCountSel[0] = m_pViewportParam.m_viewportDestWidth / (m_pViewportParam.m_iInputWidth / (m_pViewportParam.m_tileNumCol / m_pViewportParam.m_paramVideoFP.cols));
        m_tileHeightCountSel[0] = m_pViewportParam.m_viewportDestHeight / (m_pViewportParam.m_iInputHeight / (m_pViewportParam.m_tileNumRow / m_pViewportParam.m_paramVideoFP.rows));

        m_dstWidthNet = widthViewport;
        m_dstHeightNet = heightViewport;
    }
    return ret;
}

TileDef* TstitchStream::getSelectedTile()
{
    return m_pOutTile;
}

int32_t TstitchStream::getTilesByLegacyWay(TileDef* pOutTile)
{
    if (!m_pViewport) {
        LOG(WARNING) << "Viewport is not allocated";
        return -1;
    }
    int32_t ret = 0;

    ret = genViewport_process(&m_pViewportParam, m_pViewport);
    if (ret == -1) {
        LOG(WARNING) << "Error returned when using the traditional way to calculate tiles!";
        return ret;
    }
    ret = genViewport_getTilesInViewportByLegacyWay(m_pViewport, pOutTile);
    return ret;
}

int32_t TstitchStream::setViewPort(HeadPose *pose)
{
    int32_t ret = 0;
    if (m_pTileSelection) {
        ret = m_pTileSelection->SetViewportInfo(pose);
        return ret;
    }
    else if (m_bNeedPlugin)
        return SCVP_ERROR_PLUGIN_NOEXIST;
    else
        return genViewport_setViewPort(m_pViewport, pose->yaw, pose->pitch);
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
    memcpy_s(pParamStitchStream->pOutputBitstream, m_mergeStreamParam.outputiledbistreamlen, m_mergeStreamParam.pOutputBitstream, m_mergeStreamParam.outputiledbistreamlen);

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
    if ((m_pViewportParam.m_input_geoType != E_SVIDEO_EQUIRECT) && (m_pViewportParam.m_input_geoType != E_SVIDEO_CUBEMAP)) {
        SCVP_LOG(LOG_ERROR, "The input media projection type is not supported!!\n");
        return ERROR_INVALID;
    }

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
        rwpk->guardBandFlag = false;
        if (regionIdx < highTilesNum)
        {
            if (m_pViewportParam.m_input_geoType == E_SVIDEO_EQUIRECT)
                rwpk->transformType = 0;
            else if (m_pViewportParam.m_input_geoType == E_SVIDEO_CUBEMAP)
                rwpk->transformType = m_mapFaceInfo[m_tilesInfo[pSelectTiles->idx].faceId].transformType;
            rwpk->projRegWidth = highRes_tile_width;
            rwpk->projRegHeight = highRes_tile_height;
            rwpk->projRegTop = (pSelectTiles->idx / m_tileWidthCountOri[0]) * highRes_tile_height;
            rwpk->projRegLeft = (pSelectTiles->idx % m_tileWidthCountOri[0]) * highRes_tile_width;

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
            if (m_pViewportParam.m_input_geoType == E_SVIDEO_EQUIRECT)
                rwpk->transformType = 0;
            else if (m_pViewportParam.m_input_geoType == E_SVIDEO_CUBEMAP)
                rwpk->transformType = m_mapFaceInfo[m_tilesInfo[lowIdx + m_tileWidthCountOri[0]* m_tileHeightCountOri[0]].faceId].transformType;

            rwpk->packedRegWidth = lowRes_tile_width;
            rwpk->packedRegHeight = lowRes_tile_height;
            rwpk->projRegWidth  = lowRes_tile_width * m_mergeStreamParam.highRes.width / m_mergeStreamParam.lowRes.width;
            rwpk->projRegHeight = lowRes_tile_height * m_mergeStreamParam.highRes.height / m_mergeStreamParam.lowRes.height;
            rwpk->projRegTop  = (lowIdx / m_tileWidthCountOri[1] * rwpk->projRegHeight);
            rwpk->projRegLeft = (lowIdx % m_tileWidthCountOri[1] * rwpk->projRegWidth);
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

    if (m_pTileSelection) {
        ret = m_pTileSelection->GetTilesInViewport(pOutTile);
        return ret;
    }
    else if (m_bNeedPlugin)
        return SCVP_ERROR_PLUGIN_NOEXIST;
    else
    {
        ret = genViewport_getTilesInViewport(m_pViewport, pOutTile);
        m_viewportDestWidth = m_pViewportParam.m_viewportDestWidth;
        m_viewportDestHeight = m_pViewportParam.m_viewportDestHeight;
    }
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
    memset_s(&specialInfo, sizeof(hevc_specialInfo), 0);
    specialInfo.ptr = pBufferSliceCur;
    specialInfo.ptr_size = lenSlice;

    //set tile info as original one to make sure
    //following slices are decoded correctly
    hevc->pps[hevc->last_parsed_pps_id].tiles_enabled_flag
        = hevc->pps[hevc->last_parsed_pps_id].org_tiles_enabled_flag;

    memset_s(nalsize, sizeof(nalsize), 0);
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
    memcpy_s(pBitstreamCur, nalsize[SLICE_DATA], pBufferSliceCur + specialLen, nalsize[SLICE_DATA]);
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
    memcpy_s(pRWPK, sizeof(RegionWisePacking),  &m_dstRwpk, sizeof(RegionWisePacking));
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
        memset_s(&specialInfo, sizeof(hevc_specialInfo), 0);
        specialInfo.ptr = pParamStitchStream->pInputBitstream;
        specialInfo.ptr_size = pParamStitchStream->inputBitstreamLen;
        uint32_t nalsize[20];
        memset_s(nalsize, sizeof(nalsize), 0);
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
        memcpy_s(&hevcTmp, sizeof(HEVCState), m_hevcState, sizeof(HEVCState));
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
        memset_s(&specialInfo, sizeof(hevc_specialInfo), 0);
        specialInfo.ptr = pParamStitchStream->pInputBitstream;
        specialInfo.ptr_size = pParamStitchStream->inputBitstreamLen;
        uint32_t nalsize[20];
        memset_s(nalsize, sizeof(nalsize), 0);
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
        memcpy_s(&hevcTmp, sizeof(HEVCState), m_hevcState, sizeof(HEVCState));
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
    HEVCState *hevcTmp;
    uint32_t origWidth, origHeight;
    bool origFirstSliceFlag;
    uint32_t origSliceSegAddr;
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
        memset_s(&specialInfo, sizeof(hevc_specialInfo), 0);
        specialInfo.ptr = pParam360SCVP->pInputBitstream;
        specialInfo.ptr_size = pParam360SCVP->inputBitstreamLen;
        memset_s(nalsize, sizeof(nalsize), 0);
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
        origWidth = m_hevcState->sps[0].width;
        origHeight = m_hevcState->sps[0].height;
        origFirstSliceFlag = m_hevcState->s_info.first_slice_segment_in_pic_flag;
        origSliceSegAddr = m_hevcState->s_info.slice_segment_address;

        hevcTmp = m_hevcState;
        HEVC_SPS *sps = &(hevcTmp->sps[0]);
        sps->width = pParam360SCVP->destWidth;
        sps->height = pParam360SCVP->destHeight;

        HEVCSliceInfo *si = &(hevcTmp->s_info);
        if (!si)
            return -1;
        si->first_slice_segment_in_pic_flag = 1;
        if (newSliceAddr)
            si->first_slice_segment_in_pic_flag = 0;
        si->slice_segment_address = newSliceAddr;

        // write the new sliceheader
        hevc_write_slice_header(bsWrite, hevcTmp);
        m_hevcState->sps[0].width = origWidth;
        m_hevcState->sps[0].height = origHeight;
        m_hevcState->s_info.first_slice_segment_in_pic_flag = origFirstSliceFlag;
        m_hevcState->s_info.slice_segment_address = origSliceSegAddr;

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
