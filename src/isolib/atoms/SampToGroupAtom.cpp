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
//! \file:   SampToGroupAtom.cpp
//! \brief:  SampToGroupAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "SampToGroupAtom.h"
#include <limits>
#include <stdexcept>

VCD_MP4_BEGIN

SampleToGroupAtom::SampleToGroupAtom()
    : FullAtom("sbgp", 0, 0)
    , m_entryCount(0)
    , m_groupTypeParameter(0)
    , m_runOfSamples()
{
}

void SampleToGroupAtom::SetEntryCount(std::uint32_t entryCount)
{
    m_entryCount = entryCount;
}

void SampleToGroupAtom::AddSampleRun(std::uint32_t sampleNum, std::uint32_t groupDescriptionIndex)
{
    SampleRun sampleRun;
    sampleRun.sampleNum           = sampleNum;
    sampleRun.groupDescriptionIndex = groupDescriptionIndex;

    m_runOfSamples.push_back(sampleRun);

    SetEntryCount(static_cast<unsigned int>(m_runOfSamples.size()));
    UpdateInternalIndex();
}

std::uint32_t SampleToGroupAtom::GetSampleGroupDescriptionIndex(const std::uint32_t sampleIndex) const
{
    if (sampleIndex >= m_sampleToGroupIndex.size())
    {
        return 0;
    }
    std::uint32_t ret = m_sampleToGroupIndex.at(sampleIndex);
    return ret;
}

std::uint32_t SampleToGroupAtom::GetSampleId(std::uint32_t groupDescriptionIndex) const
{
    for (unsigned int i = 0; i < m_sampleToGroupIndex.size(); ++i)
    {
        if (groupDescriptionIndex == m_sampleToGroupIndex.at(i))
        {
            return i;
        }
    }
    ISO_LOG(LOG_ERROR, "SampleToGroupAtom::GetSampleId: no entry for requested sample id\n");
    throw Exception();
}

unsigned int SampleToGroupAtom::GetNumberOfSamples() const
{
    unsigned int ret = static_cast<unsigned int>(m_sampleToGroupIndex.size());
    return ret;
}

void SampleToGroupAtom::UpdateInternalIndex()
{
    m_sampleToGroupIndex.clear();
    for (const auto& sampleRun : m_runOfSamples)
    {
        m_sampleToGroupIndex.insert(m_sampleToGroupIndex.end(), sampleRun.sampleNum, sampleRun.groupDescriptionIndex);
    }
}

void SampleToGroupAtom::ToStream(Stream& str)
{
    if (m_runOfSamples.size() == 0)
    {
        ISO_LOG(LOG_ERROR, "SampleToGroupAtom::ToStreamAtom: not writing an invalid Atom without entries\n");
        throw Exception();
    }

    // Write Atom headers
    WriteFullAtomHeader(str);

    str.Write32(m_groupType.GetUInt32());

    if (GetVersion() == 1)
    {
        str.Write32(m_groupTypeParameter);
    }

    str.Write32(m_entryCount);

    for (auto entry : m_runOfSamples)
    {
        str.Write32(entry.sampleNum);
        str.Write32(entry.groupDescriptionIndex);
    }

    // Update the size of the movie Atom
    UpdateSize(str);
}

void SampleToGroupAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);

    m_groupType = str.Read32();

    if (GetVersion() == 1)
    {
        m_groupTypeParameter = str.Read32();
    }

    m_entryCount = str.Read32();
    if (m_entryCount == 0)
    {
        ISO_LOG(LOG_ERROR, "Read an empty SampleToGroupAtom without entries.\n");
        throw Exception();
    }

    uint64_t sampleNum = 0;
    for (unsigned int i = 0; i < m_entryCount; ++i)
    {
        SampleRun sampleRun;
        sampleRun.sampleNum = str.Read32();
        sampleNum += sampleRun.sampleNum;
        if (sampleNum > std::numeric_limits<std::uint32_t>::max())
        {
            ISO_LOG(LOG_ERROR, "SampleToGroupAtom  sampleNum >= 2^32\n");
            throw Exception();
        }
        sampleRun.groupDescriptionIndex = str.Read32();
        m_runOfSamples.push_back(sampleRun);
    }

    UpdateInternalIndex();
}

VCD_MP4_END