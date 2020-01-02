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
//! \file:   OneVideoExtractorTrackGenerator.h
//! \brief:  One video stream extractor track generator class definition
//! \detail: Define the operation of extractor track generator for one
//!          video stream
//!
//! Created on April 30, 2019, 6:04 AM
//!

#ifndef _ONEVIDEOEXTRACTORTRACKGENERATOR_H_
#define _ONEVIDEOEXTRACTORTRACKGENERATOR_H_

#include "ExtractorTrackGenerator.h"
#include "../utils/OmafStructure.h"

VCD_NS_BEGIN

//!
//! \class OneVideoExtractorTrackGenerator
//! \brief Define the operation of extractor track generator
//!        for one video stream
//!

class OneVideoExtractorTrackGenerator : public ExtractorTrackGenerator
{
public:
    //!
    //! \brief  Constructor
    //!
    OneVideoExtractorTrackGenerator()
    {
        m_videoIdxInMedia = NULL;
        m_360scvpParam    = NULL;
        m_360scvpHandle   = NULL;
        m_tilesInViewport = NULL;
        m_viewInfo        = NULL;
        m_tilesNumInViewport  = 0;
        m_finalViewportWidth  = 0;
        m_finalViewportHeight = 0;
        m_videoWidth    = 0;
        m_videoHeight   = 0;
        m_tileInRow     = 0;
        m_tileInCol     = 0;
        m_tileWidth     = 0;
        m_tileHeight    = 0;
        m_tilesInfo       = NULL;
        m_projType        = VCD::OMAF::ProjectionFormat::PF_ERP;
        m_packedPicWidth  = 0;
        m_packedPicHeight = 0;
        m_origVPSNalu     = NULL;
        m_origSPSNalu     = NULL;
        m_origPPSNalu     = NULL;
    };

    //!
    //! \brief  Copy Constructor
    //!
    //! \param  [in] initInfo
    //!         initial information input by the library interface
    //! \param  [in] streams
    //!         pointer to the media streams map set up in OmafPackage
    //!
    OneVideoExtractorTrackGenerator(InitialInfo *initInfo, std::map<uint8_t, MediaStream*> *streams) : ExtractorTrackGenerator(initInfo, streams)
    {
        m_videoIdxInMedia = NULL;
        m_360scvpParam    = NULL;
        m_360scvpHandle   = NULL;
        m_tilesInViewport = NULL;
        m_viewInfo        = NULL;
        m_tilesNumInViewport  = 0;
        m_finalViewportWidth  = 0;
        m_finalViewportHeight = 0;
        m_videoWidth    = 0;
        m_videoHeight   = 0;
        m_tileInRow     = 0;
        m_tileInCol     = 0;
        m_tileWidth     = 0;
        m_tileHeight    = 0;
        m_tilesInfo       = NULL;
        m_projType        = VCD::OMAF::ProjectionFormat::PF_ERP;
        m_packedPicWidth  = 0;
        m_packedPicHeight = 0;
        m_origVPSNalu     = NULL;
        m_origSPSNalu     = NULL;
        m_origPPSNalu     = NULL;
    };

    //!
    //! \brief  Destructor
    //!
    virtual ~OneVideoExtractorTrackGenerator();

    //!
    //! \brief  Initialize the extractor track generator
    //!         for two resolutions video streams
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t Initialize();

    //!
    //! \brief  Generate all extractor tracks
    //!
    //! \param  [in] extractorTrackMap
    //!         pointer to extractor tracks map which holds
    //!         all extractor tracks
    //! \param  [in] streams
    //!         pointer to the media streams map set up in OmafPackage
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t GenerateExtractorTracks(std::map<uint8_t, ExtractorTrack*>& extractorTrackMap, std::map<uint8_t, MediaStream*> *streams);

private:
    //!
    //! \brief  Calculate the total viewport number
    //!         according to the initial information
    //!
    //! \return uint16_t
    //!         the total viewport number
    //!
    virtual uint16_t CalculateViewportNum();

    //!
    //! \brief  Fill the region wise packing information
    //!         for the specified viewport
    //!
    //! \param  [in] viewportIdx
    //!         the index of the specified viewport
    //! \param  [in] dstRwpk
    //!         pointer to the region wise packing information for the
    //!         specified viewport generated according to srcRwpk and
    //!         two resolutions tiles merging strategy
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t FillDstRegionWisePacking(uint8_t viewportIdx, RegionWisePacking *dstRwpk);

    //!
    //! \brief  Fill the tiles merging direction information
    //!         for the specified viewport
    //!
    //! \param  [in] viewportIdx
    //!         the index of the specified viewport
    //! \param  [in] tilesMergeDir
    //!         pointer to the tiles merging direction information
    //!         for the specified viewport generated according to
    //!         two resolutions tiles merging strategy
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t FillTilesMergeDirection(
        uint8_t viewportIdx,
        TilesMergeDirectionInCol *tilesMergeDir);

    //!
    //! \brief  Fill the content coverage information
    //!         for the specified viewport
    //!
    //! \param  [in] viewportIdx
    //!         the index of the specified viewport
    //! \param  [in] dstCovi
    //!         pointer to the content coverage information for the
    //!         specified viewport generated according to srcCovi and
    //!         two resolutions tiles merging strategy
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t FillDstContentCoverage(uint8_t viewportIdx, ContentCoverage *dstCovi);

    //!
    //! \brief  Check the validation of initial information
    //!         input by library interface, like whether the
    //!         TilesMergingType is correct compared to actual
    //!         streams information, meanwhile fill the lacked
    //!         information according to actual streams information
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t CheckAndFillInitInfo();

    //!
    //! \brief  Generate the new SPS for tiles merged bitstream
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t GenerateNewSPS();

    //!
    //! \brief  Generate the new PPS for tiles merged bitstream
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t GenerateNewPPS();

private:
    uint8_t             *m_videoIdxInMedia;    //!< pointer to index of video streams in media streams
    param_360SCVP       *m_360scvpParam;       //!< 360SCVP library initial parameter
    void                *m_360scvpHandle;      //!< 360SCVP library handle
    TileDef             *m_tilesInViewport;    //!< the list of tiles inside the viewport
    Param_ViewPortInfo  *m_viewInfo;           //!< pointer to the viewport information for 360SCVP library
    int32_t             m_tilesNumInViewport;  //!< tiles number in viewport
    int32_t             m_finalViewportWidth;  //!< the final viewport width calculated by 360SCVP library
    int32_t             m_finalViewportHeight; //!< the final viewport height calculated by 360SCVP library

    uint16_t            m_videoWidth;          //!< frame width of high resolution video stream
    uint16_t            m_videoHeight;         //!< frame height of high resolution video stream
    //uint16_t            m_lowResWidth;
    //uint16_t            m_lowResHeight;
    uint8_t             m_tileInRow;           //!< the number of high resolution tiles in one row in original picture
    uint8_t             m_tileInCol;           //!< the number of high resolution tiles in one column in original picture
    uint16_t            m_tileWidth;           //!< the width of high resolution tile
    uint16_t            m_tileHeight;          //!< the height of high resolution tile
    TileInfo            *m_tilesInfo;          //!< pointer to tile information of all tiles in high resolution video stream
    VCD::OMAF::ProjectionFormat    m_projType;           //!< the projection type
    uint32_t            m_packedPicWidth;      //!< the width of tiles merged picture
    uint32_t            m_packedPicHeight;     //!< the height of tiles merged picture
    Nalu                *m_origVPSNalu;        //!< the pointer to original VPS nalu of high resolution video stream
    Nalu                *m_origSPSNalu;        //!< the pointer to original SPS nalu of high resolution video stream
    Nalu                *m_origPPSNalu;        //!< the pointer to original PPS nalu of high resolution video stream
};

VCD_NS_END;
#endif /* _TWORESEXTRACTORTRACKGENERATOR_H_ */
