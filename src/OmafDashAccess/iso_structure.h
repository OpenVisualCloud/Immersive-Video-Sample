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

#include "general.h"

VCD_OMAF_BEGIN
struct FourCC
{
    char symbol[5];
    inline FourCC()
    : symbol{}
    {
    }

    inline FourCC(uint32_t sym)
    {
        symbol[0] = char((sym >> 24) & 0xff);
        symbol[1] = char((sym >> 16) & 0xff);
        symbol[2] = char((sym >> 8) & 0xff);
        symbol[3] = char((sym >> 0) & 0xff);
        symbol[4] = '\0';
    }
    inline FourCC(const char* string)
    {
        symbol[0] = string[0];
        symbol[1] = string[1];
        symbol[2] = string[2];
        symbol[3] = string[3];
        symbol[4] = '\0';
    }
    inline FourCC(const FourCC& F)
    {
        symbol[0] = F.symbol[0];
        symbol[1] = F.symbol[1];
        symbol[2] = F.symbol[2];
        symbol[3] = F.symbol[3];
        symbol[4] = '\0';
    }
    inline FourCC& operator=(const FourCC& F)
    {
        symbol[0] = F.symbol[0];
        symbol[1] = F.symbol[1];
        symbol[2] = F.symbol[2];
        symbol[3] = F.symbol[3];
        symbol[4] = '\0';
        return *this;
    }
    inline bool operator==(const FourCC& F) const
    {
        return (symbol[0] == F.symbol[0]) && (symbol[1] == F.symbol[1]) && (symbol[2] == F.symbol[2]) &&
                (symbol[3] == F.symbol[3]);
    }
    inline bool operator!=(const FourCC& F) const
    {
        return (symbol[0] != F.symbol[0]) || (symbol[1] != F.symbol[1]) || (symbol[2] != F.symbol[2]) ||
                (symbol[3] != F.symbol[3]);
    }
    inline bool operator<(const FourCC& F) const
    {
        return (symbol[0] < F.symbol[0])
                ? true
                : (symbol[0] > F.symbol[0])
                ? false
                : (symbol[1] < F.symbol[1])
                ? true
                : (symbol[1] > F.symbol[1])
                ? false
                : (symbol[2] < F.symbol[2])
                ? true
                : (symbol[2] > F.symbol[2])
                ? false
                : (symbol[3] < F.symbol[3])
                ? true
                : (symbol[3] > F.symbol[3]) ? false : false;
    }
    inline bool operator<=(const FourCC& F) const
    {
        return *this == F || *this < F;
    }
    inline bool operator>=(const FourCC& F) const
    {
        return !(*this < F);
    }
    inline bool operator>(const FourCC& F) const
    {
        return !(*this <= F);
    }
};

enum TrackSampleType
{
    out_ref,
    out_non_ref,
    non_out_ref,
    display,
    samples,
};

enum class ViewIdc : uint8_t
{
    MONOSCOPIC     = 0,
    LEFT           = 1,
    RIGHT          = 2,
    LEFT_AND_RIGHT = 3,
    INVALID        = 0xff
};

typedef struct ChannelLayout
{
    uint8_t  speakerPosition;
    int16_t  azimuthDegree;
    int8_t   elevationDegree;
}ChannelLayout;

typedef struct chnlProperty
{
    uint8_t                    streamStruct;
    uint8_t                    definedLayout;
    uint64_t                   omittedChannelsMap;
    uint8_t                    objectNumber;
    uint16_t                   channelNumber;
    std::vector<ChannelLayout> channelLayoutArrays;
}chnlProperty;


enum class RegionWisePackingType : uint8_t
{
    RECTANGULAR = 0
};

enum class CoverageShapeType : uint8_t
{
    FOUR_CIRCLES = 0,
    TWO_AZIMUTH_TWO_ELEVATION_CIRCLES
};

typedef struct CoverageSphereRegion
{
    ViewIdc  viewIdc;
    int32_t  azimuthCentre;
    int32_t  elevationCentre;
    int32_t  centreTilt;
    uint32_t azimuthRange;
    uint32_t elevationRange;
    bool     interpolate;
}CoverageSphereRegion;

typedef struct CoverageInformationProperty
{
    CoverageShapeType                 covShapeType;
    bool                              viewIdcPresenceFlag;
    ViewIdc                           viewIdc;
    std::vector<CoverageSphereRegion> sphereRegions;
}CoverageInformationProperty;

typedef struct SpatialAudioProperty
{
    uint8_t               version;
    uint8_t               ambisonicType;
    uint32_t              ambisonicOrder;
    uint8_t               ambisonicChannelOrder;
    uint8_t               ambisonicNorm;
    std::vector<uint32_t> channelMap;
}SpatialAudioProperty;

enum class StereoScopic3DProperty : uint8_t
{
    MONO               = 0,
    STEREO_TOP_BOTTOM  = 1,
    STEREO_LEFT_RIGHT  = 2,
    STEREO_STEREO      = 3
};

typedef struct PoseDegreesFP
{
    int32_t yawFP;
    int32_t pitchFP;
    int32_t rollFP;
}PoseDegreesFP;

typedef struct CubemapProjection
{
    uint32_t layout;
    uint32_t padding;
}CubemapProjection;

typedef struct EquirectangularProjection
{
    uint32_t topFP;
    uint32_t bottomFP;
    uint32_t leftFP;
    uint32_t rightFP;
}EquirectangularProjection;

enum class ProjectionType : uint8_t
{
    UNKOWN          = 0,
    CUBEMAP         = 1,
    EQUIRECTANGULAR = 2,
    MESH            = 3
};

typedef struct SphericalVideoV2Property
{
    PoseDegreesFP pose;
    ProjectionType    projectionType;
    union Projection {
        CubemapProjection cubemap;
        EquirectangularProjection equirectangular;
    } projection;
}SphericalVideoV2Property;

typedef struct SphericalVideoV1Property
{
    bool              isSpherical;
    bool              isStitched;
    ProjectionType    projectionType;
    uint32_t          sourceNumber;
    PoseDegreesFP     initialViewport;
    uint64_t          timestamp;
    uint32_t          fullPanoWidthPixels;
    uint32_t          fullPanoHeightPixels;
    uint32_t          croppedAreaImageWidthPixels;
    uint32_t          croppedAreaImageHeightPixels;
    uint32_t          croppedAreaLeftPixels;
    uint32_t          croppedAreaTopPixels;
}SphericalVideoV1Property;

enum DecSpecInfoType
{
    AVC_SPS  = 7,
    AVC_PPS  = 8,
    HEVC_VPS = 32,
    HEVC_SPS = 33,
    HEVC_PPS = 34,
    AudioSpecificConfig
};

typedef struct DecoderSpecificInfo
{
    DecSpecInfoType      decodeSpecInfoType;
    std::vector<uint8_t> decodeSpecInfoData;
}DecoderSpecificInfo;

typedef struct TimestampIDPair
{
    uint64_t timeStamp;
    uint32_t itemId;
}TimestampIDPair;

typedef uint32_t FeatureBitMask;

namespace TrackFeatureEnum
{
    enum Feature
    {
        isVideoTrack            = 1u,
        isAudioTrack            = 1u << 1,
        isMetadataTrack         = 1u << 2,
        hasAlternatives         = 1u << 3,
        hasSampleGroups         = 1u << 4,
        hasAssociatedDepthTrack = 1u << 5
    };

    enum VRFeature
    {
        isAudioLSpeakerChnlStructTrack  = 1u << 2,
        isVRGoogleSpatialAudioTrack     = 1u << 8,
        isVRGoogleNonDiegeticAudioTrack = 1u << 9,
        hasVRGoogleStereoscopic3D       = 1u << 12,
        hasVRGoogleV1SpericalVideo      = 1u << 13,
        hasVRGoogleV2SpericalVideo      = 1u << 14,
    };
};

typedef struct TypeToTrackIDs
{
    FourCC type;
    std::vector<uint32_t> trackIds;
}TypeToTrackIDs;

typedef struct SampleFlagsType
{
    uint32_t reserved                   : 4,
             isLeading                  : 2,
             sampleDependsOn            : 2,
             sampleIsDependedOn         : 2,
             sampleHasRedundancy        : 2,
             samplePaddingValue         : 3,
             sampleIsNonSyncSample      : 1,
             sampleDegradationPriority  : 16;
}SampleFlagsType;

union SampleFlags {
    uint32_t        flagsAsUInt;
    SampleFlagsType flags;
};

enum SampleType
{
    OUTPUT_NON_REF_FRAME,
    OUTPUT_REF_FRAME,
    NON_OUTPUT_REF_FRAME
};

typedef struct SampleInformation
{
    uint32_t    id;
    FourCC      entryType;
    uint32_t    descriptionIndex;
    SampleType  type;
    uint32_t    initSegmentId;
    uint32_t    segmentId;
    uint64_t    earliestTimestamp;
    SampleFlags flags;
    uint64_t    durationTS;
    uint64_t    earliestTimestampTS;
}SampleInformation;

typedef struct TrackTypeInformation
{
    FourCC majorBrand;
    uint32_t minorVersion;
    std::vector<FourCC> compatibleBrandArrays;
}TrackTypeInformation;

typedef struct TrackInformation
{
    uint32_t trackId;
    uint32_t initSegId;
    uint32_t alternateGroupId;
    FeatureBitMask featureBM;
    FeatureBitMask vrFeatureBM;
    std::vector<char> trackURI;
    std::vector<uint32_t> alternateTrackIdArrays;
    std::vector<TypeToTrackIDs> referenceTrackIdArrays;
    std::vector<TypeToTrackIDs> trackGroupIdArrays;
    std::vector<SampleInformation*> samplePropertyArrays;
    uint32_t maxSampleSize;
    uint32_t timeScale;
    Fractional frameRate;
    bool hasTypeInformation;
    TrackTypeInformation type;
}TrackInformation;

typedef struct SegmentInformation
{
    uint32_t refId;
    uint32_t timescale;
    bool     refType;
    uint64_t earliestPTSinTS;
    uint32_t durationInTS;
    uint64_t startDataOffset;
    uint32_t dataSize;
    bool     isStartedWithSAP;
    uint8_t  SAPType;
}SegmentInformation;

typedef struct SchemeType
{
    FourCC type;
    uint32_t version;
    std::vector<char> uri;
}SchemeType;

typedef struct SchemeTypesProperty
{
    SchemeType mainScheme;
    std::vector<SchemeType> compatibleSchemeTypes;
}SchemeTypesProperty;

enum OmafProjectionType
{
    EQUIRECTANGULAR = 0,
    CUBEMAP
};

typedef struct ProjectionFormatProperty
{
    OmafProjectionType format;
}ProjectionFormatProperty;

enum PodvStereoVideoConfiguration
{
    TOP_BOTTOM_PACKING    = 3,
    SIDE_BY_SIDE_PACKING  = 4,
    TEMPORAL_INTERLEAVING = 5,
    MONOSCOPIC            = 0x8f
};

typedef struct Rotation
{
    int32_t yaw;
    int32_t pitch;
    int32_t roll;
}Rotation;

typedef struct SphereRegionProperty
{
    int32_t  azimuthCentre;
    int32_t  elevationCentre;
    int32_t  centreTilt;
    uint32_t azimuthRange;
    uint32_t elevationRange;
    bool     interpolate;
}SphereRegionProperty;

VCD_OMAF_END;

#endif /* ISO_STRUCTURE_H */

