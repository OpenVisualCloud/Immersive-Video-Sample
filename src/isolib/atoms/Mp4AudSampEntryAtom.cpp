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
//! \file:   Mp4AudSampEntryAtom.cpp
//! \brief:  Mp4AudSampEntryAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "Mp4AudSampEntryAtom.h"
#include <string>

VCD_MP4_BEGIN

MP4AudioSampleEntryAtom::MP4AudioSampleEntryAtom()
    : AudioSampleEntryAtom("mp4a")
    , m_ESDAtom()
    , m_hasSpatialAudioAtom(false)
    , m_spatialAudioAtom()
    , m_hasNonDiegeticAudioAtom(false)
    , m_nonDiegeticAudioAtom()
    , m_record(m_ESDAtom)
{
}

ElementaryStreamDescriptorAtom& MP4AudioSampleEntryAtom::GetESDAtom()
{
    return m_ESDAtom;
}

const ElementaryStreamDescriptorAtom& MP4AudioSampleEntryAtom::GetESDAtom() const
{
    return m_ESDAtom;
}

bool MP4AudioSampleEntryAtom::HasSpatialAudioAtom() const
{
    return m_hasSpatialAudioAtom;
}

const SpatialAudioAtom& MP4AudioSampleEntryAtom::GetSpatialAudioAtom() const
{
    return m_spatialAudioAtom;
}

void MP4AudioSampleEntryAtom::SetSpatialAudioAtom(const SpatialAudioAtom& spatialAudioAtom)
{
    m_hasSpatialAudioAtom = true;
    m_spatialAudioAtom    = spatialAudioAtom;
}

bool MP4AudioSampleEntryAtom::HasNonDiegeticAudioAtom() const
{
    return m_hasNonDiegeticAudioAtom;
}

const NonDiegeticAudioAtom& MP4AudioSampleEntryAtom::GetNonDiegeticAudioAtom() const
{
    return m_nonDiegeticAudioAtom;
}

void MP4AudioSampleEntryAtom::SetNonDiegeticAudioAtom(const NonDiegeticAudioAtom& nonDiegeticAudioAtom)
{
    m_hasNonDiegeticAudioAtom = true;
    m_nonDiegeticAudioAtom    = nonDiegeticAudioAtom;
}

void MP4AudioSampleEntryAtom::ToStream(Stream& str)
{
    AudioSampleEntryAtom::ToStream(str);
    m_ESDAtom.ToStream(str);

    if (m_hasSpatialAudioAtom)
    {
        m_spatialAudioAtom.ToStream(str);
    }

    if (m_hasNonDiegeticAudioAtom)
    {
        m_nonDiegeticAudioAtom.ToStream(str);
    }

    UpdateSize(str);
}

void MP4AudioSampleEntryAtom::FromStream(Stream& str)
{
    AudioSampleEntryAtom::FromStream(str);

    while (str.BytesRemain() > 0)
    {
        FourCCInt AtomType;
        Stream subBitstr = str.ReadSubAtomStream(AtomType);

        if (AtomType == "esds")
        {
            m_ESDAtom.FromStream(subBitstr);
        }
        else if (AtomType == "SA3D")
        {
            m_hasSpatialAudioAtom = true;
            m_spatialAudioAtom.FromStream(subBitstr);
        }
        else if (AtomType == "SAND")
        {
            m_hasNonDiegeticAudioAtom = true;
            m_nonDiegeticAudioAtom.FromStream(subBitstr);
        }
    }
}

MP4AudioSampleEntryAtom* MP4AudioSampleEntryAtom::Clone() const
{
    return (new MP4AudioSampleEntryAtom(*this));
}

const Atom* MP4AudioSampleEntryAtom::GetConfigurationAtom() const
{
    return &m_ESDAtom;
}

const DecoderConfigurationRecord* MP4AudioSampleEntryAtom::GetConfigurationRecord() const
{
    return &m_record;
}

VCD_MP4_END