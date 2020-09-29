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
//! \file:   SampGroupEntry.cpp
//! \brief:  SampGroupEntry class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "SampGroupEntry.h"

#include <stdexcept>

VCD_MP4_BEGIN

DirectReferenceSampleListEntry::DirectReferenceSampleListEntry()
    : m_sampleId(0)
{
}

void DirectReferenceSampleListEntry::SetDirectRefSampleIds(const std::vector<std::uint32_t>& referenceSampleIds)
{
    if (referenceSampleIds.size() > 255)
    {
        ISO_LOG(LOG_ERROR, "Too many entries in referenceSampleIds\n");
        throw Exception();
    }

    m_directRefSampIds = referenceSampleIds;
}

std::vector<std::uint32_t> DirectReferenceSampleListEntry::GetDirectRefSampleIds() const
{
    return m_directRefSampIds;
}

std::uint32_t DirectReferenceSampleListEntry::GetSize() const
{
    const uint32_t size = static_cast<uint32_t>(sizeof(m_sampleId) + sizeof(uint8_t) +
                                                (sizeof(uint32_t) * m_directRefSampIds.size()));
    return size;
}

void DirectReferenceSampleListEntry::WriteEntry(Stream& str)
{
    str.Write32(m_sampleId);

    str.Write8(static_cast<std::uint8_t>(m_directRefSampIds.size()));
    for (auto id : m_directRefSampIds)
    {
        str.Write32(id);
    }
}

void DirectReferenceSampleListEntry::ParseEntry(Stream& str)
{
    m_sampleId                               = str.Read32();
    const uint8_t numberOfReferencedSamples = str.Read8();
    for (unsigned int i = 0; i < numberOfReferencedSamples; ++i)
    {
        m_directRefSampIds.push_back(str.Read32());
    }
}

VCD_MP4_END