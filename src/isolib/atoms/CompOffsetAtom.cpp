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
//! \file:   CompOffsetAtom.cpp
//! \brief:  CompOffsetAtom class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!
#include "CompOffsetAtom.h"
#include <limits>
#include "Stream.h"

#include <stdexcept>

VCD_MP4_BEGIN

CompositionOffsetAtom::CompositionOffsetAtom()
    : FullAtom("ctts", 0, 0)
{
}

void CompositionOffsetAtom::AddEntryVersion0(const EntryVersion0& entry)
{
    if (m_entryVersion1.size() != 0 || GetVersion() != 0)
    {
        ISO_LOG(LOG_ERROR, "Invalid attempt to add version0 CompositionOffsetAtom entries.""Invalid attempt to add version0 CompositionOffsetAtom entries.\n");
        throw Exception();
    }
    m_entryVersion0.push_back(entry);
}

void CompositionOffsetAtom::AddEntryVersion1(const EntryVersion1& entry)
{
    if (m_entryVersion0.size() != 0 || GetVersion() != 1)
    {
        ISO_LOG(LOG_ERROR, "Invalid attempt to add version1 CompositionOffsetAtom entries.""Invalid attempt to add version0 CompositionOffsetAtom entries.\n");
        throw Exception();
    }
    m_entryVersion1.push_back(entry);
}

uint32_t CompositionOffsetAtom::GetSampleNum()
{
    uint64_t sampleNum = 0;
    if (GetVersion() == 0)
    {
        for (const auto& entry : m_entryVersion0)
        {
            sampleNum += static_cast<uint64_t>(entry.m_sampleNum);
            if (sampleNum > std::numeric_limits<std::uint32_t>::max())
            {
                ISO_LOG(LOG_ERROR, "CompositionOffsetAtom::GetSampleNum >= 2^32\n");
                throw Exception();
            }
        }
    }
    else if (GetVersion() == 1)
    {
        for (const auto& entry : m_entryVersion1)
        {
            sampleNum += static_cast<uint64_t>(entry.m_sampleNum);
            if (sampleNum > std::numeric_limits<std::uint32_t>::max())
            {
                ISO_LOG(LOG_ERROR, "CompositionOffsetAtom::GetSampleNum >= 2^32\n");
                throw Exception();
            }
        }
    }
    return static_cast<uint32_t>(sampleNum);
}

std::vector<int> CompositionOffsetAtom::GetSampleCompositionOffsets() const
{
    std::vector<int> offsets;
    if (GetVersion() == 0)
    {
        for (const auto& entry : m_entryVersion0)
        {
            for (unsigned int i = 0; i < entry.m_sampleNum; ++i)
            {
                offsets.push_back(static_cast<int>(entry.m_sampleOffset));
            }
        }
    }
    else if (GetVersion() == 1)
    {
        for (const auto& entry : m_entryVersion1)
        {
            for (unsigned int i = 0; i < entry.m_sampleNum; ++i)
            {
                offsets.push_back(entry.m_sampleOffset);
            }
        }
    }

    return offsets;
}

void CompositionOffsetAtom::ToStream(Stream& str)
{
    // Write Atom headers
    WriteFullAtomHeader(str);

    if (m_entryVersion0.empty() == false)
    {
        str.Write32(static_cast<std::uint32_t>(m_entryVersion0.size()));
        for (const auto& entry : m_entryVersion0)
        {
            str.Write32(entry.m_sampleNum);
            str.Write32(entry.m_sampleOffset);
        }
    }
    else if (m_entryVersion1.empty() == false)
    {
        str.Write32(static_cast<std::uint32_t>(m_entryVersion1.size()));
        for (const auto& entry : m_entryVersion1)
        {
            str.Write32(entry.m_sampleNum);
            str.Write32(static_cast<std::uint32_t>(entry.m_sampleOffset));
        }
    }
    else
    {
        ISO_LOG(LOG_ERROR, "Can not write an empty CompositionOffsetAtom.\n");
        throw Exception();
    }

    // Update the size of the movie Atom
    UpdateSize(str);
}

void CompositionOffsetAtom::FromStream(Stream& str)
{
    //  First parse the Atom header
    ParseFullAtomHeader(str);

    const std::uint32_t entryCount = str.Read32();

    if (GetVersion() == 0)
    {
        for (uint32_t i = 0; i < entryCount; ++i)
        {
            EntryVersion0 entryVersion0;
            entryVersion0.m_sampleNum  = str.Read32();
            entryVersion0.m_sampleOffset = str.Read32();
            m_entryVersion0.push_back(entryVersion0);
        }
    }
    else if (GetVersion() == 1)
    {
        for (uint32_t i = 0; i < entryCount; ++i)
        {
            EntryVersion1 entryVersion1;
            entryVersion1.m_sampleNum  = str.Read32();
            entryVersion1.m_sampleOffset = static_cast<std::int32_t>(str.Read32());
            m_entryVersion1.push_back(entryVersion1);
        }
    }
}

VCD_MP4_END