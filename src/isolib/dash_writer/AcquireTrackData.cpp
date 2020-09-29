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
//! \file:   AcquireTrackData.cpp
//! \brief:  AcquireTrackData class implementation
//!

#include <cassert>
#include <map>

#include "FrameWrapper.h"
#include "AcquireTrackData.h"
#include "MovieAtom.h"
#include "BoxBlockAccess.h"
#include "TrackAtom.h"
#include "Utils.h"

using namespace std;

VCD_MP4_BEGIN

SamplePTS GetPTS(const DecodePts& aDecodePts, uint32_t aTimeScale)
{
    SamplePTS pts;
    for (auto timeSample : aDecodePts.GetTimeTS())
    {
        pts.insert(make_pair(FractS64{timeSample.first, static_cast<int64_t>(aTimeScale)}, timeSample.second));
    }
    return pts;
}

struct AcquireTrackData::Acquirer
{
    const BoxBlockAccess& mp4;
    vector<FrameInfo> frameInfo;
    vector<DataBlock> frameBlock;
    FrameBuf GetFramePackedData(size_t aFrameIndex) const;
    size_t GetFramePackedSize(size_t aFrameIndex) const;
};

AcquireTrackData::GetDataOfFrameFromTrack::GetDataOfFrameFromTrack(shared_ptr<AcquireTrackData::Acquirer> aAcquirer,
                                                            size_t aFrameIndex)
    : m_acquirer(aAcquirer)
    , mFrameIndex(aFrameIndex)
{
}

AcquireTrackData::GetDataOfFrameFromTrack::~GetDataOfFrameFromTrack()
{
}

FrameBuf AcquireTrackData::GetDataOfFrameFromTrack::Get() const
{
    return m_acquirer->GetFramePackedData(mFrameIndex);
}

size_t AcquireTrackData::GetDataOfFrameFromTrack::GetDataSize() const
{
    return m_acquirer->GetFramePackedSize(mFrameIndex);
}

AcquireTrackData::GetDataOfFrameFromTrack* AcquireTrackData::GetDataOfFrameFromTrack::Clone() const
{
    return new GetDataOfFrameFromTrack(m_acquirer, mFrameIndex);
}

FrameBuf AcquireTrackData::Acquirer::GetFramePackedData(size_t aFrameIndex) const
{
    if (aFrameIndex >= frameBlock.size())
    {
        ISO_LOG(LOG_ERROR, "Frame index exceeds size !\n");
        throw exception();
    }
    const auto storage = mp4.GetData(frameBlock.at(aFrameIndex));
    FrameBuf gotFrame = FrameBuf(storage.begin(), storage.end());
    return gotFrame;
}

size_t AcquireTrackData::Acquirer::GetFramePackedSize(size_t aFrameIndex) const
{
    if (aFrameIndex >= frameBlock.size())
    {
        ISO_LOG(LOG_ERROR, "Frame index exceeds size !\n");
        throw exception();
    }
    size_t packedSize = frameBlock.at(aFrameIndex).size;
    return packedSize;
}

AcquireTrackData::AcquireTrackData(AcquireTrackData&& aOther)
    : m_acquirer(move(aOther.m_acquirer))
    , m_trackMeta(move(aOther.m_trackMeta))
{
}

AcquireTrackData::AcquireTrackData(
    const BoxBlockAccess& mp4Accessor,
    const TrackAtom& trackBox,
    const uint32_t timeScale)
    : m_acquirer(new Acquirer{mp4Accessor, {}, {}})
{
    const std::string& handlerName              = trackBox.GetMediaAtom().GetHandlerAtom().GetName();
    const MediaHeaderAtom& mdhdBox          = trackBox.GetMediaAtom().GetMediaHeaderAtom();
    const SampleTableAtom& stblBox          = trackBox.GetMediaAtom().GetMediaInformationAtom().GetSampleTableAtom();
    const TrackHeaderAtom& trackHeaderBox   = trackBox.GetTrackHeaderAtom();
    const TimeToSampleAtom& timeToSampleBox = stblBox.GetTimeToSampleAtom();
    shared_ptr<const CompositionOffsetAtom> compositionOffsetBox     = stblBox.GetCompositionOffsetAtom();
    shared_ptr<const CompositionToDecodeAtom> compositionToDecodeBox = stblBox.GetCompositionToDecodeAtom();
    shared_ptr<const EditAtom> editBox                               = trackBox.GetEditAtom();
    const EditListAtom* editListBox           = editBox ? editBox->GetEditListAtom() : nullptr;
    const SampleToChunkAtom& sampleToChunkBox = stblBox.GetSampleToChunkAtom();
    const auto& chunkOffsets                 = stblBox.GetChunkOffsetAtom().GetChunkOffsets();
    const SampleSizeAtom& sampleSizeBox       = stblBox.GetSampleSizeAtom();
    auto syncSampleBox                       = stblBox.GetSyncSampleAtom();
    const set<uint32_t> syncSamples =
        syncSampleBox
            ? ContMapSet([](uint32_t x) { return x - 1; }, syncSampleBox->GetSyncSampleIds())
            : set<uint32_t>();
    auto sampleSizes = sampleSizeBox.GetEntrySize();

    m_trackMeta.trackId   = trackHeaderBox.GetTrackID();
    m_trackMeta.timescale = FractU64(1, mdhdBox.GetTimeScale());
    m_trackMeta.type      = handlerName == "VideoHandler"
                          ? TypeOfMedia::Video
                          : handlerName == "SoundHandler"
                                ? TypeOfMedia::Audio
                                : handlerName == "DataHandler" ? TypeOfMedia::Data : TypeOfMedia::Other;

    DecodePts decodePts;
    decodePts.SetAtom(&timeToSampleBox);
    if (compositionOffsetBox)
    {
        decodePts.SetAtom(&*compositionOffsetBox);
    }
    if (compositionToDecodeBox)
    {
        decodePts.SetAtom(&*compositionToDecodeBox);
    }
    if (editListBox)
    {
        decodePts.SetAtom(editListBox, timeScale, mdhdBox.GetTimeScale());
    }

    decodePts.Unravel();

    SamplePTS timeToSample = GetPTS(decodePts, (uint32_t) m_trackMeta.timescale.m_den);
    multimap<DecodePts::SampleIndex, PTSTime> sampleToTimes;
    for (auto tTs : timeToSample)
    {
        sampleToTimes.insert(make_pair(tTs.second, tTs.first));
    }

    uint32_t chunkIndex     = 0;
    uint32_t prevChunkIndex = 0;

    uint64_t dataOffset = 0;
    for (uint32_t sampleIndex = 0; sampleIndex < sampleSizeBox.GetSampleNum(); ++sampleIndex)
    {
        auto ctsRange = sampleToTimes.equal_range(sampleIndex);
        FrameCts cts;
        auto ctsIt = ctsRange.first;
        while (ctsIt != ctsRange.second)
        {
            cts.push_back(ctsIt->second);
            ++ctsIt;
        }
        if (cts.size() != 1)
        {
            ISO_LOG(LOG_ERROR, "Invalid CTS size !\n");
            throw exception();
        }
        if (!sampleToChunkBox.GetSampleChunkIndex(sampleIndex, chunkIndex))
        {
            ISO_LOG(LOG_ERROR, "Failed to get sample chunk index !\n");
            throw exception();
        }
        auto followingCts = timeToSample.find(*cts.begin());
        if (followingCts == timeToSample.end())
        {
            ISO_LOG(LOG_ERROR, "Failed to find the beginning CTS !\n");
            throw exception();
        }
        ++followingCts;
        FrameDuration frameDur;
        if (followingCts != timeToSample.end())
        {
            frameDur = followingCts->first.cast<FrameDuration>() - cts.begin()->cast<FrameDuration>();
        }
        else
        {
            frameDur = FrameDuration();
        }

        if (chunkIndex != prevChunkIndex)
        {
            dataOffset     = chunkOffsets.at(chunkIndex - 1);
            prevChunkIndex = chunkIndex;
        }
        bool idrFrame = syncSamples.count(sampleIndex) != 0;
        SampleFlags sampleFlags{};
        sampleFlags.flags.sample_is_non_sync_sample = !idrFrame;

        FrameInfo frameInfo{cts, frameDur, idrFrame, {sampleFlags.flagsAsUInt}, {}};
        dataOffset += sampleSizes[sampleIndex];
        m_acquirer->frameInfo.push_back(frameInfo);
        m_acquirer->frameBlock.push_back(DataBlock(dataOffset, sampleSizes[sampleIndex]));
    }
}

AcquireTrackData::~AcquireTrackData()
{
}

FrameBuf AcquireTrackData::GetFramePackedData(size_t aFrameIndex) const
{
    assert(aFrameIndex < m_acquirer->frameBlock.size());
    const auto frameBlock = m_acquirer->frameBlock.at(aFrameIndex);
    const auto storage    = m_acquirer->mp4.GetData(frameBlock);
    return FrameBuf(storage.begin(), storage.end());
}

Frame AcquireTrackData::GetFrame(size_t aFrameIndex) const
{
    assert(aFrameIndex < m_acquirer->frameInfo.size());
    const auto frameInfo  = m_acquirer->frameInfo.at(aFrameIndex);
    const auto frameBlock = m_acquirer->frameBlock.at(aFrameIndex);
    const auto storage    = m_acquirer->mp4.GetData(frameBlock);
    return Frame{frameInfo, FrameBuf(storage.begin(), storage.end())};
}

Frames AcquireTrackData::GetFrames() const
{
    Frames frames;
    for (size_t frameIndex = 0; frameIndex < m_acquirer->frameInfo.size(); ++frameIndex)
    {
        const auto frameInfo = m_acquirer->frameInfo.at(frameIndex);
        auto acquire =
            unique_ptr<GetDataOfFrameFromTrack>(new GetDataOfFrameFromTrack(m_acquirer, frameIndex));
        frames.push_back(FrameWrapper{move(acquire), frameInfo});
    }
    return frames;
}

TrackMeta AcquireTrackData::GetTrackMeta() const
{
    return m_trackMeta;
}

VCD_MP4_END
