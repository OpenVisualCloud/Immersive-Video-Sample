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
//! \file:   SingleVideoPacking.cpp
//! \brief:  Single video region wise packing generator class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include <math.h>
#include <string.h>

#include "../../../utils/error.h"
#include "SingleVideoPacking.h"

SingleVideoRegionWisePackingGenerator::SingleVideoRegionWisePackingGenerator()
{

    m_packedPicWidth  = 0;
    m_packedPicHeight = 0;
    m_streamIdxInMedia[0] = 0;
    m_tilesNumInViewRow   = 0;
    m_tileRowNumInView    = 0;

    m_origHRTilesInRow    = 0;
    m_origHRTilesInCol    = 0;
    m_highTileWidth       = 0;
    m_highTileHeight      = 0;

    m_hrTilesInRow        = 0;
    m_hrTilesInCol        = 0;

    m_origTilesInView = new TilesMergeDirectionInRow;
    if (!m_origTilesInView)
        return;

    m_mergedTilesArrange = new TileArrangement;
    if (!m_mergedTilesArrange)
        return;
}

SingleVideoRegionWisePackingGenerator::SingleVideoRegionWisePackingGenerator(
    const SingleVideoRegionWisePackingGenerator& src)
{

    m_packedPicWidth  = src.m_packedPicWidth;
    m_packedPicHeight = src.m_packedPicHeight;
    m_streamIdxInMedia[0] = src.m_streamIdxInMedia[0];
    m_tilesNumInViewRow   = src.m_tilesNumInViewRow;
    m_tileRowNumInView    = src.m_tileRowNumInView;

    m_origHRTilesInRow    = src.m_origHRTilesInRow;
    m_origHRTilesInCol    = src.m_origHRTilesInCol;
    m_highTileWidth       = src.m_highTileWidth;
    m_highTileHeight      = src.m_highTileHeight;

    m_hrTilesInRow        = src.m_hrTilesInRow;
    m_hrTilesInCol        = src.m_hrTilesInCol;

    m_origTilesInView     = std::move(src.m_origTilesInView);

    m_mergedTilesArrange  = std::move(src.m_mergedTilesArrange);
}

SingleVideoRegionWisePackingGenerator::~SingleVideoRegionWisePackingGenerator()
{
    if (m_origTilesInView)
    {
        std::list<TilesInRow*>::iterator itRow;
        for (itRow = m_origTilesInView->tilesArrangeInRow.begin(); itRow != m_origTilesInView->tilesArrangeInRow.end();)
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

            itRow = m_origTilesInView->tilesArrangeInRow.erase(itRow);
        }

        delete m_origTilesInView;
        m_origTilesInView = NULL;
    }

    if (m_mergedTilesArrange)
    {
        SAFE_DELETE_ARRAY(m_mergedTilesArrange->tileRowHeight);
        SAFE_DELETE_ARRAY(m_mergedTilesArrange->tileColWidth);

        delete m_mergedTilesArrange;
        m_mergedTilesArrange = NULL;
    }
}

int32_t SingleVideoRegionWisePackingGenerator::GetTilesArrangeInViewport(
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
        m_origTilesInView->tilesArrangeInRow.push_back(currRow);
    }

    return ERROR_NONE;
}

int32_t SingleVideoRegionWisePackingGenerator::GenerateMergedTilesArrange()
{
    m_hrTilesInRow = m_tilesNumInViewRow;
    m_hrTilesInCol = m_tileRowNumInView;

    m_mergedTilesArrange->tileRowsNum = 1;
    m_mergedTilesArrange->tileColsNum = m_hrTilesInRow;
    m_mergedTilesArrange->tileRowHeight = new uint16_t[m_mergedTilesArrange->tileRowsNum];
    if (!(m_mergedTilesArrange->tileRowHeight))
        return OMAF_ERROR_NULL_PTR;

    m_mergedTilesArrange->tileRowHeight[0] = m_hrTilesInCol * m_highTileHeight;

    m_mergedTilesArrange->tileColWidth = new uint16_t[m_mergedTilesArrange->tileColsNum];
    if (!(m_mergedTilesArrange->tileColWidth))
        return OMAF_ERROR_NULL_PTR;

#define LCU_SIZE 64

    for (uint8_t i = 0; i < m_mergedTilesArrange->tileColsNum; i++)
    {
        m_mergedTilesArrange->tileColWidth[i] = m_highTileWidth / LCU_SIZE;
    }

    return ERROR_NONE;
}

int32_t SingleVideoRegionWisePackingGenerator::Initialize(
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
    m_highTileWidth = rectRwpk->projRegWidth;
    m_highTileHeight = rectRwpk->projRegHeight;

    m_streamIdxInMedia[0] = videoIdxInMedia[0];

    int32_t ret = GetTilesArrangeInViewport(
                tilesNumInViewport,
                tilesInViewport,
                finalViewportWidth,
                finalViewportHeight);
    if (ret)
        return ret;

    m_tileRowNumInView = m_origTilesInView->tilesArrangeInRow.size();
    if (!m_tileRowNumInView)
        return OMAF_ERROR_SCVP_INCORRECT_RESULT;

    TilesInRow *tilesRow = m_origTilesInView->tilesArrangeInRow.front();
    m_tilesNumInViewRow = tilesRow->size();
    if (!m_tilesNumInViewRow)
        return OMAF_ERROR_SCVP_INCORRECT_RESULT;

    ret = GenerateMergedTilesArrange();
    if (ret)
        return ret;

    return ERROR_NONE;
}

int32_t SingleVideoRegionWisePackingGenerator::GenerateTilesMergeDirection(
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

    uint8_t tileColsNum = m_hrTilesInRow;
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
    }

    SAFE_DELETE_ARRAY(highTilesIdx);

    return ERROR_NONE;
}

int32_t SingleVideoRegionWisePackingGenerator::GenerateDstRwpk(
    uint8_t viewportIdx,
    RegionWisePacking *dstRwpk)
{
    if (!dstRwpk)
        return OMAF_ERROR_NULL_PTR;

    dstRwpk->constituentPicMatching = 0;
    dstRwpk->numRegions             = m_tilesNumInViewRow * m_tileRowNumInView;

    dstRwpk->packedPicWidth         = m_highTileWidth * m_hrTilesInRow;
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
    }

    SAFE_DELETE_ARRAY(highTilesIdx);

    return ERROR_NONE;
}

extern "C" RegionWisePackingGeneratorBase* Create()
{
    return new SingleVideoRegionWisePackingGenerator;
}

extern "C" void Destroy(RegionWisePackingGeneratorBase* rwpkGen)
{
    delete rwpkGen;
    rwpkGen = NULL;
}
