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
//! \file:   TrackRunAtom.cpp
//! \brief:  TrackRunAtom class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!
#include "TrackRunAtom.h"
#include <stdexcept>

VCD_MP4_BEGIN

TrackRunAtom::TrackRunAtom(uint8_t version, std::uint32_t tr_flags)
    : FullAtom("trun", version, tr_flags)
    , m_sampleDefaultsSet(false)
    , m_sampleDefaults()
    , m_sampleNum(0)
    , m_dataOffset(0)
    , m_firstSampleFlags()
    , m_sampleDetails()
{
}

void TrackRunAtom::SetDataOffset(const int32_t dataOffset)
{
    m_dataOffset = dataOffset;
    SetFlags(GetFlags() | TrackRunFlags::pDataOffset);
}

int32_t TrackRunAtom::GetDataOffset() const
{
    if ((GetFlags() & TrackRunFlags::pDataOffset) != 0)
    {
        return m_dataOffset;
    }
    else
    {
        ISO_LOG(LOG_ERROR, "GetDataOffset() according to flags pDataOffset not present.\n");
        throw Exception();
    }
}

void TrackRunAtom::SetFirstSampleFlags(const SampleFlags firstSampleFlags)
{
    m_firstSampleFlags = firstSampleFlags;
    SetFlags(GetFlags() | TrackRunFlags::pFirstSampleFlags);
}

SampleFlags TrackRunAtom::GetFirstSampleFlags() const
{
    if ((GetFlags() & TrackRunFlags::pFirstSampleFlags) != 0)
    {
        return m_firstSampleFlags;
    }
    else
    {
        ISO_LOG(LOG_ERROR, "GetFirstSampleFlags() according to flags pFirstSampleFlags not present\n");
        throw Exception();
    }
}

void TrackRunAtom::AddSampleDetails(SampleDetails pDetails)
{
    m_sampleDetails.push_back(pDetails);
}

const std::vector<TrackRunAtom::SampleDetails>& TrackRunAtom::GetSampleDetails() const
{
    return m_sampleDetails;
}

void TrackRunAtom::SetSampleDefaults(SampleDefaults& sampleDefaults)
{
    m_sampleDefaultsSet = true;
    m_sampleDefaults    = sampleDefaults;
}

void TrackRunAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);

    str.Write32(m_sampleNum);
    if ((GetFlags() & TrackRunFlags::pDataOffset) != 0)
    {
        str.Write32(static_cast<uint32_t>(m_dataOffset));
    }
    if ((GetFlags() & TrackRunFlags::pFirstSampleFlags) != 0)
    {
        SampleFlags::Write(str, m_firstSampleFlags);
    }

    for (uint32_t i = 0; i < m_sampleNum; i++)
    {
        if ((GetFlags() & TrackRunFlags::pSampleDuration) != 0)
        {
            str.Write32(m_sampleDetails.at(i).version0.pDuration);
        }
        if ((GetFlags() & TrackRunFlags::pSampleSize) != 0)
        {
            str.Write32(m_sampleDetails.at(i).version0.pSize);
        }
        if ((GetFlags() & TrackRunFlags::pFirstSampleFlags) == 0)
        {
            if ((GetFlags() & TrackRunFlags::pSampleFlags) != 0)
            {
                SampleFlags::Write(str, m_sampleDetails.at(i).version0.pFlags);
            }
        }
        if ((GetFlags() & TrackRunFlags::pSampleCompTimeOffsets) != 0)
        {
            if (GetVersion() == 0)
            {
                str.Write32(m_sampleDetails.at(i).version0.pCompTimeOffset);
            }
            else
            {
                str.Write32(static_cast<uint32_t>(m_sampleDetails.at(i).version1.pCompTimeOffset));
            }
        }
    }
    UpdateSize(str);
}

void TrackRunAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);
    m_sampleNum = str.Read32();
    if (m_sampleNum > ABMAX_SAMP_CNT)
    {
        ISO_LOG(LOG_ERROR, "Over max sample counts from TrackRunAtom::FromStreamAtom\n");
        throw Exception();
    }
    if ((GetFlags() & TrackRunFlags::pDataOffset) != 0)
    {
        m_dataOffset = static_cast<int32_t>(str.Read32());
    }
    if ((GetFlags() & TrackRunFlags::pFirstSampleFlags) != 0)
    {
        m_firstSampleFlags = SampleFlags::Read(str);
    }

    SampleDetails pDetails;
    for (uint32_t i = 0; i < m_sampleNum; i++)
    {
        if (m_sampleDefaultsSet)
        {
            pDetails.version0.pDuration          = m_sampleDefaults.defaultSampleDuration;
            pDetails.version0.pSize              = m_sampleDefaults.defaultSampleSize;
            pDetails.version0.pFlags.flagsAsUInt = m_sampleDefaults.defaultSampleFlags.flagsAsUInt;
        }
        else
        {
            // these should never be used if right Atoms are present.
            pDetails.version0.pDuration          = 0;
            pDetails.version0.pSize              = 0;
            pDetails.version0.pFlags.flagsAsUInt = 0;
        }

        if ((GetFlags() & TrackRunFlags::pSampleDuration) != 0)
        {
            pDetails.version0.pDuration = str.Read32();
        }

        if ((GetFlags() & TrackRunFlags::pSampleSize) != 0)
        {
            pDetails.version0.pSize = str.Read32();
        }

        if ((GetFlags() & TrackRunFlags::pFirstSampleFlags) != 0)
        {
            pDetails.version0.pFlags.flagsAsUInt = m_firstSampleFlags.flagsAsUInt;

            if (i > 0)
            {
                pDetails.version0.pFlags.flags.sample_is_non_sync_sample = 1;
            }
        }
        else if ((GetFlags() & TrackRunFlags::pSampleFlags) != 0)
        {
            pDetails.version0.pFlags = SampleFlags::Read(str);
        }

        if ((GetFlags() & TrackRunFlags::pSampleCompTimeOffsets) != 0)
        {
            if (GetVersion() == 0)
            {
                pDetails.version0.pCompTimeOffset = str.Read32();
            }
            else
            {
                pDetails.version1.pCompTimeOffset = static_cast<int32_t>(str.Read32());
            }
        }
        else
        {
            if (GetVersion() == 0)
            {
                pDetails.version0.pCompTimeOffset = 0;
            }
            else
            {
                pDetails.version1.pCompTimeOffset = 0;
            }
        }
        m_sampleDetails.push_back(pDetails);
    }
}

VCD_MP4_END