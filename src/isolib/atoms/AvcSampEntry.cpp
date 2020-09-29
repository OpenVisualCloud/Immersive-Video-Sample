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
//! \file:   AvcSampEntry.cpp
//! \brief:  AvcSampEntry class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "AvcSampEntry.h"

VCD_MP4_BEGIN

AvcSampleEntry::AvcSampleEntry()
    : VisualSampleEntryAtom("avc1", "AVC Coding")
    , m_avcConfigurationAtom()
    , m_isStereoscopic3DPresent(false)
    , m_stereoscopic3DAtom()
    , m_isSphericalVideoV2AtomPresent(false)
    , m_sphericalVideoV2Atom()
{
}

AvcSampleEntry::AvcSampleEntry(const AvcSampleEntry& Atom)
    : VisualSampleEntryAtom(Atom)
    , m_avcConfigurationAtom(Atom.m_avcConfigurationAtom)
    , m_isStereoscopic3DPresent(Atom.m_isStereoscopic3DPresent)
    , m_stereoscopic3DAtom(Atom.m_stereoscopic3DAtom)
    , m_isSphericalVideoV2AtomPresent(Atom.m_isSphericalVideoV2AtomPresent)
    , m_sphericalVideoV2Atom(Atom.m_sphericalVideoV2Atom)
{
}

AvcConfigurationAtom& AvcSampleEntry::GetAvcConfigurationAtom()
{
    return m_avcConfigurationAtom;
}

void AvcSampleEntry::CreateStereoscopic3DAtom()
{
    m_isStereoscopic3DPresent = true;
}

void AvcSampleEntry::CreateSphericalVideoV2Atom()
{
    m_isSphericalVideoV2AtomPresent = true;
}

const Stereoscopic3D* AvcSampleEntry::GetStereoscopic3DAtom() const
{
    return (m_isStereoscopic3DPresent ? &m_stereoscopic3DAtom : nullptr);
}

const SphericalVideoV2Atom* AvcSampleEntry::GetSphericalVideoV2Atom() const
{
    return (m_isSphericalVideoV2AtomPresent ? &m_sphericalVideoV2Atom : nullptr);
}

void AvcSampleEntry::ToStream(Stream& str)
{
    VisualSampleEntryAtom::ToStream(str);

    m_avcConfigurationAtom.ToStream(str);

    if (m_isStereoscopic3DPresent)
    {
        m_stereoscopic3DAtom.ToStream(str);
    }

    if (m_isSphericalVideoV2AtomPresent)
    {
        m_sphericalVideoV2Atom.ToStream(str);
    }

    // Update the size of the movie Atom
    UpdateSize(str);
}

void AvcSampleEntry::FromStream(Stream& str)
{
    VisualSampleEntryAtom::FromStream(str);

    while (str.BytesRemain() > 0)
    {
        // Extract contained Atom bitstream and type
        FourCCInt AtomType;
        Stream subStream = str.ReadSubAtomStream(AtomType);

        // Handle this Atom based on the type
        if (AtomType == "avcC")
        {
            m_avcConfigurationAtom.FromStream(subStream);
        }
        else if (AtomType == "st3d")
        {
            m_stereoscopic3DAtom.FromStream(subStream);
            m_isStereoscopic3DPresent = true;
        }
        else if (AtomType == "sv3d")
        {
            m_sphericalVideoV2Atom.FromStream(subStream);
            m_isSphericalVideoV2AtomPresent = true;
        }
        else
        {
			char type[4];
			AtomType.GetString().copy(type, 4, 0);
            ISO_LOG(LOG_WARNING, "Skipping unknown Atom of type '%s' inside AvcSampleEntry\n", type);
        }
    }

    // @todo should have also CleanApertureAtom / PixelAspectRatioAtom reading here
}

AvcSampleEntry* AvcSampleEntry::Clone() const
{
    return (new AvcSampleEntry(*this));
}

const DecoderConfigurationRecord* AvcSampleEntry::GetConfigurationRecord() const
{
    return &m_avcConfigurationAtom.GetConfiguration();
}

const Atom* AvcSampleEntry::GetConfigurationAtom() const
{
    return &m_avcConfigurationAtom;
}

VCD_MP4_END
