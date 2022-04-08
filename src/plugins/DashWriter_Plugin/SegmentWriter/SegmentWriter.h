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
//! \file:   SegmentWriter.h
//! \brief:  OMAF segment writer class definition
//!

#ifndef _SEGMENTWRITER_H_
#define _SEGMENTWRITER_H_

#include "../DashSegmentWriterPluginAPI.h"

using namespace std;

VCD_MP4_BEGIN

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

class SegmentWriter : public SegmentWriterBase
{
public:
    SegmentWriter();

    SegmentWriter(SegmentWriterCfg inCfg);

    virtual ~SegmentWriter();

    void AddTrack(TrackId trackIndex, TrackMeta inTrackMeta);

    void AddH264VideoTrack(TrackId trackId, const TrackMeta& meta, const CodedMeta& inMetaData);

    void AddH265VideoTrack(TrackId trackId,
        bool isOMAF,
        bool isPackedSubPic,
        VideoFramePackingType packingType,
        const TrackMeta& meta,
        const CodedMeta& inMetaData);

    void AddH265ExtractorTrack(TrackId trackId,
        bool isOMAF,
        bool isPackedSubPic,
        VideoFramePackingType packingType,
        const TrackMeta& meta,
        const std::map<std::string, std::set<TrackId>>& references,
        const CodedMeta& inMetaData);

    void AddAACTrack(TrackId trackId,
        const TrackMeta& meta,
        bool  isOMAF,
        const CodedMeta& inMetaData);

    void WriteInitSegment(ostringstream& outStr, const bool isFraged);

    void GenPackedExtractors(TrackId trackId, std::vector<uint8_t>& extractorNALUs, Extractor *extractor);

    void Feed(TrackId trackId, const CodedMeta& codedFrameMeta, uint8_t *data,
        int32_t dataSize, const FrameCts& compositionTime);

    Action FeedEOS(TrackId aTrackId);

    void SetWriteSegmentHeader(bool toWriteHdr);

    void WriteSegments(std::ostringstream &frameString,
        uint64_t *segNum, char segName[1024], char *baseName, uint64_t *segSize);

private:
    void AddTrack(TrackMeta inTrackMeta);

    void FillOmafStructures(TrackId inTrackId,
        bool isOMAF,
        bool isPackedSubPic,
        VideoFramePackingType packingType,
        const CodedMeta& inMetaData,
        HevcVideoSampleEntry& hevcEntry,
        TrackMeta& inMeta);

    InitialSegment MakeInitSegment(const bool isFraged);

    InitialSegment GenInitSegment(const TrackDescriptionsMap& inTrackDes,
                                const MovieDescription& inMovieDes,
                                const bool isFraged);

    bool DetectNonRefFrame(uint8_t *frameData);

    Action FeedOneFrame(TrackId trackIndex, FrameWrapper oneFrame);

    void WriteSubSegments(ostream& outStr, const list<Segment> subSegList);

    list<SegmentList> ExtractSubSegments();

    TrackDescriptionsMap m_trackDescriptions;

    std::string          m_omafVideoTrackBrand = "";

    std::string          m_omafAudioTrackBrand = "";

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

//!
//! \class AcquireVideoFrameData
//! \brief Define the operation of acquiring video coded data
//!

class AcquireVideoFrameData : public GetDataOfFrame
{
public:

    //!
    //! \brief  Copy Constructor
    //!
    //! \param  [in] data
    //!         the pointer to the nalu data
    //! \param  [in] size
    //!         size of the nalu data
    //!
    AcquireVideoFrameData(uint8_t *data, uint64_t size);

    //!
    //! \brief  Destructor
    //!
    ~AcquireVideoFrameData() override;

    //!
    //! \brief  Get the coded data
    //!
    //! \return FrameBuf
    //!         the FrameBuf which includes the coded data
    //!
    FrameBuf Get() const override;

    //!
    //! \brief  Get the size of coded data
    //!
    //! \return size_t
    //!         the size of coded data
    //!
    size_t GetDataSize() const override;

    //!
    //! \brief  Clone one AcquireVideoFrameData object
    //!
    //! \return AcquireVideoFrameData*
    //!         the pointer to the cloned AcquireVideoFrameData object
    //!
    AcquireVideoFrameData* Clone() const override;

private:
    uint8_t  *m_data;        //!< the pointer to the nalu data
    uint64_t m_dataSize;     //!< nalu data size
};

extern "C" SegmentWriterBase* Create(SegmentWriterCfg inCfg);
extern "C" void Destroy(SegmentWriterBase* segWriter);

VCD_MP4_END;
#endif  // _SEGMENTWRITER_H_
