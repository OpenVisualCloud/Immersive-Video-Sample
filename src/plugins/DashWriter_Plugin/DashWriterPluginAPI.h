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
//! \file:   DashWriterPluginAPI.h
//! \brief:  Dash writer plugin interfaces
//!
//! Created on August 5, 2021, 6:04 AM
//!

#ifndef _DASHWRITERPLUGINAPI_H_
#define _DASHWRITERPLUGINAPI_H_

#include "MediaData.h"

using namespace std;

VCD_MP4_BEGIN

class SegmentWriterBase
{
public:
    SegmentWriterBase() {};

    SegmentWriterBase(SegmentWriterCfg inCfg) {};

    virtual ~SegmentWriterBase() {};

    virtual void AddTrack(TrackId trackIndex, TrackMeta inTrackMeta) = 0;

    virtual void AddH264VideoTrack(TrackId trackId, const TrackMeta& meta, const CodedMeta& inMetaData) = 0;

    virtual void AddH265VideoTrack(TrackId trackId,
        bool isOMAF,
        bool isPackedSubPic,
        VideoFramePackingType packingType,
        const TrackMeta& meta,
        const CodedMeta& inMetaData) = 0;

    virtual void AddH265ExtractorTrack(TrackId trackId,
        bool isOMAF,
        bool isPackedSubPic,
        VideoFramePackingType packingType,
        const TrackMeta& meta,
        const std::map<std::string, std::set<TrackId>>& references,
        const CodedMeta& inMetaData) = 0;

    virtual void AddAACTrack(TrackId trackId,
        const TrackMeta& meta,
        bool  isOMAF,
        const CodedMeta& inMetaData) = 0;

    virtual void WriteInitSegment(ostream& outStr, const bool isFraged) = 0;

    virtual void GenPackedExtractors(TrackId trackId,
        std::vector<uint8_t>& extractorNALUs,
        Extractor *extractor) = 0;

    virtual void Feed(TrackId trackId, const CodedMeta& codedFrameMeta, uint8_t *data,
        int32_t dataSize, const FrameCts& compositionTime) = 0;

    virtual Action FeedEOS(TrackId aTrackId) = 0;

    virtual void SetWriteSegmentHeader(bool toWriteHdr) = 0;

    virtual void WriteSegments(std::ostringstream &frameString,
        uint64_t *segNum, char segName[1024], char *baseName) = 0;

protected:
};

typedef SegmentWriterBase* CreateSegmentWriter(SegmentWriterCfg);
typedef void DestroySegmentWriter(SegmentWriterBase* segWriter);

VCD_MP4_END;
#endif /* _DASHWRITERPLUGINAPI_H_ */
