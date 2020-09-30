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
//! \file:   SampGroupDescAtom.cpp
//! \brief:  SampGroupDescAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "SampGroupDescAtom.h"

#include "Stream.h"

#include <stdexcept>

VCD_MP4_BEGIN

SampleGroupDescriptionAtom::SampleGroupDescriptionAtom()
    : FullAtom("sgpd", 0, 0)
    , m_groupType()
    , m_defaultLength(0)
    , m_sampleGroupEntry()
{
}

void SampleGroupDescriptionAtom::AddEntry(UniquePtr<SampleGroupEntry> sampleGroupEntry)
{
    m_sampleGroupEntry.push_back(std::move(sampleGroupEntry));
}

const SampleGroupEntry* SampleGroupDescriptionAtom::GetEntry(std::uint32_t index) const
{
    const SampleGroupEntry* ret = m_sampleGroupEntry.at(index - 1).get();
    return ret;
}

std::uint32_t SampleGroupDescriptionAtom::GetEntryIndexOfSampleId(const std::uint32_t sampleId) const
{
    uint32_t index = 1;
    for (const auto& entry : m_sampleGroupEntry)
    {
        DirectReferenceSampleListEntry* drsle = dynamic_cast<DirectReferenceSampleListEntry*>(entry.get());
        if ((drsle != nullptr) && (drsle->GetSampleId() == sampleId))
        {
            return index;
        }
        ++index;
    }
    ISO_LOG(LOG_ERROR, "SampleGroupDescriptionAtom::GetEntryIndexOfSampleId: no entry for sampleId found.\n");
    throw Exception();
}

void SampleGroupDescriptionAtom::ToStream(Stream& str)
{
    if (m_sampleGroupEntry.size() == 0)
    {
        ISO_LOG(LOG_ERROR, "SampleGroupDescriptionAtom::ToStreamAtom: not writing an invalid Atom without entries\n");
        throw Exception();
    }

    // Write Atom headers
    WriteFullAtomHeader(str);

    str.Write32(m_groupType.GetUInt32());
    uint8_t pVersion = GetVersion();
    if (pVersion == 1)
    {
        str.Write32(m_defaultLength);
    }

    str.Write32(static_cast<unsigned int>(m_sampleGroupEntry.size()));

    for (auto& entry : m_sampleGroupEntry)
    {
        if (pVersion == 1 && m_defaultLength == 0)
        {
            str.Write32(entry->GetSize());
        }
        entry->WriteEntry(str);
    }

    // Update the size of the movie Atom
    UpdateSize(str);
}

void SampleGroupDescriptionAtom::FromStream(Stream& str)
{
    //  First parse the Atom header
    ParseFullAtomHeader(str);

    m_groupType = str.Read32();

    if (GetVersion() == 1)
    {
        m_defaultLength = str.Read32();
    }

    const uint32_t entryCount = str.Read32();

    for (unsigned int i = 0; i < entryCount; ++i)
    {
        uint32_t desLen = m_defaultLength;
        if (GetVersion() == 1 && m_defaultLength == 0)
        {
            desLen = str.Read32();
        }

        Stream subStr;
        str.Extract(str.GetPos(), str.GetPos() + desLen,
                       subStr);  // extract "sub-bitstream" for entry
        str.SkipBytes(desLen);

        if (m_groupType == "refs")
        {
            UniquePtr<SampleGroupEntry> directReferenceSampleListEntry(new DirectReferenceSampleListEntry());
            directReferenceSampleListEntry->ParseEntry(subStr);
            m_sampleGroupEntry.push_back(std::move(directReferenceSampleListEntry));
        }
        else
        {
            ISO_LOG(LOG_WARNING, "Skipping an entry of SampleGroupDescriptionAtom of an unknown grouping type '%s'\n", m_groupType.GetString().c_str());
        }
    }
}

VCD_MP4_END
