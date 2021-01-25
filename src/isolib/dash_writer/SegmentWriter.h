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
//! \file:   SegmentWriter.h
//! \brief:  Segmenter related API definition
//! \detail: Define segment related information, like track of segment
//!          and so on.
//!

#ifndef _SEGMENTERAPI_H_
#define _SEGMENTERAPI_H_

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
#include "DataItem.h"
#include "AcquireTrackData.h"

using namespace std;

VCD_MP4_BEGIN

class MovieHeaderAtom;

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

struct InitialSegment
{
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
    OmniSampleEntry();
    virtual ~OmniSampleEntry();
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

InitialSegment GenInitSegment(const TrackDescriptionsMap& inTrackDes,
                            const MovieDescription& inMovieDes,
                            const bool isFraged);

void WriteSegmentHeader(ostream& outStr);
void WriteInitSegment(ostream& outStr, const InitialSegment& initSegment);
void WriteSampleData(ostream& outStr, const Segment& oneSeg);

struct SegmentWriterCfg
{
    bool checkIDR = false;
    FractU64 segmentDuration;
    DataItem<FractU64> subsegmentDuration;
    size_t skipSubsegments = 0;
};

struct SidxInfo
{
    ostream::pos_type position;
    size_t size;
};

class SidxWriter
{
public:
    SidxWriter() = default;
    ~SidxWriter() = default;

    void AddSubSeg(Segment)
    {
    }

    void SetFirstSubSegOffset(streampos)
    {
    }

    void AddSubSegSize(streampos)
    {
    }

    DataItem<SidxInfo> WriteSidx(ostream&, DataItem<ostream::pos_type>)
    {
        return {};
    }

    void SetOutput(ostream*)
    {
    }
};

class SegmentWriter
{
public:
    SegmentWriter(SegmentWriterCfg inCfg);

    virtual ~SegmentWriter();

    enum class Action
    {
        KeepFeeding,
        ExtractSegment
    };

    void AddTrack(TrackId trackIndex, TrackMeta inTrackMeta);

    void AddTrack(TrackMeta inTrackMeta);

    Action FeedOneFrame(TrackId trackIndex, FrameWrapper oneFrame);
    Action FeedEOS(TrackId aTrackId);

    void SetWriteSegmentHeader(bool toWriteHdr);
    void WriteInitSegment(ostream& outStr, const InitialSegment& initSegment);
    void WriteSegment(ostream& outStr, const Segment oneSeg);
    void WriteSubSegments(ostream& outStr, const list<Segment> subSegList);

    list<SegmentList> ExtractSubSegments();
    SegmentList ExtractSegments();

private:
    struct Impl;

    unique_ptr<Impl> m_impl;

    unique_ptr<SidxWriter> m_sidxWriter;

    bool m_needWriteSegmentHeader = true;
};

struct SegmentWriter::Impl
{
    struct TrackState
    {
        Impl* imple;
        TrackMeta trackMeta;
        Frames frames;

        struct SubSegment
        {
            Frames frames;
        };

        list<SubSegment> SubSegments;

        list<list<SubSegment>> FullSubSegments;

        FrameTime segDur;
        FrameTime subSegDur;

        bool isEnd = false;

        bool hasSubSeg = false;

        size_t insertSubSegNum = 0;

        FrameTime trackOffset;

        TrackState(Impl* aImpl = nullptr)
            : imple(aImpl)
        {
        }

        void FeedOneFrame(FrameWrapper oneFrame);

        void FeedEOS();

        bool IsFinished() const;

        bool IsIncomplete() const;

        bool CanTakeSegment() const;

        list<Frames> TakeSegment();
    };

    bool AllTracksFinished() const;

    bool AllTracksReadyForSegment() const;

    bool AnyTrackIncomplete() const;

    Impl* const m_imple;
    SegmentWriterCfg m_config;
    map<TrackId, TrackState> m_trackSte;
    FrameTime m_segBeginT;
    bool m_isFirstSeg = true;

    FrameTime m_offset;

    SequenceId m_seqId;

    Impl()
        : m_imple(this)
    {
    }
};

VCD_MP4_END;
#endif  // _SEGMENTERAPI_H_
