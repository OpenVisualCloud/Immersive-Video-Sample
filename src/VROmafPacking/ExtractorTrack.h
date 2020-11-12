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
//! \file:   ExtractorTrack.h
//! \brief:  Extractor track class definition
//! \detail: Define the data and data operation for extractor track.
//!
//! Created on April 30, 2019, 6:04 AM
//!

#ifndef _EXTRACTORTRACK_H_
#define _EXTRACTORTRACK_H_

#include "VROmafPacking_data.h"
#include "VROmafPacking_def.h"
#include "MediaStream.h"
#include "RegionWisePackingGenerator.h"
#include "../utils/OmafStructure.h"

#include <list>
#include <map>
#include <mutex>

VCD_NS_BEGIN

//!
//! \struct: NaluHeader
//! \brief:  define nalu header information
//!
struct NaluHeader
{
    uint8_t forbiddenZeroBit;
    uint8_t naluType;
    uint8_t nuhLayerId;
    uint8_t nuhTemporalIdPlus1;
};

//!
//! \struct: SampleConstructor
//! \brief:  define the sample data related information of
//!          tile for extractor
//!
struct SampleConstructor
{
    uint8_t  streamIdx;
    uint8_t  trackRefIndex;
    int8_t   sampleOffset;
    uint32_t dataOffset;
    uint32_t dataLength;
};

//!
//! \struct: InlineConstructor
//! \brief:  define new constructed information for tile
//!          for extractor, like new slice header
//!
struct InlineConstructor
{
    uint8_t length;
    uint8_t *inlineData; //new "sliceHeader" for the tile
};

//!
//! \struct: Extractor
//! \brief:  define the extractor
//!
struct Extractor
{
    std::list<SampleConstructor*> sampleConstructor;
    std::list<InlineConstructor*> inlineConstructor;
};

//!
//! \class ExtractorTrack
//! \brief Define the data and data operation for extractor track
//!

class ExtractorTrack
{
public:
    //!
    //! \brief  Constructor
    //!
    ExtractorTrack();

    //!
    //! \brief  Copy Constructor
    //!
    //! \param  [in] viewportIdx
    //!         the index of viewport corresponding to extractor track
    //! \param  [in] streams
    //!         pointer to the media streams map set up in OmafPackage
    //! \param  [in] projType
    //!         projection type of the video frame
    //!
    ExtractorTrack(uint8_t viewportIdx, std::map<uint8_t, MediaStream*> *streams, uint16_t projType);

    ExtractorTrack(const ExtractorTrack& src);

    ExtractorTrack& operator=(ExtractorTrack&& other);

    //!
    //! \brief  Destructor
    //!
    ~ExtractorTrack();

    //!
    //! \brief  Initialize resources for extractor track 
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t Initialize();

    //!
    //! \brief  Construct all extractors belong to this extractor track
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t ConstructExtractors();

    //!
    //! \brief  Generate all extractors belong to this extractor track
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GenerateExtractors();

    //!
    //! \brief  Destroy the resource of all extractors belong to this
    //!         extractor track
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t DestroyExtractors();

    //!
    //! \brief  Updata data of all extractors belong to this
    //!         extractor track
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t UpdateExtractors();

    //!
    //! \brief  Get all extractors belong to this extractor track
    //!
    //! \return std::map<uint8_t, Extractor*>*
    //!         the pointer to extractors map
    //!
    std::map<uint8_t, Extractor*>* GetAllExtractors() { return &m_extractors; };

    //!
    //! \brief  Get projection type of the video frame
    //!
    //! \return VCD::OMAF::ProjectionFormat
    //!         PF_ERP is ERP, and PF_CUBEMAP is CubeMap
    //!
    VCD::OMAF::ProjectionFormat GetProjType() { return m_projType; };

    //!
    //! \brief  Get the region wise packing information for this extractor track
    //!
    //! \return RegionWisePacking*
    //!         the pointer to the region wise packing information
    //!
    RegionWisePacking* GetRwpk() { return m_dstRwpk; };

    //!
    //! \brief  Get the tiles merging direction information for
    //!         the extractor track
    //!
    //! \return TilesMergeDirectionInCol*
    //!         the pointer to the tiles merging direction information
    //!
    TilesMergeDirectionInCol* GetTilesMergeDir() { return m_tilesMergeDir; };

    //!
    //! \brief  Get the content coverage information for this extractor track
    //!
    //! \return ContentCoverage*
    //!         the pointer to the content coverage information
    //!
    ContentCoverage* GetCovi() { return m_dstCovi; };

    //!
    //! \brief  Set nalu information according to source nalu information
    //!
    //! \param  [in] srcNalu
    //!         the pointer to the source nalu information
    //! \param  [out] dstNalu
    //!         pointer to the dstination nalu information
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t SetNalu(Nalu *srcNalu, Nalu *dstNalu);

    //!
    //! \brief  Get the VPS nalu information
    //!
    //! \return Nalu*
    //!         the pointer to the VPS nalu information
    //!
    Nalu* GetVPS();

    //!
    //! \brief  Get the SPS nalu information
    //!
    //! \return Nalu*
    //!         the pointer to SPS nalu information
    //!
    Nalu* GetSPS();

    //!
    //! \brief  Get the PPS nalu information
    //!
    //! \return Nalu*
    //!         the pointer to PPS nalu information
    //!
    Nalu* GetPPS();

    //!
    //! \brief  Get the video resolutions list which are
    //!         used to construct the extractor track
    //!
    //! \return std::list<PicResolution>*
    //!         the pointer to video resolutions list
    //!
    std::list<PicResolution>* GetPicRes() { return &m_picRes; };

    //!
    //! \brief  Get the projection SEI nalu information
    //!
    //! \return Nalu*
    //!         the pointer to projection SEI nalu information
    //!
    Nalu* GetProjectionSEI();

    //!
    //! \brief  Get the RWPK SEI nalu information
    //!
    //! \return Nalu*
    //!         the pointer to RWPK SEI nalu information
    //!
    Nalu* GetRwpkSEI();

    //!
    //! \brief  Add extractors nalu data pointer for one frame
    //!         into list for one segment
    //!
    //! \param  [in] extractorsData
    //!         pointer to extractors nalu data for one frame
    //!
    //! \return void
    //!
    void AddExtractorsNaluToSeg(uint8_t *extractorsData)
    {
        m_naluDataForOneSeg.push_back(extractorsData);
    };

    //!
    //! \brief  Destroy all extractors nalu data for current
    //!         segment
    //!
    //! \return void
    //!
    void DestroyCurrSegNalus();

    //!
    //! \brief  Get current processed frames number in extractor track
    //!
    //! \return uint64_t
    //!         current processed frames number in extractor track
    //!
    uint64_t GetProcessedFrmNum()
    {
        uint64_t processedFrmNum = m_processedFrmNum;
        return processedFrmNum;
    };

    //!
    //! \brief  Increase current processed frames number in extractor track
    //!
    //! \return void
    //!
    void IncreaseProcessedFrmNum()
    {
        m_processedFrmNum++;
    }

    uint8_t GetViewportId() { return m_viewportIdx; };

    void SetPackedPicWidth(uint32_t packedWidth) { m_dstWidth = packedWidth; };

    void SetPackedPicHeight(uint32_t packedHeight) { m_dstHeight = packedHeight; };

private:

    //!
    //! \brief  Generate projection SEI
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GenerateProjectionSEI();

    //!
    //! \brief  Generate RWPK SEI
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t GenerateRwpkSEI();

private:
    std::map<uint8_t, MediaStream*> *m_streams;          //!< media streams map set up in OmafPackage
    uint8_t                         m_viewportIdx;       //!< the index of viewport corresponding to extractor track
    VCD::OMAF::ProjectionFormat     m_projType;          //!< projection type of the video frame
    RegionWisePacking               *m_dstRwpk;          //!< pointer to the region wise packing information of extractor track
    ContentCoverage                 *m_dstCovi;          //!< pointer to the content coverage information of extractor track
    std::map<uint8_t, Extractor*>   m_extractors;        //!< map of all extractors belong to the extractor track

    TilesMergeDirectionInCol        *m_tilesMergeDir;    //!< pointer to the tiles merging direction information
    Nalu                            *m_vps;              //!< pointer to the extractor track VPS nalu information
    Nalu                            *m_sps;              //!< pointer to the extractor track SPS nalu information
    Nalu                            *m_pps;              //!< pointer to the extractor track PPS nalu information
    std::list<PicResolution>        m_picRes;            //!< list of video resolutions
    Nalu                            *m_projSEI;          //!< pointer to the extractor track projection SEI nalu information
    Nalu                            *m_rwpkSEI;          //!< pointer to the extractor track RWPK SEI nalu information
    std::list<uint8_t*>             m_naluDataForOneSeg; //!< extractors nalu data list for one segment
    param_360SCVP                   *m_360scvpParam;     //!< 360SCVP library parameter
    std::map<MediaStream*, void*>   m_360scvpHandles;    //!< map of 360SCVP library handle and corresponding media stream
    uint64_t                        m_processedFrmNum;   //!< processed frames number in extractor track
    uint32_t                         m_dstWidth;
    uint32_t                         m_dstHeight;
};

VCD_NS_END;
#endif /* _EXTRACTORTRACK_H_ */
