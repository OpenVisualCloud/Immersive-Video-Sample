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
//! \file:   NaluParser.h
//! \brief:  Nalu parser base class definition
//! \detail: Define the operation and needed data for Nalu parser, including
//!          parsing SPS, PPS, projection type SEI, region wise packing SEI,
//!          viewport SEI and so on, called by VideoStream to parse bitstream
//!          header data to initialize the video stream.
//!
//! Created on April 30, 2019, 6:04 AM
//!

#ifndef _NALUPARSER_H_
#define _NALUPARSER_H_

#include "stddef.h"
#include "360SCVPAPI.h"
#include "VROmafPacking_def.h"
#include "OmafStructure.h"

//!
//! \class NaluParser
//! \brief Define the operation and needed data for Nalu parser
//!

class NaluParser
{
public:

    //!
    //! \brief  Constructor
    //!
    NaluParser()
    {
        m_360scvpHandle = NULL;
        m_360scvpParam  = NULL;
        m_vpsNalu       = NULL;
        m_spsNalu       = NULL;
        m_ppsNalu       = NULL;
        m_projNalu      = NULL;
        m_picInfo       = NULL;
    };

    //!
    //! \brief  Copy Constructor
    //!
    //! \param  [in] scvpHandle
    //!         360SCVP library handle
    //! \param  [in] scvpParam
    //!         360SCVP library initial parameter
    //!
    NaluParser(void *scvpHandle, param_360SCVP *scvpParam)
    {
        m_vpsNalu       = NULL;
        m_spsNalu       = NULL;
        m_ppsNalu       = NULL;
        m_projNalu      = NULL;
        m_picInfo       = NULL;

        m_360scvpHandle = scvpHandle;
        m_360scvpParam  = scvpParam;
        if (!m_360scvpHandle || !m_360scvpParam)
            return;
    };

    //!
    //! \brief  Destructor
    //!
    virtual ~NaluParser() {};

    //!
    //! \brief  Parse the header data of video bitstream, which
    //!         should include SPS, PPS and Projection type
    //!         SEI
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t ParseHeaderData() = 0;

    //!
    //! \brief  Get the width of video frame parsed
    //!         from the header data
    //!
    //! \return uint16_t
    //!         the width of the video frame
    //!
    virtual uint16_t GetSrcWidth() = 0;

    //!
    //! \brief  Get the height of video frame parsed
    //!         from the header data
    //!
    //! \return uint16_t
    //!         the height of the video frame
    //!
    virtual uint16_t GetSrcHeight() = 0;

    //!
    //! \brief  Get the tiles number in row in video
    //!         frame parsed from the header data
    //!
    //! \return uint8_t
    //!         the tiles number in row in video frame
    //!
    virtual uint8_t  GetTileInRow() = 0;

    //!
    //! \brief  Get the tiles number in column in video
    //!         frame parsed from the header data
    //!
    //! \return uint8_t
    //!         the tiles number in column in video frame
    //!
    virtual uint8_t  GetTileInCol() = 0;

    //!
    //! \brief  Get the projection type of the video
    //!         frame parsed from projection type SEI
    //!
    //! \return int16_t
    //!         the projection type of the video frame,
    //!         0 is ERP, 1 is CubeMap, and -1 if failed
    //!
    int16_t GetProjectionType()
    {
        int16_t projType = ParseProjectionTypeSei();
        return projType;
    };

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
    virtual int32_t GetTileInfo(uint16_t tileIdx, TileInfo *tileInfo) = 0;

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
        TileInfo *tilesInfo) = 0;

    //!
    //! \brief  Get the SPS nalu of the bitstream
    //!
    //! \return Nalu*
    //!         the pointer to the SPS nalu
    //!
    Nalu* GetSPSNalu() { return m_spsNalu; };

    //!
    //! \brief  Get the PPS nalu of the bitstream
    //!
    //! \return Nalu*
    //!         the pointer to the PPS nalu
    //!
    Nalu* GetPPSNalu() { return m_ppsNalu; };

private:

    //!
    //! \brief  Parse the projection type SEI in header data
    //!
    //! \return int16_t
    //!         the projection type parsed from projection type SEI
    //!
    virtual int16_t ParseProjectionTypeSei() = 0;

protected:
    void          *m_360scvpHandle;   //!< 360SCVP library handle
    param_360SCVP *m_360scvpParam;    //!< 360SCVP library initial parameter
    Nalu          *m_vpsNalu;         //!< pointer to the VPS nalu
    Nalu          *m_spsNalu;         //!< pointer to the SPS nalu
    Nalu          *m_ppsNalu;         //!< pointer to the PPS nalu
    Nalu          *m_projNalu;        //!< pointer to the projection type SEI
    Param_PicInfo *m_picInfo;         //!< pointer to the basic picture information of video stream
};

#endif /* _NALUPARSER_H_ */
