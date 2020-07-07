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
#include <cfloat>
#include <algorithm>
#include "360SCVPViewPort.h"
#include "360SCVPViewportImpl.h"
#include "360SCVPViewportAPI.h"
#ifdef WIN32
#define strdup _strdup
#endif

using namespace std;
#define LOW_PITCH_BOUND_IN_NORTH 25
#define LOW_PITCH_BOUND_IN_SOUTH -25

#define HIGH_PITCH_BOUND_IN_NORTH 50
#define HIGH_PITCH_BOUND_IN_SOUTH -50

void* genViewport_Init(generateViewPortParam* pParamGenViewport)
{
    uint32_t maxTileNumCol = 0;
    uint32_t maxTileNumRow = 0;
    uint32_t maxTileNum = 0;
    int32_t viewPortWidth = 0;
    int32_t viewPortHeight = 0;
    int32_t viewPortHeightmax = 0;

    if (!pParamGenViewport)
        return NULL;
    TgenViewport*  cTAppConvCfg = new TgenViewport;
    if (!cTAppConvCfg)
        return NULL;

    cTAppConvCfg->m_usageType = pParamGenViewport->m_usageType;
    cTAppConvCfg->m_paramVideoFP.cols = pParamGenViewport->m_paramVideoFP.cols;
    cTAppConvCfg->m_paramVideoFP.rows = pParamGenViewport->m_paramVideoFP.rows;
    for (int i = 0; i < pParamGenViewport->m_paramVideoFP.rows; i++)
    {
        for (int j = 0; j < pParamGenViewport->m_paramVideoFP.cols; j++)
        {
            cTAppConvCfg->m_paramVideoFP.faces[i][j].faceHeight = pParamGenViewport->m_paramVideoFP.faces[i][j].faceHeight;
            cTAppConvCfg->m_paramVideoFP.faces[i][j].faceWidth = pParamGenViewport->m_paramVideoFP.faces[i][j].faceWidth;
            cTAppConvCfg->m_paramVideoFP.faces[i][j].idFace = pParamGenViewport->m_paramVideoFP.faces[i][j].idFace;
            cTAppConvCfg->m_paramVideoFP.faces[i][j].rotFace = pParamGenViewport->m_paramVideoFP.faces[i][j].rotFace;
        }
    }
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
    if (cTAppConvCfg->create(pParamGenViewport->m_tileNumRow / cTAppConvCfg->m_paramVideoFP.rows, pParamGenViewport->m_tileNumCol / cTAppConvCfg->m_paramVideoFP.cols) < 0)
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
        SPos *pUpLeft = cTAppConvCfg->m_pUpLeft;
        SPos *pDownRight = cTAppConvCfg->m_pDownRight;
        genViewport_setViewPort((void*)cTAppConvCfg, 45, -90);
        genViewport_process(pParamGenViewport, (void*)cTAppConvCfg);

        cTAppConvCfg->m_codingSVideoInfo.viewPort.fPitch = pParamGenViewport->m_viewPort_fPitch;
        cTAppConvCfg->m_codingSVideoInfo.viewPort.fYaw = pParamGenViewport->m_viewPort_fYaw;

        for (int32_t i = 0; i < 6; i++) // the max count is 6
        {
            if (pDownRight->faceIdx == -1)
            {
                pUpLeft++;
                pDownRight++;
                continue;
            }
            viewPortWidth = int32_t(pDownRight->x - pUpLeft->x);
            viewPortHeight = int32_t(pDownRight->y - pUpLeft->y);

            viewPortWidth = floor((float)(viewPortWidth) / (float)(cTAppConvCfg->m_srd[0].tilewidth) + 0.499) * cTAppConvCfg->m_srd[0].tilewidth;
            viewPortHeightmax = floor((float)(viewPortHeight) / (float)(cTAppConvCfg->m_srd[0].tileheight) + 0.499) * cTAppConvCfg->m_srd[0].tileheight;
            printf("viewPortWidthMax = %d viewPortHeightMax = %d, tile_width %d, tile_height %d\n", viewPortWidth, viewPortHeightmax, cTAppConvCfg->m_srd[0].tilewidth, cTAppConvCfg->m_srd[0].tileheight);

            maxTileNumCol = (viewPortWidth / cTAppConvCfg->m_srd[0].tilewidth + 1);
            if (maxTileNumCol > cTAppConvCfg->m_tileNumCol)
                maxTileNumCol = cTAppConvCfg->m_tileNumCol;

            maxTileNumRow = (viewPortHeightmax / cTAppConvCfg->m_srd[0].tileheight + 1);
            if (maxTileNumRow > cTAppConvCfg->m_tileNumRow)
                maxTileNumRow = cTAppConvCfg->m_tileNumRow;

            maxTileNum += maxTileNumCol * maxTileNumRow;
            pUpLeft++;
            pDownRight++;
        }
        //cTAppConvCfg->m_maxTileNum = tiles_num;
        //pParamGenViewport->m_viewportDestWidth = tiles_num_row * cTAppConvCfg->m_srd[0].tilewidth;
        //pParamGenViewport->m_viewportDestHeight = tiles_num_row * cTAppConvCfg->m_srd[0].tileheight;
    }
    else if (cTAppConvCfg->m_sourceSVideoInfo.geoType == SVIDEO_EQUIRECT)
    {
        genViewport_setViewPort((void*)cTAppConvCfg, 0, 0);
        genViewport_process(pParamGenViewport, (void*)cTAppConvCfg);

        cTAppConvCfg->m_codingSVideoInfo.viewPort.fPitch = pParamGenViewport->m_viewPort_fPitch;
        cTAppConvCfg->m_codingSVideoInfo.viewPort.fYaw = pParamGenViewport->m_viewPort_fYaw;
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

        if (pParamGenViewport->m_usageType == E_PARSER_ONENAL
            || pParamGenViewport->m_usageType == E_VIEWPORT_ONLY)
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
        pParamGenViewport->m_viewportDestWidth = maxTileNumCol * cTAppConvCfg->m_srd[0].tilewidth;
        pParamGenViewport->m_viewportDestHeight = maxTileNumRow * cTAppConvCfg->m_srd[0].tileheight;
    }
    cTAppConvCfg->m_maxTileNum = maxTileNum;
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
        x = coverBoundary ?  max(pUpLeft[0].x, pUpLeft[1].x) : pUpLeft[0].x;
        y = pUpLeft[0].y;

        w = pDownRight[0].x - pUpLeft[0].x + coverBoundary * (pDownRight[1].x - pUpLeft[1].x);
        h = pDownRight[0].y - pUpLeft[0].y;

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
    // printf("the max tile count = %d additionalTilesNum = %d\n", maxTileNum, additionalTilesNum);
    // if (additionalTilesNum < 0)
    //     printf("there is an error in the judgement\n");
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
    int32_t faceNum = (cTAppConvCfg->m_sourceSVideoInfo.geoType == SVIDEO_CUBEMAP) ? 6 : 2;
	if (cTAppConvCfg->m_usageType == E_PARSER_ONENAL)
    {
        tileNum = maxTileNum;
    }
    else
    {
        tileNum = tileNum + additionalTilesNum;
    }
    if (cTAppConvCfg->m_srd[cTAppConvCfg->m_tileNumCol*cTAppConvCfg->m_tileNumRow-1].isOccupy == 1)
    {
        for (int32_t idFace = 0; idFace < faceNum; idFace++)
        {
            idx = idFace* cTAppConvCfg->m_tileNumCol*cTAppConvCfg->m_tileNumRow + cTAppConvCfg->m_tileNumCol*cTAppConvCfg->m_tileNumRow -1;
            for (uint32_t col = cTAppConvCfg->m_tileNumCol; col > 0 ; col--)
            {
                for (uint32_t row = cTAppConvCfg->m_tileNumRow; row > 0 ; row--)
                {
                    if (cTAppConvCfg->m_srd[idx].isOccupy == 1)
                    {
                        pOutTileTmp->faceId = cTAppConvCfg->m_srd[idx].faceId;
                        pOutTileTmp->x = cTAppConvCfg->m_srd[idx].x;
                        pOutTileTmp->y = cTAppConvCfg->m_srd[idx].y;
                        pOutTileTmp->idx = idx;
                        pOutTileTmp++;
                    }
                    idx--;
                }
            }
        }
    }
    else
    {
        for (int32_t idFace = 0; idFace < faceNum; idFace++)
        {
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
        }
    }
    return tileNum;
}

int32_t genViewport_getTilesInViewport(void* pGenHandle, TileDef* pOutTile)
{
    TgenViewport* cTAppConvCfg = (TgenViewport*)(pGenHandle);
    if (!cTAppConvCfg || !pOutTile)
        return -1;

    int32_t tileNum = 0;
    // work around : low pitch area need to cover 2 tiles on both left/right sides separately.
    int32_t additionalTilesNum = 4;

    int32_t fPitch = (int32_t)(cTAppConvCfg->m_codingSVideoInfo.viewPort.fPitch);
    tileNum = cTAppConvCfg->calcTilesInViewport(cTAppConvCfg->m_srd, cTAppConvCfg->m_tileNumCol, cTAppConvCfg->m_tileNumRow);

    int32_t pos = 0;
    if (fPitch >= LOW_PITCH_BOUND_IN_NORTH || fPitch <= LOW_PITCH_BOUND_IN_SOUTH)
    {
        if (fPitch >= LOW_PITCH_BOUND_IN_NORTH)
            pos = 0;
        else
            pos = cTAppConvCfg->m_tileNumRow - 1;

        int32_t leftAddition = additionalTilesNum / 2;
        int32_t rightAddition = additionalTilesNum - leftAddition;
        for (uint32_t j = pos * cTAppConvCfg->m_tileNumCol; j < (pos+1) * cTAppConvCfg->m_tileNumCol; j++)
        {
            if (cTAppConvCfg->m_srd[j].isOccupy == 0 && cTAppConvCfg->m_srd[(j+1)%cTAppConvCfg->m_tileNumCol + pos * cTAppConvCfg->m_tileNumCol].isOccupy == 1)
            {
                uint32_t acc_idx = (j + cTAppConvCfg->m_tileNumCol) % cTAppConvCfg->m_tileNumCol + pos * cTAppConvCfg->m_tileNumCol;
                while (leftAddition-- > 0)
                {
                    cTAppConvCfg->m_srd[acc_idx].isOccupy = 1;
                    if (pos == 0)
                    {
                        acc_idx += cTAppConvCfg->m_tileNumCol;
                    }else
                    {
                        acc_idx -= cTAppConvCfg->m_tileNumCol;
                    }
                }
            }
            if (cTAppConvCfg->m_srd[j].isOccupy == 1 && cTAppConvCfg->m_srd[(j+1)%cTAppConvCfg->m_tileNumCol + pos * cTAppConvCfg->m_tileNumCol].isOccupy == 0)
            {
                uint32_t acc_idx = (j + 1 + cTAppConvCfg->m_tileNumCol) % cTAppConvCfg->m_tileNumCol +  pos * cTAppConvCfg->m_tileNumCol;
                while (rightAddition-- > 0)
                {
                    cTAppConvCfg->m_srd[acc_idx].isOccupy = 1;
                    if (pos == 0)
                    {
                        acc_idx += cTAppConvCfg->m_tileNumCol;
                    }else
                    {
                        acc_idx -= cTAppConvCfg->m_tileNumCol;
                    }
                }
            }
        }
        // work around : high pitch area need more tiles( 2 rows )
        if (fPitch >= HIGH_PITCH_BOUND_IN_NORTH)
        {
            for (uint32_t i = 0; i < min(cTAppConvCfg->m_tileNumCol * 2, cTAppConvCfg->m_tileNumCol * cTAppConvCfg->m_tileNumRow); i++)
            {
                if (cTAppConvCfg->m_srd[i].isOccupy == 0)
                    additionalTilesNum++;
                cTAppConvCfg->m_srd[i].isOccupy = 1;
            }
        }
        if (fPitch <= HIGH_PITCH_BOUND_IN_SOUTH)
        {
            uint32_t startRow = cTAppConvCfg->m_tileNumRow - 2 > 0 ? cTAppConvCfg->m_tileNumRow - 2 : 0;
            for (uint32_t i = startRow * cTAppConvCfg->m_tileNumCol; i < cTAppConvCfg->m_tileNumCol * cTAppConvCfg->m_tileNumRow; i++)
            {
                if (cTAppConvCfg->m_srd[i].isOccupy == 0)
                    additionalTilesNum++;
                cTAppConvCfg->m_srd[i].isOccupy = 1;
            }
        }
    }
    //set the occupy tile into the output parameter
    int32_t idx = 0;
    TileDef* pOutTileTmp = pOutTile;
    // correct accurate needed tile number.
    if (additionalTilesNum > 0)
        tileNum = tileNum + additionalTilesNum;

    uint32_t occupancyNum = 0;
    if (cTAppConvCfg->m_srd[cTAppConvCfg->m_tileNumCol*cTAppConvCfg->m_tileNumRow-1].isOccupy == 1)
    {
        idx = cTAppConvCfg->m_tileNumCol*cTAppConvCfg->m_tileNumRow -1;
        for (uint32_t col = cTAppConvCfg->m_tileNumCol; col > 0 ; col--)
        {
            for (uint32_t row = cTAppConvCfg->m_tileNumRow; row > 0 ; row--)
            {
                if (cTAppConvCfg->m_srd[idx].isOccupy == 1)
                {
                    pOutTileTmp->faceId = cTAppConvCfg->m_srd[idx].faceId;
                    pOutTileTmp->x = cTAppConvCfg->m_srd[idx].x;
                    pOutTileTmp->y = cTAppConvCfg->m_srd[idx].y;
                    pOutTileTmp->idx = idx;
                    pOutTileTmp++;
                    occupancyNum++;
                }
                idx--;
            }
        }

    }
    else
    {
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
                    occupancyNum++;
                }
                idx++;
            }
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
    m_srd = NULL;
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
        delete [] m_srd;
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
    if (this->m_srd && src.m_srd)
    {
        memcpy(this->m_srd, src.m_srd, FACE_NUMBER*m_tileNumRow*m_tileNumCol*sizeof(ITileInfo));
    }
    return *this;
}

int32_t TgenViewport::create(uint32_t tileNumRow, uint32_t tileNumCol)
{
    m_tileNumRow = tileNumRow;
    m_tileNumCol = tileNumCol;
    if (!m_srd)
    {
        m_srd = new ITileInfo[FACE_NUMBER*m_tileNumRow*m_tileNumCol];
        if (!m_srd)
            return -1;
    }
    return 0;
}

void TgenViewport::destroy()
{
    if(m_pUpLeft)
    {
        delete [] m_pUpLeft;
        m_pUpLeft = NULL;
    }
    if(m_pDownRight)
    {
        delete [] m_pDownRight;
        m_pDownRight = NULL;
    }
    if(m_srd)
    {
        delete [] m_srd;
        m_srd = NULL;
    }
}


// ====================================================================================================================
// Public member functions
// ====================================================================================================================

int32_t TgenViewport::parseCfg(  )
{
    m_iFrameRate = 30;
    m_faceSizeAlignment = 1;
    m_iSourceWidth = m_iInputWidth * m_paramVideoFP.cols;
    m_iSourceHeight = m_iInputHeight * m_paramVideoFP.rows;
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
    m_codingSVideoInfo.fullWidth = m_iSourceWidth;
    m_codingSVideoInfo.fullHeight = m_iSourceHeight;

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
        posY = 0;
    }
    return 0;
}

int32_t  TgenViewport::selectregion(short inputWidth, short inputHeight, short dstWidth, short dstHeight)
{
    uint32_t idx = 0;
    int32_t faceNum = (m_sourceSVideoInfo.geoType == SVIDEO_CUBEMAP) ? 6 : 1;
    float fYaw = m_codingSVideoInfo.viewPort.fYaw;
    float fPitch = m_codingSVideoInfo.viewPort.fPitch;
    // starting time
    double dResult;
    clock_t lBefore = clock();

    // seek the center tile of the FOV
    float leastDistance = FLT_MAX;
    uint32_t opt_idx = m_tileNumRow * m_tileNumCol;
    for (int32_t faceid = 0; faceid < faceNum; faceid++)
    {
        // select optimization
        float cal_yaw = fYaw + ERP_HORZ_ANGLE / 2;
        float cal_pitch = ERP_VERT_ANGLE / 2 - fPitch;
        float horzStep = ERP_HORZ_ANGLE / (float)m_tileNumCol;
        float vertStep = ERP_VERT_ANGLE / (float)m_tileNumRow;
        std::pair<uint32_t, uint32_t> fov_corr((uint32_t)cal_yaw / (uint32_t)horzStep, (uint32_t)cal_pitch / (uint32_t)vertStep);
        if (fov_corr.second == m_tileNumRow){
            fov_corr.second--;
        }
        if (fov_corr.first == m_tileNumCol){
            fov_corr.first--;
        }
        std::vector<uint32_t> selected_idxs;
        uint32_t select_idx = fov_corr.first + fov_corr.second * m_tileNumCol;
        uint32_t boundary_offset = fov_corr.first == m_tileNumCol -1 ? m_tileNumCol : 0;
        if (fov_corr.second == m_tileNumRow - 1 && faceNum == 1) // erp ( add idx at bottom to select opt_idx )
        {
            m_srd[select_idx + m_tileNumCol].x = m_srd[select_idx].x;
            m_srd[select_idx + m_tileNumCol].y = inputHeight;
            m_srd[select_idx + m_tileNumCol].tilewidth = m_srd[select_idx].tilewidth;
            m_srd[select_idx + m_tileNumCol].tileheight = m_srd[select_idx].tileheight;
            m_srd[select_idx + m_tileNumCol].horzPos = m_srd[select_idx].horzPos;
            m_srd[select_idx + m_tileNumCol].vertPos = m_srd[select_idx].vertPos - ERP_VERT_ANGLE / (float)m_tileNumRow;

            m_srd[select_idx + 1 + m_tileNumCol - boundary_offset].x = m_srd[select_idx + 1 - boundary_offset].x;
            m_srd[select_idx + 1 + m_tileNumCol - boundary_offset].y = inputHeight;
            m_srd[select_idx + 1 + m_tileNumCol - boundary_offset].tilewidth = m_srd[select_idx + 1 - boundary_offset].tilewidth;
            m_srd[select_idx + 1 + m_tileNumCol - boundary_offset].tileheight = m_srd[select_idx + 1 - boundary_offset].tileheight;
            m_srd[select_idx + 1 + m_tileNumCol - boundary_offset].horzPos = m_srd[select_idx + 1 - boundary_offset].horzPos;
            m_srd[select_idx + 1 + m_tileNumCol - boundary_offset].vertPos = m_srd[select_idx + 1 - boundary_offset].vertPos - ERP_VERT_ANGLE / (float)m_tileNumRow;
        }
        selected_idxs.push_back(select_idx);
        selected_idxs.push_back(select_idx + 1 - boundary_offset);
        selected_idxs.push_back(select_idx + m_tileNumCol);
        selected_idxs.push_back(select_idx + 1 + m_tileNumCol - boundary_offset);
        for (uint32_t it=0;it<selected_idxs.size();it++)
        {
            float correct_offset = fYaw - m_srd[selected_idxs[it]].horzPos > ERP_HORZ_ANGLE / 2 ? ERP_HORZ_ANGLE : 0;
            float distance = sqrt(pow(fYaw - m_srd[selected_idxs[it]].horzPos - correct_offset, 2) + pow(fPitch - m_srd[selected_idxs[it]].vertPos, 2));
            if (distance < leastDistance)
            {
                leastDistance = distance;
                opt_idx = selected_idxs[it];
            }
        }
    }
    if (opt_idx == m_tileNumRow * m_tileNumCol)
        opt_idx--;
    idx = opt_idx;
    //select all of the tiles to cover the whole viewport
    short centerX = m_srd[idx].x;
    short centerY = m_srd[idx].y;
    short halfVPhorz = (dstWidth - m_srd[idx].tilewidth) >> 1;
    short halfVPvert = (dstHeight - m_srd[idx].tileheight) >> 1;

    for (uint32_t i=0;i<FACE_NUMBER;i++)
    {
        m_pUpLeft[i].x = inputWidth;
        m_pUpLeft[i].y = inputHeight;
        m_pDownRight[i].x = 0;
        m_pDownRight[i].y = 0;
        m_pUpLeft[i].faceIdx = -1;
        m_pDownRight[i].faceIdx = -1;
    }
    SPos*pTmpUpLeft = m_pUpLeft;
    SPos*pTmpDownRight = m_pDownRight;
    pTmpUpLeft->faceIdx = 0;
    pTmpDownRight->faceIdx = 0;
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
    // Need to ajust after region select optimization in two pole areas
    if (pTmpDownRight->y >= inputHeight + m_srd[idx].tileheight)
    {
        pTmpUpLeft->x = 0;
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

    if (x + width > pUpLeft->x &&
        pUpLeft->x + (pDownRight->x - pUpLeft->x) > x &&
        y + height > pUpLeft->y &&
        pUpLeft->y + (pDownRight->y - pUpLeft->y) > y)
        ret = 1;
/*
    if ((x >= pUpLeft->x)
        && (x < pDownRight->x)
        && (y >= pUpLeft->y)
        && (y < pDownRight->y))
        ret = 1;
    else if ((x + width >= pUpLeft->x)
        && (x + width < pDownRight->x)
        && (y + height >= pUpLeft->y)
        && (y + height < pDownRight->y))
        ret = 1;
    else if ((x + width >= pUpLeft->x)
        && (x + width < pDownRight->x)
        && (y >= pUpLeft->y)
        && (y < pDownRight->y))
        ret = 1;
    else if ((x >= pUpLeft->x)
        && (x < pDownRight->x)
        && (y + height >= pUpLeft->y)
        && (y + height < pDownRight->y))
        ret = 1;
*/
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
                    printf("facid, x, y : %d, %d, %d\n", faceid, pTileInfoTmp->x, pTileInfoTmp->y);
                }
                pTileInfoTmp++;
            }
        }
    }
    return ret;
}
//! \}
