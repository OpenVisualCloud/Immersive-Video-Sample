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
//! \file:   DecPts.cpp
//! \brief:  DecPts class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!
#include "DecPts.h"
#include <algorithm>
#include <iterator>
#include "CompOffsetAtom.h"
#include "CompToDecAtom.h"
#include "EditAtom.h"
#include "TimeToSampAtom.h"

VCD_MP4_BEGIN

DecodePts::DecodePts()
    : m_editListAtom(nullptr)
    , m_movieTimescale(0)
    , m_mediaTimescale(0)
    , m_timeToSampAtom(nullptr)
    , m_compOffsetAtom(nullptr)
    , m_compToDecodeAtom(nullptr)
    , m_trackRunAtom(nullptr)
    , m_movieOffset(0)
{
}

template <typename T>
void DecodePts::SetEdit(T& entry)
{
    if (entry.m_mediaTime == -1)
    {
        SetEmptyEdit(entry);
    }
    if (entry.m_mediaRateInt == 0)
    {
        SetDwellEdit(entry);
    }
    if (entry.m_mediaTime >= 0 && entry.m_mediaRateInt != 0)
    {
        SetShiftEdit(entry);
    }
}

template <typename T>
std::uint64_t DecodePts::FromMovieToMediaTS(T movieTS) const
{
    if (m_movieTimescale)
    {
        return static_cast<std::uint64_t>(movieTS) * m_mediaTimescale / m_movieTimescale;
    }
    else
    {
        return static_cast<std::uint64_t>(movieTS);
    }
}

template <typename T>
void DecodePts::SetEmptyEdit(T& entry)
{
    m_movieOffset += FromMovieToMediaTS(entry.m_segDuration);
}

template <typename T>
void DecodePts::SetDwellEdit(T& entry)
{
    std::pair<PMap::iterator, PMap::iterator> iters;
    iters = m_mediaPts.equal_range(entry.m_mediaTime);

    if (iters.first->first == iters.second->first)
    {
        m_moviePts.insert(std::make_pair(m_movieOffset, std::prev(iters.first)->second));
        m_movieOffset += FromMovieToMediaTS(entry.m_segDuration);
    }
    else
    {
        m_moviePts.insert(std::make_pair(m_movieOffset, iters.first->second));
        m_movieOffset += FromMovieToMediaTS(entry.m_segDuration);
    }
}

std::uint64_t DecodePts::LastSampleDuration() const
{
    std::uint64_t lastSampleDuration = 0;
    if (m_trackRunAtom)
    {
        auto& pSample = m_trackRunAtom->GetSampleDetails();
        if (pSample.size())
        {
            auto& last = *pSample.rbegin();
            if (m_trackRunAtom->GetVersion() == 0)
            {
                lastSampleDuration = last.version0.pDuration;
            }
            else
            {
                lastSampleDuration = last.version1.pDuration;
            }
        }
    }
    else
    {
        const auto& sampleDeltas = m_timeToSampAtom->GetSampleDeltas();
        if (sampleDeltas.size())
        {
            lastSampleDuration = *sampleDeltas.rbegin();
        }
    }
    return lastSampleDuration;
}

template <typename T>
void DecodePts::SetShiftEdit(T& entry)
{
    std::int64_t endTime(INT64_MAX);

    if (entry.m_segDuration != 0)
    {
        endTime = static_cast<std::int64_t>(static_cast<std::uint64_t>(entry.m_mediaTime) +
                                                   (FromMovieToMediaTS(entry.m_segDuration)));
    }

    if (m_mediaPts.size())
    {
        m_movieOffset += static_cast<std::uint64_t>(m_mediaPts.begin()->first + m_mediaOffset - entry.m_mediaTime);
    }

    for (auto it = m_mediaPts.cbegin(); it != m_mediaPts.cend(); ++it)
    {
        if (it->first + m_mediaOffset >= static_cast<std::int64_t>(entry.m_mediaTime) &&
            it->first + m_mediaOffset < endTime)
        {
            if (it != m_mediaPts.cbegin() &&
                std::prev(it)->first + m_mediaOffset < static_cast<std::int64_t>(entry.m_mediaTime) &&
                it->first + m_mediaOffset != static_cast<std::int64_t>(entry.m_mediaTime))
            {
                m_moviePts.insert(std::make_pair(m_movieOffset, std::prev(it)->second));
                m_movieOffset += static_cast<std::uint64_t>(
                    it->first - (std::prev(it)->first + (entry.m_mediaTime - std::prev(it)->first)));
            }

            m_moviePts.insert(std::make_pair(m_movieOffset, it->second));
        }
        if (std::next(it) != m_mediaPts.cend())
        {
            m_movieOffset += static_cast<std::uint64_t>(std::next(it)->first - it->first);
        }
        else
        {
            m_movieOffset += LastSampleDuration();
        }
    }

    if (m_mediaPts.size() && entry.m_segDuration != 0)
    {
        m_movieOffset -= static_cast<std::uint64_t>(m_mediaPts.rbegin()->first + m_mediaOffset +
                                                   static_cast<std::int64_t>(LastSampleDuration()) - endTime);
    }
}

void DecodePts::SetAtom(const TimeToSampleAtom* atom)
{
    m_timeToSampAtom = atom;
}

void DecodePts::SetAtom(const CompositionOffsetAtom* atom)
{
    m_compOffsetAtom = atom;
}

void DecodePts::SetAtom(const CompositionToDecodeAtom* atom)
{
    m_compToDecodeAtom = atom;
}

void DecodePts::SetAtom(const EditListAtom* atom, std::uint32_t movieTimescale, std::uint32_t mediaTimescale)
{
    m_editListAtom    = atom;
    m_movieTimescale = movieTimescale;
    m_mediaTimescale = mediaTimescale;
}

void DecodePts::SetAtom(const TrackRunAtom* atom)
{
    m_trackRunAtom = atom;
}

void DecodePts::SetEditList()
{
    if (m_mediaPts.size())
    {
        std::uint32_t version = m_editListAtom->GetVersion();
        for (std::uint32_t i = 0; i < m_editListAtom->numEntry(); i++)
        {
            if (version == 0)
            {
                SetEdit(m_editListAtom->GetEntry<EditListAtom::EntryVersion0>(i));
            }
            else if (version == 1)
            {
                SetEdit(m_editListAtom->GetEntry<EditListAtom::EntryVersion1>(i));
            }
        }
    }
}

bool DecodePts::Unravel()
{
    bool ret = true;

    std::vector<std::uint32_t> pDts;
    pDts = m_timeToSampAtom->GetSampleTimes();

    std::vector<std::int64_t> mediaPtsTS;
    if (m_compOffsetAtom != nullptr)
    {
        std::vector<std::int32_t> DeltaPts;
        DeltaPts = m_compOffsetAtom->GetSampleCompositionOffsets();

        if (DeltaPts.size() == pDts.size())
        {
        std::transform(pDts.begin(), pDts.end(), DeltaPts.begin(), std::back_inserter(mediaPtsTS),
                       [](std::uint64_t theMediaDts, std::int32_t thePtsDelta) {
                           return std::uint64_t(theMediaDts + (std::uint64_t)(thePtsDelta));
                       });
        }
    }
    else
    {
        std::copy(pDts.begin(), pDts.end(), std::back_inserter(mediaPtsTS));
    }

    if (ret)
    {
        std::uint64_t sampleId = 0;
        for (auto pts : mediaPtsTS)
        {
            m_mediaPts.insert(std::make_pair(pts, sampleId++));
        }

        if (m_editListAtom != nullptr)
        {
            SetEditList();
        }
        else
        {
            m_moviePts = m_mediaPts;

            if (m_moviePts.size() > 0)
            {
                auto last    = std::prev(m_moviePts.end(), 1);
                m_movieOffset = static_cast<std::uint64_t>(last->first) + LastSampleDuration();
            }
            else
            {
                m_movieOffset = 0;
            }
        }
    }

    return ret;
}

void DecodePts::UnravelTrackRun()
{
    std::vector<std::uint32_t> pDts;
    uint32_t time = 0;
    bool timeOffsetFlag = false;
    std::vector<std::int32_t> DeltaPts;

    if ((m_trackRunAtom->GetFlags() & TrackRunAtom::pSampleCompTimeOffsets) != 0)
    {
        timeOffsetFlag = true;
    }

    const auto& pSample = m_trackRunAtom->GetSampleDetails();
    pDts.reserve(pSample.size());
    if (timeOffsetFlag)
    {
        DeltaPts.reserve(pSample.size());
    }
    for (const auto& sample : pSample)
    {
        pDts.push_back(time);
        time += sample.version0.pDuration;

        if (timeOffsetFlag)
        {
            if (m_trackRunAtom->GetVersion() == 0)
            {
                DeltaPts.push_back(static_cast<std::int32_t>(sample.version0.pCompTimeOffset));
            }
            else
            {
                DeltaPts.push_back(sample.version1.pCompTimeOffset);
            }
        }
    }

    std::vector<std::int64_t> pPts;
    pPts.reserve(pDts.size());
    if (timeOffsetFlag)
    {
        std::transform(pDts.begin(), pDts.end(), DeltaPts.begin(), std::back_inserter(pPts),
                       [](std::uint32_t tmpDts, std::int32_t thePtsDelta) {
                           return std::uint32_t(std::int32_t(tmpDts) + thePtsDelta);
                       });
    }
    else
    {
        std::copy(pDts.begin(), pDts.end(), std::back_inserter(pPts));
    }

    std::uint64_t sampleId = 0;
    // m_mediaPts.reserve(m_mediaPts.size() + pPts.size());
    for (auto pts : pPts)
    {
        m_mediaPts.insert(std::make_pair(pts, sampleId++));
    }
}

DecodePts::PMap DecodePts::GetTime(const std::uint32_t ts) const
{
    if (ts == 0)
    {
        ISO_LOG(LOG_ERROR, "timeScale == 0\n");
        throw Exception();
    }
    PMap pMap;
    for (const auto& entry : m_moviePts)
    {
        int64_t p1 = (entry.first * 1000) / ts;
        uint64_t p2 = entry.second;
        pMap.insert(std::make_pair(p1, p2));
    }
    return pMap;
}

DecodePts::PMapTS DecodePts::GetTimeTS() const
{
    PMapTS ts;
    for (const auto& entry : m_moviePts)
    {
        ts.insert(std::make_pair(entry.first, entry.second));
    }
    return ts;
}

void DecodePts::GetTimeTrackRun(const std::uint32_t ts, PMap& oldPMap) const
{
    std::uint64_t idx = 0;
    if (oldPMap.size())
    {
        idx = oldPMap.rbegin()->second + 1;
    }
    for (const auto& entry : m_moviePts)
    {
        int64_t p1 = (entry.first * 1000) / ts;
        uint64_t p2 = idx + entry.second;
        oldPMap.insert(std::make_pair(p1, p2));
    }
    return;
}

void DecodePts::GetTimeTrackRunTS(PMapTS& oldPMapTS) const
{
    std::uint64_t idx = 0;
    if (oldPMapTS.size())
    {
        idx = oldPMapTS.rbegin()->second + 1;
    }
    for (const auto& entry : m_moviePts)
    {
        int64_t p1 = entry.first;
        uint64_t p2 = idx + entry.second;
        oldPMapTS.insert(std::make_pair(p1, p2));
    }
    return;
}

std::uint64_t DecodePts::GetSpan() const
{
    return m_movieOffset;
}


void DecodePts::SetLocalTime(std::uint64_t pOffset)
{
    if (m_editListAtom != nullptr)
    {
        m_mediaOffset = static_cast<std::int64_t>(pOffset);
        SetEditList();
    }
    else
    {
        for (const auto& entry : m_mediaPts)
        {
            m_moviePts.insert(std::make_pair(PresentTimeTS(pOffset) + entry.first, entry.second));
        }

        if (m_moviePts.size() > 0)
        {
            auto last = std::prev(m_moviePts.end(), 1);
            m_movieOffset = static_cast<std::uint64_t>(last->first) + LastSampleDuration();
        }
        else
        {
            m_movieOffset = 0;
        }
    }
}

VCD_MP4_END
