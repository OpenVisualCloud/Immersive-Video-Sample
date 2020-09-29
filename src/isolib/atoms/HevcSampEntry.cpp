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
//! \file:   HevcSampEntry.cpp
//! \brief:  HevcSampEntry class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "HevcSampEntry.h"


VCD_MP4_BEGIN

HevcSampleEntry::HevcSampleEntry()
    : VisualSampleEntryAtom("hvc1", "HEVC Coding")
    , m_hevcConfigurationAtom()
    , m_isStereoscopic3DPresent(false)
    , m_stereoscopic3DAtom()
    , m_isSphericalVideoV2AtomPresent(false)
    , m_sphericalVideoV2Atom()
{
}

HevcSampleEntry::HevcSampleEntry(const HevcSampleEntry& Atom)
    : VisualSampleEntryAtom(Atom)
    , m_hevcConfigurationAtom(Atom.m_hevcConfigurationAtom)
    , m_isStereoscopic3DPresent(Atom.m_isStereoscopic3DPresent)
    , m_stereoscopic3DAtom(Atom.m_stereoscopic3DAtom)
    , m_isSphericalVideoV2AtomPresent(Atom.m_isSphericalVideoV2AtomPresent)
    , m_sphericalVideoV2Atom(Atom.m_sphericalVideoV2Atom)
{
}

HevcConfigurationAtom& HevcSampleEntry::GetHevcConfigurationAtom()
{
    return m_hevcConfigurationAtom;
}

void HevcSampleEntry::CreateStereoscopic3DAtom()
{
    m_isStereoscopic3DPresent = true;
}

void HevcSampleEntry::CreateSphericalVideoV2Atom()
{
    m_isSphericalVideoV2AtomPresent = true;
}

const Stereoscopic3D* HevcSampleEntry::GetStereoscopic3DAtom() const
{
    return (m_isStereoscopic3DPresent ? &m_stereoscopic3DAtom : nullptr);
}

const SphericalVideoV2Atom* HevcSampleEntry::GetSphericalVideoV2Atom() const
{
    return (m_isSphericalVideoV2AtomPresent ? &m_sphericalVideoV2Atom : nullptr);
}

void HevcSampleEntry::ToStream(Stream& str)
{
    VisualSampleEntryAtom::ToStream(str);

    m_hevcConfigurationAtom.ToStream(str);

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

void HevcSampleEntry::FromStream(Stream& str)
{

    VisualSampleEntryAtom::FromStream(str);

    while (str.BytesRemain() > 0)
    {
        // Extract contained Atom bitstream and type
        FourCCInt AtomType;
        Stream subStream = str.ReadSubAtomStream(AtomType);

        // Handle this Atom based on the type
        if (AtomType == "hvcC")
        {
            m_hevcConfigurationAtom.FromStream(subStream);
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
            ISO_LOG(LOG_WARNING, "Skipping unknown Atom of type '%s' inside HevcSampleEntry\n", type);
        }
    }
}

HevcSampleEntry* HevcSampleEntry::Clone() const
{
    return (new HevcSampleEntry(*this));
}

const Atom* HevcSampleEntry::GetConfigurationAtom() const
{
    return &m_hevcConfigurationAtom;
}

const DecoderConfigurationRecord* HevcSampleEntry::GetConfigurationRecord() const
{
    return &m_hevcConfigurationAtom.GetConfiguration();
}

VCD_MP4_END
