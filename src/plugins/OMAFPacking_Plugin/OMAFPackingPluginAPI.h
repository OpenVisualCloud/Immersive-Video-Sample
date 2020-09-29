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
//! \file:  OMAFPackingPluginAPI.h
//! \brief: VR OMAF packing plugin interfaces
//!
//! Created on April 30, 2019, 6:04 AM
//!

#ifndef _OMAFPACKINGPLUGINAPI_H_
#define _OMAFPACKINGPLUGINAPI_H_

#include <list>
#include <map>
#include <iostream>

#include "360SCVPAPI.h"
#include "error.h"

//!
//! \struct: SingleTile
//! \brief:  define tile information for tiles merging
//!
typedef struct SingleTile
{
    uint8_t  streamIdxInMedia; //the index of video stream in all media streams
    uint8_t  origTileIdx;      //the index of tile in original video frame
    uint16_t dstCTUIndex;      //the index of first CTU of tile in merged video frame
}SingleTile;

using TilesInRow = std::list<SingleTile*>;
using TilesInCol = std::list<SingleTile*>;

//!
//! \struct: TilesMergeDirectionInRow
//! \brief:  define tiles merging direction information
//!          constructed in tile row
//!
typedef struct TilesMergeDirectionInRow
{
    std::list<TilesInRow*> tilesArrangeInRow;
}TilesMergeDirectionInRow;

//!
//! \struct: TilesMergeDirectionInCol
//! \brief:  define tiles merging direction information
//!          constructed in tile column
//!
typedef struct TilesMergeDirectionInCol
{
    std::list<TilesInCol*> tilesArrangeInCol;
}TilesMergeDirectionInCol;

//!
//! \struct: VideoStreamInfo
//! \brief:  define tiles split information and region
//!          wise packing information for video stream
//!
typedef struct VideoStreamInfo
{
    uint8_t           tilesNumInRow;
    uint8_t           tilesNumInCol;
    RegionWisePacking *srcRWPK;
}VideoStreamInfo;

//!
//! \class RegionWisePackingGeneratorBase
//! \brief Define the interface class for region wise packing generator plugin
//!

class RegionWisePackingGeneratorBase
{
public:
    //!
    //! \brief  Constructor
    //!
    RegionWisePackingGeneratorBase() {};

    //!
    //! \brief  Destructor
    //!
    virtual ~RegionWisePackingGeneratorBase() {};

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
    virtual int32_t Initialize(
        std::map<uint8_t, VideoStreamInfo*> *streams,
        uint8_t *videoIdxInMedia,
        uint16_t tilesNumInViewport,
        uint16_t maxSelectedTilesNum,
        void    *externalLog) = 0;

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
    virtual int32_t GenerateDstRwpk(
        TileDef *tilesInViewport,
        RegionWisePacking *dstRwpk) = 0;

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
    virtual int32_t GenerateTilesMergeDirection(
        TileDef *tilesInViewport,
        TilesMergeDirectionInCol *tilesMergeDir) = 0;

    //!
    //! \brief  Get the width of tiles merged picture
    //!
    //! \return uint32_t
    //!         the width of tiles merged picture
    //!
    virtual uint32_t GetPackedPicWidth() = 0;

    //!
    //! \brief  Get the height of tiles merged picture
    //!
    //! \return uint32_t
    //!         the height of tiles merged picture
    //!
    virtual uint32_t GetPackedPicHeight() = 0;

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
    virtual int32_t  GenerateMergedTilesArrange(TileDef *tilesInViewport) = 0;

    //!
    //! \brief  Get the tiles arrangement information in tiles
    //!         merged picture
    //!
    //! \return TileArrangement*
    //!         the pointer to the tiles arrangement information
    //!
    virtual TileArrangement* GetMergedTilesArrange() = 0;

    //!
    //! \brief  Get total tiles number in packed sub-picture
    //!
    //! \return uint32_t
    //!         the total tiles number in packed sub-picture
    //!
    virtual uint32_t GetTilesNumInPackedPic() = 0;

protected:
};

typedef RegionWisePackingGeneratorBase* CreateRWPKGenerator();
typedef void DestroyRWPKGenerator(RegionWisePackingGeneratorBase*);

#define SAFE_DELETE_MEMORY(x) \
    if (x)               \
    {                    \
        delete x;        \
        x = NULL;        \
    }

#define SAFE_DELETE_ARRAY(x)  \
    if (x)               \
    {                    \
        delete[] x;      \
        x = NULL;        \
    }

#endif /* _OMAFPACKINGPLUGINAPI_H_ */
