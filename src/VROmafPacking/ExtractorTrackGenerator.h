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
//! \file:   ExtractorTrackGenerator.h
//! \brief:  Extractor track generator base class definition
//! \detail: Define the basic operation of extractor track generator.
//!
//! Created on April 30, 2019, 6:04 AM
//!

#ifndef _EXTRACTORTRACKGENERATOR_H_
#define _EXTRACTORTRACKGENERATOR_H_

#include "VROmafPacking_data.h"
#include "definitions.h"
#include "MediaStream.h"
#include "ExtractorTrack.h"
#include "RegionWisePackingGenerator.h"

VCD_NS_BEGIN

//!
//! \class ExtractorTrackGenerator
//! \brief Define the basic operation of extractor track generator
//!

class ExtractorTrackGenerator
{
public:
    //!
    //! \brief  Constructor
    //!
    ExtractorTrackGenerator()
    {
        m_initInfo = NULL;
        m_streams  = NULL;
        m_viewportNum = 0;
        m_rwpkGen     = NULL;
        m_newSPSNalu  = NULL;
        m_newPPSNalu  = NULL;
    };

    //!
    //! \brief  Copy Constructor
    //!
    //! \param  [in] initInfo
    //!         initial information input by the library interface
    //! \param  [in] streams
    //!         pointer to the media streams map set up in OmafPackage
    //!
    ExtractorTrackGenerator(InitialInfo *initInfo, std::map<uint8_t, MediaStream*> *streams)
    {
        m_initInfo = initInfo;
        m_streams  = streams;
        m_viewportNum = 0;
        m_rwpkGen     = NULL;
        m_newSPSNalu  = NULL;
        m_newPPSNalu  = NULL;
    };

    //!
    //! \brief  Destructor
    //!
    virtual ~ExtractorTrackGenerator() {};

    //!
    //! \brief  Initialize the extractor track generator
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t Initialize() = 0;

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
    virtual int32_t GenerateExtractorTracks(std::map<uint8_t, ExtractorTrack*>& extractorTrackMap, std::map<uint8_t, MediaStream*> *streams) = 0;

    //!
    //! \brief  Get the new SPS nalu for tiles merged bitstream
    //!
    //! \return Nalu*
    //!         the pointer to the new SPS nalu
    //!
    Nalu* GetNewSPS() { return m_newSPSNalu; };

    //!
    //! \brief  Get the new PPS nalu for tiles merged bitstream
    //!
    //! \return Nalu*
    //!         the pointer to the new PPS nalu
    //!
    Nalu* GetNewPPS() { return m_newPPSNalu; };

private:
    //!
    //! \brief  Calculate the total viewport number
    //!         according to the initial information
    //!
    //! \return uint16_t
    //!         the total viewport number
    //!
    virtual uint16_t CalculateViewportNum() = 0;

    //!
    //! \brief  Fill the region wise packing information
    //!         for the specified viewport
    //!
    //! \param  [in] viewportIdx
    //!         the index of the specified viewport
    //! \param  [in] dstRwpk
    //!         pointer to the region wise packing information for the
    //!         specified viewport generated according to srcRwpk and
    //!         detailed tiles merging strategy
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t FillDstRegionWisePacking(uint8_t viewportIdx, RegionWisePacking *dstRwpk) = 0;

    //!
    //! \brief  Fill the tiles merging direction information
    //!         for the specified viewport
    //!
    //! \param  [in] viewportIdx
    //!         the index of the specified viewport
    //! \param  [out] tilesMergeDir
    //!         pointer to the tiles merging direction information for the
    //!         specified viewport generated according to the detailed
    //!         tiles merging strategy
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t FillTilesMergeDirection(
        uint8_t viewportIdx,
        TilesMergeDirectionInCol *tilesMergeDir) = 0;

    //!
    //! \brief  Fill the content coverage information
    //!         for the specified viewport
    //!
    //! \param  [in] viewportIdx
    //!         the index of the specified viewport
    //! \param  [in] dstCovi
    //!         pointer to the content coverage information for the
    //!         specified viewport generated according to srcCovi and
    //!         detailed tiles merging strategy
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t FillDstContentCoverage(uint8_t viewportIdx, ContentCoverage *dstCovi) = 0;

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
    virtual int32_t CheckAndFillInitInfo() = 0;

    //!
    //! \brief  Generate the new SPS nalu for tiles merged bitstream
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t GenerateNewSPS() = 0;

    //!
    //! \brief  Generate the new PPS nalu for tiles merged bitstream
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t GenerateNewPPS() = 0;

protected:
    InitialInfo                     *m_initInfo;   //!< initial information input by library interface
    std::map<uint8_t, MediaStream*> *m_streams;    //!< media streams map set up in OmafPackage
    uint16_t                        m_viewportNum; //!< viewport number calculated according to initial information
    RegionWisePackingGenerator      *m_rwpkGen;    //!< pointer to region wise packing generator
    Nalu                            *m_newSPSNalu; //!< pointer to the new SPS nalu
    Nalu                            *m_newPPSNalu; //!< pointer to the new PPS nalu
};

VCD_NS_END;
#endif /* _EXTRACTORTRACKGENERATOR_H_ */
