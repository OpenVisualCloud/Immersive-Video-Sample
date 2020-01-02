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
//! \file:   RegionWisePackingGenerator.h
//! \brief:  Region wise packing generator base class definition
//! \detail: Define the basic operation of region wise packing generator.
//!
//! Created on April 30, 2019, 6:04 AM
//!

#ifndef _REGIONWISEPACKINGGENERATOR_H_
#define _REGIONWISEPACKINGGENERATOR_H_

#include <list>
#include <map>

#include "OmafPackingCommon.h"
#include "VROmafPacking_data.h"
#include "definitions.h"
#include "MediaStream.h"

VCD_NS_BEGIN

//!
//! \struct: SingleTile
//! \brief:  define tile information for tiles merging
//!
struct SingleTile
{
    uint8_t  streamIdxInMedia; //the index of video stream in all media streams
    uint8_t  origTileIdx;      //the index of tile in original video frame
    uint16_t dstCTUIndex;      //the index of first CTU of tile in merged video frame
};

using TilesInRow = std::list<SingleTile*>;
using TilesInCol = std::list<SingleTile*>;

//!
//! \struct: TilesMergeDirectionInRow
//! \brief:  define tiles merging direction information
//!          constructed in tile row
//!
struct TilesMergeDirectionInRow
{
    std::list<TilesInRow*> tilesArrangeInRow;
};

//!
//! \struct: TilesMergeDirectionInCol
//! \brief:  define tiles merging direction information
//!          constructed in tile column
//!
struct TilesMergeDirectionInCol
{
    std::list<TilesInCol*> tilesArrangeInCol;
};

//!
//! \class RegionWisePackingGenerator
//! \brief Define the basic operation of region wise packing generator
//!

class RegionWisePackingGenerator
{
public:
    //!
    //! \brief  Constructor
    //!
    RegionWisePackingGenerator()
    {
        m_packedPicWidth  = 0;
        m_packedPicHeight = 0;
    };

    //!
    //! \brief  Destructor
    //!
    virtual ~RegionWisePackingGenerator() {};

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
    virtual int32_t Initialize(
        std::map<uint8_t, MediaStream*> *streams,
        uint8_t *videoIdxInMedia,
        uint8_t tilesNumInViewport,
        TileDef *tilesInViewport,
        int32_t finalViewportWidth,
        int32_t finalViewportHeight) = 0;

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
    virtual int32_t GenerateDstRwpk(
        uint8_t viewportIdx,
        RegionWisePacking *dstRwpk) = 0;

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
    virtual int32_t GenerateTilesMergeDirection(
        uint8_t viewportIdx,
        TilesMergeDirectionInCol *tilesMergeDir) = 0;

    //!
    //! \brief  Get the number of tiles in one row in viewport
    //!
    //! \return uint8_t
    //!         the number of tiles in one row in viewport
    //!
    virtual uint8_t GetTilesNumInViewportRow() = 0;

    //!
    //! \brief  Get the number of tile rows in viewport
    //!
    //! \return uint8_t
    //!         the number of tile rows in viewport
    //!
    virtual uint8_t GetTileRowNumInViewport() = 0;

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

protected:
    std::map<uint8_t, RegionWisePacking*> m_rwpkMap;             //!< map of original region wise packing information of all video streams
    uint32_t                              m_packedPicWidth;      //!< the width of tiles merged picture
    uint32_t                              m_packedPicHeight;     //!< the height of tiles merged picture
    TileArrangement                       *m_mergedTilesArrange; //!< pointer to the tiles arrangement information
};

VCD_NS_END;
#endif /* _REGIONWISEPACKINGGENERATOR_H_ */
