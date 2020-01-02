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

 *
 */
//!
//! \file:   OmafReader.h
//! \brief:
//! \detail:
//! Created on May 28, 2019, 1:41 PM
//!

#ifndef OMAFREADER_H
#define OMAFREADER_H

#include "general.h"
#include "iso_structure.h"
#include "OmafSegment.h"
#include "360SCVPAPI.h"

VCD_OMAF_BEGIN

using namespace VCD::OMAF;

class OmafReader {
public:
    OmafReader(){};
    virtual ~OmafReader(){};

public:
    //!
    //! \brief  Initialize an Omaf Reader
    //!
    //! \param  [in] OmafSegment*
    //!              an Omaf segment pointer
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t initialize(OmafSegment* pSeg) = 0;

    //!
    //! \brief  Close file
    //!
    virtual void close() = 0;

    //!
    //! \brief  Get brand information from the file
    //!
    //! \param  [out] FourCC&
    //!               major brand information
    //!         [in] uint32_t
    //!              initialization Segment Id
    //!         [in] uint32_t
    //!              segment Id
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getMajorBrand(FourCC& majorBrand,
                                  uint32_t initializationSegmentId = 0,
                                  uint32_t segmentId               = UINT32_MAX) const = 0;

    //!
    //! \brief  Get minor version information from the file
    //!
    //! \param  [out] uint32_t&
    //!               minor Version information
    //!         [in] uint32_t
    //!              initialization Segment Id
    //!         [in] uint32_t
    //!              segment Id
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getMinorVersion(uint32_t& minorVersion,
                                    uint32_t initializationSegmentId = 0,
                                    uint32_t segmentId               = UINT32_MAX) const  = 0;

    //!
    //! \brief  Get Compatible Brands information from the file
    //!
    //! \param  [out] std::vector<VCD::OMAF::FourCC*>&
    //!               compatible Brands information
    //!         [in] uint32_t
    //!              initialization Segment Id
    //!         [in] uint32_t
    //!              segment Id
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getCompatibleBrands(std::vector<VCD::OMAF::FourCC*>& compatibleBrands,
                                        uint32_t initializationSegmentId = 0,
                                        uint32_t segmentId               = UINT32_MAX) const  = 0;

    //!
    //! \brief  Get Track Informations
    //!
    //! \param  [out] std::vector<VCD::OMAF::TrackInformation*>&
    //!               Track Informations
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getTrackInformations(std::vector<VCD::OMAF::TrackInformation*>& trackInfos) const = 0;

    //!
    //! \brief  Get Display Width
    //!
    //! \param      [in] uint32_t
    //!                  trackId
    //!             [out] uint32_t&
    //!                  Display Width
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getDisplayWidth(uint32_t trackId, uint32_t& displayWidth) const = 0;

    //!
    //! \brief  Get Display Height
    //!
    //! \param      [in] uint32_t
    //!                  trackId
    //!             [out] uint32_t&
    //!                  Display Height
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getDisplayHeight(uint32_t trackId, uint32_t& displayHeight) const = 0;

    //!
    //! \brief  Get Display Width FP
    //!
    //! \param      [in] uint32_t
    //!                  trackId
    //!             [out] uint32_t&
    //!                  Display Width
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getDisplayWidthFP(uint32_t trackId, uint32_t& displayWidth) const  = 0;

    //!
    //! \brief  Get Display Height FP
    //!
    //! \param      [in] uint32_t
    //!                  trackId
    //!             [out] uint32_t&
    //!                  Display Height
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getDisplayHeightFP(uint32_t trackId, uint32_t& displayHeight) const  = 0;

    //!
    //! \brief  Get Width of a sample
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [in] uint32_t
    //!                  sample Id
    //!             [out] uint32_t&
    //!                  Width
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getWidth(uint32_t trackId, uint32_t sampleId, uint32_t& width) const = 0;

    //!
    //! \brief  Get Height of a sample
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [in] uint32_t
    //!                  sample Id
    //!             [out] uint32_t&
    //!                  Height
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getHeight(uint32_t trackId, uint32_t sampleId, uint32_t& height) const = 0;

    //!
    //! \brief  Get Playback Duration In Seconds
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [out] double&
    //!                  duration In Seconds
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getPlaybackDurationInSecs(uint32_t trackId, double& durationInSecs) const = 0;

    //!
    //! \brief  Get Track Sample List By Type
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [in] VCD::OMAF::TrackSampleType
    //!                  sample Types
    //!             [out] std::vector<uint32_t>&
    //!                  sample Id array
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getTrackSampleListByType(uint32_t trackId, VCD::OMAF::TrackSampleType sampleType,
                                             std::vector<uint32_t>& sampleIds) const = 0;

    //!
    //! \brief  Get Track Sample Type
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [in] uint32_t
    //!                  sample id
    //!             [out] VCD::OMAF::FourCC&
    //!                  track sample box type
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getTrackSampleType(uint32_t trackId, uint32_t sampleId, VCD::OMAF::FourCC& trackSampleBoxType) const = 0;

    //!
    //! \brief  Get Extractor Track Sample data
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [in] uint32_t
    //!                  sample id
    //!             [out] char*
    //!                  memory Buffer
    //!             [out] uint32_t&
    //!                  memory Buffer Size
    //!             [in] bool
    //!                  has video Byte Stream Headers or not
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getExtractorTrackSampleData(uint32_t trackId,
                                                uint32_t sampleId,
                                                char* memoryBuffer,
                                                uint32_t& memoryBufferSize,
                                                bool videoByteStreamHeaders = true ) = 0;

    //!
    //! \brief  Get Track Sample data
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [in] uint32_t
    //!                  sample id
    //!             [out] char*
    //!                  memory Buffer
    //!             [out] uint32_t&
    //!                  memory Buffer Size
    //!             [in] bool
    //!                  has video Byte Stream Headers or not
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getTrackSampleData(uint32_t trackId,
                                       uint32_t sampleId,
                                       char* memoryBuffer,
                                       uint32_t& memoryBufferSize,
                                       bool videoByteStreamHeaders = true) = 0;

    //!
    //! \brief  Get Track Sample offset
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [in] uint32_t
    //!                  sample id
    //!             [out] uint64_t&
    //!                  sample Offset
    //!             [out] uint32_t&
    //!                  sample Length
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getTrackSampleOffset(uint32_t trackId, uint32_t sampleId, uint64_t& sampleOffset, uint32_t& sampleLength) = 0;

    //!
    //! \brief  Get Decoder Configuration
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [in] uint32_t
    //!                  sample id
    //!             [out] std::vector<VCD::OMAF::DecoderSpecificInfo>&
    //!                  decoder Information
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getDecoderConfiguration(uint32_t trackId, uint32_t sampleId, std::vector<VCD::OMAF::DecoderSpecificInfo>& decoderInfos) const = 0;

    //!
    //! \brief  Get Track Time stamps
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [out] std::vector<VCD::OMAF::TimestampIDPair>&
    //!                  timestamps
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getTrackTimestamps(uint32_t trackId, std::vector<VCD::OMAF::TimestampIDPair>& timestamps) const = 0;

    //!
    //! \brief  Get Track Time stamps of sample
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [in] uint32_t
    //!                  sample id
    //!             [out] std::vector<VCD::OMAF::TimestampIDPair>&
    //!                  timestamps
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getTimestampsOfSample(uint32_t trackId, uint32_t sampleId, std::vector<uint64_t>& timestamps) const = 0;

    //!
    //! \brief  Get Samples In Decoding Order
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [out] std::vector<VCD::OMAF::TimestampIDPair>&
    //!                  sample Decoding Order
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getSamplesInDecodingOrder(uint32_t trackId, std::vector<VCD::OMAF::TimestampIDPair>& sampleDecodingOrder) const = 0;

    //!
    //! \brief  Get decoder code type
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [in] uint32_t
    //!                  sample id
    //!             [out] FourCC&
    //!                  decoder Code Type
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getDecoderCodeType(uint32_t trackId, uint32_t sampleId, FourCC& decoderCodeType) const = 0;

    //!
    //! \brief  Get sample duration
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [in] uint32_t
    //!                  sample id
    //!             [out] uint32_t&
    //!                  sample Duration
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getSampleDuration(uint32_t trackId, uint32_t sampleId, uint32_t& sampleDuration) const  = 0;

public:

    //!
    //! \brief  Get sample duration
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [in] uint32_t
    //!                  sample id
    //!             [out] uint32_t&
    //!                  sample Duration
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getPropertyChnl(uint32_t trackId, uint32_t sampleId, VCD::OMAF::chnlProperty& chProperty) const  = 0;
   
    //!
    //! \brief  Get Property Spatial Audio
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [in] uint32_t
    //!                  sample id
    //!             [out] VCD::OMAF::SpatialAudioProperty&
    //!                  spatial audio property
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getPropertySpatialAudio(uint32_t trackId, uint32_t sampleId, VCD::OMAF::SpatialAudioProperty& spatialaudioproperty) const = 0;

    //!
    //! \brief  Get Property Stereo Scopic 3D
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [in] uint32_t
    //!                  sample id
    //!             [out] VCD::OMAF::StereoScopic3DProperty&
    //!                  stereo scopic property
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getPropertyStereoScopic3D(uint32_t trackId, uint32_t sampleId, VCD::OMAF::StereoScopic3DProperty& stereoscopicproperty) const = 0;

    //!
    //! \brief  Get Property Spherical Video V1
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [in] uint32_t
    //!                  sample id
    //!             [out] VCD::OMAF::SphericalVideoV1Property&
    //!                  spherical property
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getPropertySphericalVideoV1(uint32_t trackId, uint32_t sampleId, VCD::OMAF::SphericalVideoV1Property& sphericalproperty) const  = 0;

    //!
    //! \brief  Get Property Spherical Video V2
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [in] uint32_t
    //!                  sample id
    //!             [out] VCD::OMAF::SphericalVideoV2Property&
    //!                  spherical property
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getPropertySphericalVideoV2(uint32_t trackId, uint32_t sampleId, VCD::OMAF::SphericalVideoV2Property& sphericalproperty) const = 0;

    //!
    //! \brief  Get Property Region Wise Packing
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [in] uint32_t
    //!                  sample id
    //!             [out] RegionWisePacking*
    //!                  rwpk
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getPropertyRegionWisePacking(uint32_t trackId, uint32_t sampleId, RegionWisePacking *rwpk) const = 0;

    //!
    //! \brief  Get Property Coverage Information
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [in] uint32_t
    //!                  sample id
    //!             [out] VCD::OMAF::CoverageInformationProperty&
    //!                  covi Property
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getPropertyCoverageInformation(uint32_t trackId, uint32_t sampleId, VCD::OMAF::CoverageInformationProperty& coviProperty) const = 0;

    //!
    //! \brief  Get Property Projection Format
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [in] uint32_t
    //!                  sample id
    //!             [out] VCD::OMAF::ProjectionFormatProperty&
    //!                  projection Format Property
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getPropertyProjectionFormat(uint32_t trackId, uint32_t sampleId, VCD::OMAF::ProjectionFormatProperty& projectionFormatProperty) const = 0;

    //!
    //! \brief  Get Property Scheme Types
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [in] uint32_t
    //!                  sample id
    //!             [out] VCD::OMAF::SchemeTypesProperty&
    //!                  scheme Types Property
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getPropertySchemeTypes(uint32_t trackId, uint32_t sampleId, VCD::OMAF::SchemeTypesProperty& schemeTypesProperty) const = 0;

    //!
    //! \brief  Get Property Stereo Video Configuration
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [in] uint32_t
    //!                  sample id
    //!             [out] VCD::OMAF::PodvStereoVideoConfiguration&
    //!                  stereo Video Property
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getPropertyStereoVideoConfiguration(uint32_t trackId, uint32_t sampleId, VCD::OMAF::PodvStereoVideoConfiguration& stereoVideoProperty) const  = 0;

    //!
    //! \brief  Get Property Rotation
    //!
    //! \param      [in] uint32_t
    //!                  track Id
    //!             [in] uint32_t
    //!                  sample id
    //!             [out] VCD::OMAF::Rotation&
    //!                  Rotation Property
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t getPropertyRotation(uint32_t trackId, uint32_t sampleId, VCD::OMAF::Rotation& rotationProperty) const  = 0;
    //!
    //! \brief  Get Property Rotation
    //!
    //! \return std::map<int,int>
    //!         return Map Init Track
    //!
    std::map<int,int> getMapInitTrk() const {return mMapInitTrk;}

public:

    //!
    //! \brief  parse Initialization Segment
    //!
    //! \param      [in] OmafSegment*
    //!                  stream Interface
    //!             [in] uint32_t
    //!                  init Segment Id
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t parseInitializationSegment(OmafSegment* streamInterface, uint32_t initSegmentId)  = 0;

    //!
    //! \brief  invalidate Initialization Segment
    //!
    //! \param      [in] uint32_t
    //!                  init Segment Id
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t invalidateInitializationSegment(uint32_t initSegmentId) = 0 ;

    //!
    //! \brief  parse Segment
    //!
    //! \param      [in] OmafSegment*
    //!                  stream Interface
    //!             [in] uint32_t
    //!                  init segment id
    //!             [in] uint32_t
    //!                  segment id
    //!             [in] uint64_t
    //!                  earliest PTS in TS
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t parseSegment( OmafSegment* streamInterface,
                                  uint32_t initSegmentId,
                                  uint32_t segmentId,
                                  uint64_t earliestPTSinTS = UINT64_MAX)  = 0;

    //!
    //! \brief  invalidate Segment
    //!
    //! \param      [in] uint32_t
    //!                  init Segment Id
    //!             [in] uint32_t
    //!                  segment id
    //!
    //! \return int32_t
    //!         return value
    //!
    virtual int32_t invalidateSegment(uint32_t initSegmentId, uint32_t segmentId)  = 0;
    //!
    //! \brief  set Map Init Track
    //!
    //! \param      [in] std::map<int,int>
    //!                  map
    //!
    //! \return int32_t
    //!         return value
    //!
    void setMapInitTrk(std::map<int,int> map){mMapInitTrk = map;}
private:
    std::map<int, int>  mMapInitTrk;      //<! ID pair for InitSegID to TrackID;
};

VCD_OMAF_END;

#endif /* OMAFREADER_H */

