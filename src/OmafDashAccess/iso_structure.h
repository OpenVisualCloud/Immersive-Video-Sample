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
//! \file:   iso_structure.h
//! \brief:
//! Created on June 18, 2019, 1:03 PM
//!

#ifndef ISO_STRUCTURE_H
#define ISO_STRUCTURE_H

#include "../isolib/dash_parser/Mp4DataTypes.h"
#include "../isolib/include/Common.h"
#include "general.h"

VCD_OMAF_BEGIN

typedef struct SRDInfo {
  int32_t left = 0;
  int32_t top = 0;
  int32_t width = 0;
  int32_t height = 0;
} SRDInfo;

using FourCC = VCD::MP4::FourCC;

using TrackSampleType = VCD::MP4::SampleFrameType;

using ViewIdc = VCD::MP4::OmniViewIdc;

using ChannelLayout = VCD::MP4::ChannelLayout;

using chnlProperty = VCD::MP4::ChnlProperty;

using RegionWisePackingType = VCD::MP4::OmniRWPKType;

using RectRegionPacking = VCD::MP4::RectRWPKRegion;

using RegionWisePackingRegion = VCD::MP4::RWPKRegion;

using RegionWisePackingProperty = VCD::MP4::RWPKProperty;

using CoverageShapeType = VCD::MP4::COVIShapeType;

using CoverageSphereRegion = VCD::MP4::COVIRegion;

using CoverageInformationProperty = VCD::MP4::COVIInformation;

using SpatialAudioProperty = VCD::MP4::SpatialAudioProperty;

using StereoScopic3DProperty = VCD::MP4::OmniStereoScopic3D;

using SphericalVideoV2Property = VCD::MP4::SphericalVideoV2Property;

using SphericalVideoV1Property = VCD::MP4::SphericalVideoV1Property;

using DecSpecInfoType = VCD::MP4::MediaCodecInfoType;

using DecoderSpecificInfo = VCD::MP4::MediaCodecSpecInfo;

using TimestampIDPair = VCD::MP4::TStampID;

typedef uint32_t FeatureBitMask;

using Feature = VCD::MP4::FeatureOfTrack;
using VRFeature = VCD::MP4::ImmersiveProperty;

using TypeToTrackIDs = VCD::MP4::TypeToTrackIDs;

using SampleType = VCD::MP4::FrameCodecType;

using SampleInformation = VCD::MP4::TrackSampInfo;

using TrackInformation = VCD::MP4::TrackInformation;

using SchemeTypesProperty = VCD::MP4::SchemeTypesProperty;

using ProjectionFormatProperty = VCD::MP4::ProjFormat;

using PodvStereoVideoConfiguration = VCD::MP4::VideoFramePackingType;

using Rotation = VCD::MP4::Rotation;

VCD_OMAF_END;

#endif /* ISO_STRUCTURE_H */
