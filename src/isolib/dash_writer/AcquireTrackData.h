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
//! \file:   AcquireTrackData.h
//! \brief:  AcquireTrackData class definition and related data type
//! \detail: Define track related operation and data type
//!

#ifndef _TRACK_H_
#define _TRACK_H_

#include "Frame.h"
#include "FrameWrapper.h"
#include "../include/Index.h"
#include "Fraction.h"
#include "../atoms/DecPts.h"

using namespace std;

VCD_MP4_BEGIN

using PTSTime      = FractS64;
using SamplePTS    = std::map<PTSTime, DecodePts::SampleIndex>;

class TrackAtom;

class BoxBlockAccess;

typedef Index<uint32_t, struct TrackIdTag> TrackId;

class AcquireTrackData;

typedef list<FrameWrapper> Frames;

enum class TypeOfMedia
{
    Video,
    Audio,
    Data,
    Other
};

struct BrandSpec
{
    string majorBrand;
    uint32_t minorVersion;
    vector<string> compatibleBrands;
};

struct TrackMeta
{
    TrackId trackId;
    FractU64 timescale;
    TypeOfMedia type;
    DataItem<BrandSpec> trackType;
};

class AcquireTrackData
{
public:
    AcquireTrackData(const BoxBlockAccess& mp4Accessor, const TrackAtom& trackBox, const uint32_t timeScale);

    AcquireTrackData(const AcquireTrackData& other) = delete;
    AcquireTrackData(AcquireTrackData&& other);
    AcquireTrackData& operator=(const AcquireTrackData& other) = delete;
    AcquireTrackData& operator=(AcquireTrackData&& other) = delete;

    virtual ~AcquireTrackData();

    Frame GetFrame(size_t aFrameIndex) const;
    FrameBuf GetFramePackedData(size_t aFrameIndex) const;

    Frames GetFrames() const;

    TrackMeta GetTrackMeta() const;

private:
    struct Acquirer;

    class GetDataOfFrameFromTrack : public GetDataOfFrame
    {
    public:
        GetDataOfFrameFromTrack(shared_ptr<AcquireTrackData::Acquirer> aAcquirer, size_t aFrameIndex);

        ~GetDataOfFrameFromTrack();

        FrameBuf Get() const override;
        size_t GetDataSize() const override;

        GetDataOfFrameFromTrack* Clone() const override;

    private:
        shared_ptr<AcquireTrackData::Acquirer> m_acquirer;
        size_t mFrameIndex;
    };

    friend class GetDataOfFrameFromTrack;

    shared_ptr<Acquirer> m_acquirer;
    TrackMeta m_trackMeta;
};

VCD_MP4_END;
#endif  // _TRACK_H_
