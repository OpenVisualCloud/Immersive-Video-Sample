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
//! \file:   Mp4DataTypes.h
//! \brief:  Mp4 file related information definition
//! \detail: Define related information about mp4 file, like
//!          track description and so on.
//!

#ifndef _MP4DATATYPES_H_
#define _MP4DATATYPES_H_

#include <stddef.h>
#include <stdint.h>

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "../include/Common.h"
#include "../include/Index.h"
#include "../atoms/FormAllocator.h"

#include "../atoms/Atom.h"
#include "../atoms/RestSchemeInfoAtom.h"
#include "../atoms/DecPts.h"
#include "../atoms/TypeAtom.h"

#include "Mp4StreamIO.h"
#include "../atoms/TrackExtAtom.h"
#include "../atoms/TrackRefTypeAtom.h"

//#ifndef _SCL_SECURE_NO_WARNINGS
//#define _SCL_SECURE_NO_WARNINGS
//#endif

using namespace std;

VCD_MP4_BEGIN

template <typename T>
struct VarLenArray
{
    typedef T valType;
    typedef T* iterator;
    typedef const T* constIter;
    typedef size_t sizeType;
    typedef ptrdiff_t diffType;
    size_t size;
    T* arrayElets;
    VarLenArray();
    VarLenArray(size_t n);
    VarLenArray(const VarLenArray& other);
    VarLenArray& operator=(const VarLenArray& other);
    inline T& operator[](size_t index)
    {
        return arrayElets[index];
    }
    inline const T& operator[](size_t index) const
    {
        return arrayElets[index];
    }
    inline T* GetBegin()
    {
        return arrayElets;
    }
    inline T* GetEnd()
    {
        return arrayElets + size;
    }
    inline const T* GetBegin() const
    {
        return arrayElets;
    }
    inline const T* GetEnd() const
    {
        return arrayElets + size;
    }
    ~VarLenArray();
};

enum SampleFrameType
{
    OUT_REF_PIC,
    OUT_NONREF_PIC,
    NON_OUT_REFPIC,
    DISPLAY_PIC,
    SAMPLES_PIC,
};

struct ChannelLayout
{
    uint8_t speakerPosition;
    int16_t azimuth;
    int8_t elevation;
};

struct ChnlProperty
{
    uint8_t strStruct;
    uint8_t layOut;
    uint64_t omitChnlMap;
    uint8_t objCnt;
    uint16_t chnlCnt;
    VarLenArray<ChannelLayout> chnlLayOuts;
};

enum class OmniRWPKType : uint8_t
{
    OMNI_RECTANGULAR = 0
};

struct RectRWPKRegion
{
    uint8_t  transformType;
    bool     guardBandFlag;
    uint32_t projRegWidth;
    uint32_t projRegHeight;
    uint32_t projRegTop;
    uint32_t projRegLeft;
    uint16_t packedRegWidth;
    uint16_t packedRegHeight;
    uint16_t packedRegTop;
    uint16_t packedRegLeft;

    uint8_t leftGbWidth;
    uint8_t rightGbWidth;
    uint8_t topGbHeight;
    uint8_t bottomGbHeight;
    bool    gbNotUsedForPredFlag;
    uint8_t gbType0;
    uint8_t gbType1;
    uint8_t gbType2;
    uint8_t gbType3;
};

struct RWPKRegion
{
    bool guardBandFlag;
    OmniRWPKType packingType;
    union Region {
        RectRWPKRegion rectReg;
    } region;
};

struct RWPKProperty
{
    bool constituentPictureMatching = false;
    uint32_t projPicWidth;
    uint32_t projPicHeight;
    uint16_t packedPicWidth;
    uint16_t packedPicHeight;
    VarLenArray<RWPKRegion> regions;
};

struct COVIInformation
{
    COVIShapeType coviShapeType;
    bool viewIdcPresenceFlag;
    OmniViewIdc defaultViewIdc;
    VarLenArray<COVIRegion> sphereRegions;
};

struct SpatialAudioProperty
{
    uint8_t  version;
    uint8_t  ambisonicType;
    uint32_t ambisonicOrder;
    uint8_t  ambisonicChnlSeq;
    uint8_t  ambisonicNorml;
    VarLenArray<uint32_t> chnlMap;
};

enum class OmniStereoScopic3D : uint8_t
{
    OMNI_MONOSCOPIC                 = 0,
    OMNI_STEREOSCOPIC_TOP_BOTTOM    = 1,
    OMNI_STEREOSCOPIC_LEFT_RIGHT    = 2,
    OMNI_STEREOSCOPIC_STEREO_CUSTOM = 3
};

struct RotateDegree1616FP
{
    int32_t fixedPntYaw;
    int32_t fixedPntPitch;
    int32_t fixedPntRoll;
};

struct CubemapProjectionSetting
{
    uint32_t layout;
    uint32_t padding;
};

struct EquirectangularProjectionSetting
{
    uint32_t topBoundFP;
    uint32_t btmBoundFP;
    uint32_t leftBoundFP;
    uint32_t rightBoundFP;
};

enum class OmniProjType : uint8_t
{
    OMNI_UNKOWN          = 0,
    OMNI_CUBEMAP         = 1,
    OMNI_ERP             = 2,
    OMNI_MESH            = 3
};

struct SphericalVideoV2Property
{
    RotateDegree1616FP rotateDegree;
    OmniProjType projType;
    union Projection {
        CubemapProjectionSetting cubemap;
        EquirectangularProjectionSetting equirectangular;
    } projection;
};

struct SphericalVideoV1Property
{
    bool isSphe;
    bool isStitched;
    OmniProjType projType;
    uint32_t srcCnt;

    RotateDegree1616FP initView;
    uint64_t timestamp;
    uint32_t panoWidth;
    uint32_t panoHeight;
    uint32_t croppedWidth;
    uint32_t croppedHeight;
    uint32_t croppedLeftW;
    uint32_t croppedTopH;
};

enum MediaCodecInfoType
{
    AVC_SPS  = 7,
    AVC_PPS  = 8,
    HEVC_VPS = 32,
    HEVC_SPS = 33,
    HEVC_PPS = 34,
    AudioSpecificConfig
};

struct MediaCodecSpecInfo
{
    MediaCodecInfoType codecSpecInfoType;
    VarLenArray<uint8_t> codecSpecInfoBits;

    MediaCodecSpecInfo() : codecSpecInfoType(HEVC_VPS) {};
};

struct TStampID
{
    uint64_t timeStamp;
    uint32_t itemId;
};

enum FeatureOfFile
{
    CONTAIN_ALT_TRACKS = 1u << 3
};

typedef uint32_t FeatureBitMask;

struct FileFeaturesInfo
{
    FeatureBitMask features;
};

enum FeatureOfTrack
{
    IsVideo            = 1u,
    IsAudio            = 1u << 1,
    IsMetadata         = 1u << 2,
    HasAlternatives         = 1u << 3,
    HasSampleGroups         = 1u << 4,
    HasAssociatedDepthTrack = 1u << 5
};

enum ImmersiveProperty
{
    IsAudioLSpeakerChnlStructTrack = 1u << 2,
    IsVRSpatialAudioTrack = 1u << 8,
    IsVRNonDiegeticAudioTrack = 1u << 9,
    HasVRStereoscopic3D = 1u << 12,
    HasVRV1SpericalVideo = 1u << 13,
    HasVRV2SpericalVideo = 1u << 14,
};

struct TypeToTrackIDs
{
    FourCC type;
    VarLenArray<uint32_t> trackIds;
};

enum FrameCodecType
{
    OUTPUT_NONREF_FRAME,
    OUTPUT_REF_FRAME,
    NON_OUTPUT_REF_FRAME
};

struct TrackSampInfo
{
    uint32_t sampleId;
    FourCC sampleEntryType;
    uint32_t sampleDescriptionIndex;
    FrameCodecType sampleType;
    uint32_t initSegmentId;
    uint32_t segmentId;
    uint64_t earliestTStamp;
    FlagsOfSample sampleFlags;
    uint64_t sampleDurationTS;
    uint64_t earliestTStampTS;
};

struct RatValue
{
    uint64_t num;
    uint64_t den;
};

struct TrackTypeInformation
{
    FourCC majorBrand;
    uint32_t minorVersion;
    VarLenArray<FourCC> compatibleBrands;
};

struct TrackInformation
{
    uint32_t initSegmentId;
    uint32_t trackId;
    uint32_t alternateGroupId;
    FeatureBitMask features;
    FeatureBitMask vrFeatures;
    VarLenArray<char> trackURI;
    VarLenArray<uint32_t> alternateTrackIds;
    VarLenArray<TypeToTrackIDs> referenceTrackIds;
    VarLenArray<TypeToTrackIDs> trackGroupIds;
    VarLenArray<TrackSampInfo> sampleProperties;
    uint32_t maxSampleSize;
    uint32_t timeScale;
    RatValue frameRate;
    bool hasTypeInformation;
    TrackTypeInformation type;

    TrackInformation() :
        initSegmentId(0), trackId(0), alternateGroupId(0), features(0),
        vrFeatures(0), maxSampleSize(0), timeScale(1), hasTypeInformation(false) {};
};

struct SegInfo
{
    uint32_t segmentId;
    uint32_t referenceId;
    uint32_t timescale;
    bool referenceType;
    uint64_t earliestPTSinTS;
    uint32_t durationInTS;
    uint64_t startDataOffset;
    uint32_t dataSize;
    bool startsWithSAP;
    uint8_t SAPType;
};

struct SchemeType
{
    FourCC type;
    uint32_t version;
    VarLenArray<char> uri;
};

struct SchemeTypesProperty
{
    SchemeType mainScheme;
    VarLenArray<SchemeType> compatibleSchemeTypes;
};

struct ProjFormat
{
    OmniProjFormat format;
};

struct ChnlPropertyInternal
{
    uint8_t strStruct;
    uint8_t layOut;
    uint64_t omitChnlMap;
    uint8_t objCnt;
    uint16_t chnlCnt;
    vector<ChannelLayout> chnlLayOuts;
};

struct RWPKPropertyInternal
{
    bool constituentPicMatching;
    uint32_t projPicWidth;
    uint32_t projPicHeight;
    uint16_t packedPicWidth;
    uint16_t packedPicHeight;
    vector<RWPKRegion> regions;
};

struct COVIInformationInternal
{
    COVIShapeType coviShapeType;
    bool viewIdcPresenceFlag;
    OmniViewIdc defaultViewIdc;
    vector<COVIRegion> sphereRegions;
};

struct SA3DPropertyInternal
{
    uint8_t  version;
    uint8_t  ambisonicType;
    uint32_t ambisonicOrder;
    uint8_t  ambisonicChnlSeq;
    uint8_t  ambisonicNorml;
    vector<uint32_t> chnlMap;
};

struct SchemeTypesPropertyInternal
{
    SchemeTypeAtom mainScheme;
    vector<CompatibleSchemeTypeAtom> compatibleSchemes;
};

struct SampleRes
{
    uint32_t width;
    uint32_t height;
};

// Convenience types
class SegmentIdTag
{
};
class InitSegmentIdTag
{
};
class ContextIdTag
{
};
class ItemIdTag
{
};
class SmpDesIndexTag
{
};
class SequenceTag
{
};

typedef Index<uint32_t, SegmentIdTag> SegmentId;
typedef Index<uint32_t, InitSegmentIdTag> InitSegmentId;
typedef IndexExplicit<uint32_t, ContextIdTag> ContextId;
typedef Index<uint32_t, SmpDesIndexTag> SmpDesIndex;
typedef Index<uint32_t, SequenceTag> Sequence;
typedef uint64_t TStamp;
typedef IndexCalculation<uint32_t, ItemIdTag> ItemId;
typedef std::vector<pair<ItemId, TStamp>> DecodingOrderVector;
typedef std::vector<uint32_t> IdVector;
typedef VectorT<ContextId> ContextIdVector;
typedef std::vector<uint8_t> DataVector;
typedef std::vector<TStamp> TStampVector;
typedef std::map<TStamp, ItemId> TStampMap;
typedef std::map<FourCCInt, IdVector> TypeToIdsMap;
typedef std::map<FourCCInt, ContextIdVector> TypeToCtxIdsMap;
typedef std::map<FourCCInt, std::vector<IdVector>> GroupingMap;
typedef std::map<MediaCodecInfoType, DataVector> ParameterSetMap;
typedef VarLenArray<SegInfo> SegmentIndex;

typedef pair<InitSegmentId, ContextId> InitSegmentTrackId;
typedef pair<SegmentId, ContextId> SegmentTrackId;

class FileProperty
{
public:
    FileProperty()
        : m_trackId(0){};
    // typedef Set<FeatureOfFile> FilePropertySet;
    typedef set<FeatureOfFile> FilePropertySet;

    bool HasProperty(FeatureOfFile feature) const
    {
        return m_filePro.count(feature) != 0;
    }
    void SetProperty(FeatureOfFile feature)
    {
        m_filePro.insert(feature);
    }
    uint32_t GetTrackIndex() const
    {
        return m_trackId;
    }
    void SetTrackIndex(uint32_t id)
    {
        m_trackId = id;
    }
    unsigned int GetFeatureMask() const
    {
        unsigned int mask = 0;
        for (auto set : m_filePro)
        {
            mask |= (unsigned int) set;
        }
        return mask;
    }

private:
    uint32_t m_trackId;
    FilePropertySet m_filePro;
};

class TrackProperty
{
public:
    // typedef Set<FeatureOfTrack> TrackPropertySet;
    // typedef Set<ImmersiveProperty> TrackVRFeatureSet;
    typedef set<FeatureOfTrack> TrackPropertySet;
    typedef set<ImmersiveProperty> TrackVRFeatureSet;

    bool HasProperty(FeatureOfTrack feature) const
    {
        return m_trackProSet.count(feature) != 0;
    }
    void SetProperty(FeatureOfTrack feature)
    {
        m_trackProSet.insert(feature);
    }
    bool HasImmersiveProperty(ImmersiveProperty vrFeature) const
    {
        return m_trackVRFeaSet.count(vrFeature) != 0;
    }
    void SetImmersiveProperty(ImmersiveProperty vrFeature)
    {
        m_trackVRFeaSet.insert(vrFeature);
    }
    unsigned int GetFeatureMask() const
    {
        unsigned int mask = 0;
        for (auto set : m_trackProSet)
        {
            mask |= (unsigned int) set;
        }
        return mask;
    }
    unsigned int GetVRFeatureMask() const
    {
        unsigned int mask = 0;
        for (auto set : m_trackVRFeaSet)
        {
            mask |= (unsigned int) set;
        }
        return mask;
    }

private:
    TrackPropertySet m_trackProSet;
    TrackVRFeatureSet m_trackVRFeaSet;
};

class MoovProperty
{
public:
    /// Enumerated list of Moov Features
    enum Property
    {
        HasMoovLevelMetaBox,
        HasCoverImage
    };
    // typedef Set<Property> MoovPropertySet;
    typedef set<Property> MoovPropertySet;

    bool HasProperty(Property property) const
    {
        return m_moovProSet.count(property) != 0;
    }
    void SetProperty(Property property)
    {
        m_moovProSet.insert(property);
    }

private:
    MoovPropertySet m_moovProSet;
};

typedef pair<InitSegmentTrackId, ItemId> InitSegTrackIdPair;
typedef pair<ItemId, TStamp> ItemIdTStampPair;

struct MoovProperties
{
    MoovProperty moovFeature;
    uint64_t fragmentDuration;
    std::vector<SampleDefaults> fragmentSampleDefaults;
};

struct TrackProperties
{
    uint32_t alternateGroupId;
    std::string trackURI;
    TrackProperty trackProperty;
    IdVector alternateTrackIds;              ///< other tracks IDs with the same alternate_group id.
    TypeToCtxIdsMap referenceTrackIds;   ///< <reference_type, reference track ID> (coming from 'tref')
    TypeToIdsMap trackGroupIds;              ///< <group_type, track IDs> ... coming from Track Group Box 'trgr'
    shared_ptr<const EditAtom> editBox;  ///< If set, an edit list box exists
};
typedef std::map<ContextId, TrackProperties> TrackPropertiesMap;

struct SampleInfo
{
    SegmentId segmentId;
    uint32_t sampleId;
    vector<uint64_t> compositionTimes;
    vector<uint64_t> compositionTimesTS;
    uint64_t dataOffset = 0;
    uint32_t dataLength = 0;
    uint32_t width      = 0;
    uint32_t height     = 0;
    uint32_t sampleDuration = 0;

    FourCCInt sampleEntryType;
    SmpDesIndex sampleDescriptionIndex;
    FrameCodecType sampleType;
    FlagsOfSample sampleFlags;
};
typedef vector<SampleInfo> SampleInfoVector;

struct TrackBasicInfo
{
    uint32_t timeScale;
    uint32_t width;
    uint32_t height;

    FourCCInt sampleEntryType;

    std::map<SmpDesIndex, ParameterSetMap> parameterSetMaps;
    std::map<SmpDesIndex, ChnlPropertyInternal> chnlProperties;
    std::map<SmpDesIndex, SA3DPropertyInternal> sa3dProperties;
    std::map<SmpDesIndex, OmniStereoScopic3D> st3dProperties;
    std::map<SmpDesIndex, SphericalVideoV1Property> sphericalV1Properties;
    std::map<SmpDesIndex, SphericalVideoV2Property> sv3dProperties;
    std::map<SmpDesIndex, SampleRes> sampleRes;
    std::map<SmpDesIndex, ProjFormat> pfrmProperties;
    std::map<SmpDesIndex, RWPKPropertyInternal> rwpkProperties;
    std::map<SmpDesIndex, Rotation> rotnProperties;
    std::map<SmpDesIndex, VideoFramePackingType> stviProperties;
    std::map<SmpDesIndex, COVIInformationInternal> coviProperties;
    std::map<SmpDesIndex, SchemeTypesPropertyInternal> schemeTypesProperties;
    std::map<SmpDesIndex, uint8_t> nalLengthSizeMinus1;
};

struct TrackDecInfo
{
    ItemId itemIdBase;
    SampleInfoVector samples;

    DecodePts::PresentTimeTS durationTS    = 0;
    DecodePts::PresentTimeTS earliestPTSTS = 0;
    DecodePts::PresentTimeTS noSidxFallbackPTSTS = 0;

    DecodePts::PresentTimeTS nextPTSTS = 0;

    std::map<ItemId, FourCCInt> decoderCodeTypeMap;

    DecodePts::PMap pMap;
    DecodePts::PMapTS pMapTS;

    bool hasEditList = false;
    bool hasTtyp = false;
    TrackTypeAtom ttyp;
};

struct SegmentIO
{
    UniquePtr<StreamIOInternal> strIO;
    UniquePtr<StreamIO> fileStrIO;
    int64_t size = 0;
};

typedef std::map<InitSegTrackIdPair, SmpDesIndex> ItemToParameterSetMap;

struct SegmentProperties
{
    InitSegmentId initSegmentId;
    SegmentId segmentId;
    set<Sequence> sequences;
    SegmentIO io;

    SegmentTypeAtom styp;

    std::map<ContextId, TrackDecInfo> trackDecInfos;
    ItemToParameterSetMap itemToParameterSetMap;
};

typedef std::map<SegmentId, SegmentProperties> SegPropMap;
typedef std::map<Sequence, SegmentId> SeqToSegMap;

struct InitSegmentProperties
{
    FileProperty fileProperty;
    MoovProperties moovProperties;
    TrackPropertiesMap trackProperties;
    uint32_t movieTScale;

    FileTypeAtom ftyp;  ///< File Type Box for later information retrieval

    std::map<ContextId, TrackBasicInfo> basicTrackInfos;

    SegPropMap segPropMap;
    SeqToSegMap seqToSeg;

    SegmentIndex segmentIndex;

    ContextId    corresTrackId;
};

struct ExtNalHdr
{
    uint8_t forbidden_zero_bit    = 0;
    uint8_t nal_unit_type         = 0;
    uint8_t nuh_layer_id          = 0;
    uint8_t nuh_temporal_id_plus1 = 0;
};

struct ExtSample
{
    struct SampleConstruct
    {
        uint8_t order_idx;
        uint8_t constructor_type;
        uint8_t track_ref_index;
        int8_t sample_offset;
        uint32_t data_offset;
        uint32_t data_length;
    };
    struct InlineConstruct
    {
        uint8_t order_idx;
        uint8_t constructor_type;
        uint8_t data_length;
        vector<uint8_t> inline_data;
    };

    struct Extractor
    {
        vector<InlineConstruct> inlineConstruct;
        vector<SampleConstruct> sampleConstruct;
    };
    vector<Extractor> extractors;
};

struct Hvc2ExtractorNal
{
    ExtNalHdr extNalHdr = {};
    ExtSample extNalDat = {};
};

VCD_MP4_END;
#endif /* _MP4DATATYPES_H_ */
