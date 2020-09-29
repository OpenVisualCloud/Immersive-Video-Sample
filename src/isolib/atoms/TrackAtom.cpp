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
//! \file:   TrackAtom.cpp
//! \brief:  TrackAtom class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include "TrackAtom.h"
#include "BasicVideoAtom.h"

VCD_MP4_BEGIN

TrackAtom::TrackAtom()
    : Atom("trak")
    , m_trackHeaderAtom()
    , m_mediaAtom()
    , m_trackRefAtom()
    , m_hasTrackRef(false)
    , m_trackGroupAtom()
    , m_hasTrackGroupAtom(false)
    , m_hasTrackTypeAtom(false)
    , m_editAtom(nullptr)
    , m_hasSphericalVideoV1Atom(false)
    , m_sphericalVideoV1Atom()
{
}

void TrackAtom::SetHasTrackReferences(bool value)
{
    m_hasTrackRef = value;
}

bool TrackAtom::GetHasTrackReferences() const
{
    return m_hasTrackRef;
}

const TrackHeaderAtom& TrackAtom::GetTrackHeaderAtom() const
{
    return m_trackHeaderAtom;
}

TrackHeaderAtom& TrackAtom::GetTrackHeaderAtom()
{
    return m_trackHeaderAtom;
}

const MediaAtom& TrackAtom::GetMediaAtom() const
{
    return m_mediaAtom;
}

MediaAtom& TrackAtom::GetMediaAtom()
{
    return m_mediaAtom;
}

const TrackReferenceAtom& TrackAtom::GetTrackReferenceAtom() const
{
    return m_trackRefAtom;
}

TrackReferenceAtom& TrackAtom::GetTrackReferenceAtom()
{
    return m_trackRefAtom;
}

const TrackGroupAtom& TrackAtom::GetTrackGroupAtom() const
{
    return m_trackGroupAtom;
}

TrackGroupAtom& TrackAtom::GetTrackGroupAtom()
{
    return m_trackGroupAtom;
}

const TrackTypeAtom& TrackAtom::GetTrackTypeAtom() const
{
    return m_trackTypeAtom;
}

TrackTypeAtom& TrackAtom::GetTrackTypeAtom()
{
    return m_trackTypeAtom;
}

void TrackAtom::SetEditAtom(const EditAtom& editAtom)
{
    if (m_editAtom == nullptr)
    {
        m_editAtom = MakeShared<EditAtom>(editAtom);
    }
    else
    {
        *m_editAtom = editAtom;
    }
}

std::shared_ptr<const EditAtom> TrackAtom::GetEditAtom() const
{
    return m_editAtom;
}

const SphericalVideoV1Atom& TrackAtom::GetSphericalVideoV1Atom() const
{
    return m_sphericalVideoV1Atom;
}

SphericalVideoV1Atom& TrackAtom::GetSphericalVideoV1Atom()
{
    return m_sphericalVideoV1Atom;
}

void TrackAtom::ToStream(Stream& str)
{
    // Write Atom headers
    WriteAtomHeader(str);

    // Write other Atoms contained in the movie Atom
    // The TrackHeaderAtom
    m_trackHeaderAtom.ToStream(str);

    if (m_hasTrackRef)
    {
        m_trackRefAtom.ToStream(str);
    }

    // The MediaAtom
    m_mediaAtom.ToStream(str);

    if (m_hasTrackGroupAtom)
    {
        m_trackGroupAtom.ToStream(str);
    }

    if (m_editAtom)
    {
        m_editAtom->ToStream(str);
    }

    if (m_hasSphericalVideoV1Atom)
    {
        m_sphericalVideoV1Atom.ToStream(str);
    }

    if (m_hasTrackTypeAtom)
    {
        m_trackTypeAtom.ToStream(str);
    }

    // Update the size of the movie Atom
    UpdateSize(str);
}

void TrackAtom::FromStream(Stream& str)
{
    //  First parse the Atom header
    ParseAtomHeader(str);

    // if there a data available in the file
    while (str.BytesRemain() > 0)
    {
        // Extract contained Atom bitstream and type
        FourCCInt AtomType;
        Stream subBitstr = str.ReadSubAtomStream(AtomType);

        if (AtomType == "tkhd")
        {
            m_trackHeaderAtom.FromStream(subBitstr);
        }
        else if (AtomType == "mdia")
        {
            m_mediaAtom.FromStream(subBitstr);
        }
        else if (AtomType == "meta")
        {
            // @todo Implement this when reading meta Atom in tracks is supported
        }
        else if (AtomType == "tref")
        {
            m_trackRefAtom.FromStream(subBitstr);
            m_hasTrackRef = true;
        }
        else if (AtomType == "trgr")
        {
            m_hasTrackGroupAtom = true;
            m_trackGroupAtom.FromStream(subBitstr);
        }
        else if (AtomType == "ttyp")
        {
            m_hasTrackTypeAtom = true;
            m_trackTypeAtom.FromStream(subBitstr);
        }
        else if (AtomType == "edts")
        {
            m_editAtom = MakeShared<EditAtom>();
            m_editAtom->FromStream(subBitstr);
        }
        else if (AtomType == "uuid")
        {
            std::vector<uint8_t> extendedType;
            subBitstr.ReadUUID(extendedType);

            std::vector<uint8_t> comparison = SPHERICAL_VIDEOV1_GENERAL_UUID;
            if (extendedType == comparison)
            {
                m_sphericalVideoV1Atom.FromStream(subBitstr);
                m_hasSphericalVideoV1Atom = true;
            }
            else
            {
                ISO_LOG(LOG_WARNING, "Skipping an unsupported UUID Atom inside TrackAtom.\n");
            }
        }
        else
        {
			char type[4];
            AtomType.GetString().copy(type, 4, 0);
            ISO_LOG(LOG_WARNING, "Skipping an unsupported Atom '%s' inside TrackAtom.\n", type);
        }
    }
}

VCD_MP4_END
