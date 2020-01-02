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
//! \file:   TwoResRegionWisePackingGenerator.h
//! \brief:  Two resolutions region wise packing generator class definition
//! \detail: Define the operation of two resolutions region wise packing generator.
//!
//! Created on April 30, 2019, 6:04 AM
//!

#ifndef _TWORESREGIONWISEPACKINGGENERATOR_H_
#define _TWORESREGIONWISEPACKINGGENERATOR_H_

#include "RegionWisePackingGenerator.h"

VCD_NS_BEGIN

//!
//! \class TwoResRegionWisePackingGenerator
//! \brief Define the operation of two resolutions region wise packing generator
//!

class TwoResRegionWisePackingGenerator : public RegionWisePackingGenerator
{
public:
    //!
    //! \brief  Constructor
    //!
    TwoResRegionWisePackingGenerator();

    //!
    //! \brief  Destructor
    //!
    ~TwoResRegionWisePackingGenerator();

    //!
    //! \brief  Initialize the region wise packing generator
    //!
    //! \param  [in] streams
    //!         pointer to the media streams map set up in OmafPackage
    //! \param  [in] videoIdxInMedia
    //!         pointer to the index of each video in media streams
    //! \param  [in] tilesNumInViewport
    //!         the number of tiles in viewport
    //! \param  [in] tilesInViewport
    //!         pointer to tile information of all tiles in viewport
    //! \param  [in] finalViewportWidth
    //!         the final viewport width calculated by 360SCVP library
    //! \param  [in] finalViewportHeight
    //!         the final viewport height calculated by 360SCVP library
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t Initialize(
        std::map<uint8_t, MediaStream*> *streams,
        uint8_t *videoIdxInMedia,
        uint8_t tilesNumInViewport,
        TileDef *tilesInViewport,
        int32_t finalViewportWidth,
        int32_t finalViewportHeight);

    //!
    //! \brief  Generate the region wise packing information for
    //!         specified viewport
    //!
    //! \param  [in]  viewportIdx
    //!         the index of specified viewport
    //! \param  [out] dstRwpk
    //!         pointer to the region wise packing information for
    //!         the specified viewport
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GenerateDstRwpk(uint8_t viewportIdx, RegionWisePacking *dstRwpk);

    //!
    //! \brief  Generate the tiles merging direction information for
    //!         specified viewport
    //!
    //! \param  [in]  viewportIdx
    //!         the index of specified viewport
    //! \param  [out] tilesMergeDir
    //!         pointer to the tiles merging direction information for
    //!         the specified viewport
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GenerateTilesMergeDirection(
        uint8_t viewportIdx,
        TilesMergeDirectionInCol *tilesMergeDir);

    //!
    //! \brief  Get the number of tiles in one row in viewport
    //!
    //! \return uint8_t
    //!         the number of tiles in one row in viewport
    //!
    uint8_t GetTilesNumInViewportRow() { return m_tilesNumInViewRow; };

    //!
    //! \brief  Get the number of tile rows in viewport
    //!
    //! \return uint8_t
    //!         the number of tile rows in viewport
    //!
    uint8_t GetTileRowNumInViewport() { return m_tileRowNumInView; };

private:
    //!
    //! \brief  Get the original high resolution tiles arrangement
    //!         in viewport
    //!
    //! \param  [in] tilesNumInViewport
    //!         the number of high resolution tiles in viewport
    //! \param  [in] tilesInViewport
    //!         pointer to the tile information of all tiles
    //!         in viewport
    //! \param  [in] finalViewportWidth
    //!         the final viewport width calculated by 360SCVP library
    //! \param  [in] finalViewportHeight
    //!         the final viewport height calculated by 360SCVP library
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GetOrigHighResTilesArrange(
        uint8_t tilesNumInViewport,
        TileDef *tilesInViewport,
        int32_t finalViewportWidth,
        int32_t finalViewportHeight);

    //!
    //! \brief  Generate tiles arrangement in tiles merged picture
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GenerateMergedTilesArrange();

private:
    uint8_t                  m_streamIdxInMedia[2]; //!< array for video index in media streams
    TilesMergeDirectionInRow *m_highResTilesInView; //!< pointer to original high resolution tiles arrangement in viewport
    uint8_t                  m_tilesNumInViewRow;   //!< the number of high resolution tiles in one row in viewport
    uint8_t                  m_tileRowNumInView;    //!< the number of high resolution tile rows in viewport
    uint8_t                  m_origHRTilesInRow;    //!< the number of tiles in one row in high resolution video stream
    uint8_t                  m_origHRTilesInCol;    //!< the number of tiles in one column in high resolution video stream
    uint16_t                 m_highTileWidth;       //!< the width of high resolution tile
    uint16_t                 m_highTileHeight;      //!< the height of high resolution tile
    uint8_t                  m_origLRTilesInRow;    //!< the number of tiles in one row in low resolution video stream
    uint8_t                  m_origLRTilesInCol;    //!< the number of tiles in one column in low resolution video stream
    uint16_t                 m_lowTileWidth;        //!< the width of low resolution tile
    uint16_t                 m_lowTileHeight;       //!< the height of low resolution tile

    uint8_t                  m_hrTilesInRow;        //!< the number of high resolution tiles in one row in tiles merged picture
    uint8_t                  m_hrTilesInCol;        //!< the number of high resolution tiles in one column in tiles merged picture
    uint8_t                  m_lrTilesInRow;        //!< the number of low resolution tiles in one row in tiles merged picture
    uint8_t                  m_lrTilesInCol;        //!< the number of low resolution tiles in one column in tiles merged picture
};

VCD_NS_END;
#endif /* _TWORESREGIONWISEPACKING_H_ */
