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

#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <cstring>
#include <string>
#include <limits>
#include <math.h>
#include <iomanip>
#include "360SCVPViewPort.h"
#include "360SCVPViewportImpl.h"
#include "360SCVPViewportAPI.h"
#ifdef WIN32
#define strdup _strdup
#endif

using namespace std;

void* genViewport_Init(generateViewPortParam* pParamGenViewport)
{
    if (!pParamGenViewport)
        return NULL;
    TgenViewport*  cTAppConvCfg = new TgenViewport;
    if (!cTAppConvCfg)
        return NULL;

    cTAppConvCfg->m_usageType = pParamGenViewport->m_usageType;
    memset(&cTAppConvCfg->m_sourceSVideoInfo, 0, sizeof(struct SVideoInfo));
    memset(&cTAppConvCfg->m_codingSVideoInfo, 0, sizeof(struct SVideoInfo));
    cTAppConvCfg->m_iCodingFaceWidth = pParamGenViewport->m_iViewportWidth;
    cTAppConvCfg->m_iCodingFaceHeight = pParamGenViewport->m_iViewportHeight;
    cTAppConvCfg->m_codingSVideoInfo.viewPort.fPitch = pParamGenViewport->m_viewPort_fPitch;
    cTAppConvCfg->m_codingSVideoInfo.viewPort.fYaw = pParamGenViewport->m_viewPort_fYaw;
    cTAppConvCfg->m_codingSVideoInfo.viewPort.hFOV = pParamGenViewport->m_viewPort_hFOV;
    cTAppConvCfg->m_codingSVideoInfo.viewPort.vFOV = pParamGenViewport->m_viewPort_vFOV;
    cTAppConvCfg->m_codingSVideoInfo.geoType = pParamGenViewport->m_output_geoType;
    cTAppConvCfg->m_sourceSVideoInfo.geoType = pParamGenViewport->m_input_geoType;
    cTAppConvCfg->m_iInputWidth = pParamGenViewport->m_iInputWidth;
    cTAppConvCfg->m_iInputHeight = pParamGenViewport->m_iInputHeight;
    if (cTAppConvCfg->create(pParamGenViewport->m_tileNumRow, pParamGenViewport->m_tileNumCol) < 0)
    {
        delete cTAppConvCfg;
        cTAppConvCfg = NULL;
        return NULL;
    }

    //calculate the max tile num if the source project is cube map
    cTAppConvCfg->m_maxTileNum = 0;
    if (pParamGenViewport->m_tileNumCol == 0 || pParamGenViewport->m_tileNumRow == 0)
        return NULL;
    if (cTAppConvCfg->m_sourceSVideoInfo.geoType == SVIDEO_CUBEMAP)
    {
        int32_t tilewidth = cTAppConvCfg->m_iInputWidth / pParamGenViewport->m_tileNumCol;
        int32_t tileheight = cTAppConvCfg->m_iInputHeight / pParamGenViewport->m_tileNumRow;
        uint32_t tiles_num_row = 0;
        uint32_t tiles_num_col = 0;
        uint32_t tiles_num = 0;
        int32_t idx = 0;
        float fovh = MAX_FOV_ANGLE;
        for (int32_t j = 0; j < FOV_Angle_NUM; j++)
        {
            if (cTAppConvCfg->m_codingSVideoInfo.viewPort.hFOV <= fovh && cTAppConvCfg->m_codingSVideoInfo.viewPort.hFOV > fovh-10)
            {
                idx = j;
                break;
            }
            fovh -= 10;
        }
        if (tilewidth == 0 || tileheight == 0)
            return NULL;
        for (int32_t i = 0; i < 4; i++)
        {
            tiles_num_row = (Max_Viewport_Size[idx][i].x / tilewidth + ((Max_Viewport_Size[idx][i].x !=0) ? 2 : 0));
            tiles_num_col = (Max_Viewport_Size[idx][i].y / tileheight + ((Max_Viewport_Size[idx][i].y!=0) ? 2 : 0));
            if (tiles_num_row > pParamGenViewport->m_tileNumRow)
                tiles_num_row = pParamGenViewport->m_tileNumRow;
            if (tiles_num_col > pParamGenViewport->m_tileNumCol)
                tiles_num_col = pParamGenViewport->m_tileNumCol;
            tiles_num += tiles_num_row * tiles_num_col;
        }
        cTAppConvCfg->m_maxTileNum = tiles_num;
        pParamGenViewport->m_viewportDestWidth = tiles_num_row * cTAppConvCfg->m_srd[0].tilewidth;
        pParamGenViewport->m_viewportDestHeight = tiles_num_row * cTAppConvCfg->m_srd[0].tileheight;
    }
    else if (cTAppConvCfg->m_sourceSVideoInfo.geoType == SVIDEO_EQUIRECT)
    {
        genViewport_setViewPort((void*)cTAppConvCfg, 0, 0);
        genViewport_process(pParamGenViewport, (void*)cTAppConvCfg);

        cTAppConvCfg->m_codingSVideoInfo.viewPort.fPitch = pParamGenViewport->m_viewPort_fPitch;
        cTAppConvCfg->m_codingSVideoInfo.viewPort.fYaw = pParamGenViewport->m_viewPort_fYaw;
        uint32_t maxTileNumCol = 0;
        uint32_t maxTileNumRow = 0;
        uint32_t maxTileNum = 0;
        int32_t viewPortWidth = 0;
        int32_t viewPortHeight = 0;
        int32_t viewPortHeightmax = 0;
        SPos *pUpLeft = cTAppConvCfg->m_pUpLeft;
        SPos *pDownRight = cTAppConvCfg->m_pDownRight;
        for (int32_t i = 0; i < 2; i++) // the max count is 2
        {
            if (pDownRight->faceIdx == -1)
            {
                pUpLeft++;
                pDownRight++;
                continue;
            }
            viewPortWidth += int32_t(pDownRight->x - pUpLeft->x);
            viewPortHeight = int32_t(pDownRight->y - pUpLeft->y);
            if (viewPortHeightmax < viewPortHeight)
                viewPortHeightmax = viewPortHeight;
            printf("viewPortWidthMax = %d viewPortHeightMax = %d\n", viewPortWidth, viewPortHeightmax);

            pUpLeft++;
            pDownRight++;
        }

        if (pParamGenViewport->m_usageType == E_PARSER_ONENAL)
        {
            viewPortWidth = floor((float)(viewPortWidth) / (float)(cTAppConvCfg->m_srd[0].tilewidth) + 0.499) * cTAppConvCfg->m_srd[0].tilewidth;
            viewPortHeightmax = floor((float)(viewPortHeightmax) / (float)(cTAppConvCfg->m_srd[0].tileheight) + 0.499) * cTAppConvCfg->m_srd[0].tileheight;
            printf("viewPortWidthMax = %d viewPortHeightMax = %d, tile_width %d, tile_height %d\n", viewPortWidth, viewPortHeightmax, cTAppConvCfg->m_srd[0].tilewidth, cTAppConvCfg->m_srd[0].tileheight);

            maxTileNumCol = (viewPortWidth / cTAppConvCfg->m_srd[0].tilewidth + 1);
            if (maxTileNumCol > cTAppConvCfg->m_tileNumCol)
                maxTileNumCol = cTAppConvCfg->m_tileNumCol;

            maxTileNumRow = (viewPortHeightmax / cTAppConvCfg->m_srd[0].tileheight + 1);
            if (maxTileNumRow > cTAppConvCfg->m_tileNumRow)
                maxTileNumRow = cTAppConvCfg->m_tileNumRow;
        }
        else
        {
            maxTileNumCol = (viewPortWidth / cTAppConvCfg->m_srd[0].tilewidth + 2);
            if (maxTileNumCol > cTAppConvCfg->m_tileNumCol)
                maxTileNumCol = cTAppConvCfg->m_tileNumCol;
            maxTileNumRow = (viewPortHeightmax / cTAppConvCfg->m_srd[0].tileheight + 2);
            if (maxTileNumRow > cTAppConvCfg->m_tileNumRow)
                maxTileNumRow = cTAppConvCfg->m_tileNumRow;
        }

        maxTileNum = maxTileNumCol * maxTileNumRow;
        cTAppConvCfg->m_maxTileNum = maxTileNum;
        pParamGenViewport->m_viewportDestWidth = maxTileNumCol * cTAppConvCfg->m_srd[0].tilewidth;
        pParamGenViewport->m_viewportDestHeight = maxTileNumRow * cTAppConvCfg->m_srd[0].tileheight;
    }
    return (void*)cTAppConvCfg;
}

int32_t   genViewport_process(generateViewPortParam* pParamGenViewport, void* pGenHandle)
{
    TgenViewport* cTAppConvCfg = (TgenViewport*)(pGenHandle);
    if (!cTAppConvCfg || !pParamGenViewport)
        return -1;
    if (cTAppConvCfg->parseCfg() < 0)
        return -1;

    if (cTAppConvCfg->convert() < 0)
        return -1;

    pParamGenViewport->m_numFaces = cTAppConvCfg->m_numFaces;

    point* pTmpUpleftDst = pParamGenViewport->m_pUpLeft;
    point* pTmpDownRightDst = pParamGenViewport->m_pDownRight;
    SPos * pTmpUpleftSrc = cTAppConvCfg->m_pUpLeft;
    SPos * pTmpDownRightSrc = cTAppConvCfg->m_pDownRight;

    for (int32_t i = 0; i < FACE_NUMBER; i++)
    {
        if (pTmpUpleftSrc->faceIdx >= 0)
        {
            pTmpUpleftDst->faceId = (int32_t)pTmpUpleftSrc->faceIdx;
            pTmpUpleftDst->x = (int32_t)pTmpUpleftSrc->x;
            pTmpUpleftDst->y = (int32_t)pTmpUpleftSrc->y;
            pTmpDownRightDst->faceId = (int32_t)pTmpDownRightSrc->faceIdx;
            pTmpDownRightDst->x = (int32_t)pTmpDownRightSrc->x;
            pTmpDownRightDst->y = (int32_t)pTmpDownRightSrc->y;
            pTmpUpleftDst++;
            pTmpDownRightDst++;
        }

        pTmpUpleftSrc++;
        pTmpDownRightSrc++;
    }

    return 0;

}


int32_t   genViewport_postprocess(generateViewPortParam* pParamGenViewport, void* pGenHandle)
{
    TgenViewport* cTAppConvCfg = (TgenViewport*)(pGenHandle);
    if (!cTAppConvCfg || !pParamGenViewport)
        return -1;

    cTAppConvCfg->selectregion(pParamGenViewport->m_iInputWidth, pParamGenViewport->m_iInputHeight, pParamGenViewport->m_viewportDestWidth, pParamGenViewport->m_viewportDestHeight);

    pParamGenViewport->m_numFaces = cTAppConvCfg->m_numFaces;
    point* pTmpUpleftDst = pParamGenViewport->m_pUpLeft;
    point* pTmpDownRightDst = pParamGenViewport->m_pDownRight;
    SPos * pTmpUpleftSrc = cTAppConvCfg->m_pUpLeft;
    SPos * pTmpDownRightSrc = cTAppConvCfg->m_pDownRight;

    for (int32_t i = 0; i < FACE_NUMBER; i++)
    {
        if (pTmpUpleftSrc->faceIdx >= 0)
        {
            pTmpUpleftDst->faceId = (int32_t)pTmpUpleftSrc->faceIdx;
            pTmpUpleftDst->x = (int32_t)pTmpUpleftSrc->x;
            pTmpUpleftDst->y = (int32_t)pTmpUpleftSrc->y;
            pTmpDownRightDst->faceId = (int32_t)pTmpDownRightSrc->faceIdx;
            pTmpDownRightDst->x = (int32_t)pTmpDownRightSrc->x;
            pTmpDownRightDst->y = (int32_t)pTmpDownRightSrc->y;
            pTmpUpleftDst++;
            pTmpDownRightDst++;
        }

        pTmpUpleftSrc++;
        pTmpDownRightSrc++;
    }
    return 0;
}


int32_t genViewport_setMaxSelTiles(void* pGenHandle, int32_t maxSelTiles)
{
    TgenViewport* cTAppConvCfg = (TgenViewport*)(pGenHandle);
    if (!cTAppConvCfg)
        return -1;
    cTAppConvCfg->m_maxTileNum = maxSelTiles;
    return 0;

}
int32_t genViewport_setViewPort(void* pGenHandle, float yaw, float pitch)
{
    TgenViewport* cTAppConvCfg = (TgenViewport*)(pGenHandle);
    if (!cTAppConvCfg)
        return -1;
    cTAppConvCfg->m_codingSVideoInfo.viewPort.fPitch = pitch;
    cTAppConvCfg->m_codingSVideoInfo.viewPort.fYaw = yaw;
    return 0;
}

bool genViewport_isInside(void* pGenHandle, int32_t x, int32_t y, int32_t width, int32_t height, int32_t faceId)
{
    bool ret = 0;
    TgenViewport* cTAppConvCfg = (TgenViewport*)(pGenHandle);
    if (!cTAppConvCfg)
        return ret;
    ret = cTAppConvCfg->isInside(x, y, width, height, faceId);

    return ret;
}

// output is the centre and range of Azimuth and Elevation of 3D sphere
int32_t genViewport_getContentCoverage(void* pGenHandle, CCDef* pOutCC)
{
    TgenViewport* cTAppConvCfg = (TgenViewport*)(pGenHandle);
    if (!cTAppConvCfg)
        return -1;

    // pOutCC should be allocated before this func called, or we new it here
    if(!pOutCC)
        return -1;

    SPos *pUpLeft = cTAppConvCfg->m_pUpLeft;
    SPos *pDownRight = cTAppConvCfg->m_pDownRight;

    int32_t videoWidth = cTAppConvCfg->m_iInputWidth;
    int32_t videoHeight = cTAppConvCfg->m_iInputHeight;

    int32_t faceNum = (cTAppConvCfg->m_sourceSVideoInfo.geoType==SVIDEO_CUBEMAP) ? 6 : 2;
    // ERP mode may have 2 faces for boundary case
    if(cTAppConvCfg->m_sourceSVideoInfo.geoType == SVIDEO_EQUIRECT)
    {
        int32_t w = 0, h = 0, x = 0, y = 0;
        bool coverBoundary = pUpLeft[1].faceIdx == 0 ? true : false;
        x = pUpLeft[0].x;
        y = pUpLeft[0].y;

        w = pDownRight[0].x - pUpLeft[0].x + coverBoundary * (pDownRight[1].x - pUpLeft[1].x);
        h = pDownRight[0].y - pUpLeft[0].y + coverBoundary * (pDownRight[1].y - pUpLeft[1].y);

        pOutCC->centreAzimuth   = (int32_t)((((videoWidth / 2) - (float)(x + w / 2)) * 360 * 65536) / videoWidth);
        pOutCC->centreElevation = (int32_t)((((videoHeight / 2) - (float)(y + h / 2)) * 180 * 65536) / videoHeight);
        pOutCC->azimuthRange    = (uint32_t)((w * 360.f * 65536) / videoWidth);
        pOutCC->elevationRange  = (uint32_t)((h * 180.f * 65536) / videoHeight);
    }
    else //if(cTAppConvCfg->m_sourceSVideoInfo.geoType == SVIDEO_CUBEMAP)
    {
        for (int32_t faceid = 0; faceid < faceNum; faceid++)
        {

            pUpLeft++;
            pDownRight++;
        }
    }

    return 0;
}

int32_t genViewport_getFixedNumTiles(void* pGenHandle, TileDef* pOutTile)
{
    TgenViewport* cTAppConvCfg = (TgenViewport*)(pGenHandle);
    if (!cTAppConvCfg || !pOutTile)
        return -1;

    int32_t tileNum = 0;
    int32_t maxTileNum = 0;
    int32_t additionalTilesNum = 0;

    tileNum = cTAppConvCfg->calcTilesInViewport(cTAppConvCfg->m_srd, cTAppConvCfg->m_tileNumCol, cTAppConvCfg->m_tileNumRow);

    maxTileNum = cTAppConvCfg->m_maxTileNum;

    //select the additional tiles randomly
    if (cTAppConvCfg->m_usageType == E_PARSER_ONENAL)
    {
        additionalTilesNum = 0;
    }
    else
    {
        additionalTilesNum = maxTileNum - tileNum;
    }
    printf("the max tile count = %d additionalTilesNum = %d\n", maxTileNum, additionalTilesNum);
    if (additionalTilesNum < 0)
        printf("there is an error in the judgement\n");
    int32_t pos = 0;
    for (int32_t i = 0; i < additionalTilesNum; i++)
    {
        for (uint32_t j = pos; j < cTAppConvCfg->m_tileNumCol*cTAppConvCfg->m_tileNumRow; j++)
        {
            if (cTAppConvCfg->m_srd[j].isOccupy == 0)
            {
                cTAppConvCfg->m_srd[j].isOccupy = 1;
                pos = j;
                break;
            }
        }
    }

    //set the occupy tile into the output parameter
    int32_t idx = 0;
    TileDef* pOutTileTmp = pOutTile;
    if (cTAppConvCfg->m_usageType == E_PARSER_ONENAL)
    {
        tileNum = maxTileNum;
    }
    else
    {
        tileNum = tileNum + additionalTilesNum;
    }
    for (uint32_t col = 0; col < cTAppConvCfg->m_tileNumCol; col++)
    {
        for (uint32_t row = 0; row < cTAppConvCfg->m_tileNumRow; row++)
        {
            if (cTAppConvCfg->m_srd[idx].isOccupy == 1)
            {
                pOutTileTmp->faceId = cTAppConvCfg->m_srd[idx].faceId;
                pOutTileTmp->x = cTAppConvCfg->m_srd[idx].x;
                pOutTileTmp->y = cTAppConvCfg->m_srd[idx].y;
                pOutTileTmp->idx = idx;
                pOutTileTmp++;
            }
            idx++;
        }
    }
    return tileNum;
}


int32_t genViewport_getViewportTiles(void* pGenHandle, TileDef* pOutTile)
{
    TgenViewport* cTAppConvCfg = (TgenViewport*)(pGenHandle);
    if (!cTAppConvCfg || !pOutTile)
        return -1;
    int32_t tileNum = 0;
    uint32_t maxTileNumCol = 0;
    uint32_t maxTileNumRow = 0;
    int32_t viewPortWidth = 0;
    int32_t viewPortHeight = 0;
    SPos *pUpLeft = cTAppConvCfg->m_pUpLeft;
    SPos *pDownRight = cTAppConvCfg->m_pDownRight;
    int32_t needRevert = -1;

    tileNum = cTAppConvCfg->calcTilesInViewport(cTAppConvCfg->m_srd, cTAppConvCfg->m_tileNumCol, cTAppConvCfg->m_tileNumRow);

    int32_t idx = 0;
    TileDef* pOutTileTmp = pOutTile;

    //for erp, count the max tiles
    if (cTAppConvCfg->m_sourceSVideoInfo.geoType == SVIDEO_EQUIRECT)
    {
        viewPortWidth = int32_t(pDownRight->x - pUpLeft->x);
        viewPortHeight = int32_t(pDownRight->y - pUpLeft->y);

        pUpLeft++;
        pDownRight++;
        if (pUpLeft->faceIdx != -1)
        {
            needRevert = 1;
        }

        if (needRevert > 0)
        {
            viewPortWidth += int32_t(pDownRight->x - pUpLeft->x);
            viewPortHeight = int32_t(pDownRight->y - pUpLeft->y);
        }
        pUpLeft--;
        pDownRight--;

        uint32_t colStartIdx = pUpLeft->x / cTAppConvCfg->m_srd[0].tilewidth;
        uint32_t colStartIdxOri = colStartIdx;
        uint32_t rowStartIdx = pUpLeft->y / cTAppConvCfg->m_srd[0].tileheight;
        maxTileNumCol = (viewPortWidth / cTAppConvCfg->m_srd[0].tilewidth + 2);
        maxTileNumRow = (viewPortHeight / cTAppConvCfg->m_srd[0].tileheight + 2);
        if (maxTileNumCol > cTAppConvCfg->m_tileNumCol)
            maxTileNumCol = cTAppConvCfg->m_tileNumCol;
        if (maxTileNumRow > cTAppConvCfg->m_tileNumRow)
            maxTileNumRow = cTAppConvCfg->m_tileNumRow;
        for (uint32_t rowIdx = 0; rowIdx < maxTileNumRow; rowIdx++)
        {
            for (uint32_t colIdx = 0; colIdx < maxTileNumCol; colIdx++)
            {
                idx = rowStartIdx * cTAppConvCfg->m_tileNumCol + colStartIdx;
                pOutTileTmp->faceId = cTAppConvCfg->m_srd[idx].faceId;
                pOutTileTmp->x = cTAppConvCfg->m_srd[idx].x;
                pOutTileTmp->y = cTAppConvCfg->m_srd[idx].y;
                pOutTileTmp->idx = idx;
                pOutTileTmp++;

                colStartIdx++;
                if (colStartIdx >= cTAppConvCfg->m_tileNumCol)
                {
                    colStartIdx = 0;
                }
            }
            rowStartIdx++;
            colStartIdx = colStartIdxOri;
            if (rowStartIdx >= cTAppConvCfg->m_tileNumCol)
            {
                rowStartIdx = 0;
            }
        }
    }
    tileNum = cTAppConvCfg->m_maxTileNum;
    return tileNum;
}

int32_t   genViewport_unInit(void* pGenHandle)
{
    TgenViewport* cTAppConvCfg = (TgenViewport*)(pGenHandle);
    if (!cTAppConvCfg)
        return 1;

    cTAppConvCfg->destroy();
    if(pGenHandle)
    {
        free(pGenHandle);
        pGenHandle = NULL;
    }

    return 0;
}


TgenViewport::TgenViewport()
{
    m_faceSizeAlignment = 8;
    m_pUpLeft = new SPos[FACE_NUMBER];//
    m_pDownRight = new SPos[FACE_NUMBER];//
    memset(&m_codingSVideoInfo, 0, sizeof(SVideoInfo));
    memset(&m_sourceSVideoInfo, 0, sizeof(SVideoInfo));
    m_iCodingFaceWidth = 0;
    m_iCodingFaceHeight = 0;
    m_iSourceWidth = 0;
    m_iSourceHeight = 0;
    m_tileNumCol = 0;
    m_tileNumRow = 0;
    m_iFrameRate = 0;
    m_iInputWidth = 0;
    m_iInputHeight = 0;
    m_maxTileNum = 0;
    m_usageType = E_STREAM_STITCH_ONLY;
    m_numFaces = 0;
    m_srd = new ITileInfo;
}

TgenViewport::~TgenViewport()
{
    if(m_pUpLeft)
    {
        delete[] m_pUpLeft;
        m_pUpLeft = NULL;
    }
    if(m_pDownRight)
    {
        delete[] m_pDownRight;
        m_pDownRight = NULL;
    }
    if(m_srd)
    {
        delete m_srd;
        m_srd = NULL;
    }
}

TgenViewport& TgenViewport::operator=(const TgenViewport& src)
{
    if (&src == this)
        return *this;
    // memcpy(m_faceSizeAlignment, src.m_faceSizeAlignment, sizeof(int32_t));
    this->m_faceSizeAlignment = src.m_faceSizeAlignment;
    memcpy(this->m_pUpLeft, src.m_pUpLeft, sizeof(SPos));
    memcpy(this->m_pDownRight, src.m_pDownRight, sizeof(SPos));
    this->m_codingSVideoInfo = src.m_codingSVideoInfo;
    this->m_sourceSVideoInfo = src.m_sourceSVideoInfo;
    this->m_iCodingFaceWidth = src.m_iCodingFaceHeight;
    this->m_iCodingFaceHeight = src.m_iCodingFaceHeight;
    this->m_iSourceWidth = src.m_iSourceWidth;
    this->m_iSourceHeight = src.m_iSourceHeight;
    this->m_tileNumCol = src.m_tileNumCol;
    this->m_tileNumRow = src.m_tileNumRow;
    this->m_iFrameRate = src.m_iFrameRate;
    this->m_iInputWidth = src.m_iInputWidth;
    this->m_iInputHeight = src.m_iInputHeight;
    this->m_usageType = src.m_usageType;
    memcpy(this->m_srd, src.m_srd, sizeof(ITileInfo));
    return *this;
}

int32_t TgenViewport::create(uint32_t tileNumRow, uint32_t tileNumCol)
{
    m_tileNumRow = tileNumRow;
    m_tileNumCol = tileNumCol;
    m_srd = new ITileInfo[FACE_NUMBER*m_tileNumRow*m_tileNumCol];
    if (m_srd)
        return 0;
    else
        return -1;
}

void TgenViewport::destroy()
{
    if(m_srd)
        delete[] m_srd;
    m_srd = NULL;
}


// ====================================================================================================================
// Public member functions
// ====================================================================================================================

int32_t TgenViewport::parseCfg(  )
{
    m_iFrameRate = 30;
    m_faceSizeAlignment = 1;
    m_iSourceWidth = m_iInputWidth;
    m_iSourceHeight = m_iInputHeight;
    m_sourceSVideoInfo.iNumFaces = 1;
    m_sourceSVideoInfo.iFaceWidth = m_iInputWidth;
    m_sourceSVideoInfo.iFaceHeight = m_iInputHeight;
    if(!m_faceSizeAlignment)
    {
        printf("FaceSizeAlignment must be greater than 0, it is reset to 8 (default value).\n");
        m_faceSizeAlignment = 8;
    }
    // if(m_faceSizeAlignment &1) // && numberToChromaFormat(tmpInputChromaFormat)==CHROMA_420
    // {
    //     //to fix the chroma resolution and luma resolution issue for 4:2:0;
    //     printf("FaceSizeAlignment must be even for chroma 4:2:0 format, it is reset to %d.\n", m_faceSizeAlignment+1);

    //     m_faceSizeAlignment = m_faceSizeAlignment+1;
    // }
    m_codingSVideoInfo.iNumFaces = 1;
    m_codingSVideoInfo.iFaceWidth = m_iCodingFaceWidth ;
    m_codingSVideoInfo.iFaceHeight = m_iCodingFaceHeight;

    m_aiPad[1] = m_aiPad[0] = 0;

    if (m_tileNumCol == 0 || m_tileNumRow == 0)
        return -1;
    int32_t posY = 0;
    int32_t stepX = m_iInputWidth / m_tileNumCol;
    int32_t stepY = m_iInputHeight / m_tileNumRow;
    float stepHorzPos = ERP_HORZ_ANGLE / (float)m_tileNumCol;
    float stepVertPos = ERP_VERT_ANGLE / (float)m_tileNumRow;
    float vertPos = ERP_VERT_START;
    int32_t idx = 0;

    int32_t faceNum = (m_sourceSVideoInfo.geoType == SVIDEO_CUBEMAP) ? 6 : 1;
    m_sourceSVideoInfo.iNumFaces = faceNum;

    for (int32_t faceid = 0; faceid < faceNum; faceid++)
    {
        for (uint32_t i = 0; i < m_tileNumRow; i++)
        {
            int32_t posX = 0;
            float horzPos = ERP_HORZ_START;
            for (uint32_t j = 0; j < m_tileNumCol; j++)
            {
                m_srd[idx].x = posX;
                m_srd[idx].y = posY;
                m_srd[idx].tilewidth = stepX;
                m_srd[idx].tileheight = stepY;
                m_srd[idx].faceId = faceid;
                m_srd[idx].isOccupy = 0;
                m_srd[idx].horzPos = horzPos;
                m_srd[idx].vertPos = vertPos;
                posX += stepX;
                horzPos += stepHorzPos;
                idx++;
            }
            posY += stepY;
            vertPos -= stepVertPos;
        }
    }
    return 0;
}

int32_t  TgenViewport::selectregion(short inputWidth, short inputHeight, short dstWidth, short dstHeight)
{
    uint32_t idx = 0;
    int32_t faceNum = (m_sourceSVideoInfo.geoType == SVIDEO_CUBEMAP) ? 6 : 1;
    float fYaw = m_codingSVideoInfo.viewPort.fYaw;
    float fPitch = m_codingSVideoInfo.viewPort.fPitch;
    float stepHorzPos = ERP_HORZ_ANGLE / (float)m_tileNumCol;
    float stepVertPos = ERP_VERT_ANGLE / (float)m_tileNumRow;
    // starting time
    double dResult;
    clock_t lBefore = clock();

    // seek the center tile of the FOV
    for (int32_t faceid = 0; faceid < faceNum; faceid++)
    {
        for (uint32_t i = 0; i < m_tileNumRow; i++)
        {
            for (uint32_t j = 0; j < m_tileNumCol; j++)
            {
                m_srd[idx].faceId = faceid;
                m_srd[idx].isOccupy = 0;
                if (fPitch >= m_srd[idx].horzPos && fPitch <= m_srd[idx].horzPos + stepHorzPos
                  && fYaw <= m_srd[idx].vertPos && fYaw >= m_srd[idx].vertPos - stepVertPos)
                     break;
		idx++;
            }
        }
    }
    if (idx == m_tileNumRow * m_tileNumCol)
        idx--;
    //select all of the tiles to cover the whole viewport
    short centerX = m_srd[idx].x;
    short centerY = m_srd[idx].y;
    short halfVPhorz = (dstWidth - m_srd[idx].tilewidth) >> 1;
    short halfVPvert = (dstHeight - m_srd[idx].tileheight) >> 1;
    SPos*pTmpUpLeft = m_pUpLeft;
    SPos*pTmpDownRight = m_pDownRight;
    pTmpUpLeft->x = centerX - halfVPhorz;
    pTmpDownRight->x = centerX + halfVPhorz;
    pTmpUpLeft->y = centerY - halfVPvert;
    pTmpDownRight->y = centerY + halfVPvert;
    if (pTmpUpLeft->y < m_srd[idx].tileheight*(-1))
    {
        pTmpUpLeft->y = 0;
        pTmpUpLeft->x = 0;
        pTmpDownRight->x = inputWidth;
    }
    else if(pTmpUpLeft->y < 0)
        pTmpUpLeft->y = 0;

    if (pTmpDownRight->y >= inputHeight + m_srd[idx].tileheight)
    {
        pTmpUpLeft->x = 0;
        pTmpUpLeft->y = pTmpUpLeft->y + halfVPvert;
        pTmpDownRight->x = inputWidth;
        pTmpDownRight->y = inputHeight;
    }
    else if (pTmpDownRight->y > inputHeight)
        pTmpDownRight->y = inputHeight;

    if (pTmpUpLeft->x < 0)
    {
        pTmpUpLeft->x = 0;
        pTmpDownRight++;
        pTmpDownRight->faceIdx = 0;
        pTmpDownRight->x = inputWidth;
        pTmpDownRight->y = m_pDownRight->y;
        pTmpUpLeft++;
        pTmpUpLeft->faceIdx = 0;
        pTmpUpLeft->x = inputWidth - (halfVPhorz - centerX);
        pTmpUpLeft->y = m_pUpLeft->y;
    }
    else if (pTmpDownRight->x > inputWidth)
    {
        pTmpDownRight->x = inputWidth;
        pTmpDownRight++;
        pTmpDownRight->faceIdx = 0;
        pTmpUpLeft++;
        pTmpUpLeft->faceIdx = 0;
        pTmpUpLeft->x = 0;
        pTmpUpLeft->y = m_pUpLeft->y;
        pTmpDownRight->x = centerX + halfVPhorz - inputWidth;
        pTmpDownRight->y = m_pDownRight->y;
    }
    dResult = (double)(clock() - lBefore) / CLOCKS_PER_SEC;
    printf("\n Total Time for tile selection: %12.3f sec.\n", dResult);
    return 0;
}

int32_t  TgenViewport::convert()
{
    Geometry  *pcInputGeomtry = NULL;
    Geometry  *pcCodingGeomtry = NULL;
    pcInputGeomtry = Geometry::create(m_sourceSVideoInfo);
    if(!pcInputGeomtry)
    {
        return -1;
    }
    pcCodingGeomtry = Geometry::create(m_codingSVideoInfo);
    if (!pcCodingGeomtry)
    {
        delete pcInputGeomtry;
        pcInputGeomtry = NULL;
        return -1;
    }

    // starting time
    double dResult;
    clock_t lBefore = clock();

    pcInputGeomtry->geoConvert(pcCodingGeomtry);

    if (pcCodingGeomtry->getType() == SVIDEO_VIEWPORT)
    {
        ViewPort* pViewPort = (ViewPort*)pcCodingGeomtry;
        m_numFaces = pViewPort->m_numFaces;
        SPos *pUpLeftSrc = pViewPort->m_upLeft;
        SPos *pDownRightSrc = pViewPort->m_downRight;
        SPos *pUpLeftDst = m_pUpLeft;
        SPos *pDownRightDst = m_pDownRight;
        for (int32_t i = 0; i < FACE_NUMBER; i++)
        {
            pUpLeftDst->faceIdx = pUpLeftSrc->faceIdx;
            pUpLeftDst->x = pUpLeftSrc->x;
            pUpLeftDst->y = pUpLeftSrc->y;
            pDownRightDst->faceIdx = pDownRightSrc->faceIdx;
            pDownRightDst->x = pDownRightSrc->x;
            pDownRightDst->y = pDownRightSrc->y;
            pUpLeftDst++;
            pDownRightDst++;
            pUpLeftSrc++;
            pDownRightSrc++;
        }
        pcCodingGeomtry->geoUnInit();
    }

    dResult = (double)(clock() - lBefore) / CLOCKS_PER_SEC;
    printf("\n Total Time: %12.3f sec.\n", dResult);

    if(pcInputGeomtry)
    {
        delete pcInputGeomtry;
        pcInputGeomtry=NULL;
    }
    if(pcCodingGeomtry)
    {
        delete pcCodingGeomtry;
        pcCodingGeomtry=NULL;
    }
    return 0;
}
bool TgenViewport::isInside(int32_t x, int32_t y, int32_t width, int32_t height, int32_t faceId)
{
    bool ret = 0;
    SPos *pUpLeft = m_pUpLeft;
    SPos *pDownRight = m_pDownRight;

    int32_t numFaces = (m_sourceSVideoInfo.geoType == SVIDEO_CUBEMAP) ? 6 : 1;
    int32_t bFind = 0;
    for (int32_t i = 0; i < numFaces; i++)
    {
        if (pUpLeft->faceIdx == faceId)
        {
            bFind = 1;
            break;
        }
        pUpLeft++;
        pDownRight++;
    }
    if (bFind == 0)
        return ret;

    if ((x >= pUpLeft->x)
        && (x <= pDownRight->x)
        && (y >= pUpLeft->y)
        && (y <= pDownRight->y))
        ret = 1;
    else if ((x + width >= pUpLeft->x)
        && (x + width <= pDownRight->x)
        && (y + height >= pUpLeft->y)
        && (y + height <= pDownRight->y))
        ret = 1;
    else if ((x + width >= pUpLeft->x)
        && (x + width <= pDownRight->x)
        && (y >= pUpLeft->y)
        && (y <= pDownRight->y))
        ret = 1;
    else if ((x >= pUpLeft->x)
        && (x <= pDownRight->x)
        && (y + height >= pUpLeft->y)
        && (y + height <= pDownRight->y))
        ret = 1;

    //for erp format source, need to judge the boudary
    if (m_sourceSVideoInfo.geoType == SVIDEO_EQUIRECT)
    {
        pUpLeft++;
        pDownRight++;
        if (pUpLeft->faceIdx == 0)
        {
            if ((x >= pUpLeft->x)
                && (x <= pDownRight->x)
                && (y >= pUpLeft->y)
                && (y <= pDownRight->y))
                ret = 1;
            else if ((x + width >= pUpLeft->x)
                && (x + width <= pDownRight->x)
                && (y + height >= pUpLeft->y)
                && (y + height <= pDownRight->y))
                ret = 1;
            else if ((x + width >= pUpLeft->x)
                && (x + width <= pDownRight->x)
                && (y >= pUpLeft->y)
                && (y <= pDownRight->y))
                ret = 1;
            else if ((x >= pUpLeft->x)
                && (x <= pDownRight->x)
                && (y + height >= pUpLeft->y)
                && (y + height <= pDownRight->y))
                ret = 1;
        }
    }
    return ret;
}

int32_t TgenViewport::calcTilesInViewport(ITileInfo* pTileInfo, int32_t tileCol, int32_t tileRow)
{
    if (!pTileInfo)
        return -1;
    int32_t ret = 0;
    ITileInfo *pTileInfoTmp = pTileInfo;
    int32_t faceNum = (m_sourceSVideoInfo.geoType==SVIDEO_CUBEMAP) ? 6 : 1;
    for (int32_t faceid = 0; faceid < faceNum; faceid++)
    {
        for (int32_t row = 0; row < tileRow; row++)
        {
            for (int32_t col = 0; col < tileCol; col++)
            {
                pTileInfoTmp->isOccupy = isInside(pTileInfoTmp->x, pTileInfoTmp->y, pTileInfoTmp->tilewidth, pTileInfoTmp->tileheight, faceid);
                if (pTileInfoTmp->isOccupy == 1)
                {
                    ret++;
                }
                pTileInfoTmp++;
            }
        }
    }
    return ret;
}
//! \}
