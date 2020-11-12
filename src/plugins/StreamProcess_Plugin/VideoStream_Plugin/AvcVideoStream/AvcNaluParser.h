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
//! \file:   AvcNaluParser.h
//! \brief:  Avc nalu parser class definition
//! \detail: Define the operation and needed data for Avc nalu parser, including
//!          parsing SPS, PPS, projection type SEI, region wise packing SEI,
//!          viewport SEI and so on, called by VideoStream to parse bitstream
//!          header data to initialize the video stream.
//!
//! Created on April 30, 2019, 6:04 AM
//!

#ifndef _AVCNALUPARSER_H_
#define _AVCNALUPARSER_H_

#include "NaluParser.h"

VCD_NS_BEGIN

//!
//! \class AvcNaluParser
//! \brief Define the operation and needed data for Avc nalu parser
//!

class AvcNaluParser : public NaluParser
{
public:

    //!
    //! \brief  Constructor
    //!
    AvcNaluParser() {};

    //!
    //! \brief  Copy Constructor
    //!
    //! \param  [in] scvpHandle
    //!         360SCVP library handle
    //! \param  [in] scvpParam
    //!         360SCVP library initial parameter
    //!
    AvcNaluParser(void *scvpHandle, param_360SCVP *scvpParam) : NaluParser(scvpHandle, scvpParam) {};

    //!
    //! \brief  Destructor
    //!
    virtual ~AvcNaluParser() {};

    //!
    //! \brief  Parse the header data of video bitstream, which
    //!         should include SPS, PPS and Projection type
    //!         SEI
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t ParseHeaderData() { return ERROR_NONE; };

    //!
    //! \brief  Get the width of video frame parsed
    //!         from the header data
    //!
    //! \return uint16_t
    //!         the width of the video frame
    //!
    virtual uint16_t GetSrcWidth() { return 0; };

    //!
    //! \brief  Get the height of video frame parsed
    //!         from the header data
    //!
    //! \return uint16_t
    //!         the height of the video frame
    //!
    virtual uint16_t GetSrcHeight() { return 0; };

    //!
    //! \brief  Get the tiles number in row in video
    //!         frame parsed from the header data
    //!
    //! \return uint8_t
    //!         the tiles number in row in video frame
    //!
    virtual uint8_t  GetTileInRow() { return 0; };

    //!
    //! \brief  Get the tiles number in column in video
    //!         frame parsed from the header data
    //!
    //! \return uint8_t
    //!         the tiles number in column in video frame
    //!
    virtual uint8_t  GetTileInCol() { return 0; };

    //!
    //! \brief  Get the tile information of the specified
    //!         tile, including tile position and tile size
    //!
    //! \param  [in] tileIdx
    //!         the index of the specified tile in video frame
    //! \param  [in] tileInfo
    //!         the pointer to the tile information of the
    //!         specified tile
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t GetTileInfo(uint16_t tileIdx, TileInfo *tileInfo) { return ERROR_NONE; };

    //!
    //! \brief  Parse nalu information for each tile in frame
    //!         bitstream
    //!
    //! \param  [in] frameData
    //!         pointer to the frame bitstream data
    //! \param  [in] frameDataSize
    //!         size of frame bitstream data
    //! \param  [in] tilesNum
    //!         tiles number in video frame
    //! \param  [out] tilesInfo
    //!         the pointer to the tile information of all tiles
    //!         in video frame, including nalu information
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t ParseSliceNalu(
        uint8_t *frameData,
        int32_t frameDataSize,
        uint16_t tilesNum,
        TileInfo *tilesInfo) { return ERROR_NONE; };

private:

    //!
    //! \brief  Parse the projection type SEI in header data
    //!
    //! \return int16_t
    //!         the projection type parsed from projection type SEI
    //!
    virtual int16_t ParseProjectionTypeSei() { return 0; };

private:
};

VCD_NS_END;
#endif /* _AVCNALUPARSER_H_ */
