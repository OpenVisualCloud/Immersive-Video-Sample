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
#include "360SCVPLog.h"

#ifdef WIN32
#define strdup _strdup
#endif

using namespace std;

int32_t cubeMapFaceMap[6] = {0, 1, 4, 5, 2, 3};
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

    /* Check the paramVideoFP rows / cols exceeds the maximum array size */
    if (pParamGenViewport->m_paramVideoFP.rows > 6 || pParamGenViewport->m_paramVideoFP.cols > 6) {
        SAFE_DELETE(cTAppConvCfg);
        return NULL;
    }

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
    memset_s(&cTAppConvCfg->m_sourceSVideoInfo, sizeof(struct SVideoInfo), 0);
    memset_s(&cTAppConvCfg->m_codingSVideoInfo, sizeof(struct SVideoInfo), 0);
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
        SAFE_DELETE(cTAppConvCfg);
        return NULL;
    }

    //calculate the max tile num if the source project is cube map
    cTAppConvCfg->m_maxTileNum = 0;
    if (pParamGenViewport->m_tileNumCol == 0 || pParamGenViewport->m_tileNumRow == 0) {
        SAFE_DELETE(cTAppConvCfg);
        return NULL;
    }

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
            SCVP_LOG(LOG_INFO, "viewPortWidthMax is %d, viewPortHeightMax is %d\n", viewPortWidth, viewPortHeightmax);
            SCVP_LOG(LOG_INFO, "tilewidth is %d , tileheight is %d\n", cTAppConvCfg->m_srd[0].tilewidth, cTAppConvCfg->m_srd[0].tileheight);

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

        int32_t sqrtSize = (int32_t)sqrt(maxTileNum);
        int32_t minTileNumDiff = maxTileNum;
        for (int32_t i = sqrtSize; i <= sqrtSize+1; i++) {
            for (int32_t j = i; j <= sqrtSize+1; j++) {
                int32_t tileNumDiff = i * j - maxTileNum;
                if ((tileNumDiff < minTileNumDiff) && (tileNumDiff >= 0)) {
                    pParamGenViewport->m_viewportDestWidth = i * cTAppConvCfg->m_srd[0].tilewidth;
                    pParamGenViewport->m_viewportDestHeight = j * cTAppConvCfg->m_srd[0].tileheight;
                    minTileNumDiff = tileNumDiff;
                }
            }
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
            SCVP_LOG(LOG_INFO, "viewPortWidthMax = %d, viewPortHeightMax = %d\n", viewPortWidth, viewPortHeightmax);

            pUpLeft++;
            pDownRight++;
        }

        // if (pParamGenViewport->m_usageType == E_PARSER_ONENAL
        //     || pParamGenViewport->m_usageType == E_VIEWPORT_ONLY)
        // {
        //     viewPortWidth = floor((float)(viewPortWidth) / (float)(cTAppConvCfg->m_srd[0].tilewidth) + 0.499) * cTAppConvCfg->m_srd[0].tilewidth;
        //     viewPortHeightmax = floor((float)(viewPortHeightmax) / (float)(cTAppConvCfg->m_srd[0].tileheight) + 0.499) * cTAppConvCfg->m_srd[0].tileheight;
        //     printf("viewPortWidthMax = %d viewPortHeightMax = %d, tile_width %d, tile_height %d\n", viewPortWidth, viewPortHeightmax, cTAppConvCfg->m_srd[0].tilewidth, cTAppConvCfg->m_srd[0].tileheight);

        //     maxTileNumCol = (viewPortWidth / cTAppConvCfg->m_srd[0].tilewidth + 1);
        //     if (maxTileNumCol > cTAppConvCfg->m_tileNumCol)
        //         maxTileNumCol = cTAppConvCfg->m_tileNumCol;

        //     maxTileNumRow = (viewPortHeightmax / cTAppConvCfg->m_srd[0].tileheight + 1);
        //     if (maxTileNumRow > cTAppConvCfg->m_tileNumRow)
        //         maxTileNumRow = cTAppConvCfg->m_tileNumRow;
        // }
        // else
        // {
            maxTileNumCol = (viewPortWidth / cTAppConvCfg->m_srd[0].tilewidth + 2);
            if (maxTileNumCol > cTAppConvCfg->m_tileNumCol)
                maxTileNumCol = cTAppConvCfg->m_tileNumCol;
            maxTileNumRow = (viewPortHeightmax / cTAppConvCfg->m_srd[0].tileheight + 2);
            if (maxTileNumRow > cTAppConvCfg->m_tileNumRow)
                maxTileNumRow = cTAppConvCfg->m_tileNumRow;
        // }

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

    cTAppConvCfg->ERPselectregion(pParamGenViewport->m_iInputWidth, pParamGenViewport->m_iInputHeight, pParamGenViewport->m_viewportDestWidth, pParamGenViewport->m_viewportDestHeight);

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
    int32_t coverageShapeType;

    if (!cTAppConvCfg)
        return -1;

    // pOutCC should be allocated before this func called, or we new it here
    if(!pOutCC)
        return -1;

    // ERP mode may have 2 faces for boundary case
    if(cTAppConvCfg->m_sourceSVideoInfo.geoType == SVIDEO_EQUIRECT)
    {
        /* ERP used shape type 0 too. Utilize the FOV to generate CC info */
        coverageShapeType = 0;
        cTAppConvCfg->getContentCoverage(pOutCC, coverageShapeType);
#if 0
        SPos *pUpLeft = cTAppConvCfg->m_pUpLeft;
        SPos *pDownRight = cTAppConvCfg->m_pDownRight;

        int32_t videoWidth = cTAppConvCfg->m_iInputWidth;
        int32_t videoHeight = cTAppConvCfg->m_iInputHeight;

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
#endif
    }
    else if(cTAppConvCfg->m_sourceSVideoInfo.geoType == SVIDEO_CUBEMAP)
    {
         coverageShapeType = 0;
         cTAppConvCfg->getContentCoverage(pOutCC, coverageShapeType);
    }
    else
    {
        SCVP_LOG(LOG_WARNING, "Only Support GeoType ERP and Cubemap for Content Coverage\n");
        return -1;
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
    SCVP_LOG(LOG_INFO, "the max tile count = %d additionalTilesNum = %d\n", maxTileNum, additionalTilesNum);
    if (additionalTilesNum < 0)
        SCVP_LOG(LOG_WARNING, "there is an error in the judgement!\n");
    int32_t pos = 0;
    for (int32_t i = 0; i < additionalTilesNum; i++)
    {
        for (uint32_t j = pos; j < cTAppConvCfg->m_tileNumCol*cTAppConvCfg->m_tileNumRow*cTAppConvCfg->m_numFaces; j++)
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
    if (cTAppConvCfg->m_srd[cTAppConvCfg->m_tileNumCol*cTAppConvCfg->m_tileNumRow*cTAppConvCfg->m_numFaces-1].isOccupy == 1)
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

    tileNum = cTAppConvCfg->calcTilesInViewport(cTAppConvCfg->m_srd, cTAppConvCfg->m_tileNumCol, cTAppConvCfg->m_tileNumRow);
    int32_t idx = 0;
    TileDef* pOutTileTmp = pOutTile;
    int32_t faceNum = (cTAppConvCfg->m_sourceSVideoInfo.geoType == SVIDEO_CUBEMAP) ? 6 : 2;
    // correct accurate needed tile number in ERP.

    if (cTAppConvCfg->m_srd[cTAppConvCfg->m_tileNumCol*cTAppConvCfg->m_tileNumRow-1].isOccupy == 1)
    {
        for (int32_t idFace = 0; idFace < faceNum; idFace++)
        {
            idx = idFace * cTAppConvCfg->m_tileNumCol * cTAppConvCfg->m_tileNumRow + cTAppConvCfg->m_tileNumCol * cTAppConvCfg->m_tileNumRow - 1;
            for (uint32_t col = cTAppConvCfg->m_tileNumCol; col > 0 ; col--)
            {
                for (uint32_t row = cTAppConvCfg->m_tileNumRow; row > 0 ; row--)
                {
                    if (cTAppConvCfg->m_srd[idx].isOccupy == 1)
                    {
                        pOutTileTmp->faceId = cubeMapFaceMap[cTAppConvCfg->m_srd[idx].faceId];
                        pOutTileTmp->x = cTAppConvCfg->m_srd[idx].x;
                        pOutTileTmp->y = cTAppConvCfg->m_srd[idx].y;
                        pOutTileTmp->idx = idx;
                        SCVP_LOG(LOG_INFO, "final decision is idx %d and face_id %d\n", idx, pOutTileTmp->faceId);
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
                        pOutTileTmp->faceId = cubeMapFaceMap[cTAppConvCfg->m_srd[idx].faceId];
                        pOutTileTmp->x = cTAppConvCfg->m_srd[idx].x;
                        pOutTileTmp->y = cTAppConvCfg->m_srd[idx].y;
                        pOutTileTmp->idx = idx;
                        SCVP_LOG(LOG_INFO, "final decision is idx %d and face_id %d\n", idx, pOutTileTmp->faceId);
                        pOutTileTmp++;
                    }
                    idx++;
                }
            }
        }
    }
    SCVP_LOG(LOG_INFO, "Tile Selection Final Resulted %d Tiles in Total!!!\n", tileNum);
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

int32_t genViewport_getTilesInViewportByLegacyWay(void* pGenHandle, TileDef* pOutTile)
{
    TgenViewport* cTAppConvCfg = (TgenViewport*)(pGenHandle);
    if (!cTAppConvCfg || !pOutTile)
        return -1;

    ITileInfo* pTileInfoTmp = cTAppConvCfg->m_srd;
    int32_t faceNum = (cTAppConvCfg->m_sourceSVideoInfo.geoType == SVIDEO_CUBEMAP) ? 6 : 1;
    for (int32_t faceid = 0; faceid < faceNum; faceid++)
    {
        for (uint32_t row = 0; row < cTAppConvCfg->m_tileNumRow; row++)
        {
            for (uint32_t col = 0; col < cTAppConvCfg->m_tileNumCol; col++)
            {
                pTileInfoTmp->isOccupy = cTAppConvCfg->isInside(pTileInfoTmp->x, pTileInfoTmp->y, pTileInfoTmp->tilewidth, pTileInfoTmp->tileheight, faceid);
                pTileInfoTmp++;
            }
        }
    }

    int32_t idx = 0;
    TileDef* pOutTileTmp = pOutTile;
    faceNum = (cTAppConvCfg->m_sourceSVideoInfo.geoType == SVIDEO_CUBEMAP) ? 6 : 2;

    uint32_t occupancyNum = 0;
    if (cTAppConvCfg->m_srd[cTAppConvCfg->m_tileNumCol * cTAppConvCfg->m_tileNumRow - 1].isOccupy == 1)
    {
        for (int32_t idFace = 0; idFace < faceNum; idFace++)
        {
            idx = idFace * cTAppConvCfg->m_tileNumCol * cTAppConvCfg->m_tileNumRow + cTAppConvCfg->m_tileNumCol * cTAppConvCfg->m_tileNumRow - 1;
            for (uint32_t col = cTAppConvCfg->m_tileNumCol; col > 0; col--)
            {
                for (uint32_t row = cTAppConvCfg->m_tileNumRow; row > 0; row--)
                {
                    if (cTAppConvCfg->m_srd[idx].isOccupy == 1)
                    {
                        pOutTileTmp->faceId = cubeMapFaceMap[cTAppConvCfg->m_srd[idx].faceId];
                        pOutTileTmp->x = cTAppConvCfg->m_srd[idx].x;
                        pOutTileTmp->y = cTAppConvCfg->m_srd[idx].y;
                        pOutTileTmp->idx = idx;
                        SCVP_LOG(LOG_INFO, "final decision is idx %d and face_id %d\n", idx, pOutTileTmp->faceId);
                        pOutTileTmp++;
                        occupancyNum++;
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
                        pOutTileTmp->faceId = cubeMapFaceMap[cTAppConvCfg->m_srd[idx].faceId];
                        pOutTileTmp->x = cTAppConvCfg->m_srd[idx].x;
                        pOutTileTmp->y = cTAppConvCfg->m_srd[idx].y;
                        pOutTileTmp->idx = idx;
                        SCVP_LOG(LOG_INFO, "final decision is idx %d and face_id %d\n", idx, pOutTileTmp->faceId);
                        pOutTileTmp++;
                        occupancyNum++;
                    }
                    idx++;
                }
            }
        }
    }
    return occupancyNum;
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
    memset_s(&m_codingSVideoInfo, sizeof(SVideoInfo), 0);
    memset_s(&m_sourceSVideoInfo, sizeof(SVideoInfo), 0);
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
    m_pViewportHorizontalBoudaryPoints = NULL;
    m_paramVideoFP.cols = 0;
    m_paramVideoFP.rows = 0;
}

TgenViewport::TgenViewport(TgenViewport& src)
{
    m_faceSizeAlignment = src.m_faceSizeAlignment;
    m_pUpLeft = new SPos[FACE_NUMBER];
    m_pDownRight = new SPos[FACE_NUMBER];
    memcpy_s(m_pUpLeft, sizeof(SPos), src.m_pUpLeft, sizeof(SPos));
    memcpy_s(m_pDownRight, sizeof(SPos), src.m_pDownRight, sizeof(SPos));
    m_codingSVideoInfo = src.m_codingSVideoInfo;
    m_sourceSVideoInfo = src.m_sourceSVideoInfo;
    m_iCodingFaceWidth = src.m_iCodingFaceHeight;
    m_iCodingFaceHeight = src.m_iCodingFaceHeight;
    m_iSourceWidth = src.m_iSourceWidth;
    m_iSourceHeight = src.m_iSourceHeight;
    m_tileNumCol = src.m_tileNumCol;
    m_tileNumRow = src.m_tileNumRow;
    m_iFrameRate = src.m_iFrameRate;
    m_iInputWidth = src.m_iInputWidth;
    m_iInputHeight = src.m_iInputHeight;
    m_maxTileNum = src.m_maxTileNum;
    m_usageType = src.m_usageType;
    m_numFaces = src.m_numFaces;
    m_srd = NULL;
    m_pViewportHorizontalBoudaryPoints = NULL;
    m_paramVideoFP.cols = src.m_paramVideoFP.cols;
    m_paramVideoFP.rows = src.m_paramVideoFP.rows;
}

TgenViewport::~TgenViewport()
{
    SAFE_DELETE_ARRAY(m_pUpLeft);
    SAFE_DELETE_ARRAY(m_pDownRight);
    SAFE_DELETE_ARRAY(m_srd);
    SAFE_DELETE_ARRAY(m_pViewportHorizontalBoudaryPoints);
}

TgenViewport& TgenViewport::operator=(const TgenViewport& src)
{
    if (&src == this)
        return *this;
    // memcpy(m_faceSizeAlignment, src.m_faceSizeAlignment, sizeof(int32_t));
    this->m_faceSizeAlignment = src.m_faceSizeAlignment;
    memcpy_s(this->m_pUpLeft, sizeof(SPos), src.m_pUpLeft, sizeof(SPos));
    memcpy_s(this->m_pDownRight, sizeof(SPos), src.m_pDownRight, sizeof(SPos));
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
        int32_t totalTileInfoSize = FACE_NUMBER*m_tileNumRow*m_tileNumCol*sizeof(ITileInfo);
        memcpy_s(this->m_srd, totalTileInfoSize, src.m_srd, totalTileInfoSize);
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
    if (!m_pViewportHorizontalBoudaryPoints)
    {
        m_pViewportHorizontalBoudaryPoints = new SpherePoint[ERP_VERT_ANGLE / HORZ_BOUNDING_STEP + 1];
        if (!m_pViewportHorizontalBoudaryPoints)
            return -1;
    }

    return 0;
}

void TgenViewport::destroy()
{
    SAFE_DELETE_ARRAY(m_pUpLeft);
    SAFE_DELETE_ARRAY(m_pDownRight);
    SAFE_DELETE_ARRAY(m_srd);
    SAFE_DELETE_ARRAY(m_pViewportHorizontalBoudaryPoints);
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
        SCVP_LOG(LOG_WARNING, "FaceSizeAlignment must be greater than 0, it is reset to 8 (default value)\n");
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

    if (m_sourceSVideoInfo.geoType == SVIDEO_CUBEMAP)
    {
        for (int32_t faceid = 0; faceid < faceNum; faceid++)
        {
            for (uint32_t i = 0; i < m_tileNumRow; i++)
            {
                int32_t posX = 0;
                for (uint32_t j = 0; j < m_tileNumCol; j++)
                {
                    m_srd[idx].x = posX;
                    m_srd[idx].y = posY;
                    m_srd[idx].tilewidth = stepX;
                    m_srd[idx].tileheight = stepY;
                    m_srd[idx].faceId = faceid;
                    m_srd[idx].isOccupy = 0;
                    posX += stepX;
                    idx++;
                }
                posY += stepY;
            }
            posY = 0;
        }
        CubemapCalcTilesGrid();
    }
    else //ERP uses uniform tile split
    {
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
    }

    return 0;
}
static float clampAngle(float angleIn, float minDegree, float maxDegree)
{
    float angleOut;
    angleOut = angleIn;

    while (angleOut > maxDegree)
        angleOut -= 360;

    while (angleOut < minDegree)
        angleOut += 360;

    return angleOut;
}

int32_t TgenViewport::CubemapCalcTilesGrid()
{
    uint32_t i, j, face;
    POSType x, y, z;
    POSType pu, pv;
    ITileInfo* pTileGridCMP = m_srd;

    SPos* gridPoint3D;
    SPos *pTileGrid;

    gridPoint3D = new SPos[FACE_NUMBER * (m_tileNumRow+1) * (m_tileNumCol+1)];
    if (!gridPoint3D) {
        SCVP_LOG(LOG_ERROR, "Allocate 3D Grid Point Coordinates Failed\n");
        return -1;
    }

    for (i = 0; i <= m_tileNumRow; i++) {
        for (j = 0; j <= m_tileNumCol; j++) {
            pu = (POSType)j*2 / m_tileNumCol - 1;
            pv = (POSType)i*2 / m_tileNumRow - 1;

            pTileGrid = gridPoint3D + (i * (m_tileNumCol+1)) + j;
            pTileGrid->x = 1.0;
            pTileGrid->y = -pv;
            pTileGrid->z = -pu;

            pTileGrid += (m_tileNumRow + 1) * (m_tileNumCol + 1);
            pTileGrid->x = -1.0;
            pTileGrid->y = -pv;
            pTileGrid->z = pu;

            pTileGrid += (m_tileNumRow + 1) * (m_tileNumCol + 1);
            pTileGrid->x = pu;
            pTileGrid->y = 1.0;
            pTileGrid->z = pv;

            pTileGrid += (m_tileNumRow + 1) * (m_tileNumCol + 1);
            pTileGrid->x = pu;
            pTileGrid->y = -1.0;
            pTileGrid->z = -pv;

            pTileGrid += (m_tileNumRow + 1) * (m_tileNumCol + 1);
            pTileGrid->x = pu;
            pTileGrid->y = -pv;
            pTileGrid->z = 1.0;

            pTileGrid += (m_tileNumRow + 1) * (m_tileNumCol + 1);
            pTileGrid->x = -pu;
            pTileGrid->y = -pv;
            pTileGrid->z = -1.0;
        }
    }
    pTileGrid = gridPoint3D;
    pTileGridCMP = m_srd;
    for (face = 0; face < FACE_NUMBER; face++)
        for (i = 0; i < m_tileNumRow; i++)
            for (j = 0; j < m_tileNumCol; j++) {
                pTileGrid = &gridPoint3D[face * (m_tileNumRow + 1) * (m_tileNumCol + 1) + i * (m_tileNumRow + 1) + j];
                x = pTileGrid->x;
                y = pTileGrid->y;
                z = pTileGrid->z;
                pTileGridCMP->vertPos = HALF_PI_IN_DEGREE * sasin(y / ssqrt(x * x + y * y + z * z))/ S_PI_2;
                pTileGridCMP->horzPos = HALF_PI_IN_DEGREE * satan(-z / x) / S_PI_2;
                if (x < 0)
                    pTileGridCMP->horzPos += PI_IN_DEGREE;
                pTileGridCMP->horzPos = clampAngle(pTileGridCMP->horzPos, -180, 180);

                pTileGrid++;
                x = pTileGrid->x;
                y = pTileGrid->y;
                z = pTileGrid->z;
                pTileGridCMP->vertPosTopRight = HALF_PI_IN_DEGREE * sasin(y / ssqrt(x * x + y * y + z * z)) / S_PI_2;
                pTileGridCMP->horzPosTopRight = HALF_PI_IN_DEGREE * satan(-z / x) / S_PI_2;
                if (x < 0)
                    pTileGridCMP->horzPosTopRight += PI_IN_DEGREE;
                pTileGridCMP->horzPosTopRight = clampAngle(pTileGridCMP->horzPosTopRight, -180, 180);

                pTileGrid += m_tileNumCol;
                x = pTileGrid->x;
                y = pTileGrid->y;
                z = pTileGrid->z;
                pTileGridCMP->vertPosBottomLeft = HALF_PI_IN_DEGREE * sasin(y / ssqrt(x * x + y * y + z * z)) / S_PI_2;
                pTileGridCMP->horzPosBottomLeft = HALF_PI_IN_DEGREE * satan(-z / x) / S_PI_2;
                if (x < 0)
                    pTileGridCMP->horzPosBottomLeft += PI_IN_DEGREE;
                pTileGridCMP->horzPosBottomLeft = clampAngle(pTileGridCMP->horzPosBottomLeft, -180, 180);

                pTileGrid++;
                x = pTileGrid->x;
                y = pTileGrid->y;
                z = pTileGrid->z;
                pTileGridCMP->vertPosBottomRight = HALF_PI_IN_DEGREE * sasin(y / ssqrt(x * x + y * y + z * z)) / S_PI_2;
                pTileGridCMP->horzPosBottomRight = HALF_PI_IN_DEGREE * satan(-z / x) / S_PI_2;
                if (x < 0)
                    pTileGridCMP->horzPosBottomRight += PI_IN_DEGREE;
                pTileGridCMP->horzPosBottomRight = clampAngle(pTileGridCMP->horzPosBottomRight, -180, 180);

                pTileGrid -= m_tileNumCol + 2;
                pTileGridCMP++;
            }
    SAFE_DELETE_ARRAY(gridPoint3D);
    return 0;
}


float TgenViewport::calculateLongitudeFromThita(float Latti, float phi, float maxLongiOffset) //Latti is the point lattitude we're interested
{
    float longi = 0;
    if (Latti < 90 - phi)
        longi = sasin(ssin(DEG2RAD_FACTOR * phi) / scos(DEG2RAD_FACTOR * Latti)) * RAD2DEG_FACTOR;
    else
        longi = maxLongiOffset;
    return longi;
}

float TgenViewport::calculateLattitudeFromPhi(float phi, float pitch) //pitch is the top point lattitude
{
    float latti = 0;
    latti = sasin(ssin(DEG2RAD_FACTOR * pitch) * scos(DEG2RAD_FACTOR * phi)) * RAD2DEG_FACTOR;
    return latti;
}

float TgenViewport::calculateLatti(float pitch, float hFOV) //pitch is the top point lattitude when yaw=pitch=0
{
    double fDen, fNum;
    fDen = ssin(DEG2RAD_FACTOR * hFOV / 2) * ssin(DEG2RAD_FACTOR * pitch);
    fDen *= fDen;
    fDen = 1 - fDen;
    fNum = scos(DEG2RAD_FACTOR * pitch);
    fNum *= fNum;
    return sacos(sqrt(fNum / fDen))* RAD2DEG_FACTOR;
}

float TgenViewport::calculateLongiByLatti(float latti, float pitch) //pitch is the curve top point's lattitude
{
    float instantPhi, ret;
    instantPhi = sacos(ssin(latti * DEG2RAD_FACTOR) / ssin(pitch * DEG2RAD_FACTOR)) * RAD2DEG_FACTOR;
    ret = sasin(ssin(instantPhi * DEG2RAD_FACTOR) / scos(latti * DEG2RAD_FACTOR)) * RAD2DEG_FACTOR;
    return ret;
}

int32_t TgenViewport::ERPselectTilesInsideOnOneRow(ITileInfo *pTileInfo, int32_t tileNumCol, float leftCol, float rightCol, int32_t row)
{
    if (leftCol >= 0 && rightCol < tileNumCol)
    {
        for (int32_t j = (int32_t)leftCol; j < rightCol; j++)
            pTileInfo[j + row * tileNumCol].isOccupy = 1;
    }
    else if (leftCol < 0 && rightCol < tileNumCol)
    {
        if (rightCol >= 0)
        {
            for (int32_t j = 0; j < rightCol; j++)
                pTileInfo[j + row * tileNumCol].isOccupy = 1;
            for (int32_t j = (int32_t)(tileNumCol + leftCol); j < tileNumCol; j++)
                pTileInfo[j + row * tileNumCol].isOccupy = 1;
        }
        else
        {
            for (int32_t j = (int32_t)(tileNumCol + leftCol); j < tileNumCol+rightCol; j++)
                pTileInfo[j + row * tileNumCol].isOccupy = 1;
        }
    }
    else if (leftCol >= 0 && rightCol >= tileNumCol)
    {
        if (leftCol < tileNumCol)
        {
            for (int32_t j = (int32_t)leftCol; j < tileNumCol; j++)
                pTileInfo[j + row * tileNumCol].isOccupy = 1;
            for (int32_t j = 0; j < rightCol - tileNumCol; j++)
                pTileInfo[j + row * tileNumCol].isOccupy = 1;
        }
        else
        {
            for (int32_t j = (int32_t)(leftCol - tileNumCol); j < rightCol - tileNumCol; j++)
                pTileInfo[j + row * tileNumCol].isOccupy = 1;
        }
    }
    else
    {
        for (int32_t j = 0; j < tileNumCol; j++)
            pTileInfo[j + row * tileNumCol].isOccupy = 1;
    }
    return 0;
}

int32_t  TgenViewport::ERPselectregion(short inputWidth, short inputHeight, short dstWidth, short dstHeight)
{
    float fYaw = m_codingSVideoInfo.viewPort.fYaw;
    float fPitch = m_codingSVideoInfo.viewPort.fPitch;
    float vFOV = m_codingSVideoInfo.viewPort.vFOV;
    float hFOV = m_codingSVideoInfo.viewPort.hFOV;

    SCVP_LOG(LOG_INFO, "Yaw is %f and Pitch is %f\n", fYaw, fPitch);
    SCVP_LOG(LOG_INFO, "vFOV is %f and hFOV is %f\n", vFOV, hFOV);

    // starting time
    double dResult;
    clock_t lBefore = clock();

    float cal_yaw = fYaw + ERP_HORZ_ANGLE / 2;
    float cal_pitch = ERP_VERT_ANGLE / 2 - fPitch;
    float horzStep = ERP_HORZ_ANGLE / (float)m_tileNumCol;
    float vertStep = ERP_VERT_ANGLE / (float)m_tileNumRow;
    float leftCol, rightCol;
    float thita, phi;
    int32_t topRow = (int32_t)((cal_pitch - vFOV / 2) / vertStep);
    int32_t bottomRow = (int32_t)((cal_pitch + vFOV / 2) / vertStep);
    SpherePoint topPoint, bottomPoint;
    SpherePoint topLeftPoint, topRightPoint, bottomLeftPoint, bottomRightPoint;
    float slope, longiOffsetOnHorzBoundary;
    float vertPos, vertPosBottom;
    SPos* pTmpUpLeft = m_pUpLeft;
    SPos* pTmpDownRight = m_pDownRight;

    for (uint32_t i = 0; i < m_tileNumCol * m_tileNumRow; i++)
    {
        m_srd[i].horzPosTopRight = m_srd[i].horzPosBottomRight = m_srd[i].horzPos + horzStep;
        m_srd[i].horzPosBottomLeft = m_srd[i].horzPos;
        m_srd[i].vertPosBottomLeft = m_srd[i].vertPosBottomRight = m_srd[i].vertPos - vertStep;
        m_srd[i].vertPosTopRight = m_srd[i].vertPos;
        m_srd[i].isOccupy = 0;
    }
    for (uint32_t i = 0; i < FACE_NUMBER; i++)
    {
        m_pUpLeft[i].x = 0;
        m_pUpLeft[i].y = 0;
        m_pDownRight[i].x = inputWidth;
        m_pDownRight[i].y = inputHeight;
        m_pUpLeft[i].faceIdx = -1;
        m_pDownRight[i].faceIdx = -1;
    }
    pTmpUpLeft->faceIdx = 0;
    pTmpDownRight->faceIdx = 0;

    /* Calculate the topLeft point lattitude when yaw=pitch=0 */
    thita = calculateLatti(vFOV / 2, hFOV);
    /* Phi is half of the open angle of the topLeft/topRight point with the sphere center, which won't change under different pitch/yaw */
    phi = sacos(ssin(thita * DEG2RAD_FACTOR) / ssin((vFOV/2) * DEG2RAD_FACTOR)) * RAD2DEG_FACTOR;
    /* Calculate the topLeft/topRight point position with current pitch */
    topLeftPoint.thita = topRightPoint.thita = sasin(scos(phi * DEG2RAD_FACTOR) * ssin((fPitch+vFOV/2) * DEG2RAD_FACTOR)) * RAD2DEG_FACTOR;
    topLeftPoint.alpha = cal_yaw - fabs(sasin(ssin(phi * DEG2RAD_FACTOR) / scos(topLeftPoint.thita * DEG2RAD_FACTOR)) * RAD2DEG_FACTOR);
    topRightPoint.alpha = cal_yaw + fabs(sasin(ssin(phi * DEG2RAD_FACTOR) / scos(topLeftPoint.thita * DEG2RAD_FACTOR)) * RAD2DEG_FACTOR);
    bottomLeftPoint.thita = bottomRightPoint.thita = sasin(scos(phi * DEG2RAD_FACTOR) * ssin((fPitch - vFOV / 2) * DEG2RAD_FACTOR)) * RAD2DEG_FACTOR;
    bottomLeftPoint.alpha = cal_yaw - fabs(sasin(ssin(phi * DEG2RAD_FACTOR) / scos(bottomLeftPoint.thita * DEG2RAD_FACTOR)) * RAD2DEG_FACTOR);
    bottomRightPoint.alpha = cal_yaw + fabs(sasin(ssin(phi * DEG2RAD_FACTOR) / scos(bottomLeftPoint.thita * DEG2RAD_FACTOR)) * RAD2DEG_FACTOR);

    /* Calculate top/bottom point position */
    topPoint.thita = fPitch + vFOV / 2;
    bottomPoint.thita = fPitch - vFOV / 2;
    topPoint.alpha = bottomPoint.alpha = cal_yaw;

    /* Calculate boundary thita/alpha between every 180/(5*m_tileNumRow) degrees */
    SpherePoint* pHorzBoundaryPoint = m_pViewportHorizontalBoudaryPoints;
    SpherePoint* pHorzBoundaryPointHist = pHorzBoundaryPoint;

    for (float offsetAngle = vFOV / 2; offsetAngle >= -vFOV / 2; offsetAngle -= HORZ_BOUNDING_STEP)
    {
        float instantThita = calculateLatti(offsetAngle, hFOV);
        float instantPhi;
        if (fabs(offsetAngle) <= 1e-9)
            instantPhi = hFOV / 2;
        else
            instantPhi = sacos(ssin(instantThita * DEG2RAD_FACTOR) / ssin((fabs(offsetAngle)) * DEG2RAD_FACTOR)) * RAD2DEG_FACTOR;
        pHorzBoundaryPoint->thita = calculateLattitudeFromPhi(instantPhi, fPitch + offsetAngle);
        pHorzBoundaryPoint->alpha = calculateLongitudeFromThita(pHorzBoundaryPoint->thita, instantPhi, hFOV/2);
        pHorzBoundaryPointHist = pHorzBoundaryPoint;
        pHorzBoundaryPoint++;
    }

    float topLatti, bottomLatti;
    float leftColOnCurrentLine, rightColOnCurrentLine;
    float leftColOnHorzBound, rightColOnHorzBound;

    if (fPitch + vFOV / 2 > 90) //Top row crosses the north polar
    {
        topLatti = topLeftPoint.thita;
        topRow = (int32_t)((ERP_VERT_ANGLE / 2 - topLatti) / vertStep);
        leftCol = topLeftPoint.alpha / horzStep;
        rightCol = topRightPoint.alpha / horzStep;

        for (int32_t i = 0; i <= topRow; i++)
        {
            vertPos = m_srd[i * m_tileNumCol].vertPos;
            if (vertPos >= ERP_VERT_ANGLE - (fPitch + vFOV / 2))
            {
                /* Select all tiles in current row */
                ERPselectTilesInsideOnOneRow(m_srd, m_tileNumCol, 0, m_tileNumCol, i);
            }
            else {
                leftColOnCurrentLine = (cal_yaw - 180 + calculateLongiByLatti(vertPos, fPitch + vFOV / 2)) / horzStep;
                rightColOnCurrentLine = (cal_yaw + 180 - calculateLongiByLatti(vertPos, fPitch + vFOV / 2)) / horzStep;
                pHorzBoundaryPoint = m_pViewportHorizontalBoudaryPoints;
                pHorzBoundaryPointHist = pHorzBoundaryPoint;
                pHorzBoundaryPoint++;
                longiOffsetOnHorzBoundary = 90;
                for (int32_t j = 0; j <= vFOV / HORZ_BOUNDING_STEP; j++)
                {
                    if (vertPos >= pHorzBoundaryPoint->thita && vertPos <= pHorzBoundaryPointHist->thita)
                    {
                        slope = (pHorzBoundaryPoint->alpha - pHorzBoundaryPointHist->alpha) / (pHorzBoundaryPoint->thita - pHorzBoundaryPointHist->thita);
                        longiOffsetOnHorzBoundary = slope * (vertPos - pHorzBoundaryPointHist->thita) + pHorzBoundaryPointHist->alpha;
                        break;
                    }
                    else
                    {
                        pHorzBoundaryPoint++;
                        pHorzBoundaryPointHist++;
                    }
                }
                leftColOnHorzBound = (cal_yaw - 180 + longiOffsetOnHorzBoundary) / horzStep;
                rightColOnHorzBound = (cal_yaw +180 - longiOffsetOnHorzBoundary) / horzStep;
                ERPselectTilesInsideOnOneRow(m_srd, m_tileNumCol, leftColOnCurrentLine, leftColOnHorzBound, i);
                ERPselectTilesInsideOnOneRow(m_srd, m_tileNumCol, rightColOnHorzBound, rightColOnCurrentLine, i);
            }
        }
    }
    else if (fPitch + vFOV / 2 < 0) //Top row below the equator
    {
        topLatti = topLeftPoint.thita;
        topRow = (int32_t)((ERP_VERT_ANGLE / 2 - topLatti) / vertStep);
        leftCol = topLeftPoint.alpha / horzStep;
        rightCol = topRightPoint.alpha / horzStep;
        int32_t i;
        for (i = topRow; i <= (int32_t)((ERP_VERT_ANGLE / 2 - topPoint.thita) / vertStep); i++)
        {
            vertPosBottom = m_srd[i * m_tileNumCol].vertPosBottomLeft;
            pHorzBoundaryPoint = m_pViewportHorizontalBoudaryPoints;
            pHorzBoundaryPointHist = pHorzBoundaryPoint;
            pHorzBoundaryPoint++;
            longiOffsetOnHorzBoundary = 90;
            for (int32_t j = 0; j < vFOV / HORZ_BOUNDING_STEP; j++)
            {
                if (vertPosBottom >= pHorzBoundaryPoint->thita && vertPosBottom <= pHorzBoundaryPointHist->thita)
                {
                    slope = (pHorzBoundaryPoint->alpha - pHorzBoundaryPointHist->alpha) / (pHorzBoundaryPoint->thita - pHorzBoundaryPointHist->thita);
                    longiOffsetOnHorzBoundary = slope * (vertPosBottom - pHorzBoundaryPointHist->thita) + pHorzBoundaryPointHist->alpha;
                    break;
                }
                else
                {
                    pHorzBoundaryPoint++;
                    pHorzBoundaryPointHist++;
                }
            }

            leftColOnHorzBound = (cal_yaw - longiOffsetOnHorzBoundary) / horzStep;
            rightColOnHorzBound = (cal_yaw + longiOffsetOnHorzBoundary) / horzStep;
            if (vertPosBottom <= topPoint.thita)
            {
                ERPselectTilesInsideOnOneRow(m_srd, m_tileNumCol, leftColOnHorzBound, rightColOnHorzBound, i); //To be modified with left/right boundary
            }
            else
            {
                leftColOnCurrentLine = (cal_yaw - calculateLongiByLatti(vertPosBottom, topPoint.thita)) / horzStep;
                rightColOnCurrentLine = (cal_yaw + calculateLongiByLatti(vertPosBottom, topPoint.thita)) / horzStep;
                ERPselectTilesInsideOnOneRow(m_srd, m_tileNumCol, leftColOnHorzBound, leftColOnCurrentLine, i); //To be modified with left/right boundary
                ERPselectTilesInsideOnOneRow(m_srd, m_tileNumCol, rightColOnCurrentLine, rightColOnHorzBound, i); //To be modified with left/right boundary
            }
        }
        if (fPitch - vFOV / 2 <= -ERP_VERT_ANGLE / 2)
        {
            for (; i < (int32_t)m_tileNumRow; i++)
            {
                vertPosBottom = m_srd[i * m_tileNumCol].vertPosBottomLeft;
                longiOffsetOnHorzBoundary = 90;
                pHorzBoundaryPoint = m_pViewportHorizontalBoudaryPoints;
                pHorzBoundaryPointHist = pHorzBoundaryPoint;
                pHorzBoundaryPoint++;
                for (int32_t j = 0; j < vFOV / HORZ_BOUNDING_STEP; j++)
                {
                    if (vertPosBottom >= pHorzBoundaryPoint->thita && vertPosBottom <= pHorzBoundaryPointHist->thita)
                    {
                        slope = (pHorzBoundaryPoint->alpha - pHorzBoundaryPointHist->alpha) / (pHorzBoundaryPoint->thita - pHorzBoundaryPointHist->thita);
                        longiOffsetOnHorzBoundary = slope * (vertPosBottom - pHorzBoundaryPointHist->thita) + pHorzBoundaryPointHist->alpha;
                        break;
                    }
                    else
                    {
                        pHorzBoundaryPoint++;
                        pHorzBoundaryPointHist++;
                    }
                }

                leftColOnHorzBound = (cal_yaw - longiOffsetOnHorzBoundary) / horzStep;
                rightColOnHorzBound = (cal_yaw + longiOffsetOnHorzBoundary) / horzStep;
                ERPselectTilesInsideOnOneRow(m_srd, m_tileNumCol, leftColOnHorzBound, rightColOnHorzBound, i);
            }
        }
    }
    else //normal top row between north polar and equator
    {
        topLatti = topLeftPoint.thita;

        if (topLatti == 0)
            topRow = m_tileNumRow / 2;
        else
            topRow = (int32_t)((ERP_VERT_ANGLE / 2 - topLatti) / vertStep);
        leftCol = topLeftPoint.alpha / horzStep;
        rightCol = topRightPoint.alpha / horzStep;

        /* Select tiles on the row of viewport's topLeft/topRight Point */
        ERPselectTilesInsideOnOneRow(m_srd, m_tileNumCol, leftCol, rightCol, topRow);

        /* Select tiles on rows between viewport's topLeft/topRightPoint and topPoint */
        if (topPoint.thita != 0) {
            if (topRow > (ERP_VERT_ANGLE / 2 - topPoint.thita) / vertStep) {
                for (int32_t i = topRow-1; i >= (int32_t)((ERP_VERT_ANGLE / 2 - topPoint.thita) / vertStep); i--)
                {
                    vertPos = m_srd[i * m_tileNumCol].vertPosBottomLeft;

                    leftColOnCurrentLine = (cal_yaw - calculateLongiByLatti(vertPos, topPoint.thita)) / horzStep;
                    rightColOnCurrentLine = (cal_yaw + calculateLongiByLatti(vertPos, topPoint.thita)) / horzStep;
                    if (leftColOnCurrentLine != rightColOnCurrentLine)
                        ERPselectTilesInsideOnOneRow(m_srd, m_tileNumCol, leftColOnCurrentLine, rightColOnCurrentLine, i);
                }
            }
        }
    }

    if (fPitch - vFOV / 2 < -ERP_VERT_ANGLE /2) //Bottom row crosses the south polar
    {
        bottomLatti = bottomLeftPoint.thita;
        bottomRow = (int32_t)((ERP_VERT_ANGLE / 2 - bottomLatti) / vertStep);
        leftCol = bottomLeftPoint.alpha / horzStep;
        rightCol = bottomRightPoint.alpha / horzStep;

        for (int32_t i = m_tileNumRow-1; i >= bottomRow; i--)
        {
            vertPosBottom = m_srd[i * m_tileNumCol].vertPosBottomLeft;
            if (vertPosBottom <= -ERP_VERT_ANGLE - (fPitch - vFOV / 2))
            {
                /* Select all tiles in current row */
                ERPselectTilesInsideOnOneRow(m_srd, m_tileNumCol, 0, m_tileNumCol, i);
            }
            else {
                leftColOnCurrentLine = (cal_yaw - 180 + calculateLongiByLatti(vertPosBottom, -ERP_VERT_ANGLE - (fPitch - vFOV / 2))) / horzStep;
                rightColOnCurrentLine = (cal_yaw + 180 - calculateLongiByLatti(vertPosBottom, -ERP_VERT_ANGLE - (fPitch - vFOV / 2))) / horzStep;
                pHorzBoundaryPoint = m_pViewportHorizontalBoudaryPoints;
                pHorzBoundaryPointHist = pHorzBoundaryPoint;
                pHorzBoundaryPoint++;
                longiOffsetOnHorzBoundary = 90;
                for (int32_t j = 0; j < vFOV / HORZ_BOUNDING_STEP; j++)
                {
                    if (vertPosBottom >= pHorzBoundaryPoint->thita && vertPosBottom <= pHorzBoundaryPointHist->thita)
                    {
                        slope = (pHorzBoundaryPoint->alpha - pHorzBoundaryPointHist->alpha) / (pHorzBoundaryPoint->thita - pHorzBoundaryPointHist->thita);
                        longiOffsetOnHorzBoundary = slope * (vertPosBottom - pHorzBoundaryPointHist->thita) + pHorzBoundaryPointHist->alpha;
                        break;
                    }
                    else
                    {
                        pHorzBoundaryPoint++;
                        pHorzBoundaryPointHist++;
                    }
                }
                leftColOnHorzBound = (cal_yaw - 180 + longiOffsetOnHorzBoundary) / horzStep;
                rightColOnHorzBound = (cal_yaw + 180 - longiOffsetOnHorzBoundary) / horzStep;
                ERPselectTilesInsideOnOneRow(m_srd, m_tileNumCol, leftColOnCurrentLine, leftColOnHorzBound, i);
                ERPselectTilesInsideOnOneRow(m_srd, m_tileNumCol, rightColOnHorzBound, rightColOnCurrentLine, i);
            }
        }
    }
    else if (fPitch - vFOV / 2 > 0) //Bottom row above the equator
    {
        bottomLatti = bottomLeftPoint.thita;
        bottomRow = (int32_t)((ERP_VERT_ANGLE / 2 - bottomLatti) / vertStep);
        leftCol = bottomLeftPoint.alpha / horzStep;
        rightCol = bottomRightPoint.alpha / horzStep;
        int32_t i;
        for (i = bottomRow; i >= (int32_t)((ERP_VERT_ANGLE / 2 - bottomPoint.thita) / vertStep); i--)
        {
            vertPos = m_srd[i * m_tileNumCol].vertPos;
            longiOffsetOnHorzBoundary = 90;
            pHorzBoundaryPoint = m_pViewportHorizontalBoudaryPoints;
            pHorzBoundaryPointHist = pHorzBoundaryPoint;
            pHorzBoundaryPoint++;
            for (int32_t j = 0; j < vFOV / HORZ_BOUNDING_STEP; j++)
            {
                if (vertPos >= pHorzBoundaryPoint->thita && vertPos <= pHorzBoundaryPointHist->thita)
                {
                    slope = (pHorzBoundaryPoint->alpha - pHorzBoundaryPointHist->alpha) / (pHorzBoundaryPoint->thita - pHorzBoundaryPointHist->thita);
                    longiOffsetOnHorzBoundary = slope * (vertPos - pHorzBoundaryPointHist->thita) + pHorzBoundaryPointHist->alpha;
                    break;
                }
                else
                {
                    pHorzBoundaryPoint++;
                    pHorzBoundaryPointHist++;
                }
            }

            leftColOnHorzBound = (cal_yaw - longiOffsetOnHorzBoundary) / horzStep;
            rightColOnHorzBound = (cal_yaw + longiOffsetOnHorzBoundary) / horzStep;
            if (vertPos >= bottomPoint.thita)
            {
                ERPselectTilesInsideOnOneRow(m_srd, m_tileNumCol, leftColOnHorzBound, rightColOnHorzBound, i);
            }
            else {
                leftColOnCurrentLine = (cal_yaw - calculateLongiByLatti(vertPos, bottomPoint.thita)) / horzStep;
                rightColOnCurrentLine = (cal_yaw + calculateLongiByLatti(vertPos, bottomPoint.thita)) / horzStep;
                ERPselectTilesInsideOnOneRow(m_srd, m_tileNumCol, leftColOnHorzBound, leftColOnCurrentLine, i);
                ERPselectTilesInsideOnOneRow(m_srd, m_tileNumCol, rightColOnCurrentLine, rightColOnHorzBound, i);
            }
        }
        if (fPitch + vFOV / 2 >= ERP_VERT_ANGLE / 2)
        {
            for (; i >= 0; i--)
            {
                vertPos = m_srd[i * m_tileNumCol].vertPos;
                longiOffsetOnHorzBoundary = 90;
                pHorzBoundaryPoint = m_pViewportHorizontalBoudaryPoints;
                pHorzBoundaryPointHist = pHorzBoundaryPoint;
                pHorzBoundaryPoint++;
                for (int32_t j = 0; j < vFOV / HORZ_BOUNDING_STEP; j++)
                {
                    if (vertPos >= pHorzBoundaryPoint->thita && vertPos <= pHorzBoundaryPointHist->thita)
                    {
                        slope = (pHorzBoundaryPoint->alpha - pHorzBoundaryPointHist->alpha) / (pHorzBoundaryPoint->thita - pHorzBoundaryPointHist->thita);
                        longiOffsetOnHorzBoundary = slope * (vertPos - pHorzBoundaryPointHist->thita) + pHorzBoundaryPointHist->alpha;
                        break;
                    }
                    else
                    {
                        pHorzBoundaryPoint++;
                        pHorzBoundaryPointHist++;
                    }
                }

                leftColOnHorzBound = (cal_yaw - longiOffsetOnHorzBoundary) / horzStep;
                rightColOnHorzBound = (cal_yaw + longiOffsetOnHorzBoundary) / horzStep;
                ERPselectTilesInsideOnOneRow(m_srd, m_tileNumCol, leftColOnHorzBound, rightColOnHorzBound, i);
            }
	}
    }
    else { //normal bottom row between south polar and equator
        bottomLatti = bottomLeftPoint.thita;
        if (bottomLatti == 0)
            bottomRow = m_tileNumRow / 2 - 1;
        else
            bottomRow = (int32_t)((ERP_VERT_ANGLE / 2 - bottomLatti) / vertStep);
        leftCol = bottomLeftPoint.alpha / horzStep;
        rightCol = bottomRightPoint.alpha / horzStep;

        /* Select tiles on the row of viewport's bottomLeft/bottomRight Point */
        ERPselectTilesInsideOnOneRow(m_srd, m_tileNumCol, leftCol, rightCol, bottomRow);

        /* Select tiles on rows between viewport's bottomLeft/bottomRight and bottomPoint */
        if (bottomPoint.thita != 0) {
            if (bottomRow < (ERP_VERT_ANGLE / 2 - bottomPoint.thita) / vertStep) {
                for (int32_t i = bottomRow+1; i <= (int32_t)((ERP_VERT_ANGLE / 2 - bottomPoint.thita) / vertStep); i++)
                {
                    vertPos = m_srd[i * m_tileNumCol].vertPos;
                    leftColOnCurrentLine = (cal_yaw - calculateLongiByLatti(vertPos, bottomPoint.thita)) / horzStep;
                    rightColOnCurrentLine = (cal_yaw + calculateLongiByLatti(vertPos, bottomPoint.thita)) / horzStep;
                    if (leftColOnCurrentLine != rightColOnCurrentLine)
                        ERPselectTilesInsideOnOneRow(m_srd, m_tileNumCol, leftColOnCurrentLine, rightColOnCurrentLine, i);
                }
            }
        }
    }

    if (topRow != bottomRow)
    {
        int32_t startRow, endRow;
        if (fPitch > 0) {
            /* The topLeft point has the biggest longitude offset for the topRow, then start from topRow+1 */
            startRow = topRow + 1;
            endRow = bottomRow;
        }
        else {
            /* The bottomLeft point has the biggest longitude offset for the bottomRow, so end at bottomRow-1 */
            startRow = topRow;
            endRow = bottomRow - 1;
        }
        for (int32_t i = startRow; i <= endRow; i++)
        {
            if (fPitch > 0)
               vertPos = m_srd[i * m_tileNumCol].vertPos;
            else
               vertPos = m_srd[i * m_tileNumCol].vertPosBottomLeft;

            pHorzBoundaryPoint = m_pViewportHorizontalBoudaryPoints;
            pHorzBoundaryPointHist = pHorzBoundaryPoint;
            pHorzBoundaryPoint++;
            longiOffsetOnHorzBoundary = 90;
            for (int32_t j = 0; j < vFOV / HORZ_BOUNDING_STEP; j++)
            {
                if (vertPos >= pHorzBoundaryPoint->thita && vertPos <= pHorzBoundaryPointHist->thita)
                {
                    slope = (pHorzBoundaryPoint->alpha - pHorzBoundaryPointHist->alpha) / (pHorzBoundaryPoint->thita - pHorzBoundaryPointHist->thita);
                    longiOffsetOnHorzBoundary = slope * (vertPos - pHorzBoundaryPointHist->thita) + pHorzBoundaryPointHist->alpha;
                    break;
                }
                else
                {
                    pHorzBoundaryPoint++;
                    pHorzBoundaryPointHist++;
                }
            }

            leftColOnHorzBound = (cal_yaw - longiOffsetOnHorzBoundary) / horzStep;
            rightColOnHorzBound = (cal_yaw + longiOffsetOnHorzBoundary) / horzStep;
            ERPselectTilesInsideOnOneRow(m_srd, m_tileNumCol, leftColOnHorzBound, rightColOnHorzBound, i); //To be modified with left/right boundary
        }
    }

    dResult = (double)(clock() - lBefore) / CLOCKS_PER_SEC;
    SCVP_LOG(LOG_INFO, "Total Time for tile selection: %f s\n", dResult);
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
    if (pcCodingGeomtry == NULL) {
        SAFE_DELETE(pcInputGeomtry);
        return ERROR_INVALID;
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
    SCVP_LOG(LOG_INFO, "Total Time: %f second. \n", dResult);

    SAFE_DELETE(pcInputGeomtry);
    SAFE_DELETE(pcCodingGeomtry);
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

int32_t TgenViewport::CubemapGetFaceBoundaryCrossingPoints(SpherePoint* upLeftPoint, SpherePoint* downRightPoint, int32_t faceWidth, int32_t faceHeight, std::list<SpherePoint>* crossBoundaryPoints)
{
    SpherePoint crossUpLeftPoint, crossDownRightPoint;
    POSType newAX, newAY;

    if ( !upLeftPoint || !downRightPoint) {
        SCVP_LOG(LOG_ERROR, "The given sphere point is NULL!\n");
        return ERROR_NULL_PTR;
    }
    if (faceWidth <= 0 || faceHeight <= 0) {
        SCVP_LOG(LOG_ERROR, "The face width/height is less than 0!\n");
        return ERROR_INVALID;
    }

    /* No crossing point should be added when the given two points are inside the same face */
    if (upLeftPoint->cord2D.faceIdx == downRightPoint->cord2D.faceIdx)
        return ERROR_NONE;

    /* CrossPointA is the coordinate on face A, *
     * CrossPointB is the coordinate on face B  */
    crossUpLeftPoint.cord2D.faceIdx = upLeftPoint->cord2D.faceIdx;
    crossDownRightPoint.cord2D.faceIdx = downRightPoint->cord2D.faceIdx;
    newAX = 0;
    newAY = 0;
    if (upLeftPoint->cord2D.faceIdx == FACE_PY) {
        /* In case the top point locates in top face */
        switch (downRightPoint->cord2D.faceIdx) {
        case FACE_PX:
            newAX = faceHeight - upLeftPoint->cord2D.y;
            newAY = upLeftPoint->cord2D.x;
            break;
        case FACE_NX:
            newAX = upLeftPoint->cord2D.y;
            newAY = faceWidth - upLeftPoint->cord2D.x;
            break;
        case FACE_PZ:
            newAX = upLeftPoint->cord2D.x;
            newAY = upLeftPoint->cord2D.y;
            break;
        case FACE_NZ:
            newAX = faceWidth - upLeftPoint->cord2D.x;
            newAY = faceHeight - upLeftPoint->cord2D.y;
        default:break;
        }
        float slope = (newAX - downRightPoint->cord2D.x) / (newAY - downRightPoint->cord2D.y - faceWidth);
        crossDownRightPoint.cord2D.x = -slope * downRightPoint->cord2D.y + downRightPoint->cord2D.x;
        crossDownRightPoint.cord2D.y = 0;
        switch (downRightPoint->cord2D.faceIdx) {
        case FACE_PX:
            crossUpLeftPoint.cord2D.x = faceWidth;
            crossUpLeftPoint.cord2D.y = faceHeight - crossDownRightPoint.cord2D.x;
            break;
        case FACE_NX:
            crossUpLeftPoint.cord2D.x = 0;
            crossUpLeftPoint.cord2D.y = crossDownRightPoint.cord2D.x;
            break;
        case FACE_PZ:
            crossUpLeftPoint.cord2D.x = crossDownRightPoint.cord2D.x;
            crossUpLeftPoint.cord2D.y = faceHeight;
            break;
        case FACE_NZ:
            crossUpLeftPoint.cord2D.x = faceWidth - crossDownRightPoint.cord2D.x;
            crossUpLeftPoint.cord2D.y = 0;
	    break;
        default:break;
        }
    }
    else if (downRightPoint->cord2D.faceIdx == FACE_PY) {
        /* In case the top point locates in top face */
        switch (upLeftPoint->cord2D.faceIdx) {
        case FACE_PX:
            newAX = faceHeight - downRightPoint->cord2D.y;
            newAY = downRightPoint->cord2D.x;
            break;
        case FACE_NX:
            newAX = downRightPoint->cord2D.y;
            newAY = faceWidth - downRightPoint->cord2D.x;
            break;
	case FACE_PZ:
            newAX = downRightPoint->cord2D.x;
            newAY = downRightPoint->cord2D.y;
            break;
        case FACE_NZ:
            newAX = faceWidth - downRightPoint->cord2D.x;
            newAY = faceHeight - downRightPoint->cord2D.y;
            break;
        default:break;
        }
        float slope = (newAX - upLeftPoint->cord2D.x) / (newAY - upLeftPoint->cord2D.y - faceWidth);
        crossUpLeftPoint.cord2D.x = slope * (-upLeftPoint->cord2D.y) + upLeftPoint->cord2D.x;
        crossUpLeftPoint.cord2D.y = 0;
        switch (upLeftPoint->cord2D.faceIdx) {
        case FACE_PX:
            crossDownRightPoint.cord2D.x = faceWidth;
            crossDownRightPoint.cord2D.y = faceHeight - crossUpLeftPoint.cord2D.x;
            break;
        case FACE_NX:
            crossDownRightPoint.cord2D.x = 0;
            crossDownRightPoint.cord2D.y = crossUpLeftPoint.cord2D.x;
            break;
        case FACE_PZ:
            crossDownRightPoint.cord2D.x = crossUpLeftPoint.cord2D.x;
            crossDownRightPoint.cord2D.y = faceHeight;
            break;
        case FACE_NZ:
            crossDownRightPoint.cord2D.x = faceWidth - crossUpLeftPoint.cord2D.x;
            crossDownRightPoint.cord2D.y = 0;
            break;
        default:break;
        }
    }
    /* In case the bottom point locates in bottom face */
    else if (downRightPoint->cord2D.faceIdx == FACE_NY) {
        switch (upLeftPoint->cord2D.faceIdx) {
        case FACE_PX:
            newAX = downRightPoint->cord2D.y;
            newAY = faceWidth - downRightPoint->cord2D.x;
            break;
        case FACE_NX:
            newAX = faceWidth - downRightPoint->cord2D.y;
            newAY = downRightPoint->cord2D.x;
            break;
        case FACE_PZ:
            newAX = downRightPoint->cord2D.x;
            newAY = downRightPoint->cord2D.y;
            break;
        case FACE_NZ:
            newAX = faceWidth - downRightPoint->cord2D.x;
            newAY = faceHeight - downRightPoint->cord2D.y;
            break;
        default:break;
        }
        float slope = (upLeftPoint->cord2D.x - newAX) / (upLeftPoint->cord2D.y - faceHeight - newAY);
        crossUpLeftPoint.cord2D.x = slope * (faceHeight - upLeftPoint->cord2D.y) + upLeftPoint->cord2D.x;
        crossUpLeftPoint.cord2D.y = faceHeight;
        switch (upLeftPoint->cord2D.faceIdx) {
        case FACE_PX:
            crossDownRightPoint.cord2D.x = faceWidth;
            crossDownRightPoint.cord2D.y = crossUpLeftPoint.cord2D.x;
            break;
        case FACE_NX:
            crossDownRightPoint.cord2D.x = 0;
            crossDownRightPoint.cord2D.y = faceHeight - crossUpLeftPoint.cord2D.x;
            break;
        case FACE_PZ:
            crossDownRightPoint.cord2D.x = crossUpLeftPoint.cord2D.x;
            crossDownRightPoint.cord2D.y = 0;
            break;
        case FACE_NZ:
            crossDownRightPoint.cord2D.x = faceWidth - crossUpLeftPoint.cord2D.x;
            crossDownRightPoint.cord2D.y = faceHeight;
            break;
        default:break;
        }
    }
    else if (upLeftPoint->cord2D.faceIdx == FACE_NY) {
        switch (downRightPoint->cord2D.faceIdx) {
        case FACE_PX:
            newAX = upLeftPoint->cord2D.y;
            newAY = faceWidth - upLeftPoint->cord2D.x;
            break;
        case FACE_NX:
            newAX = faceWidth - upLeftPoint->cord2D.y;
            newAY = upLeftPoint->cord2D.x;
            break;
        case FACE_PZ:
            newAX = upLeftPoint->cord2D.x;
            newAY = upLeftPoint->cord2D.y;
            break;
        case FACE_NZ:
            newAX = faceWidth - upLeftPoint->cord2D.x;
            newAY = faceHeight - upLeftPoint->cord2D.y;
            break;
        default:break;
        }
        float slope = (downRightPoint->cord2D.x - newAX) / (downRightPoint->cord2D.y - faceHeight - newAY);
        crossDownRightPoint.cord2D.x = slope * (faceHeight - downRightPoint->cord2D.y) + downRightPoint->cord2D.x;
        crossDownRightPoint.cord2D.y = faceHeight;
        switch (downRightPoint->cord2D.faceIdx) {
        case FACE_PX:
            crossUpLeftPoint.cord2D.x = faceWidth;
            crossUpLeftPoint.cord2D.y = crossDownRightPoint.cord2D.x;
            break;
        case FACE_NX:
            crossUpLeftPoint.cord2D.x = 0;
            crossUpLeftPoint.cord2D.y = faceHeight - crossDownRightPoint.cord2D.x;
            break;
        case FACE_PZ:
            crossUpLeftPoint.cord2D.x = crossDownRightPoint.cord2D.x;
            crossUpLeftPoint.cord2D.y = 0;
            break;
        case FACE_NZ:
            crossUpLeftPoint.cord2D.x = faceWidth - crossDownRightPoint.cord2D.x;
            crossUpLeftPoint.cord2D.y = faceHeight;
            break;
        default:break;
        }
    }
    /* In case the two points are not in the same face */
    else if (upLeftPoint->cord2D.faceIdx != downRightPoint->cord2D.faceIdx) {
        if (((upLeftPoint->cord2D.faceIdx == FACE_PX) && (downRightPoint->cord2D.faceIdx == FACE_NZ))
            || ((upLeftPoint->cord2D.faceIdx == FACE_NZ) && (downRightPoint->cord2D.faceIdx == FACE_NX))
            || ((upLeftPoint->cord2D.faceIdx == FACE_NX) && (downRightPoint->cord2D.faceIdx == FACE_PZ))
            || ((upLeftPoint->cord2D.faceIdx == FACE_PZ) && (downRightPoint->cord2D.faceIdx == FACE_PX))) {
            float slope = (downRightPoint->cord2D.y - upLeftPoint->cord2D.y) / (downRightPoint->cord2D.x + faceWidth - upLeftPoint->cord2D.x);
            crossUpLeftPoint.cord2D.x = faceWidth;
            crossUpLeftPoint.cord2D.y = slope * (faceWidth - upLeftPoint->cord2D.x) + upLeftPoint->cord2D.y;
            crossDownRightPoint.cord2D.x = 0;
            crossDownRightPoint.cord2D.y = crossUpLeftPoint.cord2D.y;
        }
        else {
            float slope = (upLeftPoint->cord2D.y - downRightPoint->cord2D.y) / (upLeftPoint->cord2D.x + faceWidth - downRightPoint->cord2D.x);
            crossDownRightPoint.cord2D.x = faceWidth;
            crossDownRightPoint.cord2D.y = slope * (faceWidth - downRightPoint->cord2D.x) + downRightPoint->cord2D.y;
            crossUpLeftPoint.cord2D.x = 0;
            crossUpLeftPoint.cord2D.y = crossDownRightPoint.cord2D.y;
        }
    }
    else {
        return ERROR_NONE;
    }
    crossBoundaryPoints->push_back(crossUpLeftPoint);
    crossBoundaryPoints->push_back(crossDownRightPoint);
    return ERROR_NONE;
}

int32_t TgenViewport::CubemapGetViewportProjInFace(int32_t faceId, std::list<SpherePoint>* Points)
{
    SpherePoint point;
    int32_t tileWidth = m_srd[0].tilewidth;
    int32_t tileHeight = m_srd[0].tileheight;
    SPos* pTmpUpLeft = &m_pUpLeft[faceId];
    SPos* pTmpDownRight = &m_pDownRight[faceId];

    if (Points->size() == 0) {
        SCVP_LOG(LOG_WARNING, "Cubemap viewport projection reference points list is NULL!\n")
	return ERROR_NO_VALUE;
    }
    pTmpUpLeft->x = m_sourceSVideoInfo.iFaceWidth;
    pTmpUpLeft->y = m_sourceSVideoInfo.iFaceHeight;
    pTmpDownRight->x = 0;
    pTmpDownRight->y = 0;
    for (auto it = Points->begin(); it != Points->end(); it++) {
        point = *it;
        if (point.cord2D.faceIdx == faceId) {
            pTmpUpLeft->x = fmin(point.cord2D.x, pTmpUpLeft->x);
            pTmpUpLeft->y = fmin(point.cord2D.y, pTmpUpLeft->y);
            pTmpDownRight->x = fmax(point.cord2D.x, pTmpDownRight->x);
            pTmpDownRight->y = fmax(point.cord2D.y, pTmpDownRight->y);
        }
    }
    for (uint32_t i = 0; i < m_tileNumRow; i++) {
        for (uint32_t j = 0; j < m_tileNumCol; j++) {
            if ((pTmpUpLeft->x < (int32_t)((j + 1) * tileWidth)) && (pTmpUpLeft->y < (int32_t)((i + 1) * tileHeight))
                && ((pTmpDownRight->x >= (int32_t)(j * tileWidth))) && (pTmpDownRight->y >= (int32_t)(i * tileHeight)))
            {
                m_srd[faceId * m_tileNumRow * m_tileNumCol + i * m_tileNumCol + j].isOccupy = 1;
                m_srd[faceId * m_tileNumRow * m_tileNumCol + i * m_tileNumCol + j].faceId = faceId;
            }
        }
    }
    pTmpUpLeft->faceIdx = faceId;
    pTmpDownRight->faceIdx = faceId;
    return ERROR_NONE;
}


int32_t TgenViewport::CubemapPolar2Cartesian(SpherePoint* pPoint)
{
    if (!pPoint) {
        SCVP_LOG(LOG_ERROR, "The input spherer point is NULL!\n");
	return ERROR_NULL_PTR;
    }
    pPoint->cord3D.x = scos(pPoint->thita * DEG2RAD_FACTOR) * scos(pPoint->alpha * DEG2RAD_FACTOR);
    pPoint->cord3D.y = ssin(pPoint->thita * DEG2RAD_FACTOR);
    pPoint->cord3D.z = -scos(pPoint->thita * DEG2RAD_FACTOR) * ssin(pPoint->alpha * DEG2RAD_FACTOR);

    POSType aX = sfabs(pPoint->cord3D.x);
    POSType aY = sfabs(pPoint->cord3D.y);
    POSType aZ = sfabs(pPoint->cord3D.z);
    POSType pu, pv;
    if (aX >= aY && aX >= aZ)
    {
        if (pPoint->cord3D.x > 0)
        {
            pPoint->cord2D.faceIdx = FACE_PX;
            pu = -pPoint->cord3D.z / aX;
            pv = -pPoint->cord3D.y / aX;
        }
        else
        {
            pPoint->cord2D.faceIdx = FACE_NX;
            pu = pPoint->cord3D.z / aX;
            pv = -pPoint->cord3D.y / aX;
        }
    }
    else if (aY >= aX && aY >= aZ)
    {
        if (pPoint->cord3D.y > 0)
        {
            pPoint->cord2D.faceIdx = FACE_PY;
            pu = pPoint->cord3D.x / aY;
            pv = pPoint->cord3D.z / aY;
        }
        else
        {
            pPoint->cord2D.faceIdx = FACE_NY;
            pu = pPoint->cord3D.x / aY;
            pv = -pPoint->cord3D.z / aY;
        }
    }
    else
    {
        if (pPoint->cord3D.z > 0)
        {
            pPoint->cord2D.faceIdx = FACE_PZ;
            pu = pPoint->cord3D.x / aZ;
            pv = -pPoint->cord3D.y / aZ;
        }
        else
        {
            pPoint->cord2D.faceIdx = FACE_NZ;
            pu = -pPoint->cord3D.x / aZ;
            pv = -pPoint->cord3D.y / aZ;
        }
    }
    //convert pu, pv to [0, width], [0, height];
    pPoint->cord2D.z = 0;
    pPoint->cord2D.x = (POSType)((pu + 1.0) * (m_sourceSVideoInfo.iFaceWidth >> 1) + (-0.5));
    pPoint->cord2D.y = (POSType)((pv + 1.0) * (m_sourceSVideoInfo.iFaceHeight >> 1) + (-0.5));

    return ERROR_NONE;
}

int32_t TgenViewport::CubemapIsInsideFaces()
{
    int32_t faceWidth = m_sourceSVideoInfo.iFaceWidth;
    int32_t faceHeight = m_sourceSVideoInfo.iFaceHeight;

    float fPitch = m_codingSVideoInfo.viewPort.fPitch;
    float fYaw = m_codingSVideoInfo.viewPort.fYaw;
    float hFOV = m_codingSVideoInfo.viewPort.hFOV;
    float vFOV = m_codingSVideoInfo.viewPort.vFOV;
    SpherePoint topPoint, bottomPoint, leftPoint, rightPoint;
    SpherePoint centerPoint;
    SpherePoint topLeftPoint, topRightPoint, bottomLeftPoint, bottomRightPoint;

    int32_t selectedTilesNum = 0;
    int32_t idx;
    std::list<SpherePoint> referencePoints;

    double dResult;
    clock_t lBefore = clock();

    float cal_yaw = fYaw + ERP_HORZ_ANGLE / 2;
    /* Calculate the topLeft point lattitude when yaw=pitch=0 */
    float thita = calculateLatti(vFOV / 2, hFOV);
    /* Phi is half of the open angle of the topLeft/topRight point with the sphere center, which won't change under different pitch/yaw */
    float phi = sacos(ssin(thita * DEG2RAD_FACTOR) / ssin((vFOV / 2) * DEG2RAD_FACTOR)) * RAD2DEG_FACTOR;
    /* Calculate the topLeft/topRight point position with current pitch */
    topLeftPoint.thita = topRightPoint.thita = sasin(scos(phi * DEG2RAD_FACTOR) * ssin((fPitch + vFOV / 2) * DEG2RAD_FACTOR)) * RAD2DEG_FACTOR;
    float tempValue = ssin(phi * DEG2RAD_FACTOR) / scos(topLeftPoint.thita * DEG2RAD_FACTOR);
    if (tempValue > 1)
        tempValue = 1;
    else if (tempValue < -1)
        tempValue = -1;
    topLeftPoint.alpha = cal_yaw - fabs(sasin(tempValue) * RAD2DEG_FACTOR);
    topRightPoint.alpha = cal_yaw + fabs(sasin(tempValue) * RAD2DEG_FACTOR);
    bottomLeftPoint.thita = bottomRightPoint.thita = sasin(scos(phi * DEG2RAD_FACTOR) * ssin((fPitch - vFOV / 2) * DEG2RAD_FACTOR)) * RAD2DEG_FACTOR;
    tempValue = ssin(phi * DEG2RAD_FACTOR) / scos(bottomLeftPoint.thita * DEG2RAD_FACTOR);
    if (tempValue > 1)
        tempValue = 1;
    else if (tempValue < -1)
        tempValue = -1;
    bottomLeftPoint.alpha = cal_yaw - fabs(sasin(tempValue) * RAD2DEG_FACTOR);
    bottomRightPoint.alpha = cal_yaw + fabs(sasin(tempValue) * RAD2DEG_FACTOR);

    /* Calculate coordinates of each viewport vertex on the cube faces */
    if (fPitch + vFOV / 2 >= ERP_VERT_ANGLE / 2) {
        float fTmp = topRightPoint.alpha;
        topRightPoint.alpha = topLeftPoint.alpha;
        topLeftPoint.alpha = fTmp;
    }
    else {
        topLeftPoint.alpha -= ERP_HORZ_ANGLE / 2;
        topRightPoint.alpha -= ERP_HORZ_ANGLE / 2;
    }
    if (fPitch - vFOV / 2 <= -ERP_VERT_ANGLE / 2) {
        float fTmp = bottomRightPoint.alpha;
        bottomRightPoint.alpha = bottomLeftPoint.alpha;
        bottomLeftPoint.alpha = fTmp;
    }
    else {
        bottomLeftPoint.alpha -= ERP_HORZ_ANGLE / 2;
        bottomRightPoint.alpha -= ERP_HORZ_ANGLE / 2;
    }

    CubemapPolar2Cartesian(&topLeftPoint);
    CubemapPolar2Cartesian(&topRightPoint);
    CubemapPolar2Cartesian(&bottomLeftPoint);
    CubemapPolar2Cartesian(&bottomRightPoint);

    /* Calculate top/bottom point position */
    topPoint.thita = fPitch + vFOV / 2;
    bottomPoint.thita = fPitch - vFOV / 2;
    topPoint.alpha = bottomPoint.alpha = fYaw;
    CubemapPolar2Cartesian(&topPoint);
    CubemapPolar2Cartesian(&bottomPoint);

    /* Calculate Left/Right point */
    leftPoint.thita = rightPoint.thita = sasin(scos(hFOV / 2 * DEG2RAD_FACTOR) * ssin(fPitch  * DEG2RAD_FACTOR)) * RAD2DEG_FACTOR;
    leftPoint.alpha = fYaw - fabs(sasin(ssin(hFOV / 2 * DEG2RAD_FACTOR) / scos(leftPoint.thita * DEG2RAD_FACTOR)) * RAD2DEG_FACTOR);
    rightPoint.alpha = fYaw + fabs(sasin(ssin(hFOV / 2 * DEG2RAD_FACTOR) / scos(rightPoint.thita * DEG2RAD_FACTOR)) * RAD2DEG_FACTOR);
    if (leftPoint.alpha < -ERP_HORZ_ANGLE / 2)
        leftPoint.alpha += ERP_HORZ_ANGLE;
    if (rightPoint.alpha > ERP_HORZ_ANGLE / 2)
        rightPoint.alpha -= ERP_HORZ_ANGLE;
    CubemapPolar2Cartesian(&leftPoint);
    CubemapPolar2Cartesian(&rightPoint);

    /* Calculate center point position */
    centerPoint.thita = fPitch;
    centerPoint.alpha = fYaw;
    CubemapPolar2Cartesian(&centerPoint);

    /* Get viewport horizontal boundary points */
    SpherePoint* pHorzBoundaryPoint = m_pViewportHorizontalBoudaryPoints;
    for (float offsetAngle = vFOV / 2; offsetAngle >= -vFOV / 2; offsetAngle -= HORZ_BOUNDING_STEP)
    {
        float instantThita = calculateLatti(offsetAngle, hFOV);
        float instantPhi;
        if (fabs(offsetAngle) <= 1e-9)
            instantPhi = hFOV / 2;
        else
            instantPhi = sacos(ssin(instantThita * DEG2RAD_FACTOR) / ssin((fabs(offsetAngle)) * DEG2RAD_FACTOR)) * RAD2DEG_FACTOR;
        pHorzBoundaryPoint->thita = calculateLattitudeFromPhi(instantPhi, fPitch + offsetAngle);
        pHorzBoundaryPoint->alpha = calculateLongitudeFromThita(pHorzBoundaryPoint->thita, instantPhi, hFOV / 2);
        pHorzBoundaryPoint->alpha = fYaw + pHorzBoundaryPoint->alpha;
        CubemapPolar2Cartesian(pHorzBoundaryPoint);
        referencePoints.push_back(*pHorzBoundaryPoint);
        pHorzBoundaryPoint->alpha -= fYaw;
        pHorzBoundaryPoint->alpha = fYaw - pHorzBoundaryPoint->alpha;
        CubemapPolar2Cartesian(pHorzBoundaryPoint);
        referencePoints.push_back(*pHorzBoundaryPoint);
        pHorzBoundaryPoint++;
    }

    /* Reset viewport area */
    for (int32_t i = 0; i < FACE_NUMBER; i++) {
        m_pUpLeft[i].faceIdx = -1;
        m_pDownRight[i].faceIdx = -1;
    }
    for (idx = 0; idx < (int32_t)(FACE_NUMBER * m_tileNumRow * m_tileNumCol); idx++) {
        m_srd[idx].faceId = -1;
        m_srd[idx].isOccupy = 0;
    }

    /* Calculate the crossing points on     *
     * each face if the neighbor reference  *
     * points are not in the same face      */
    CubemapGetFaceBoundaryCrossingPoints(&topLeftPoint,    &topPoint,         faceWidth, faceHeight, &referencePoints);
    CubemapGetFaceBoundaryCrossingPoints(&topPoint,        &topRightPoint,    faceWidth, faceHeight, &referencePoints);
    CubemapGetFaceBoundaryCrossingPoints(&topLeftPoint,    &leftPoint,        faceWidth, faceHeight, &referencePoints);
    CubemapGetFaceBoundaryCrossingPoints(&leftPoint,       &bottomLeftPoint,  faceWidth, faceHeight, &referencePoints);
    CubemapGetFaceBoundaryCrossingPoints(&bottomLeftPoint, &bottomPoint,      faceWidth, faceHeight, &referencePoints);
    CubemapGetFaceBoundaryCrossingPoints(&bottomPoint,     &bottomRightPoint, faceWidth, faceHeight, &referencePoints);
    CubemapGetFaceBoundaryCrossingPoints(&topRightPoint,   &rightPoint,       faceWidth, faceHeight, &referencePoints);
    CubemapGetFaceBoundaryCrossingPoints(&rightPoint,      &bottomRightPoint, faceWidth, faceHeight, &referencePoints);
    CubemapGetFaceBoundaryCrossingPoints(&leftPoint,       &centerPoint,      faceWidth, faceHeight, &referencePoints);
    CubemapGetFaceBoundaryCrossingPoints(&topPoint,        &centerPoint,      faceWidth, faceHeight, &referencePoints);
    CubemapGetFaceBoundaryCrossingPoints(&centerPoint,     &rightPoint,       faceWidth, faceHeight, &referencePoints);
    CubemapGetFaceBoundaryCrossingPoints(&centerPoint,     &bottomPoint,      faceWidth, faceHeight, &referencePoints);
    CubemapGetFaceBoundaryCrossingPoints(&topLeftPoint,    &bottomLeftPoint,  faceWidth, faceHeight, &referencePoints);
    CubemapGetFaceBoundaryCrossingPoints(&topRightPoint,   &bottomRightPoint, faceWidth, faceHeight, &referencePoints);

    /* The viewport bounding points are also useful for tile selection */
    referencePoints.push_back(topLeftPoint);
    referencePoints.push_back(topPoint);
    referencePoints.push_back(leftPoint);
    referencePoints.push_back(bottomLeftPoint);
    referencePoints.push_back(bottomPoint);
    referencePoints.push_back(bottomRightPoint);
    referencePoints.push_back(rightPoint);
    referencePoints.push_back(topRightPoint);
    referencePoints.push_back(centerPoint);

    /* Get the tile selection on each face */
    for (int32_t faceIdx = 0; faceIdx < FACE_NUMBER; faceIdx++)
        CubemapGetViewportProjInFace(faceIdx, &referencePoints);

    ITileInfo* pTileInfoTmp = m_srd;
    int32_t faceNum = (m_sourceSVideoInfo.geoType == SVIDEO_CUBEMAP) ? 6 : 1;
    for (int32_t faceid = 0; faceid < faceNum; faceid++)
    {
        SCVP_LOG(LOG_INFO, "Face is %d\n", faceid);
        for (uint32_t row = 0; row < m_tileNumRow; row++)
        {
            for (uint32_t col = 0; col < m_tileNumCol; col++)
            {
                if (pTileInfoTmp->isOccupy) {
                    SCVP_LOG(LOG_INFO, "Selecte Tile at row %d and col %d\n", row, col);
                    selectedTilesNum++;
                }
                pTileInfoTmp++;
            }
        }
    }
    SCVP_LOG(LOG_INFO, "Total Selected Tile Number is %d\n", selectedTilesNum);

    referencePoints.clear();
    dResult = (double)(clock() - lBefore) / CLOCKS_PER_SEC;
    SCVP_LOG(LOG_INFO, "Total Time for tile selection: %f ms to find inside tile number is %d\n", dResult, selectedTilesNum);

    return selectedTilesNum;
}
int32_t TgenViewport::calcTilesInViewport(ITileInfo* pTileInfo, int32_t tileCol, int32_t tileRow)
{
    if (!pTileInfo)
        return -1;
    int32_t ret = 0;
    ITileInfo *pTileInfoTmp = pTileInfo;
    int32_t faceNum = (m_sourceSVideoInfo.geoType==SVIDEO_CUBEMAP) ? 6 : 1;
    if (m_sourceSVideoInfo.geoType == SVIDEO_EQUIRECT) {
    for (int32_t faceid = 0; faceid < faceNum; faceid++)
    {
        for (int32_t row = 0; row < tileRow; row++)
        {
            for (int32_t col = 0; col < tileCol; col++)
            {
                //pTileInfoTmp->isOccupy = isInside(pTileInfoTmp->x, pTileInfoTmp->y, pTileInfoTmp->tilewidth, pTileInfoTmp->tileheight, faceid);
                if (pTileInfoTmp->isOccupy == 1)
                    ret++;
                pTileInfoTmp++;
            }
        }
    }
    }
    else if (m_sourceSVideoInfo.geoType == SVIDEO_CUBEMAP) {
        ret = CubemapIsInsideFaces();
    }
    return ret;
}

int32_t TgenViewport::getContentCoverage(CCDef* pOutCC, int32_t coverageShapeType) {
    int32_t ret;
    /* Shape Type:
     * 0: CubeMap
     * 1: ERP
     */
    switch (coverageShapeType) {
    case 0:
        pOutCC->azimuthRange = m_codingSVideoInfo.viewPort.hFOV * 65536.f;
        pOutCC->elevationRange = m_codingSVideoInfo.viewPort.vFOV * 65536.f;
        pOutCC->centreAzimuth = m_codingSVideoInfo.viewPort.fYaw * 65536.f;
        pOutCC->centreElevation = m_codingSVideoInfo.viewPort.fPitch * 65536.f;
        ret = 0;
        break;
    case 1:
        /* TBD: implemented by Yaw/Pitch/hFOV/vFOV directly
         * double hFOVInRadian, pitchInRadian;
         * hFOVInRadian = m_codingSVideoInfo.viewPort.hFOV / 180.f * S_PI;
         * pitchInRadian = m_codingSVideoInfo.viewPort.fPitch / 180.f * S_PI;

         * pOutCC->azimuthRange = sasin(sfabs(ssin(hFOVInRadian/2) / scos(pitchInRadian/2))) / S_PI * 360.f * 65536.f;
         * pOutCC->elevationRange = m_codingSVideoInfo.viewPort.vFOV * 65536.f;
         * pOutCC->centreAzimuth = m_codingSVideoInfo.viewPort.fYaw * 65536.f;
         * pOutCC->centreElevation = m_codingSVideoInfo.viewPort.fPitch * 65536.f;
         */
        SCVP_LOG(LOG_WARNING, "Doesnt' Support to Get CC by Viewport Settings Directly for Shape Type 1\n");
        ret = -1;
        break;
    default:
        SCVP_LOG(LOG_WARNING, "Coverage type must be 0 or 1\n");
        ret = -1;
        break;
    }
    return ret;
}
