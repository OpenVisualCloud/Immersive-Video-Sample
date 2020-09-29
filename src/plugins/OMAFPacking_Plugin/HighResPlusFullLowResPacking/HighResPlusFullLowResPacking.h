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
//! \file:   HighResPlusFullLowResPacking.h
//! \brief:  Region wise packing generator class definition for packing of high
//!          resolution video plus full low resolution video
//! \detail: Define the operation of region wise packing generator for packing of
//!          high resolution video plus full low resolution video
//!
//! Created on April 30, 2019, 6:04 AM
//!

#ifndef _HIGHRESPLUSFULLLOWRESPACKING_H_
#define _HIGHRESPLUSFULLLOWRESPACKING_H_

#include "OMAFPackingPluginAPI.h"

typedef struct RectArea
{
    uint32_t xTopLeft;
    uint32_t yTopLeft;
    uint32_t xBotRight;
    uint32_t yBotRight;
}RectArea;

//!
//! \class HighPlusFullLowRegionWisePackingGenerator
//! \brief Define the operation of region wise packing generator for packing of
//!        high resolution video plus full low resolution video
//!

class HighPlusFullLowRegionWisePackingGenerator : public RegionWisePackingGeneratorBase
{
public:
    //!
    //! \brief  Constructor
    //!
    HighPlusFullLowRegionWisePackingGenerator();

    HighPlusFullLowRegionWisePackingGenerator(const HighPlusFullLowRegionWisePackingGenerator& src);

    HighPlusFullLowRegionWisePackingGenerator& operator=(const HighPlusFullLowRegionWisePackingGenerator&) = default;

    //!
    //! \brief  Destructor
    //!
    ~HighPlusFullLowRegionWisePackingGenerator();

    //!
    //! \brief  Initialize the region wise packing generator
    //!
    //! \param  [in] streams
    //!         pointer to the map of video stream index and info
    //! \param  [in] videoIdxInMedia
    //!         pointer to the index of each video in media streams
    //! \param  [in] tilesNumInViewport
    //!         the number of tiles in viewport
    //! \param  [in] maxSelectedTilesNum
    //!         the maxmum selected tiles number in viewport
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t Initialize(
        std::map<uint8_t, VideoStreamInfo*> *streams,
        uint8_t *videoIdxInMedia,
        uint16_t tilesNumInViewport,
        uint16_t maxSelectedTilesNum,
        void    *externalLog);

    //!
    //! \brief  Generate the region wise packing information for
    //!         specified viewport
    //!
    //! \param  [in]  tilesInViewport
    //!         the pointer to all tiles information in packed
    //!         sub-picture
    //! \param  [out] dstRwpk
    //!         pointer to the region wise packing information for
    //!         the specified viewport
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GenerateDstRwpk(TileDef *tilesInViewport, RegionWisePacking *dstRwpk);

    //!
    //! \brief  Generate the tiles merging direction information for
    //!         specified viewport
    //!
    //! \param  [in]  tilesInViewport
    //!         the pointer to all tiles information in packed
    //!         sub-picture
    //! \param  [out] tilesMergeDir
    //!         pointer to the tiles merging direction information for
    //!         the specified viewport
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GenerateTilesMergeDirection(
        TileDef *tilesInViewport,
        TilesMergeDirectionInCol *tilesMergeDir);

    //!
    //! \brief  Get the width of tiles merged picture
    //!
    //! \return uint32_t
    //!         the width of tiles merged picture
    //!
    uint32_t GetPackedPicWidth() { return m_packedPicWidth; };

    //!
    //! \brief  Get the height of tiles merged picture
    //!
    //! \return uint32_t
    //!         the height of tiles merged picture
    //!
    uint32_t GetPackedPicHeight() { return m_packedPicHeight; };

    //!
    //! \brief  Get the tiles arrangement information in tiles
    //!         merged picture
    //!
    //! \return TileArrangement*
    //!         the pointer to the tiles arrangement information
    //!
    TileArrangement* GetMergedTilesArrange() { return m_mergedTilesArrange; };

    //!
    //! \brief  Get total tiles number in packed sub-picture
    //!
    //! \return uint32_t
    //!         the total tiles number in packed sub-picture
    //!
    uint32_t GetTilesNumInPackedPic() { return m_tilesNumInPackedPic; };

    //!
    //! \brief  Generate all tiles information in packed sub-picture
    //!
    //! \param  [out] tilesInViewport
    //!         pointer to all tiles information in packed
    //!         sub-picture
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GenerateMergedTilesArrange(TileDef *tilesInViewport);

private:

    //!
    //! \brief  Choose low resolution tiles for packed sub-picture
    //!
    //! \param  [out]  tilesInViewport
    //!         the pointer to all tiles information in packed
    //!         sub-picture
    //! \param  [in] supplementaryLRTilesNum
    //!         repetitive low resolution tiles number to make sure
    //!         all low resolution tiles can be packed in sub-picture
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t ChooseLowResTilesForPacking(TileDef *tilesInViewport, uint32_t supplementaryLRTilesNum);

    //!
    //! \brief  Generate high resolution tiles layout in packed
    //!         sub-picture according to high resolution tiles
    //!         number in viewport
    //!
    //! \param  [in] tilesNumInView
    //!         high resolution tiles number in viewport
    //! \param  [in] maxSelectedTilesNum
    //!         the maxmum selected tiles number in viewport
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GenerateHighTilesArrange(uint16_t tilesNumInView, uint16_t maxSelectedTilesNum);

private:
    std::map<uint8_t, RegionWisePacking*> m_rwpkMap;             //!< map of original region wise packing information of all video streams
    uint32_t                              m_highResWidth;
    uint32_t                              m_highResHeight;
    uint32_t                              m_lowResWidth;
    uint32_t                              m_lowResHeight;
    uint32_t                              m_packedPicWidth;      //!< the width of tiles merged picture
    uint32_t                              m_packedPicHeight;     //!< the height of tiles merged picture
    uint16_t                              m_selectedTilesNum;
    uint16_t                              m_maxSelectedTilesNum;
    TileArrangement                       *m_mergedTilesArrange; //!< pointer to the tiles arrangement information
    uint8_t                               m_streamIdxInMedia[2]; //!< array for video index in media streams
    uint8_t                               m_origHRTilesInRow;    //!< the number of tiles in one row in high resolution video stream
    uint8_t                               m_origHRTilesInCol;    //!< the number of tiles in one column in high resolution video stream
    uint16_t                              m_highTileWidth;       //!< the width of high resolution tile
    uint16_t                              m_highTileHeight;      //!< the height of high resolution tile
    uint8_t                               m_origLRTilesInRow;    //!< the number of tiles in one row in low resolution video stream
    uint8_t                               m_origLRTilesInCol;    //!< the number of tiles in one column in low resolution video stream
    uint16_t                              m_lowTileWidth;        //!< the width of low resolution tile
    uint16_t                              m_lowTileHeight;       //!< the height of low resolution tile

    uint8_t                               m_hrTilesInRow;        //!< the number of high resolution tiles in one row in tiles merged picture
    uint8_t                               m_hrTilesInCol;        //!< the number of high resolution tiles in one column in tiles merged picture
    uint8_t                               m_lrTilesInRow;        //!< the number of low resolution tiles in one row in tiles merged picture
    uint8_t                               m_lrTilesInCol;        //!< the number of low resolution tiles in one column in tiles merged picture
    std::map<uint32_t, RectArea>          m_rectAreas;

    uint32_t                              m_regNum;

    uint32_t                              m_tilesNumInPackedPic;
};

extern "C" RegionWisePackingGeneratorBase* Create();

extern "C" void Destroy(RegionWisePackingGeneratorBase* rwpkGen);

#endif /* _HIGHRESPLUSFULLLOWRESPACKING_H_ */
