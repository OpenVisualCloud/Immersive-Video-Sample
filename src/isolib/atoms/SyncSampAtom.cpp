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
//! \file:   SyncSampAtom.cpp
//! \brief:  SyncSampAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "SyncSampAtom.h"

VCD_MP4_BEGIN

SyncSampleAtom::SyncSampleAtom()
    : FullAtom("stss", 0, 0)
    , m_sampleNumber()
    , m_sampleNumMax(-1)
{
}

void SyncSampleAtom::AddSample(std::uint32_t sampleNumber)
{
    m_sampleNumber.push_back(sampleNumber);
}

const std::vector<std::uint32_t> SyncSampleAtom::GetSyncSampleIds() const
{
    return m_sampleNumber;
}

void SyncSampleAtom::SetSampleNumMaxSafety(int64_t sampleNumMax)
{
    m_sampleNumMax = sampleNumMax;
}

void SyncSampleAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);

    str.Write32(static_cast<unsigned int>(m_sampleNumber.size()));

    for (auto sampleNumber : m_sampleNumber)
    {
        str.Write32(sampleNumber);
    }

    // Update the size
    UpdateSize(str);
}

void SyncSampleAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);

    std::uint32_t entryCount = str.Read32();

    if (m_sampleNumMax != -1 && (entryCount > m_sampleNumMax))
    {
        ISO_LOG(LOG_ERROR, "FromStreamAtom entryCount is larger than total number of samples\n");
        throw Exception();
    }

    for (std::uint32_t i = 0; i < entryCount; ++i)
    {
        m_sampleNumber.push_back(str.Read32());
    }
}

VCD_MP4_END