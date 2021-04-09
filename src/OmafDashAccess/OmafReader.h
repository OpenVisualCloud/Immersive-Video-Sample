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

//!
//! \class OmafReader
//! \brief Define the operation and needed data for omaf file reading
//!

class OmafReader {
public:
    //!
    //! \brief  Constructor
    //!
    OmafReader(){};

    //!
    //! \brief  Destructor
    //!
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
    //! \brief  Free resource of OmafReader
    //!
    //! \return void
    //!
    virtual void close() = 0;

    //!
    //! \brief  Get the major brand from specified segment
    //!         of specified track
    //!
    //! \param  [out] majorBrand
    //!         output major brand
    //! \param  [in]  initializationSegmentId
    //!         initial segment index corresponding to
    //!         specified track
    //! \param  [in] segmentId
    //!         index of specified segment
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getMajorBrand(FourCC& majorBrand,
                                  uint32_t initializationSegmentId = 0,
                                  uint32_t segmentId               = UINT32_MAX) const = 0;

    //!
    //! \brief  Get the minor version from specified segment
    //!         of specified track
    //!
    //! \param  [out] minorVersion
    //!         output minor version
    //! \param  [in]  initializationSegmentId
    //!         initial segment index corresponding to
    //!         specified track
    //! \param  [in] segmentId
    //!         index of specified segment
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getMinorVersion(uint32_t& minorVersion,
                                    uint32_t initializationSegmentId = 0,
                                    uint32_t segmentId               = UINT32_MAX) const  = 0;

    //!
    //! \brief  Get the compatible brands from specified segment
    //!         of specified track
    //!
    //! \param  [out] compatibleBrands
    //!         output compatible brands
    //! \param  [in]  initializationSegmentId
    //!         initial segment index corresponding to
    //!         specified track
    //! \param  [in]  segmentId
    //!         index of specified segment
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getCompatibleBrands(std::vector<VCD::OMAF::FourCC*>& compatibleBrands,
                                        uint32_t initializationSegmentId = 0,
                                        uint32_t segmentId               = UINT32_MAX) const  = 0;

    //!
    //! \brief  Get the track information for all tracks
    //!         after one segment file is parsed and OmafReader
    //!         is initialized
    //!
    //! \param  [out] trackInfos
    //!         track information for all tracks
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getTrackInformations(std::vector<VCD::OMAF::TrackInformation*>& trackInfos) const = 0;

    //!
    //! \brief  Get picture display width
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [out] displayWidth
    //!         picture display width
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getDisplayWidth(uint32_t trackId, uint32_t& displayWidth) const = 0;

    //!
    //! \brief  Get picture display height
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [out] displayHeight
    //!         picture display height
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getDisplayHeight(uint32_t trackId, uint32_t& displayHeight) const = 0;

    //!
    //! \brief  Get fixed point picture display width
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [out] displayWidth
    //!         fixed point picture display width
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getDisplayWidthFP(uint32_t trackId, uint32_t& displayWidth) const  = 0;

    //!
    //! \brief  Get fixed point picture display height
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [out] displayHeight
    //!         fixed point picture display height
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getDisplayHeightFP(uint32_t trackId, uint32_t& displayHeight) const  = 0;

    //!
    //! \brief  Get width of specified sample in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] width
    //!         width of specified sample in pixels
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getWidth(uint32_t trackId, uint32_t sampleId, uint32_t& width) const = 0;

    //!
    //! \brief  Get height of specified sample in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] height
    //!         height of specified sample in pixels
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getHeight(uint32_t trackId, uint32_t sampleId, uint32_t& height) const = 0;

  virtual int32_t getDims(uint32_t trackId, uint32_t sampleId, uint32_t& width, uint32_t& height) const = 0;
    //!
    //! \brief  Get playback duration of specified track,
    //!         and unit is second
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [out] durationInSecs
    //!         playback duration in second
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getPlaybackDurationInSecs(uint32_t trackId, double& durationInSecs) const = 0;

    //!
    //! \brief  Get samples list of specified sample frame type from
    //!         the specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleType
    //!         specified sample frame type, like reference frame type
    //! \param  [out] sampleIds
    //!         samples list
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getTrackSampleListByType(uint32_t trackId, VCD::OMAF::TrackSampleType sampleType,
                                             std::vector<uint32_t>& sampleIds) const = 0;

    //!
    //! \brief  Get sample type of specified sample in
    //!         the specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] trackSampleBoxType
    //!         detailed sample type
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getTrackSampleType(uint32_t trackId, uint32_t sampleId, VCD::OMAF::FourCC& trackSampleBoxType) const = 0;

    //!
    //! \brief  Get complete data for specified sample in
    //!         the specified extractor track
    //!
    //! \param  [in]  trackId
    //!         index of specific extractor track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] memoryBuffer
    //!         pointer to the allocated memory to store
    //!         the sample data
    //! \param  [out] memoryBufferSize
    //!         size of sample data
    //! \param  [in]  videoByteStreamHeaders
    //!         whether to insert NAL unit start codes into
    //!         sample data
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getExtractorTrackSampleData(uint32_t trackId,
                                                uint32_t sampleId,
                                                char* memoryBuffer,
                                                uint32_t& memoryBufferSize,
                                                bool videoByteStreamHeaders = true ) = 0;

    //!
    //! \brief  Get complete data for specified sample in
    //!         the specified normal track
    //!
    //! \param  [in]  trackId
    //!         index of specific normal track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] memoryBuffer
    //!         pointer to the allocated memory to store
    //!         the sample data
    //! \param  [out] memoryBufferSize
    //!         size of sample data
    //! \param  [in]  videoByteStreamHeaders
    //!         whether to insert NAL unit start codes into
    //!         sample data
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getTrackSampleData(uint32_t trackId,
                                       uint32_t sampleId,
                                       char* memoryBuffer,
                                       uint32_t& memoryBufferSize,
                                       bool videoByteStreamHeaders = true) = 0;

    //!
    //! \brief  Get track sample data offset and length for
    //!         the specified sample in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] sampleOffset
    //!         output sample offset
    //! \param  [out] sampleLength
    //!         output sample length
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getTrackSampleOffset(uint32_t trackId, uint32_t sampleId, uint64_t& sampleOffset, uint32_t& sampleLength) = 0;

    //!
    //! \brief  Get media codec related specific information,
    //!         like SPS, PPS and so on, for specified sample in
    //!         specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] decoderInfos
    //!         output media codec related specific information
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getDecoderConfiguration(uint32_t trackId, uint32_t sampleId, std::vector<VCD::OMAF::DecoderSpecificInfo>& decoderInfos) const = 0;

    //!
    //! \brief  Get display time stamp of each sample in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [out] timestamps
    //!         output time stamps for each sample
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getTrackTimestamps(uint32_t trackId, std::vector<VCD::OMAF::TimestampIDPair>& timestamps) const = 0;

    //!
    //! \brief  Get display time stamp of the specified sample
    //!         in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of the specified sample
    //! \param  [out] timestamps
    //!         output time stamps for the sample
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getTimestampsOfSample(uint32_t trackId, uint32_t sampleId, std::vector<uint64_t>& timestamps) const = 0;

    //!
    //! \brief  Get samples in decoding sequence in specified track,
    //!         gotten samples are presented by TimestampIDPair structure,
    //!         that is <timestamp, sampleId> pair
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [out] sampleDecodingOrder
    //!         output samples in decoding sequence
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getSamplesInDecodingOrder(uint32_t trackId, std::vector<VCD::OMAF::TimestampIDPair>& sampleDecodingOrder) const = 0;

    //!
    //! \brief  Get decoder code type for specified sample
    //!         in specified track, like "hvc1" and so on.
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] decoderCodeType
    //!         output decoder code type
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getDecoderCodeType(uint32_t trackId, uint32_t sampleId, FourCC& decoderCodeType) const = 0;

    //!
    //! \brief  Get duration for specified sample
    //!         in specified track, in milliseconds.
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] sampleDuration
    //!         output sample duration
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getSampleDuration(uint32_t trackId, uint32_t sampleId, uint32_t& sampleDuration) const  = 0;

public:

    //!
    //! \brief  Get audio channel layout box information for
    //!         specified sample in specified 'chnl' track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] chProperty
    //!         output channel property
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getPropertyChnl(uint32_t trackId, uint32_t sampleId, VCD::OMAF::chnlProperty& chProperty) const  = 0;

    //!
    //! \brief  Get spatial audio box information for
    //!         specified sample in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] spatialaudioproperty
    //!         output spatial audio property
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getPropertySpatialAudio(uint32_t trackId, uint32_t sampleId, VCD::OMAF::SpatialAudioProperty& spatialaudioproperty) const = 0;

    //!
    //! \brief  Get stereo scopic 3D information for spherical video for
    //!         specified sample in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] stereoscopicproperty
    //!         output stereo scopic 3D property
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getPropertyStereoScopic3D(uint32_t trackId, uint32_t sampleId, VCD::OMAF::StereoScopic3DProperty& stereoscopicproperty) const = 0;

    //!
    //! \brief  Get spherical video V1 information for
    //!         specified sample in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] sphericalproperty
    //!         output spherical video V1 property
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getPropertySphericalVideoV1(uint32_t trackId, uint32_t sampleId, VCD::OMAF::SphericalVideoV1Property& sphericalproperty) const  = 0;

    //!
    //! \brief  Get spherical video V2 information for
    //!         specified sample in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] sphericalproperty
    //!         output spherical video V2 property
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getPropertySphericalVideoV2(uint32_t trackId, uint32_t sampleId, VCD::OMAF::SphericalVideoV2Property& sphericalproperty) const = 0;

    //!
    //! \brief  Get region wise packing information for
    //!         specified sample in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] rwpk
    //!         output region wise packing property
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getPropertyRegionWisePacking(uint32_t trackId, uint32_t sampleId, RegionWisePacking *rwpk) const = 0;

    //!
    //! \brief  Get content coverage information for
    //!         specified sample in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] coviProperty
    //!         output content coverage property
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getPropertyCoverageInformation(uint32_t trackId, uint32_t sampleId, VCD::OMAF::CoverageInformationProperty& coviProperty) const = 0;

    //!
    //! \brief  Get projection format information for
    //!         specified sample in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] projectionFormatProperty
    //!         output projection format property
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getPropertyProjectionFormat(uint32_t trackId, uint32_t sampleId, VCD::OMAF::ProjectionFormatProperty& projectionFormatProperty) const = 0;

    //!
    //! \brief  Get scheme type information for
    //!         specified sample in specified track
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] schemeTypesProperty
    //!         output scheme type property
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getPropertySchemeTypes(uint32_t trackId, uint32_t sampleId, VCD::OMAF::SchemeTypesProperty& schemeTypesProperty) const = 0;

    //!
    //! \brief  Get stereo video information for
    //!         specified sample in specified track,
    //!         only podv scheme is supported
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] stereoVideoProperty
    //!         output stereo video property
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getPropertyStereoVideoConfiguration(uint32_t trackId, uint32_t sampleId, VCD::OMAF::PodvStereoVideoConfiguration& stereoVideoProperty) const  = 0;

    //!
    //! \brief  Get rotation information for
    //!         specified sample in specified track,
    //!
    //! \param  [in]  trackId
    //!         index of specific track
    //! \param  [in]  sampleId
    //!         index of specified sample
    //! \param  [out] rotationProperty
    //!         output podv rotation property
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t getPropertyRotation(uint32_t trackId, uint32_t sampleId, VCD::OMAF::Rotation& rotationProperty) const  = 0;

    //!
    //! \brief  Get the <initSegmentId, trackId> map
    //!
    //! \return std::map<int,int>
    //!         map of <initSegmentId, trackId>
    //!
    std::map<int,int> getMapInitTrk() const {return mMapInitTrk;}
    //!
    //! \brief  set the segment sample size
    //!
    void SetSegSampleSize(uint32_t size) { mSegSampleSize = size; };
    //!
    //! \brief  get the segment sample size
    //!
    uint32_t GetSegSampleSize() { return mSegSampleSize; };

public:

    //!
    //! \brief  Parse specified initial segment
    //!
    //! \param  [in]  streamInterface
    //!         pointer to specified initial segment
    //! \param  [in]  initSegmentId
    //!         index of specified initial segment,
    //!         corresponding to specified track
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t parseInitializationSegment(OmafSegment* streamInterface, uint32_t initSegmentId)  = 0;

    //!
    //! \brief  Invalidate specified initial segment
    //!         Disable the data buffer pointer to the
    //!         specified initial segment, then the data
    //!         from the segment can not be accessed any longer
    //!
    //! \param  [in]  initSegmentId
    //!         index of specified initial segment,
    //!         corresponding to specified track
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t invalidateInitializationSegment(uint32_t initSegmentId) = 0 ;

    //!
    //! \brief  Parse specified segment for specified track
    //!
    //! \param  [in]  streamInterface
    //!         pointer to specified segment handler
    //! \param  [in]  initSegmentId
    //!         index of specified initial segment, this index
    //!         is corresponding to track index
    //! \param  [in]  segmentId
    //!         index of specified segment
    //! \param  [in]  earliestPTSinTS
    //!         the earliest presentation time in timescale for
    //!         the specified sample
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t parseSegment( OmafSegment* streamInterface,
                                  uint32_t initSegmentId,
                                  uint32_t segmentId,
                                  uint64_t earliestPTSinTS = UINT64_MAX)  = 0;

    //!
    //! \brief  Invalidate specified segment for specified track
    //!         Disable the data buffer pointer to the specified
    //!         segment, then the data from the segment can not
    //!         be accessed any longer
    //!
    //! \param  [in]  initSegmentId
    //!         index of specified initial segment, which is
    //!         corresponding to track index
    //! \param  [in]  segmentId
    //!         index of specified segment
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t invalidateSegment(uint32_t initSegmentId, uint32_t segmentId)  = 0;

    //!
    //! \brief  Set the <initSegmentId, trackId> map to
    //!         input map
    //!
    //! \param  [in]  map
    //!         input map of <initSegmentId, trackId> type
    //!
    //! \return void
    //!
    void setMapInitTrk(std::map<int,int> map){mMapInitTrk = map;}

private:
    std::map<int, int>  mMapInitTrk;           //!< the map of <initSegmentId, trackId>
    uint32_t mSegSampleSize = 0;               //!< segment sample size
};

VCD_OMAF_END;

#endif /* OMAFREADER_H */
