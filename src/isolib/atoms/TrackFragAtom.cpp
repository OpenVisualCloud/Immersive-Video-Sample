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
//! \file:   TrackFragAtom.cpp
//! \brief:  TrackFragAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "TrackFragAtom.h"
#include <stdexcept>

VCD_MP4_BEGIN

TrackFragmentHeaderAtom::TrackFragmentHeaderAtom(std::uint32_t tr_flags)
    : FullAtom("tfhd", 0, tr_flags)
    , m_trackId(0)
    , m_baseDataOffset(0)
    , m_sampleDescrIndex(0)
    , m_defaultSampleDuration(0)
    , m_defaultSampleSize(0)
{
    m_defaultSampleFlags.flagsAsUInt = 0;
}

void TrackFragmentHeaderAtom::SetBaseDataOffset(const uint64_t baseDataOffset)
{
    m_baseDataOffset = baseDataOffset;
    SetFlags(GetFlags() | TrackFragmentHeaderAtom::pDataOffset);
}

uint64_t TrackFragmentHeaderAtom::GetBaseDataOffset() const
{
    if ((GetFlags() & TrackFragmentHeaderAtom::pDataOffset) != 0)
    {
        return m_baseDataOffset;
    }
    else
    {
        ISO_LOG(LOG_ERROR, "TrackFragmentHeaderAtom::GetBaseDataOffset() according to flags pDataOffset not present.\n");
        throw Exception();
    }
}

void TrackFragmentHeaderAtom::SetDefaultSampleDuration(const uint32_t defaultSampleDuration)
{
    m_defaultSampleDuration = defaultSampleDuration;
    SetFlags(GetFlags() | TrackFragmentHeaderAtom::pSampleDuration);
}

uint32_t TrackFragmentHeaderAtom::GetDefaultSampleDuration() const
{
    if ((GetFlags() & TrackFragmentHeaderAtom::pSampleDuration) != 0)
    {
        return m_defaultSampleDuration;
    }
    else
    {
        ISO_LOG(LOG_ERROR, "TrackFragmentHeaderAtom::GetDefaultSampleDuration() according to flags pSampleDuration\n");
        throw Exception();
    }
}

void TrackFragmentHeaderAtom::SetDefaultSampleSize(const uint32_t defaultSampleSize)
{
    m_defaultSampleSize = defaultSampleSize;
    SetFlags(GetFlags() | TrackFragmentHeaderAtom::pSampleSize);
}

uint32_t TrackFragmentHeaderAtom::GetDefaultSampleSize() const
{
    if ((GetFlags() & TrackFragmentHeaderAtom::pSampleSize) != 0)
    {
        return m_defaultSampleSize;
    }
    else
    {
        ISO_LOG(LOG_ERROR, "TrackFragmentHeaderAtom::GetDefaultSampleSize() according to flags pSampleSize not present.\n");
        throw Exception();
    }
}

void TrackFragmentHeaderAtom::SetDefaultSampleFlags(const SampleFlags defaultSampleFlags)
{
    m_defaultSampleFlags = defaultSampleFlags;
    SetFlags(GetFlags() | TrackFragmentHeaderAtom::pSampleFlags);
}

SampleFlags TrackFragmentHeaderAtom::GetDefaultSampleFlags() const
{
    if ((GetFlags() & TrackFragmentHeaderAtom::pSampleFlags) != 0)
    {
        return m_defaultSampleFlags;
    }
    else
    {
        ISO_LOG(LOG_ERROR, "TrackFragmentHeaderAtom::SetDefaultSampleFlags() according to flags pSampleFlags\n");
        throw Exception();
    }
}

void TrackFragmentHeaderAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);

    str.Write32(m_trackId);
    if ((GetFlags() & TrackFragmentHeaderAtom::pDataOffset) != 0)
    {
        str.Write64(m_baseDataOffset);
    }
    if ((GetFlags() & TrackFragmentHeaderAtom::pSampleDescrIndex) != 0)
    {
        str.Write32(m_sampleDescrIndex);
    }
    if ((GetFlags() & TrackFragmentHeaderAtom::pSampleDuration) != 0)
    {
        str.Write32(m_defaultSampleDuration);
    }
    if ((GetFlags() & TrackFragmentHeaderAtom::pSampleSize) != 0)
    {
        str.Write32(m_defaultSampleSize);
    }
    if ((GetFlags() & TrackFragmentHeaderAtom::pSampleFlags) != 0)
    {
        SampleFlags::Write(str, m_defaultSampleFlags);
    }

    UpdateSize(str);
}

void TrackFragmentHeaderAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);

    m_trackId = str.Read32();
    if ((GetFlags() & TrackFragmentHeaderAtom::pDataOffset) != 0)
    {
        m_baseDataOffset = str.Read64();
    }
    if ((GetFlags() & TrackFragmentHeaderAtom::pSampleDescrIndex) != 0)
    {
        m_sampleDescrIndex = str.Read32();
    }
    if ((GetFlags() & TrackFragmentHeaderAtom::pSampleDuration) != 0)
    {
        m_defaultSampleDuration = str.Read32();
    }
    if ((GetFlags() & TrackFragmentHeaderAtom::pSampleSize) != 0)
    {
        m_defaultSampleSize = str.Read32();
    }
    if ((GetFlags() & TrackFragmentHeaderAtom::pSampleFlags) != 0)
    {
        m_defaultSampleFlags = SampleFlags::Read(str);
    }
}

TrackFragmentBaseMediaDecodeTimeAtom::TrackFragmentBaseMediaDecodeTimeAtom()
    : FullAtom("tfdt", 0, 0)
    , m_baseMediaDecodeTime(0)
{
}

void TrackFragmentBaseMediaDecodeTimeAtom::SetBaseMediaDecodeTime(const uint64_t baseMediaDecodeTime)
{
    m_baseMediaDecodeTime = baseMediaDecodeTime;
}

uint64_t TrackFragmentBaseMediaDecodeTimeAtom::GetBaseMediaDecodeTime() const
{
    return m_baseMediaDecodeTime;
}

void TrackFragmentBaseMediaDecodeTimeAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);
    if (GetVersion() == 0)
    {
        str.Write32(static_cast<uint32_t>(m_baseMediaDecodeTime));
    }
    else if (GetVersion() == 1)
    {
        str.Write64(m_baseMediaDecodeTime);
    }
    else
    {
        ISO_LOG(LOG_ERROR, "ToStream() supports only 'tfdt' version 0 or 1\n");
        throw Exception();
    }
    UpdateSize(str);
}

void TrackFragmentBaseMediaDecodeTimeAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);
    if (GetVersion() == 0)
    {
        m_baseMediaDecodeTime = str.Read32();
    }
    else if (GetVersion() == 1)
    {
        m_baseMediaDecodeTime = str.Read64();
    }
    else
    {
        ISO_LOG(LOG_ERROR, "FromStream() supports only 'tfdt' version 0 or 1\n");
        throw Exception();
    }
}

TrackFragmentAtom::TrackFragmentAtom(std::vector<SampleDefaults>& sampleDefaults)
    : Atom("traf")
    , m_trackFragmentHeaderAtom()
    , m_sampleDefaults(sampleDefaults)
    , m_trackRunAtoms()
    , m_trackFragmentDecodeTimeAtom()
{
}

TrackFragmentHeaderAtom& TrackFragmentAtom::GetTrackFragmentHeaderAtom()
{
    return m_trackFragmentHeaderAtom;
}

void TrackFragmentAtom::AddTrackRunAtom(UniquePtr<TrackRunAtom> trackRunAtom)
{
    m_trackRunAtoms.push_back(std::move(trackRunAtom));
}

std::vector<TrackRunAtom*> TrackFragmentAtom::GetTrackRunAtoms()
{
    std::vector<TrackRunAtom*> trackRunAtoms;
    for (auto& trackRuns : m_trackRunAtoms)
    {
        trackRunAtoms.push_back(trackRuns.get());
    }
    return trackRunAtoms;
}

void TrackFragmentAtom::SetTrackFragmentDecodeTimeAtom(
    UniquePtr<TrackFragmentBaseMediaDecodeTimeAtom> trackFragmentDecodeTimeAtom)
{
    m_trackFragmentDecodeTimeAtom = std::move(trackFragmentDecodeTimeAtom);
}

TrackFragmentBaseMediaDecodeTimeAtom* TrackFragmentAtom::GetTrackFragmentBaseMediaDecodeTimeAtom()
{
    return m_trackFragmentDecodeTimeAtom.get();
}

void TrackFragmentAtom::ToStream(Stream& str)
{
    WriteAtomHeader(str);
    m_trackFragmentHeaderAtom.ToStream(str);

    if (m_trackFragmentDecodeTimeAtom)
    {
        m_trackFragmentDecodeTimeAtom->ToStream(str);
    }
    for (auto& trackRuns : m_trackRunAtoms)
    {
        trackRuns->ToStream(str);
    }
    UpdateSize(str);
}

void TrackFragmentAtom::FromStream(Stream& str)
{
    ParseAtomHeader(str);

    bool foundTfhd = false;

    while (str.BytesRemain() > 0)
    {
        FourCCInt AtomType;
        Stream subBitstr = str.ReadSubAtomStream(AtomType);

        if (AtomType == "tfhd")
        {
            if (foundTfhd)
            {
                ISO_LOG(LOG_ERROR, "TrackFragmentAtom: exactly one tfhd expected!\n");
                throw Exception();
            }

            m_trackFragmentHeaderAtom.FromStream(subBitstr);

            bool foundDefault                      = false;
            uint32_t defaultSampleDescriptionIndex = 0;
            for (size_t i = 0; i < m_sampleDefaults.size(); i++)
            {
                if (m_sampleDefaults[i].trackId == m_trackFragmentHeaderAtom.GetTrackId())
                {
                    defaultSampleDescriptionIndex = m_sampleDefaults[i].defaultSampleDescriptionIndex;
                    foundDefault                  = true;
                    break;
                }
            }
            if (!foundDefault)
            {
                ISO_LOG(LOG_ERROR, "default sample description index not found\n");
                throw Exception();
            }

            if ((m_trackFragmentHeaderAtom.GetFlags() & TrackFragmentHeaderAtom::pSampleDescrIndex) == 0)
            {
                m_trackFragmentHeaderAtom.SetSampleDescrIndex(defaultSampleDescriptionIndex);
            }
            foundTfhd = true;
        }
        else if (AtomType == "tfdt")
        {
            UniquePtr<TrackFragmentBaseMediaDecodeTimeAtom> trackFragmentDecodeTimeAtom(new TrackFragmentBaseMediaDecodeTimeAtom());
            trackFragmentDecodeTimeAtom->FromStream(subBitstr);
            m_trackFragmentDecodeTimeAtom = std::move(trackFragmentDecodeTimeAtom);
        }
        else if (AtomType == "trun")
        {
            SampleDefaults sampleDefaults{};
            bool defaultsFound = false;
            for (size_t i = 0; i < m_sampleDefaults.size(); i++)
            {
                if (m_sampleDefaults[i].trackId == m_trackFragmentHeaderAtom.GetTrackId())
                {
                    sampleDefaults.trackId                       = m_sampleDefaults[i].trackId;
                    sampleDefaults.defaultSampleDescriptionIndex = m_sampleDefaults[i].defaultSampleDescriptionIndex;
                    sampleDefaults.defaultSampleDuration         = m_sampleDefaults[i].defaultSampleDuration;
                    sampleDefaults.defaultSampleSize             = m_sampleDefaults[i].defaultSampleSize;
                    sampleDefaults.defaultSampleFlags            = m_sampleDefaults[i].defaultSampleFlags;
                    defaultsFound                                = true;
                    break;
                }
            }
            if ((m_trackFragmentHeaderAtom.GetFlags() & TrackFragmentHeaderAtom::pSampleDuration) != 0)
            {
                sampleDefaults.defaultSampleDuration = m_trackFragmentHeaderAtom.GetDefaultSampleDuration();
            }
            if ((m_trackFragmentHeaderAtom.GetFlags() & TrackFragmentHeaderAtom::pSampleSize) != 0)
            {
                sampleDefaults.defaultSampleSize = m_trackFragmentHeaderAtom.GetDefaultSampleSize();
            }
            if ((m_trackFragmentHeaderAtom.GetFlags() & TrackFragmentHeaderAtom::pSampleFlags) != 0)
            {
                sampleDefaults.defaultSampleFlags = m_trackFragmentHeaderAtom.GetDefaultSampleFlags();
            }
            UniquePtr<TrackRunAtom> trackRunAtom(new TrackRunAtom());
            if (defaultsFound)
            {
                trackRunAtom->SetSampleDefaults(sampleDefaults);
            }

            trackRunAtom->FromStream(subBitstr);
            m_trackRunAtoms.push_back(std::move(trackRunAtom));
        }
        else
        {
            char type[4];
            AtomType.GetString().copy(type, 4, 0);
			ISO_LOG(LOG_WARNING, "Skipping an unsupported Atom '%s' inside TrackFragmentAtom.\n", type);
        }
    }
    if (!foundTfhd)
    {
        ISO_LOG(LOG_ERROR, "tfhd Atom missing (mandatory)\n");
        throw Exception();
    }
}

VCD_MP4_END
