/*
 * Copyright (c) 2021, Intel Corporation
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
//! \file:   MediaData.h
//! \brief:  Media type and media data related definition
//!

#ifndef _MEDIADATA_H_
#define _MEDIADATA_H_

#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Frame.h"
#include "AcquireTrackData.h"

using namespace std;

VCD_MP4_BEGIN

class MovieHeaderAtom;

//!
//! \enum:   CodedFormat
//! \brief:  indicate the coded format of the data
//!
enum CodedFormat
{
    NoneFormat,
    H264,
    H265,
    AAC,
    TimedMetadata,
    H265Extractor
};

//!
//! \enum:   ConfigType
//! \brief:  indicate the paramter set type
//!
enum ConfigType
{
    SPS,                    // for AVC and HEVC
    PPS,                    // for AVC and HEVC
    VPS,                    // for HEVC
    AudioSpecificConfig     // for AAC
};

//!
//! \struct: Bitrate
//! \brief:  define the bitrate information of input data
//!
struct Bitrate
{
    uint32_t avgBitrate;
    uint32_t maxBitrate;
};

//!
//! \enum:   FrameType
//! \brief:  indicate the frame type, whether IDR or not
//!
enum FrameType
{
    NA,
    IDR,
    NONIDR,
    // more types directly from the H264 header
};

//!
//! \struct: SegmenterMeta
//! \brief:  define the meta data of the segment
//!
struct SegmenterMeta
{
    // the duration of the produced segment
    FrameDuration segmentDuration;
};

//!
//! \enum:   OmafProjectionType
//! \brief:  define the picture projection type
//!
enum OmafProjectionType
{
    NoneProjection,
    EQUIRECTANGULAR,
    CUBEMAP,
    PLANAR
};

//!
//! \struct: Region
//! \brief:  define the region wise packing information
//!          for one region
//!
struct Region
{
    uint32_t projTop;
    uint32_t projLeft;
    uint32_t projWidth;
    uint32_t projHeight;
    int32_t transform;
    uint16_t packedTop;
    uint16_t packedLeft;
    uint16_t packedWidth;
    uint16_t packedHeight;
};

//!
//! \struct: RegionPacking
//! \brief:  define the region wise packing information
//!          for all regions
//!
struct RegionPacking
{
    bool constituentPictMatching;
    uint32_t projPictureWidth;
    uint32_t projPictureHeight;
    uint16_t packedPictureWidth;
    uint16_t packedPictureHeight;

    std::vector<Region> regions;
};

//!
//! \struct: Spherical
//! \brief:  define the content coverage information
//!          for one region on the sphere
//!
struct Spherical
{
    int32_t cAzimuth;
    int32_t cElevation;
    int32_t cTilt;
    uint32_t rAzimuth;
    uint32_t rElevation;
    //bool interpolate;
};

//!
//! \struct: QualityInfo
//! \brief:  define the quality rank related information
//!          for one region
//!
struct QualityInfo
{
    uint8_t qualityRank;
    uint16_t origWidth = 0;    // used only with multi-res cases
    uint16_t origHeight = 0;   // used only with multi-res cases
    DataItem<Spherical> sphere;    // not used with remaining area info
};

//!
//! \struct: Quality3d
//! \brief:  define the quality rank related information
//!          for all regions
//!
struct Quality3d
{
    uint8_t shapeType = 0;
    uint8_t qualityType = 0;
    bool remainingArea = false;
    std::vector<QualityInfo> qualityInfo;
};

//!
//! \struct: CodedMeta
//! \brief:  define the meta data for the coded data
//!
struct CodedMeta
{
    int64_t presIndex;        // presentation index (as in RawFormatMeta)
    int64_t codingIndex; // coding index
    FrameTime codingTime;
    FrameTime presTime;
    FrameDuration duration;

    TrackId trackId;

    bool inCodingOrder;

    CodedFormat format = CodedFormat::NoneFormat;

    std::map<ConfigType, std::vector<uint8_t>> decoderConfig;

    uint32_t width = 0;
    uint32_t height = 0;

    uint8_t channelCfg = 0;
    uint32_t samplingFreq = 0;
    Bitrate bitrate = {}; // bitrate information

    FrameType type = FrameType::NA;

    SegmenterMeta segmenterMeta;

    // applicable in OMAF
    OmafProjectionType projection = OmafProjectionType::EQUIRECTANGULAR;
    DataItem<RegionPacking> regionPacking;
    DataItem<Spherical> sphericalCoverage;
    DataItem<Quality3d> qualityRankCoverage;

    bool isEOS = false;

    bool isIDR() const
    {
        return type == FrameType::IDR;
    }
};

struct TrackInfo
{
    FrameTime tBegin;
    TrackMeta trackMeta;
    FrameTime dtsCtsOffset;
};

struct TrackOfSegment
{
    TrackInfo trackInfo;
    Frames frames;
};

typedef map<TrackId, TrackOfSegment> TrackSegmentMap;

typedef IndexCalculation<uint32_t, class SequenceTag> SequenceId;

struct Segment
{
    TrackSegmentMap tracks;
    SequenceId sequenceId;
    FrameTime tBegin;
    FractU64 duration;
};

typedef list<Segment> SegmentList;

struct FramesForTrack
{
    TrackMeta trackMeta;
    Frames frames;

    FramesForTrack()
    {
    }

    FramesForTrack(TrackMeta aTrackMeta, Frames aFrames)
        : trackMeta(aTrackMeta)
        , frames(aFrames)
    {
    }

    FramesForTrack(FramesForTrack&& aOther)
        : trackMeta(move(aOther.trackMeta))
        , frames(move(aOther.frames))
    {
    }

    FramesForTrack(const FramesForTrack& aOther) = default;
    FramesForTrack& operator=(const FramesForTrack&) = default;
};

typedef vector<FramesForTrack> TrackFrames;

struct FileTypeBoxWrapper;
struct MovieBoxWrapper;
struct MediaHeaderBoxWrapper;
struct HandlerBoxWrapper;
struct TrackHeaderBoxWrapper;
struct SampleEntryBoxWrapper;
struct RegionBlock;

class InitialSegment
{
public:
    InitialSegment();
    InitialSegment(const InitialSegment& initSeg);
    InitialSegment& operator=(const InitialSegment& initSeg);
    InitialSegment& operator=(InitialSegment&& initSeg);
    ~InitialSegment();
    unique_ptr<MovieBoxWrapper> moov;
    unique_ptr<FileTypeBoxWrapper> ftyp;
};

struct FileInfo
{
    uint64_t creationTime;
    uint64_t modificationTime;
    FractU64 duration;
};

struct OmniSampleEntry
{
    OmniSampleEntry() = default;
    virtual ~OmniSampleEntry() = default;
    virtual unique_ptr<SampleEntryBoxWrapper> GenSampleEntryBox() const = 0;
    virtual unique_ptr<HandlerBoxWrapper> GenHandlerBox() const         = 0;

    virtual uint32_t GetWidthFP() const = 0;

    virtual uint32_t GetHeightFP() const = 0;
};

enum class OmniMediaType
{
    OMNI_Mono,
    OMNI_StereoLR,
    OMNI_StereoTB
};

struct RWPKGuardBand
{
    uint8_t leftGbWidth;
    uint8_t rightGbWidth;
    uint8_t topGbHeight;
    uint8_t bottomGbHeight;

    bool gbNotUsedForPredFlag;

    uint8_t gbType0;
    uint8_t gbType1;
    uint8_t gbType2;
    uint8_t gbType3;
};

struct RWPKRegion
{
    virtual ~RWPKRegion() = default;
    virtual uint8_t packingType() const                = 0;
    virtual unique_ptr<RegionBlock> GenRegion() const = 0;
};

struct RwpkRectRegion : public RWPKRegion
{
    uint8_t packingType() const override;
    unique_ptr<RegionBlock> GenRegion() const override;

    uint32_t projRegWidth;
    uint32_t projRegHeight;
    uint32_t projRegTop;
    uint32_t projRegLeft;

    uint8_t  transformType;

    uint16_t packedRegWidth;
    uint16_t packedRegHeight;
    uint16_t packedRegTop;
    uint16_t packedRegLeft;

    DataItem<RWPKGuardBand> rwpkGuardBand;
};

struct RegionWisePacking
{
    bool constituenPicMatching;
    uint32_t projPicWidth;
    uint32_t projPicHeight;
    uint16_t packedPicWidth;
    uint16_t packedPicHeight;

    vector<unique_ptr<RWPKRegion>> regions;
};

struct CoverageInformation
{
    COVIShapeType coverageShape;
    bool viewIdcPresenceFlag;
    OmniViewIdc defaultViewIdc;
    vector<unique_ptr<COVIRegion>> sphereRegions;
};

struct Mp4SchemeType
{
    string type;
    uint32_t version;
    string uri;
};

struct VideoSampleEntry : public OmniSampleEntry
{
    uint16_t width;
    uint16_t height;

    uint32_t GetWidthFP() const override;
    uint32_t GetHeightFP() const override;

    DataItem<OmniProjFormat> projFmt;
    DataItem<RegionWisePacking> rwpk;
    DataItem<CoverageInformation> covi;
    DataItem<VideoFramePackingType> stvi;
    DataItem<Rotation> rotn;
    vector<Mp4SchemeType> compatibleSchemes;

protected:
    void GenPovdBoxes(unique_ptr<SampleEntryBoxWrapper>& box) const;
};

struct AvcVideoSampleEntry : public VideoSampleEntry
{
    vector<uint8_t> sps;
    vector<uint8_t> pps;

    unique_ptr<SampleEntryBoxWrapper> GenSampleEntryBox() const override;
    unique_ptr<HandlerBoxWrapper> GenHandlerBox() const override;
};

struct HevcVideoSampleEntry : public VideoSampleEntry
{
    FourCC sampleEntryType = "hvc1";

    float frameRate;

    vector<uint8_t> sps;
    vector<uint8_t> pps;
    vector<uint8_t> vps;

    unique_ptr<SampleEntryBoxWrapper> GenSampleEntryBox() const override;
    unique_ptr<HandlerBoxWrapper> GenHandlerBox() const override;
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

struct HevcExtractorSampleConstructor
{
    int8_t trackId;
    int8_t sampleOffset;
    uint32_t dataOffset;
    uint32_t dataLength;

    FrameBuf GenFrameData() const;
};

struct HevcExtractorInlineConstructor
{
    vector<uint8_t> inlineData;

    FrameBuf GenFrameData() const;
};

struct HevcExtractor
{
    DataItem<HevcExtractorSampleConstructor> sampleConstructor;
    DataItem<HevcExtractorInlineConstructor> inlineConstructor;

    FrameBuf GenFrameData() const;
};

struct HevcExtractorTrackPackedData
{
    uint8_t nuhTemporalIdPlus1;
    vector<HevcExtractor> samples;

    FrameBuf GenFrameData() const;
};

struct Ambisonic
{
    uint8_t type;
    uint32_t order;
    uint8_t channelOrdering;
    uint8_t normalization;
    vector<uint32_t> channelMap;
};

struct ChannelPosition
{
    int speakerPosition = 0;
    int azimuth   = 0;
    int elevation = 0;

    ChannelPosition() = default;
};

struct ChannelLayout
{
    int streamStructure = 0;
    int layout = 0;
    vector<ChannelPosition> positions;
    set<int> omitted;
    int objectCount = 0;

    ChannelLayout() = default;
};

struct MP4AudioSampleEntry : public OmniSampleEntry
{
    uint16_t sizeOfSample;
    uint16_t cntOfChannels;
    uint32_t rateOfSample;
    uint16_t idOfES;
    uint16_t esIdOfDepends;
    string strUrl;
    uint32_t sizeOfBuf;
    uint32_t maxBitrate;
    uint32_t avgBitrate;
    string decSpecificInfo;  // tag 5

    bool isNonDiegetic;
    DataItem<Ambisonic> ambisonicItem;
    DataItem<ChannelLayout> chnLayoutItem;

    unique_ptr<SampleEntryBoxWrapper> GenSampleEntryBox() const override;
    unique_ptr<HandlerBoxWrapper> GenHandlerBox() const override;
    uint32_t GetWidthFP() const override;
    uint32_t GetHeightFP() const override;

    MP4AudioSampleEntry() = default;
};

struct TrackDescription
{
    TrackDescription();
    TrackDescription(TrackDescription&&);
    TrackDescription(TrackMeta, FileInfo, const OmniSampleEntry&);
    TrackDescription(TrackMeta,
                     list<unique_ptr<SampleEntryBoxWrapper>>,
                     unique_ptr<MediaHeaderBoxWrapper>&&,
                     unique_ptr<HandlerBoxWrapper>&&,
                     unique_ptr<TrackHeaderBoxWrapper>&&);
    TrackDescription& operator=(const TrackDescription&) = default;
    ~TrackDescription();

    TrackMeta trackMeta;
    list<unique_ptr<SampleEntryBoxWrapper>> sampleEntryBoxes;
    unique_ptr<MediaHeaderBoxWrapper> mediaHeaderBox;
    unique_ptr<HandlerBoxWrapper> handlerBox;
    unique_ptr<TrackHeaderBoxWrapper> trackHeaderBox;
    map<string, set<TrackId>> trackReferences;
    DataItem<uint16_t> alternateGroup;

    DataItem<OmniMediaType> vrType;
};

typedef map<TrackId, TrackDescription> TrackDescriptionsMap;

struct SegmentCfg
{
    FractU64 duration;
    SequenceId baseSequenceId;
};

struct MovieDescription
{
    uint64_t creationTime;
    uint64_t modificationTime;
    FractU64 duration;
    vector<int32_t> matrix;
    DataItem<BrandSpec> fileType;
};

//!
//! \enum:  ChunkInfoType
//! \brief: CMAF chunk info type
//!
enum ChunkInfoType
{
    NO_CHUNKINFO = 0,
    CHUNKINFO_SIDX_ONLY,
    CHUNKINFO_CLOC_ONLY,
    CHUNKINFO_SIDX_AND_CLOC
};

struct SegmentWriterCfg
{
    bool checkIDR = false;
    FractU64 segmentDuration;
    DataItem<FractU64> subsegmentDuration;
    size_t skipSubsegments = 0;
    bool   useSeparatedSidx = false;
    ChunkInfoType chunkInfoType = ChunkInfoType::NO_CHUNKINFO;
};

struct SidxInfo
{
    ostream::pos_type position;
    size_t size;
};

enum class Action
{
    KeepFeeding,
    ExtractSegment
};

struct VideoAdaptationSetInfo
{
    uint32_t fullWidth;
    uint32_t fullHeight;
    uint16_t tileWidth;
    uint16_t tileHeight;
    uint16_t tileHPos;
    uint16_t tileVPos;
    uint16_t tileCubemapHPos;
    uint16_t tileCubemapVPos;
    uint32_t viewHIdx;
    uint32_t viewVIdx;
};

struct MPDAdaptationSetCtx
{
    VideoAdaptationSetInfo       *videoInfo;

    std::list<VCD::MP4::TrackId> refTrackIdxs;

    VCD::MP4::TrackId            trackIdx;

    VCD::MP4::CodedMeta          codedMeta;

    uint8_t                      qualityRanking;

    MPDAdaptationSetCtx()
    {
        videoInfo = NULL;
        qualityRanking = 0;
    }

    ~MPDAdaptationSetCtx()
    {
        videoInfo = NULL;
    }
};

VCD_MP4_END;
#endif /* _MEDIADATA_H_ */
