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
//! \file:   Mp4VisualSampEntryAtom.cpp
//! \brief:  Mp4VisualSampEntryAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!
#include "Mp4VisualSampEntryAtom.h"
#include <string>


VCD_MP4_BEGIN

MP4VisualSampleEntryAtom::MP4VisualSampleEntryAtom()
    : VisualSampleEntryAtom("mp4v", "MPEG4 Visual Coding")
    , m_ESDAtom()
    , m_isStereoscopic3DPresent(false)
    , m_stereoscopic3DAtom()
    , m_isSphericalVideoV2AtomPresent(false)
    , m_sphericalVideoV2Atom()
{
}

MP4VisualSampleEntryAtom::MP4VisualSampleEntryAtom(const MP4VisualSampleEntryAtom& Atom)
    : VisualSampleEntryAtom(Atom)
    , m_ESDAtom(Atom.m_ESDAtom)
    , m_isStereoscopic3DPresent(Atom.m_isStereoscopic3DPresent)
    , m_stereoscopic3DAtom(Atom.m_stereoscopic3DAtom)
    , m_isSphericalVideoV2AtomPresent(Atom.m_isSphericalVideoV2AtomPresent)
    , m_sphericalVideoV2Atom(Atom.m_sphericalVideoV2Atom)
{
}

void MP4VisualSampleEntryAtom::CreateStereoscopic3DAtom()
{
    m_isStereoscopic3DPresent = true;
}

void MP4VisualSampleEntryAtom::CreateSphericalVideoV2Atom()
{
    m_isSphericalVideoV2AtomPresent = true;
}

ElementaryStreamDescriptorAtom& MP4VisualSampleEntryAtom::GetESDAtom()
{
    return m_ESDAtom;
}

const Stereoscopic3D* MP4VisualSampleEntryAtom::GetStereoscopic3DAtom() const
{
    return (m_isStereoscopic3DPresent ? &m_stereoscopic3DAtom : nullptr);
}

const SphericalVideoV2Atom* MP4VisualSampleEntryAtom::GetSphericalVideoV2Atom() const
{
    return (m_isSphericalVideoV2AtomPresent ? &m_sphericalVideoV2Atom : nullptr);
}

void MP4VisualSampleEntryAtom::ToStream(Stream& str)
{
    VisualSampleEntryAtom::ToStream(str);
    m_ESDAtom.ToStream(str);

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

void MP4VisualSampleEntryAtom::FromStream(Stream& str)
{
    VisualSampleEntryAtom::FromStream(str);

    while (str.BytesRemain() > 0)
    {
        FourCCInt AtomType;
        Stream subBitstr = str.ReadSubAtomStream(AtomType);

        if (AtomType == "esds")
        {
            m_ESDAtom.FromStream(subBitstr);
        }
        else if (AtomType == "st3d")
        {
            m_stereoscopic3DAtom.FromStream(subBitstr);
            m_isStereoscopic3DPresent = true;
        }
        else if (AtomType == "sv3d")
        {
            m_sphericalVideoV2Atom.FromStream(subBitstr);
            m_isSphericalVideoV2AtomPresent = true;
        }
        else
        {
			char type[4];
            AtomType.GetString().copy(type, 4, 0);
            ISO_LOG(LOG_WARNING, "Skipping an unsupported Atom '%s' inside MP4VisualSampleEntryAtom.\n", type);
        }
    }
}

MP4VisualSampleEntryAtom* MP4VisualSampleEntryAtom::Clone() const
{
    return (new MP4VisualSampleEntryAtom(*this));
}

const Atom* MP4VisualSampleEntryAtom::GetConfigurationAtom() const
{
    ISO_LOG(LOG_ERROR, "MP4VisualSampleEntryAtom::GetConfigurationAtom() not impelmented \n");
    return nullptr;
}

const DecoderConfigurationRecord* MP4VisualSampleEntryAtom::GetConfigurationRecord() const
{
    ISO_LOG(LOG_ERROR, "MP4VisualSampleEntryAtom::GetConfigurationRecord() not impelmented \n");
    return nullptr;
}

VCD_MP4_END
