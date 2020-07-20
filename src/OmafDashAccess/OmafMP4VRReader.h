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
//! \file:   OmafMP4VRReader.h
//! \brief:
//! Created on May 28, 2019, 1:41 PM
//!

#ifndef OMAFSEGMENTREADER_H
#define OMAFSEGMENTREADER_H

#include "OmafReader.h"

VCD_OMAF_BEGIN

class OmafMP4VRReader : public OmafReader{
public:
    OmafMP4VRReader();
    OmafMP4VRReader(OmafMP4VRReader&& other);
    OmafMP4VRReader& operator=(const OmafMP4VRReader&) = default;
    virtual ~OmafMP4VRReader();

public:

    ///implementation of abstract interface based on Nokia Omaf reader
    virtual int32_t initialize(OmafSegment* pSeg)  ;

    virtual void close()  ;

    virtual int32_t getMajorBrand(  FourCC& majorBrand,
                                    uint32_t initializationSegmentId = 0,
                                    uint32_t segmentId               = UINT32_MAX) const  ;

    virtual int32_t getMinorVersion(uint32_t& minorVersion,
                                    uint32_t initializationSegmentId = 0,
                                    uint32_t segmentId               = UINT32_MAX) const ;

    virtual int32_t getCompatibleBrands(std::vector<VCD::OMAF::FourCC*>& compatibleBrands,
                                        uint32_t initializationSegmentId = 0,
                                        uint32_t segmentId               = UINT32_MAX) const ;

    virtual int32_t getTrackInformations(std::vector<VCD::OMAF::TrackInformation*>& trackInfos) const  ;

    virtual int32_t getDisplayWidth(uint32_t trackId, uint32_t& displayWidth) const  ;

    virtual int32_t getDisplayHeight(uint32_t trackId, uint32_t& displayHeight) const  ;

    virtual int32_t getDisplayWidthFP(uint32_t trackId, uint32_t& displayWidth) const ;

    virtual int32_t getDisplayHeightFP(uint32_t trackId, uint32_t& displayHeight) const ;

    virtual int32_t getWidth(uint32_t trackId, uint32_t sampleId, uint32_t& width) const  ;

    virtual int32_t getHeight(uint32_t trackId, uint32_t sampleId, uint32_t& height) const  ;

  virtual int32_t getDims(uint32_t trackId, uint32_t sampleId, uint32_t& width, uint32_t& height) const;

  virtual int32_t getPlaybackDurationInSecs(uint32_t trackId, double& durationInSecs) const;

    virtual int32_t getTrackSampleListByType(uint32_t trackId, VCD::OMAF::TrackSampleType sampleType,
                                             std::vector<uint32_t>& sampleIds) const  ;

    virtual int32_t getTrackSampleType(uint32_t trackId, uint32_t sampleId, FourCC& trackSampleBoxType) const  ;


    virtual int32_t getExtractorTrackSampleData(uint32_t trackId,
                                                uint32_t sampleId,
                                                char* memoryBuffer,
                                                uint32_t& memoryBufferSize,
                                                bool videoByteStreamHeaders = true);

    virtual int32_t getTrackSampleData(uint32_t trackId,
                                       uint32_t sampleId,
                                       char* memoryBuffer,
                                       uint32_t& memoryBufferSize,
                                       bool videoByteStreamHeaders = true)  ;

    virtual int32_t getTrackSampleOffset(uint32_t trackId, uint32_t sampleId, uint64_t& sampleOffset, uint32_t& sampleLength)  ;

    virtual int32_t getDecoderConfiguration(uint32_t trackId, uint32_t sampleId, std::vector<VCD::OMAF::DecoderSpecificInfo>& decoderInfos) const  ;

    virtual int32_t getTrackTimestamps(uint32_t trackId, std::vector<VCD::OMAF::TimestampIDPair>& timestamps) const  ;

    virtual int32_t getTimestampsOfSample(uint32_t trackId, uint32_t sampleId, std::vector<uint64_t>& timestamps) const  ;

    virtual int32_t getSamplesInDecodingOrder(uint32_t trackId, std::vector<VCD::OMAF::TimestampIDPair>& sampleDecodingOrder) const  ;

    virtual int32_t getDecoderCodeType(uint32_t trackId, uint32_t sampleId, VCD::OMAF::FourCC& decoderCodeType) const  ;

    virtual int32_t getSampleDuration(uint32_t trackId, uint32_t sampleId, uint32_t& sampleDuration) const ;

public:  // MP4VR specific methods

    virtual int32_t getPropertyChnl(uint32_t trackId, uint32_t sampleId, VCD::OMAF::chnlProperty& chProperty) const ;

    virtual int32_t getPropertySpatialAudio(uint32_t trackId, uint32_t sampleId, VCD::OMAF::SpatialAudioProperty& spatialaudioproperty) const  ;

    virtual int32_t getPropertyStereoScopic3D(uint32_t trackId, uint32_t sampleId, VCD::OMAF::StereoScopic3DProperty& stereoscopicproperty) const  ;

    virtual int32_t getPropertySphericalVideoV1(uint32_t trackId, uint32_t sampleId, VCD::OMAF::SphericalVideoV1Property& sphericalproperty) const ;

    virtual int32_t getPropertySphericalVideoV2(uint32_t trackId, uint32_t sampleId, VCD::OMAF::SphericalVideoV2Property& sphericalproperty) const  ;

    virtual int32_t getPropertyRegionWisePacking(uint32_t trackId, uint32_t sampleId, RegionWisePacking *rwpk) const  ;

    virtual int32_t getPropertyCoverageInformation(uint32_t trackId, uint32_t sampleId, VCD::OMAF::CoverageInformationProperty& coviProperty) const  ;

    virtual int32_t getPropertyProjectionFormat(uint32_t trackId, uint32_t sampleId, VCD::OMAF::ProjectionFormatProperty& projectionFormatProperty) const  ;

    virtual int32_t getPropertySchemeTypes(uint32_t trackId, uint32_t sampleId, VCD::OMAF::SchemeTypesProperty& schemeTypesProperty) const  ;

    virtual int32_t getPropertyStereoVideoConfiguration(uint32_t trackId, uint32_t sampleId, VCD::OMAF::PodvStereoVideoConfiguration& stereoVideoProperty) const ;

    virtual int32_t getPropertyRotation(uint32_t trackId, uint32_t sampleId, VCD::OMAF::Rotation& rotationProperty) const ;

public:  // MP4VR segment parsing methods (DASH/Streaming)

    virtual int32_t parseInitializationSegment(OmafSegment* streamInterface, uint32_t initSegmentId) ;

    virtual int32_t invalidateInitializationSegment(uint32_t initSegmentId)   ;

    virtual int32_t parseSegment( OmafSegment* streamInterface,
                                  uint32_t initSegmentId,
                                  uint32_t segmentId,
                                  uint64_t earliestPTSinTS = UINT64_MAX) ;

    virtual int32_t invalidateSegment(uint32_t initSegmentId, uint32_t segmentId) ;

private:
    void*  mMP4ReaderImpl;
    void SelectedTrackInfos(std::vector<VCD::OMAF::TrackInformation*>& trackInfos, std::vector<VCD::OMAF::TrackInformation*> middleTrackInfos) const;
};

VCD_OMAF_END;

#endif /* OMAFSEGMENTREADER_H */

