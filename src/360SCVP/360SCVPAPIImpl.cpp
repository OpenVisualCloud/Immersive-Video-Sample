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
#include "360SCVPAPI.h"
#include "360SCVPCommonDef.h"
#include "360SCVPHevcEncHdr.h"
#include "360SCVPLog.h"
#include "360SCVPImpl.h"

void* I360SCVP_Init(param_360SCVP* pParam360SCVP)
{
    if (pParam360SCVP == NULL)
        return NULL;
    TstitchStream*  pStitchstream = new TstitchStream;

    if(pStitchstream->init(pParam360SCVP) < 0)
    {
        SAFE_DELETE(pStitchstream);
        return NULL;
    }
    return (void*)pStitchstream;
}

void* I360SCVP_New(void* p360SCVPHandle)
{
    if (p360SCVPHandle == NULL)
        return NULL;

    TstitchStream* newHandle = new TstitchStream(*((TstitchStream*)(p360SCVPHandle)));
    if (newHandle == NULL)
        return NULL;

    return (void*)newHandle;
}

int32_t   I360SCVP_process(param_360SCVP* pParam360SCVP, void* p360SCVPHandle)
{
    TstitchStream* pStitch = (TstitchStream*)(p360SCVPHandle);
    if (!pStitch || !pParam360SCVP)
        return -1;
    int32_t ret = 0;
    if (pParam360SCVP->usedType == E_STREAM_STITCH_ONLY)
    {
        ret = pStitch->doStreamStitch(pParam360SCVP);
    }
    else if (pParam360SCVP->usedType == E_MERGE_AND_VIEWPORT)
    {
        //according to the FOV information, get the tiles in the viewport area
        pStitch->parseNals(pParam360SCVP, E_MERGE_AND_VIEWPORT, NULL, 0);
        pStitch->parseNals(pParam360SCVP, E_MERGE_AND_VIEWPORT, NULL, 1);
        // feed the parameters to the stitch lib
        ret = pStitch->feedParamToGenStream(pParam360SCVP);

        //do the stitch process
        ret = pStitch->doMerge(pParam360SCVP);
    }
    else if (pParam360SCVP->usedType == E_PARSER_ONENAL)
    {
        //according to the FOV information, get the tiles in the viewport area
        int32_t numTiles = pStitch->getViewPortTiles();
        if (numTiles < 0)
            ret = - 1;
    }
    return ret;
}

int32_t I360SCVP_setViewPort(void* p360SCVPHandle, float yaw, float pitch)
{
    int32_t ret = 0;
    HeadPose pose;
    TstitchStream* pStitch = (TstitchStream*)(p360SCVPHandle);
    if (!pStitch)
        return 1;
    pose.yaw = yaw;
    pose.pitch = pitch;
    ret = pStitch->setViewPort(&pose);

    pStitch->getViewPortTiles();

    return ret;
}

int32_t I360SCVP_setViewPortEx(void* p360SCVPHandle, HeadPose* pose)
{
    int32_t ret = 0;
    TstitchStream* pStitch = (TstitchStream*)(p360SCVPHandle);
    if (!pStitch)
        return 1;
    ret = pStitch->setViewPort(pose);

    pStitch->getViewPortTiles();

    return ret;
}

int32_t   I360SCVP_unInit(void* p360SCVPHandle)
{
    int32_t ret = 0;
    TstitchStream* pStitch = (TstitchStream*)(p360SCVPHandle);
    if (!pStitch)
        return -1;
    ret = pStitch->uninit();
    SAFE_DELETE(pStitch);
    return ret;
}

int32_t I360SCVP_getFixedNumTiles(TileDef* pOutTile, Param_ViewportOutput* pParamViewPortOutput, void* p360SCVPHandle)
{
    int32_t ret = 0;
    TstitchStream* pStitch = (TstitchStream*)(p360SCVPHandle);
    if (!pStitch || !pOutTile || !pParamViewPortOutput)
        return 1;
    ret = pStitch->getFixedNumTiles(pOutTile);
    pParamViewPortOutput->dstWidthAlignTile = pStitch->m_viewportDestWidth;
    pParamViewPortOutput->dstHeightAlignTile = pStitch->m_viewportDestHeight;
    pParamViewPortOutput->dstWidthNet = pStitch->m_dstWidthNet;
    pParamViewPortOutput->dstHeightNet = pStitch->m_dstHeightNet;
    pParamViewPortOutput->xTopLeftNet = pStitch->m_xTopLeftNet;
    pParamViewPortOutput->yTopLeftNet = pStitch->m_yTopLeftNet;
    return ret;
}

int32_t I360SCVP_getTilesInViewport(TileDef* pOutTile, Param_ViewportOutput* pParamViewPortOutput, void* p360SCVPHandle)
{
    int32_t ret = 0;
    TstitchStream* pStitch = (TstitchStream*)(p360SCVPHandle);
    if (!pStitch || !pOutTile || !pParamViewPortOutput)
        return 1;
    ret = pStitch->getTilesInViewport(pOutTile);
    pParamViewPortOutput->dstWidthAlignTile = pStitch->m_viewportDestWidth;
    pParamViewPortOutput->dstHeightAlignTile = pStitch->m_viewportDestHeight;
    pParamViewPortOutput->dstWidthNet = pStitch->m_dstWidthNet;
    pParamViewPortOutput->dstHeightNet = pStitch->m_dstHeightNet;
    pParamViewPortOutput->xTopLeftNet = pStitch->m_xTopLeftNet;
    pParamViewPortOutput->yTopLeftNet = pStitch->m_yTopLeftNet;
    return ret;
}

int32_t I360SCVP_ParseNAL(Nalu* pNALU, void* p360SCVPHandle)
{
    TstitchStream* pStitch = (TstitchStream*)(p360SCVPHandle);
    if (!pStitch || !pNALU)
        return 1;

    int32_t parseType = pStitch->m_usedType;//E_PARSER_ONENAL;
    if (pStitch->parseNals(NULL, parseType, pNALU, 0) < 0)
        return 1;

    pNALU->naluType = pStitch->m_nalType;
    pNALU->sliceHeaderLen = pStitch->m_sliceHeaderLen;
    pNALU->seiPayloadType = pStitch->m_seiPayloadType;
    pNALU->startCodesSize = pStitch->m_startCodesSize;
    pNALU->dataSize = pStitch->m_dataSize + pStitch->m_startCodesSize;

    return 0;
}

int32_t I360SCVP_GenerateSPS(param_360SCVP* pParam360SCVP, void* p360SCVPHandle)
{
    int32_t ret = 0;
    TstitchStream* pStitch = (TstitchStream*)(p360SCVPHandle);
    if (!pStitch || !pParam360SCVP)
        return 1;

    ret = pStitch->GenerateSPS(pParam360SCVP);

    return ret;
}

int32_t I360SCVP_GeneratePPS(param_360SCVP* pParam360SCVP, TileArrangement* pTileArrange, void* p360SCVPHandle)
{
    int32_t ret = 0;
    TstitchStream* pStitch = (TstitchStream*)(p360SCVPHandle);
    if (!pStitch || !pParam360SCVP || !pTileArrange)
        return 1;

    ret = pStitch->GeneratePPS(pParam360SCVP, pTileArrange);

    return ret;
}

int32_t I360SCVP_GenerateSliceHdr(param_360SCVP* pParam360SCVP, int32_t newSliceAddr, void* p360SCVPHandle)
{
    int32_t ret = 0;
    TstitchStream* pStitch = (TstitchStream*)(p360SCVPHandle);
    if (!pStitch || !pParam360SCVP)
        return 1;
    ret = pStitch->GenerateSliceHdr(pParam360SCVP, newSliceAddr);
    return ret;
}

int32_t I360SCVP_GenerateRWPK(void* p360SCVPHandle, RegionWisePacking* pRWPK, uint8_t *pRWPKBits, int32_t* pRWPKBitsSize)
{
    int32_t ret = 0;
    TstitchStream* pStitch = (TstitchStream*)(p360SCVPHandle);
    if (!pStitch || !pRWPK || !pRWPKBits ||!pRWPKBitsSize)
        return 1;
    ret = pStitch->GenerateRWPK(pRWPK, pRWPKBits, pRWPKBitsSize);
    return ret;
}

int32_t I360SCVP_GenerateProj(void* p360SCVPHandle, int32_t projType, uint8_t *pProjBits, int32_t* pProjBitsSize)
{
    int32_t ret = 0;
    TstitchStream* pStitch = (TstitchStream*)(p360SCVPHandle);
    if (!pStitch || !pProjBits)
        return 1;
    ret = pStitch->GenerateProj(projType, pProjBits, pProjBitsSize);
    return ret;
}

int32_t I360SCVP_ParseRWPK(void* p360SCVPHandle, RegionWisePacking* pRWPK, uint8_t *pRWPKBits, uint32_t RWPKBitsSize)
{
    int32_t ret = 0;
    TstitchStream* pStitch = (TstitchStream*)(p360SCVPHandle);
    if (!pStitch)
        return 1;
    ret = pStitch->DecRWPKSEI(pRWPK, pRWPKBits, RWPKBitsSize);
    return ret;
}

// output is the centre and range of Azimuth and Elevation of 3D sphere
int32_t I360SCVP_getContentCoverage(void* p360SCVPHandle, CCDef* pOutCC)
{
    int32_t ret = 0;
    TstitchStream* pStitch = (TstitchStream*)(p360SCVPHandle);
    if (!pStitch)
        return -1;

    if (!pOutCC)
        return -1;

    ret = pStitch->getContentCoverage(pOutCC);
        return ret;
}

int32_t I360SCVP_GetParameter(void* p360SCVPHandle, int32_t paramID, void** pValue)
{
    int32_t ret = 0;
    TstitchStream* pStitch = (TstitchStream*)(p360SCVPHandle);
    if (!pStitch || !pValue)
        return 1;
    Param_PicInfo* pPicInfo = NULL;
    Param_BSHeader* bsHeader = NULL;
    RegionWisePacking* pRWPK = NULL;
    switch (paramID)
    {
        case ID_SCVP_PARAM_PICINFO:
            pPicInfo = (Param_PicInfo*)*pValue;

            ret = pStitch->getPicInfo(pPicInfo);

            break;
        case ID_SCVP_BITSTREAMS_HEADER:
            bsHeader = (Param_BSHeader*)*pValue;

            ret = pStitch->getBSHeader(bsHeader);
            break;
        case ID_SCVP_RWPK_INFO:
            pRWPK = (RegionWisePacking *)*pValue;
            ret = pStitch->getRWPKInfo(pRWPK);
            break;
        default:
            break;
    }
    return ret;
}

int32_t I360SCVP_SetParameter(void* p360SCVPHandle, int32_t paramID, void* pValue)
{
    int32_t ret = 0;
    TstitchStream* pStitch = (TstitchStream*)(p360SCVPHandle);
    if (!pStitch || !pValue)
        return 1;
    Param_ViewPortInfo* pViewPortInfo = NULL;
    RegionWisePacking*  pRWPK = NULL;
    SphereRotation*     pSphereRot = NULL;
    FramePacking*       pFramePacking = NULL;
    OMNIViewPort*       pSeiViewport;
    int32_t             projType = 0;
    switch (paramID)
    {
    case ID_SCVP_PARAM_VIEWPORT:
        pViewPortInfo = (Param_ViewPortInfo*)pValue;
        ret = pStitch->setViewPortInfo(pViewPortInfo);
        break;
    case ID_SCVP_PARAM_SEI_PROJECTION:
        projType = *((int32_t*)pValue);
        ret = pStitch->setSEIProjInfo(projType);
        break;
    case ID_SCVP_PARAM_SEI_RWPK:
        pRWPK = (RegionWisePacking*)pValue;
        ret = pStitch->setSEIRWPKInfo(pRWPK);
        break;
    case ID_SCVP_PARAM_SEI_ROTATION:
        pSphereRot = (SphereRotation*)pValue;
        ret = pStitch->setSphereRot(pSphereRot);
        break;
    case ID_SCVP_PARAM_SEI_FRAMEPACKING:
        pFramePacking = (FramePacking*)pValue;
        ret = pStitch->setFramePacking(pFramePacking);
        break;
    case ID_SCVP_PARAM_SEI_VIEWPORT:
        pSeiViewport = (OMNIViewPort*)pValue;
        ret = pStitch->setViewportSEI(pSeiViewport);
        break;
    default:
        break;
    }
    return ret;
}

int32_t I360SCVP_GetTilesByLegacyWay(TileDef* pOutTile, void* p360SCVPHandle)
{
    int32_t ret = 0;
    TstitchStream* pStitch = (TstitchStream*)(p360SCVPHandle);
    if (!pStitch)
        return -1;
    ret = pStitch->getTilesByLegacyWay(pOutTile);
    return ret;
}

int32_t I360SCVPSetLogCallBack(void* p360SCVPHandle, void* externalLog)
{
    TstitchStream* pStitch = (TstitchStream*)p360SCVPHandle;
    if (!pStitch)
        return 1;

    LogFunction logFunction = (LogFunction)externalLog;
    if (!logFunction)
        return OMAF_ERROR_NULL_PTR;

    int32_t ret = pStitch->SetLogCallBack(logFunction);
    if (ret)
        return ret;

    return ERROR_NONE;
}
