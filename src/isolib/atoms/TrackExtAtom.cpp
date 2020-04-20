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
//! \file:   TrackExtAtom.cpp
//! \brief:  TrackExtAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "TrackExtAtom.h"

VCD_MP4_BEGIN

TrackExtendsAtom::TrackExtendsAtom()
    : FullAtom("trex", 0, 0)
    , m_sampleDefaults()
{
}

void TrackExtendsAtom::SetFragmentSampleDefaults(const SampleDefaults& fragmentSampleDefaults)
{
    m_sampleDefaults = fragmentSampleDefaults;
}

const SampleDefaults& TrackExtendsAtom::GetFragmentSampleDefaults() const
{
    return m_sampleDefaults;
}

void TrackExtendsAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);
    str.Write32(m_sampleDefaults.trackId);
    str.Write32(m_sampleDefaults.defaultSampleDescriptionIndex);
    str.Write32(m_sampleDefaults.defaultSampleDuration);
    str.Write32(m_sampleDefaults.defaultSampleSize);
    SampleFlags::Write(str, m_sampleDefaults.defaultSampleFlags);
    UpdateSize(str);
}

void TrackExtendsAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);
    m_sampleDefaults.trackId                       = str.Read32();
    m_sampleDefaults.defaultSampleDescriptionIndex = str.Read32();
    m_sampleDefaults.defaultSampleDuration         = str.Read32();
    m_sampleDefaults.defaultSampleSize             = str.Read32();
    m_sampleDefaults.defaultSampleFlags            = SampleFlags::Read(str);
}

VCD_MP4_END