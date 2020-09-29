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
//! \file:   TimeToSampAtom.cpp
//! \brief:  TimeToSampAtom class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!
#include "TimeToSampAtom.h"


#include <limits>
#include <stdexcept>

VCD_MP4_BEGIN

TimeToSampleAtom::TimeToSampleAtom()
    : FullAtom("stts", 0, 0)
{
}

std::vector<std::uint32_t> TimeToSampleAtom::GetSampleTimes() const
{
    std::vector<std::uint32_t> pTimes;
    uint32_t time = 0;
    for (const auto& entry : m_entryVersion0)
    {
        for (unsigned int i = 0; i < entry.m_sampleNum; ++i)
        {
            pTimes.push_back(time);
            time += entry.m_sampleDelta;
        }
    }

    return pTimes;
}

std::vector<std::uint32_t> TimeToSampleAtom::GetSampleDeltas() const
{
    std::vector<std::uint32_t> pDeltas;

    if (m_entryVersion0.size())
    {
        pDeltas.reserve(m_entryVersion0.at(0).m_sampleNum);
        for (const auto& entry : m_entryVersion0)
        {
            for (unsigned int i = 0; i < entry.m_sampleNum; ++i)
            {
                pDeltas.push_back(entry.m_sampleDelta);
            }
        }
    }

    return pDeltas;
}

std::uint32_t TimeToSampleAtom::GetSampleNum() const
{
    std::uint64_t sampleNum = 0;

    if (m_entryVersion0.size())
    {
        for (const auto& entry : m_entryVersion0)
        {
            sampleNum += entry.m_sampleNum;
            if (sampleNum > std::numeric_limits<std::uint32_t>::max())
            {
                ISO_LOG(LOG_ERROR, "TimeToSampleAtom::sampleNum >= 2^32\n");
                throw Exception();
            }
        }
    }
    std::uint32_t ret = std::uint32_t(sampleNum);
    return ret;
}

TimeToSampleAtom::EntryVersion0& TimeToSampleAtom::GetDecodeDeltaEntry()
{
    m_entryVersion0.resize(m_entryVersion0.size() + 1);
    EntryVersion0& ret = m_entryVersion0.back();
    return ret;
}

void TimeToSampleAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);
    str.Write32(static_cast<unsigned int>(m_entryVersion0.size()));
    for (auto entry : m_entryVersion0)
    {
        str.Write32(entry.m_sampleNum);
        str.Write32(entry.m_sampleDelta);
    }

    UpdateSize(str);
}

void TimeToSampleAtom::FromStream(Stream& str)
{
    //  First parse the Atom header
    ParseFullAtomHeader(str);

    std::uint32_t pCnt = str.Read32();
    for (uint32_t i = 0; i < pCnt; ++i)
    {
        EntryVersion0 pVersion;
        pVersion.m_sampleNum = str.Read32();
        pVersion.m_sampleDelta = str.Read32();
        m_entryVersion0.push_back(pVersion);
    }
}

void TimeToSampleAtom::AddSampleDelta(std::uint32_t delta)
{
    if (!m_entryVersion0.size() || delta != m_entryVersion0.back().m_sampleDelta)
    {
        m_entryVersion0.push_back({1, delta});
    }
    else
    {
        ++m_entryVersion0.back().m_sampleNum;
    }
}

VCD_MP4_END