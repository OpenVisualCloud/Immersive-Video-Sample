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
#include "HighResPlusFullLowResPacking.h"

HighPlusFullLowRegionWisePackingGenerator::HighPlusFullLowRegionWisePackingGenerator()
{
    m_highResWidth    = 0;
    m_highResHeight   = 0;
    m_lowResWidth     = 0;
    m_lowResHeight    = 0;
    m_packedPicWidth  = 0;
    m_packedPicHeight = 0;
    m_streamIdxInMedia[0] = 0;
    m_streamIdxInMedia[1] = 0;
    m_tilesNumInViewRow   = 0;
    m_tileRowNumInView    = 0;

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

    m_highResTilesInView = new TilesMergeDirectionInRow;
    if (!m_highResTilesInView)
        return;

    m_mergedTilesArrange = new TileArrangement;
    if (!m_mergedTilesArrange)
        return;
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
    m_streamIdxInMedia[0] = src.m_streamIdxInMedia[0];
    m_streamIdxInMedia[1] = src.m_streamIdxInMedia[1];
    m_tilesNumInViewRow   = src.m_tilesNumInViewRow;
    m_tileRowNumInView    = src.m_tileRowNumInView;

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

    m_highResTilesInView  = std::move(src.m_highResTilesInView);

    m_mergedTilesArrange  = std::move(src.m_mergedTilesArrange);
}

HighPlusFullLowRegionWisePackingGenerator::~HighPlusFullLowRegionWisePackingGenerator()
{
    if (m_highResTilesInView)
    {
        std::list<TilesInRow*>::iterator itRow;
        for (itRow = m_highResTilesInView->tilesArrangeInRow.begin(); itRow != m_highResTilesInView->tilesArrangeInRow.end();)
        {
            TilesInRow *tileRow = *itRow;
            std::list<SingleTile*>::iterator itTile;
            for (itTile = tileRow->begin(); itTile != tileRow->end();)
            {
                SingleTile *tile = *itTile;
                SAFE_DELETE_MEMORY(tile);
                itTile = tileRow->erase(itTile);
            }

            delete tileRow;
            tileRow = NULL;

            itRow = m_highResTilesInView->tilesArrangeInRow.erase(itRow);
        }

        delete m_highResTilesInView;
        m_highResTilesInView = NULL;
    }

    if (m_mergedTilesArrange)
    {
        SAFE_DELETE_ARRAY(m_mergedTilesArrange->tileRowHeight);
        SAFE_DELETE_ARRAY(m_mergedTilesArrange->tileColWidth);

        delete m_mergedTilesArrange;
        m_mergedTilesArrange = NULL;
    }
}

int32_t HighPlusFullLowRegionWisePackingGenerator::GetOrigHighResTilesArrange(
    uint8_t tilesNumInViewport,
    TileDef *tilesInViewport,
    int32_t finalViewportWidth,
    int32_t finalViewportHeight)
{
    if (!tilesInViewport)
        return OMAF_ERROR_NULL_PTR;

    if (!tilesNumInViewport)
        return OMAF_ERROR_SCVP_INCORRECT_RESULT;

    if (!finalViewportWidth || !finalViewportHeight)
        return OMAF_ERROR_SCVP_INCORRECT_RESULT;

    uint16_t tileRowNum = finalViewportHeight / m_highTileHeight;
    uint16_t tileColNum = finalViewportWidth / m_highTileWidth;

    if (tileRowNum * tileColNum != tilesNumInViewport)
        return OMAF_ERROR_SCVP_INCORRECT_RESULT;

    for (uint16_t i = 0; i < tileRowNum; i++)
    {
        TilesInRow *currRow  = new TilesInRow;
        if (!currRow)
            return OMAF_ERROR_NULL_PTR;

        for (uint16_t j = 0; j < tileColNum; j++)
        {
            SingleTile *tile = new SingleTile;
            if (!tile)
            {
                SAFE_DELETE_MEMORY(currRow);
                return OMAF_ERROR_NULL_PTR;
            }

            tile->streamIdxInMedia = m_streamIdxInMedia[0];

            currRow->push_back(tile);
        }
        m_highResTilesInView->tilesArrangeInRow.push_back(currRow);
    }

    return ERROR_NONE;
}

static uint32_t gcd(uint32_t a, uint32_t b)
{
    for ( ; ; )
    {
        if (a == 0) return b;
        b %= a;
        if (b == 0) return a;
        a %= b;
    }
}

static uint32_t lcm(uint32_t a, uint32_t b)
{
    uint32_t temp = gcd(a, b);

    return temp ? (a / temp * b) : 0;
}

int32_t HighPlusFullLowRegionWisePackingGenerator::GenerateMergedTilesArrange()
{
    uint16_t highResTilesNum = m_tilesNumInViewRow * m_tileRowNumInView;
    uint16_t lowResTilesNum  = m_origLRTilesInRow * m_origLRTilesInCol;

    uint32_t height = 0;
    uint8_t  tilesNumInHeight = 0;

    height = lcm(m_highTileHeight, m_lowTileHeight);
    uint16_t sqrtH = (uint16_t)sqrt(highResTilesNum);
    while(sqrtH && highResTilesNum%sqrtH) { sqrtH--; }

    tilesNumInHeight = height / m_highTileHeight;
    tilesNumInHeight = lcm(tilesNumInHeight, sqrtH);
    height = tilesNumInHeight * m_highTileHeight;

    if (height == 0 ||
        tilesNumInHeight == 0 ||
        height % m_lowTileHeight ||
        highResTilesNum % tilesNumInHeight ||
        lowResTilesNum % (height / m_lowTileHeight))
        return OMAF_ERROR_UNDEFINED_OPERATION;

    m_hrTilesInCol = tilesNumInHeight;
    m_hrTilesInRow = highResTilesNum / m_hrTilesInCol;
    m_lrTilesInCol = height / m_lowTileHeight;
    m_lrTilesInRow = lowResTilesNum / m_lrTilesInCol;

    m_mergedTilesArrange->tileRowsNum = 1;
    m_mergedTilesArrange->tileColsNum = m_hrTilesInRow + m_lrTilesInRow;
    m_mergedTilesArrange->tileRowHeight = new uint16_t[m_mergedTilesArrange->tileRowsNum];
    if (!(m_mergedTilesArrange->tileRowHeight))
        return OMAF_ERROR_NULL_PTR;

    m_mergedTilesArrange->tileRowHeight[0] = height;

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

    return ERROR_NONE;
}

int32_t HighPlusFullLowRegionWisePackingGenerator::Initialize(
    std::map<uint8_t, VideoStreamInfo*> *streams,
    uint8_t *videoIdxInMedia,
    uint8_t tilesNumInViewport,
    TileDef *tilesInViewport,
    int32_t finalViewportWidth,
    int32_t finalViewportHeight)
{
    if (!streams || !videoIdxInMedia || !tilesInViewport)
        return OMAF_ERROR_NULL_PTR;

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

    int32_t ret = GetOrigHighResTilesArrange(
                tilesNumInViewport,
                tilesInViewport,
                finalViewportWidth,
                finalViewportHeight);
    if (ret)
        return ret;

    m_tileRowNumInView = m_highResTilesInView->tilesArrangeInRow.size();
    if (!m_tileRowNumInView)
        return OMAF_ERROR_SCVP_INCORRECT_RESULT;

    TilesInRow *tilesRow = m_highResTilesInView->tilesArrangeInRow.front();
    m_tilesNumInViewRow = tilesRow->size();
    if (!m_tilesNumInViewRow)
        return OMAF_ERROR_SCVP_INCORRECT_RESULT;

    ret = GenerateMergedTilesArrange();
    if (ret)
        return ret;

    return ERROR_NONE;
}

int32_t HighPlusFullLowRegionWisePackingGenerator::GenerateTilesMergeDirection(
    uint8_t viewportIdx,
    TilesMergeDirectionInCol *tilesMergeDir)
{
    if (!tilesMergeDir)
        return OMAF_ERROR_NULL_PTR;

    uint8_t highTilesNum = m_tilesNumInViewRow * m_tileRowNumInView;
    uint8_t *highTilesIdx = new uint8_t[highTilesNum];
    if (!highTilesIdx)
        return OMAF_ERROR_NULL_PTR;

    highTilesIdx[0] = viewportIdx;
    for (uint8_t i = 1; i < highTilesNum; i++)
    {
        if (i % m_tilesNumInViewRow)
        {
            highTilesIdx[i] = highTilesIdx[i-1] + 1;
            if (highTilesIdx[i] >= (highTilesIdx[i-1] / m_origHRTilesInRow + 1) * m_origHRTilesInRow)
            {
                highTilesIdx[i] = highTilesIdx[i] - m_origHRTilesInRow;
            }
        }
        else
        {
            highTilesIdx[i] = (i / m_tilesNumInViewRow) * m_origHRTilesInRow + highTilesIdx[0];
            if (highTilesIdx[i] >= m_origHRTilesInRow * m_origHRTilesInCol)
            {
                highTilesIdx[i] = highTilesIdx[i] - m_origHRTilesInRow * m_origHRTilesInCol;
            }
        }
    }
#define LCU_SIZE 64

    uint8_t tileColsNum = m_hrTilesInRow + m_lrTilesInRow;
    uint16_t tileIdx = 0;
    for (uint8_t i = 0; i < tileColsNum; i++)
    {
        TilesInCol *tileCol = new TilesInCol;
        if (!tileCol)
        {
            SAFE_DELETE_ARRAY(highTilesIdx);
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
                    SAFE_DELETE_ARRAY(highTilesIdx);
                    return OMAF_ERROR_NULL_PTR;
                }

                tile->streamIdxInMedia = m_streamIdxInMedia[0];
                tile->origTileIdx      = highTilesIdx[(tileIdx % m_hrTilesInCol) * m_hrTilesInRow + tileIdx / m_hrTilesInCol];
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
                    SAFE_DELETE_ARRAY(highTilesIdx);
                    return OMAF_ERROR_NULL_PTR;
                }

                tile->streamIdxInMedia = m_streamIdxInMedia[1];
                tile->origTileIdx      = tileIdx - highTilesNum;
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

    SAFE_DELETE_ARRAY(highTilesIdx);

    return ERROR_NONE;
}

int32_t HighPlusFullLowRegionWisePackingGenerator::GenerateDstRwpk(
    uint8_t viewportIdx,
    RegionWisePacking *dstRwpk)
{
    if (!dstRwpk)
        return OMAF_ERROR_NULL_PTR;

    dstRwpk->constituentPicMatching = 0;
    dstRwpk->numRegions             = m_tilesNumInViewRow * m_tileRowNumInView + m_origLRTilesInRow * m_origLRTilesInCol;

    dstRwpk->packedPicWidth         = m_highTileWidth * m_hrTilesInRow + m_lowTileWidth * m_lrTilesInRow;
    dstRwpk->packedPicHeight        = m_highTileHeight * m_hrTilesInCol;


    m_packedPicWidth                = dstRwpk->packedPicWidth;
    m_packedPicHeight               = dstRwpk->packedPicHeight;

    dstRwpk->rectRegionPacking      = new RectangularRegionWisePacking[dstRwpk->numRegions];
    if (!(dstRwpk->rectRegionPacking))
        return OMAF_ERROR_NULL_PTR;

    uint8_t highTilesNum = m_tilesNumInViewRow * m_tileRowNumInView;
    uint8_t *highTilesIdx = new uint8_t[highTilesNum];
    if (!highTilesIdx)
        return OMAF_ERROR_NULL_PTR;

    highTilesIdx[0] = viewportIdx;
    for (uint8_t i = 1; i < highTilesNum; i++)
    {
        if (i % m_tilesNumInViewRow)
        {
            highTilesIdx[i] = highTilesIdx[i-1] + 1;
            if (highTilesIdx[i] >= (highTilesIdx[i-1] / m_origHRTilesInRow + 1) * m_origHRTilesInRow)
            {
                highTilesIdx[i] = highTilesIdx[i] - m_origHRTilesInRow;
            }
        }
        else
        {
            highTilesIdx[i] = (i / m_tilesNumInViewRow) * m_origHRTilesInRow + highTilesIdx[0];
            if (highTilesIdx[i] >= m_origHRTilesInRow * m_origHRTilesInCol)
            {
                highTilesIdx[i] = highTilesIdx[i] - m_origHRTilesInRow * m_origHRTilesInCol;
            }
        }
    }

    std::map<uint8_t, RegionWisePacking*>::iterator it;
    it = m_rwpkMap.find(0);
    if (it == m_rwpkMap.end())
    {
        SAFE_DELETE_ARRAY(highTilesIdx);
        return OMAF_ERROR_STREAM_NOT_FOUND;
    }
    RegionWisePacking *rwpkHighRes = it->second;
    it = m_rwpkMap.find(1);
    if (it == m_rwpkMap.end())
    {
        SAFE_DELETE_ARRAY(highTilesIdx);
        return OMAF_ERROR_STREAM_NOT_FOUND;
    }
    RegionWisePacking *rwpkLowRes = it->second;

    for (uint8_t regionIdx = 0; regionIdx < dstRwpk->numRegions; regionIdx++)
    {
        RectangularRegionWisePacking *rwpk = &(dstRwpk->rectRegionPacking[regionIdx]);
        memset(rwpk, 0, sizeof(RectangularRegionWisePacking));
        rwpk->transformType = 0;
        rwpk->guardBandFlag = false;
        if (regionIdx < highTilesNum)
        {
            RectangularRegionWisePacking *rectRwpkHigh = &(rwpkHighRes->rectRegionPacking[highTilesIdx[(regionIdx % m_hrTilesInCol) * m_hrTilesInRow + regionIdx / m_hrTilesInCol]]);
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
            RectangularRegionWisePacking *rectRwpkLow = &(rwpkLowRes->rectRegionPacking[regionIdx-highTilesNum]);

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

    SAFE_DELETE_ARRAY(highTilesIdx);

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
