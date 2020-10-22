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
//! \file:   HighResPlusFullLowResPacking.cpp
//! \brief:  Region wise packing generator class implementation for packing of
//!          high resolution video plus full low resolution video
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include <math.h>
#include <string.h>

#include "../../../utils/error.h"
#include "../PackingPluginLog.h"
#include "HighResPlusFullLowResPacking.h"

HighPlusFullLowRegionWisePackingGenerator::HighPlusFullLowRegionWisePackingGenerator()
{
    m_highResWidth    = 0;
    m_highResHeight   = 0;
    m_lowResWidth     = 0;
    m_lowResHeight    = 0;
    m_packedPicWidth  = 0;
    m_packedPicHeight = 0;
    m_selectedTilesNum    = 0;
    m_maxSelectedTilesNum = 0;
    m_streamIdxInMedia[0] = 0;
    m_streamIdxInMedia[1] = 0;

    m_origHRTilesInRow    = 0;
    m_origHRTilesInCol    = 0;
    m_highTileWidth       = 0;
    m_highTileHeight      = 0;
    m_origLRTilesInRow    = 0;
    m_origLRTilesInCol    = 0;
    m_lowTileWidth        = 0;
    m_lowTileHeight       = 0;

    m_hrTilesInRow        = 0;
    m_hrTilesInCol        = 0;
    m_lrTilesInRow        = 0;
    m_lrTilesInCol        = 0;
    m_regNum              = 0;
    m_tilesNumInPackedPic = 0;
    m_mergedTilesArrange = NULL;
}

HighPlusFullLowRegionWisePackingGenerator::HighPlusFullLowRegionWisePackingGenerator(
    const HighPlusFullLowRegionWisePackingGenerator& src)
{
    m_highResWidth    = src.m_highResWidth;
    m_highResHeight   = src.m_highResHeight;
    m_lowResWidth     = src.m_lowResWidth;
    m_lowResHeight    = src.m_lowResHeight;
    m_packedPicWidth  = src.m_packedPicWidth;
    m_packedPicHeight = src.m_packedPicHeight;
    m_selectedTilesNum    = src.m_selectedTilesNum;
    m_maxSelectedTilesNum = src.m_maxSelectedTilesNum;
    m_streamIdxInMedia[0] = src.m_streamIdxInMedia[0];
    m_streamIdxInMedia[1] = src.m_streamIdxInMedia[1];

    m_origHRTilesInRow    = src.m_origHRTilesInRow;
    m_origHRTilesInCol    = src.m_origHRTilesInCol;
    m_highTileWidth       = src.m_highTileWidth;
    m_highTileHeight      = src.m_highTileHeight;
    m_origLRTilesInRow    = src.m_origLRTilesInRow;
    m_origLRTilesInCol    = src.m_origLRTilesInCol;
    m_lowTileWidth        = src.m_lowTileWidth;
    m_lowTileHeight       = src.m_lowTileHeight;

    m_hrTilesInRow        = src.m_hrTilesInRow;
    m_hrTilesInCol        = src.m_hrTilesInCol;
    m_lrTilesInRow        = src.m_lrTilesInRow;
    m_lrTilesInCol        = src.m_lrTilesInCol;
    m_regNum              = src.m_regNum;
    m_rectAreas           = src.m_rectAreas;
    m_tilesNumInPackedPic = src.m_tilesNumInPackedPic;
    m_mergedTilesArrange  = std::move(src.m_mergedTilesArrange);
}

HighPlusFullLowRegionWisePackingGenerator::~HighPlusFullLowRegionWisePackingGenerator()
{
    if (m_mergedTilesArrange)
    {
        SAFE_DELETE_ARRAY(m_mergedTilesArrange->tileRowHeight);
        SAFE_DELETE_ARRAY(m_mergedTilesArrange->tileColWidth);

        delete m_mergedTilesArrange;
        m_mergedTilesArrange = NULL;
    }

    m_rectAreas.clear();
}

int32_t HighPlusFullLowRegionWisePackingGenerator::GenerateHighTilesArrange(
    uint16_t tilesNumInView,
    uint16_t maxSelectedTilesNum)
{
    m_selectedTilesNum = tilesNumInView;
    m_maxSelectedTilesNum = maxSelectedTilesNum;

    uint16_t highResTilesNum = m_maxSelectedTilesNum;

    uint16_t sqrtH = (uint16_t)sqrt(highResTilesNum);
    while(sqrtH && highResTilesNum%sqrtH) { sqrtH--; }
    uint16_t dividedVal = highResTilesNum / sqrtH;

    if (sqrtH > dividedVal)
    {
        m_hrTilesInCol = dividedVal;
        m_hrTilesInRow = sqrtH;
    }
    else
    {
        m_hrTilesInCol = sqrtH;
        m_hrTilesInRow = dividedVal;
    }

    return ERROR_NONE;
}

int32_t HighPlusFullLowRegionWisePackingGenerator::ChooseLowResTilesForPacking(
    TileDef *tilesInViewport,
    uint32_t supplementaryLRTilesNum)
{
    RegionWisePacking *lowRwpk = m_rwpkMap[1];
    if (!lowRwpk)
        return OMAF_ERROR_NULL_PTR;

    for (uint8_t regIdx = 0; regIdx < lowRwpk->numRegions; regIdx++)
    {
        tilesInViewport[m_hrTilesInRow * m_hrTilesInCol + regIdx].x = lowRwpk->rectRegionPacking[regIdx].projRegLeft;
        tilesInViewport[m_hrTilesInRow * m_hrTilesInCol + regIdx].y = lowRwpk->rectRegionPacking[regIdx].projRegTop;
        tilesInViewport[m_hrTilesInRow * m_hrTilesInCol + regIdx].idx = regIdx;
        tilesInViewport[m_hrTilesInRow * m_hrTilesInCol + regIdx].faceId = 0; //change later
    }

    if (supplementaryLRTilesNum)
    {
        if (supplementaryLRTilesNum <= lowRwpk->numRegions)
        {
            for (uint8_t regIdx = 0; regIdx < supplementaryLRTilesNum; regIdx++)
            {
                tilesInViewport[m_hrTilesInRow * m_hrTilesInCol + lowRwpk->numRegions + regIdx].x      = lowRwpk->rectRegionPacking[regIdx].projRegLeft;
                tilesInViewport[m_hrTilesInRow * m_hrTilesInCol + lowRwpk->numRegions + regIdx].y      = lowRwpk->rectRegionPacking[regIdx].projRegTop;
                tilesInViewport[m_hrTilesInRow * m_hrTilesInCol + lowRwpk->numRegions + regIdx].idx    = regIdx;
                tilesInViewport[m_hrTilesInRow * m_hrTilesInCol + lowRwpk->numRegions + regIdx].faceId = 0; //change later
            }
        }
        else
        {
            uint32_t rounds = supplementaryLRTilesNum / lowRwpk->numRegions;
            for (uint32_t roundIdx = 0; roundIdx < rounds; roundIdx++)
            {
                for (uint8_t regIdx = 0; regIdx < lowRwpk->numRegions; regIdx++)
                {
                    tilesInViewport[m_hrTilesInRow * m_hrTilesInCol + lowRwpk->numRegions + lowRwpk->numRegions * roundIdx + regIdx].x = lowRwpk->rectRegionPacking[regIdx].projRegLeft;
                    tilesInViewport[m_hrTilesInRow * m_hrTilesInCol + lowRwpk->numRegions + lowRwpk->numRegions * roundIdx + regIdx].y = lowRwpk->rectRegionPacking[regIdx].projRegTop;
                    tilesInViewport[m_hrTilesInRow * m_hrTilesInCol + lowRwpk->numRegions + lowRwpk->numRegions * roundIdx + regIdx].idx = regIdx;
                    tilesInViewport[m_hrTilesInRow * m_hrTilesInCol + lowRwpk->numRegions + lowRwpk->numRegions * roundIdx + regIdx].faceId = 0; //change later
                }
            }

            uint32_t remainder = supplementaryLRTilesNum % lowRwpk->numRegions;
            for (uint8_t regIdx = 0; regIdx < remainder; regIdx++)
            {
                tilesInViewport[m_hrTilesInRow * m_hrTilesInCol + lowRwpk->numRegions + lowRwpk->numRegions * rounds + regIdx].x = lowRwpk->rectRegionPacking[regIdx].projRegLeft;
                tilesInViewport[m_hrTilesInRow * m_hrTilesInCol + lowRwpk->numRegions + lowRwpk->numRegions * rounds + regIdx].y = lowRwpk->rectRegionPacking[regIdx].projRegTop;
                tilesInViewport[m_hrTilesInRow * m_hrTilesInCol + lowRwpk->numRegions + lowRwpk->numRegions * rounds + regIdx].idx = regIdx;
                tilesInViewport[m_hrTilesInRow * m_hrTilesInCol + lowRwpk->numRegions + lowRwpk->numRegions * rounds + regIdx].faceId = 0; //change later
            }
        }
    }

    return ERROR_NONE;
}

int32_t HighPlusFullLowRegionWisePackingGenerator::GenerateMergedTilesArrange(TileDef *tilesInViewport)
{
    int32_t ret = ERROR_NONE;

    if (!m_selectedTilesNum || !m_maxSelectedTilesNum || (m_selectedTilesNum > m_maxSelectedTilesNum))
    {
        OMAF_LOG(LOG_ERROR, "Invalid maxmum selected tiles number and actual selected tiles number in viewport !\n");
        return OMAF_ERROR_INVALID_DATA;
    }

    if (m_selectedTilesNum != m_maxSelectedTilesNum)
    {
        for (uint16_t suppleIdx = 0; suppleIdx < (m_maxSelectedTilesNum - m_selectedTilesNum); suppleIdx++)
        {
            uint16_t repeatedIdx = 0;
            if ((m_selectedTilesNum - suppleIdx - 2) >= 0)
            {
                repeatedIdx = m_selectedTilesNum - suppleIdx - 2;
            }
            else
            {
                repeatedIdx = suppleIdx - (m_selectedTilesNum - 2);
            }
            tilesInViewport[m_selectedTilesNum + suppleIdx].x = tilesInViewport[repeatedIdx].x;//tilesInViewport[0] maybe have been repetitive before when selected tiles num is prime number
            tilesInViewport[m_selectedTilesNum + suppleIdx].y = tilesInViewport[repeatedIdx].y;
            tilesInViewport[m_selectedTilesNum + suppleIdx].idx = tilesInViewport[repeatedIdx].idx;
            tilesInViewport[m_selectedTilesNum + suppleIdx].faceId = tilesInViewport[repeatedIdx].faceId;
        }
    }

    if (!m_hrTilesInCol || !m_hrTilesInRow)
    {
        OMAF_LOG(LOG_ERROR, "High resolution tiles row or column numbers are 0 !\n");
        return OMAF_ERROR_INVALID_DATA;
    }

    uint16_t lowResTilesNum = m_origLRTilesInRow * m_origLRTilesInCol;

    uint32_t packedHeight = 0;
    packedHeight = m_highTileHeight * m_hrTilesInCol;
    if (packedHeight % m_lowTileHeight)
    {
        OMAF_LOG(LOG_ERROR, "Packed sub-picture height can't be divided by low resolution tile height !\n");
        return OMAF_ERROR_INVALID_DATA;
    }

    m_lrTilesInCol = packedHeight / m_lowTileHeight;
    uint32_t supplementaryLRTilesNum = 0;
    if (m_lrTilesInCol <= lowResTilesNum)
    {
        if (lowResTilesNum % m_lrTilesInCol)
        {
            supplementaryLRTilesNum = m_lrTilesInCol - (lowResTilesNum % m_lrTilesInCol);
            m_lrTilesInRow = lowResTilesNum / m_lrTilesInCol + 1;
        }
        else
        {
            m_lrTilesInRow = lowResTilesNum / m_lrTilesInCol;
        }
    }
    else
    {
        supplementaryLRTilesNum = m_lrTilesInCol - lowResTilesNum;
        m_lrTilesInRow = 1;
    }

    m_tilesNumInPackedPic = m_hrTilesInRow * m_hrTilesInCol + m_lrTilesInRow * m_lrTilesInCol;

    ret = ChooseLowResTilesForPacking(tilesInViewport, supplementaryLRTilesNum);
    if (ret)
        return ret;

    if (!m_mergedTilesArrange)
    {
        m_mergedTilesArrange = new TileArrangement;
        if (!m_mergedTilesArrange)
            return OMAF_ERROR_NULL_PTR;

        m_mergedTilesArrange->tileRowsNum = 1;
        m_mergedTilesArrange->tileColsNum = m_hrTilesInRow + m_lrTilesInRow;
        m_mergedTilesArrange->tileRowHeight = new uint16_t[m_mergedTilesArrange->tileRowsNum];
        if (!(m_mergedTilesArrange->tileRowHeight))
            return OMAF_ERROR_NULL_PTR;

        m_mergedTilesArrange->tileRowHeight[0] = packedHeight;

        m_mergedTilesArrange->tileColWidth = new uint16_t[m_mergedTilesArrange->tileColsNum];
        if (!(m_mergedTilesArrange->tileColWidth))
            return OMAF_ERROR_NULL_PTR;

#define LCU_SIZE 64

        for (uint8_t i = 0; i < m_mergedTilesArrange->tileColsNum; i++)
        {
            if (i < m_hrTilesInRow)
            {
                m_mergedTilesArrange->tileColWidth[i] = m_highTileWidth / LCU_SIZE;
            }
            else
            {
                m_mergedTilesArrange->tileColWidth[i] = m_lowTileWidth / LCU_SIZE;
            }
        }
    }

    return ERROR_NONE;
}

int32_t HighPlusFullLowRegionWisePackingGenerator::Initialize(
    std::map<uint8_t, VideoStreamInfo*> *streams,
    uint8_t *videoIdxInMedia,
    uint16_t tilesNumInViewport,
    uint16_t maxSelectedTilesNum,
    void    *externalLog)
{
    if (!streams || !videoIdxInMedia)
        return OMAF_ERROR_NULL_PTR;

    if (externalLog)
        logCallBack = (LogFunction)externalLog;
    else
        logCallBack = GlogFunction;

    uint8_t videoStreamIdx = 0;
    std::map<uint8_t, VideoStreamInfo*>::iterator it;
    it = streams->find(videoIdxInMedia[0]);
    if (it == streams->end())
        return OMAF_ERROR_STREAM_NOT_FOUND;

    VideoStreamInfo *vs1 = (VideoStreamInfo*)(it->second);
    m_origHRTilesInRow = vs1->tilesNumInRow;
    m_origHRTilesInCol = vs1->tilesNumInCol;
    RegionWisePacking *rwpk1 = vs1->srcRWPK;
    m_rwpkMap.insert(std::make_pair(videoStreamIdx, rwpk1));
    RectangularRegionWisePacking *rectRwpk = &(rwpk1->rectRegionPacking[0]);
    m_highResWidth  = rwpk1->projPicWidth;
    m_highResHeight = rwpk1->projPicHeight;
    m_highTileWidth = rectRwpk->projRegWidth;
    m_highTileHeight = rectRwpk->projRegHeight;

    videoStreamIdx++;
    it = streams->find(videoIdxInMedia[1]);
    VideoStreamInfo *vs2 = (VideoStreamInfo*)(it->second);
    m_origLRTilesInRow = vs2->tilesNumInRow;
    m_origLRTilesInCol = vs2->tilesNumInCol;
    RegionWisePacking *rwpk2 = vs2->srcRWPK;
    m_rwpkMap.insert(std::make_pair(videoStreamIdx, rwpk2));
    rectRwpk = &(rwpk2->rectRegionPacking[0]);
    m_lowResWidth = rwpk2->projPicWidth;
    m_lowResHeight = rwpk2->projPicHeight;
    m_lowTileWidth = rectRwpk->projRegWidth;
    m_lowTileHeight = rectRwpk->projRegHeight;

    m_streamIdxInMedia[0] = videoIdxInMedia[0];
    m_streamIdxInMedia[1] = videoIdxInMedia[1];

    int32_t ret = GenerateHighTilesArrange(tilesNumInViewport, maxSelectedTilesNum);
    if (ret)
        return ret;

    return ERROR_NONE;
}

int32_t HighPlusFullLowRegionWisePackingGenerator::GenerateTilesMergeDirection(
    TileDef *tilesInViewport,
    TilesMergeDirectionInCol *tilesMergeDir)
{
    if (!tilesInViewport || !tilesMergeDir)
        return OMAF_ERROR_NULL_PTR;

    uint8_t highTilesNum = m_hrTilesInRow * m_hrTilesInCol;

#define LCU_SIZE 64

    uint8_t tileColsNum = m_hrTilesInRow + m_lrTilesInRow;
    uint16_t tileIdx = 0;
    for (uint8_t i = 0; i < tileColsNum; i++)
    {
        TilesInCol *tileCol = new TilesInCol;
        if (!tileCol)
        {
            return OMAF_ERROR_NULL_PTR;
        }

        if (i < m_hrTilesInRow)
        {
            for (uint8_t j = 0; j < m_hrTilesInCol; j++)
            {
                SingleTile *tile = new SingleTile;
                if (!tile)
                {
                    SAFE_DELETE_MEMORY(tileCol);
                    return OMAF_ERROR_NULL_PTR;
                }

                tile->streamIdxInMedia = m_streamIdxInMedia[0];
                tile->origTileIdx      = tilesInViewport[tileIdx].idx;
                tile->dstCTUIndex      = (tileIdx % m_hrTilesInCol) *
                                         (m_highTileHeight / LCU_SIZE) *
                                         (m_packedPicWidth / LCU_SIZE) +
                                         (tileIdx / m_hrTilesInCol) *
                                         (m_highTileWidth / LCU_SIZE);

                tileCol->push_back(tile);

                tileIdx++;
            }
            tilesMergeDir->tilesArrangeInCol.push_back(tileCol);
        }
        else
        {
            for (uint8_t j = 0; j < m_lrTilesInCol; j++)
            {
                SingleTile *tile = new SingleTile;
                if (!tile)
                {
                    SAFE_DELETE_MEMORY(tileCol);
                    return OMAF_ERROR_NULL_PTR;
                }

                tile->streamIdxInMedia = m_streamIdxInMedia[1];
                tile->origTileIdx      = tilesInViewport[tileIdx].idx;//tileIdx - highTilesNum;
                tile->dstCTUIndex      = ((tileIdx - highTilesNum) % m_lrTilesInCol) *
                                         (m_lowTileHeight / LCU_SIZE) *
                                         (m_packedPicWidth / LCU_SIZE) +
                                         ((tileIdx - highTilesNum) / m_lrTilesInCol) *
                                         (m_lowTileWidth / LCU_SIZE) +
                                         m_hrTilesInRow * (m_highTileWidth / LCU_SIZE);

                tileCol->push_back(tile);

                tileIdx++;
            }
            tilesMergeDir->tilesArrangeInCol.push_back(tileCol);
        }
    }

    return ERROR_NONE;
}

int32_t HighPlusFullLowRegionWisePackingGenerator::GenerateDstRwpk(
    TileDef *tilesInViewport,
    RegionWisePacking *dstRwpk)
{
    if (!tilesInViewport || !dstRwpk)
        return OMAF_ERROR_NULL_PTR;

    dstRwpk->constituentPicMatching = 0;
    dstRwpk->numRegions             = m_hrTilesInRow * m_hrTilesInCol + m_lrTilesInRow * m_lrTilesInCol;

    dstRwpk->packedPicWidth         = m_highTileWidth * m_hrTilesInRow + m_lowTileWidth * m_lrTilesInRow;
    dstRwpk->packedPicHeight        = m_highTileHeight * m_hrTilesInCol;


    m_packedPicWidth                = dstRwpk->packedPicWidth;
    m_packedPicHeight               = dstRwpk->packedPicHeight;

    dstRwpk->rectRegionPacking      = new RectangularRegionWisePacking[dstRwpk->numRegions];
    if (!(dstRwpk->rectRegionPacking))
        return OMAF_ERROR_NULL_PTR;

    uint8_t highTilesNum = m_hrTilesInRow * m_hrTilesInCol;

    std::map<uint8_t, RegionWisePacking*>::iterator it;
    it = m_rwpkMap.find(0);
    if (it == m_rwpkMap.end())
    {
        return OMAF_ERROR_STREAM_NOT_FOUND;
    }
    RegionWisePacking *rwpkHighRes = it->second;
    it = m_rwpkMap.find(1);
    if (it == m_rwpkMap.end())
    {
        return OMAF_ERROR_STREAM_NOT_FOUND;
    }
    RegionWisePacking *rwpkLowRes = it->second;

    for (uint8_t regionIdx = 0; regionIdx < dstRwpk->numRegions; regionIdx++)
    {
        RectangularRegionWisePacking *rwpk = &(dstRwpk->rectRegionPacking[regionIdx]);
        memset(rwpk, 0, sizeof(RectangularRegionWisePacking));
        //rwpk->transformType = 0;
        rwpk->guardBandFlag = false;
        if (regionIdx < highTilesNum)
        {
            RectangularRegionWisePacking *rectRwpkHigh = &(rwpkHighRes->rectRegionPacking[tilesInViewport[regionIdx].idx]);
            rwpk->transformType = rectRwpkHigh->transformType;
            rwpk->projRegWidth  = rectRwpkHigh->projRegWidth;
            rwpk->projRegHeight = rectRwpkHigh->projRegHeight;
            rwpk->projRegTop    = rectRwpkHigh->projRegTop;
            rwpk->projRegLeft   = rectRwpkHigh->projRegLeft;

            rwpk->packedRegWidth  = rwpk->projRegWidth;
            rwpk->packedRegHeight = rwpk->projRegHeight;
            rwpk->packedRegTop    = (regionIdx % m_hrTilesInCol) * m_highTileHeight;
            rwpk->packedRegLeft   = (regionIdx / m_hrTilesInCol) * m_highTileWidth;

            rwpk->leftGbWidth          = 0;
            rwpk->rightGbWidth         = 0;
            rwpk->topGbHeight          = 0;
            rwpk->bottomGbHeight       = 0;
            rwpk->gbNotUsedForPredFlag = true;
            rwpk->gbType0              = 0;
            rwpk->gbType1              = 0;
            rwpk->gbType2              = 0;
            rwpk->gbType3              = 0;
        }
        else
        {
            RectangularRegionWisePacking *rectRwpkLow = &(rwpkLowRes->rectRegionPacking[tilesInViewport[regionIdx].idx]);

            rwpk->transformType   = rectRwpkLow->transformType;
            rwpk->packedRegWidth  = rectRwpkLow->projRegWidth;
            rwpk->packedRegHeight = rectRwpkLow->projRegHeight;;

            rwpk->projRegWidth  = (rectRwpkLow->projRegWidth * m_highResWidth) / m_lowResWidth;
            rwpk->projRegHeight = (rectRwpkLow->projRegHeight * m_highResHeight) / m_lowResHeight;
            rwpk->projRegTop    = (rectRwpkLow->projRegTop * m_highResHeight) / m_lowResHeight;
            rwpk->projRegLeft   = (rectRwpkLow->projRegLeft * m_highResWidth) / m_lowResWidth;

            rwpk->packedRegTop    = ((regionIdx-highTilesNum) % m_lrTilesInCol) * m_lowTileHeight;
            rwpk->packedRegLeft   = ((regionIdx-highTilesNum) / m_lrTilesInCol) * m_lowTileWidth + m_highTileWidth * m_hrTilesInRow;

            rwpk->leftGbWidth          = 0;
            rwpk->rightGbWidth         = 0;
            rwpk->topGbHeight          = 0;
            rwpk->bottomGbHeight       = 0;
            rwpk->gbNotUsedForPredFlag = true;
            rwpk->gbType0              = 0;
            rwpk->gbType1              = 0;
            rwpk->gbType2              = 0;
            rwpk->gbType3              = 0;
        }
    }

    return ERROR_NONE;
}

extern "C" RegionWisePackingGeneratorBase* Create()
{
    HighPlusFullLowRegionWisePackingGenerator *rwpkGen = new HighPlusFullLowRegionWisePackingGenerator;
    return (RegionWisePackingGeneratorBase*)(rwpkGen);
}

extern "C" void Destroy(RegionWisePackingGeneratorBase* rwpkGen)
{
    delete rwpkGen;
    rwpkGen = NULL;
}
