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
//! \file:   MovieAtom.cpp
//! \brief:  MovieAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "MovieAtom.h"
#include "Stream.h"
#include <stdexcept>

VCD_MP4_BEGIN

MovieExtendsHeaderAtom::MovieExtendsHeaderAtom(uint8_t version)
    : FullAtom("mehd", version, 0)
    , m_fragmentDuration(0)
{
}

void MovieExtendsHeaderAtom::SetFragmentDuration(const uint64_t fragmentDuration)
{
    m_fragmentDuration = fragmentDuration;
}

uint64_t MovieExtendsHeaderAtom::GetFragmentDuration() const
{
    return m_fragmentDuration;
}

void MovieExtendsHeaderAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);
    if (GetVersion() == 0)
    {
        str.Write32(static_cast<uint32_t>(m_fragmentDuration));
    }
    else if (GetVersion() == 1)
    {
        str.Write64(m_fragmentDuration);
    }
    else
    {
        ISO_LOG(LOG_ERROR, "ToStream() supports only 'mehd' version 0 or 1\n");
        throw Exception();
    }
    UpdateSize(str);
}

void MovieExtendsHeaderAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);
    if (GetVersion() == 0)
    {
        m_fragmentDuration = str.Read32();
    }
    else if (GetVersion() == 1)
    {
        m_fragmentDuration = str.Read64();
    }
    else
    {
        ISO_LOG(LOG_ERROR, "FromStream() supports only 'mehd' version 0 or 1\n");
        throw Exception();
    }
}

MovieExtendsAtom::MovieExtendsAtom()
    : Atom("mvex")
    , m_movieExtendsHeaderAtomPresent(false)
    , m_movieExtendsHeaderAtom()
    , m_trackExtends()
{
}

void MovieExtendsAtom::AddMovieExtendsHeaderAtom(const MovieExtendsHeaderAtom& movieExtendsHeaderAtom)
{
    m_movieExtendsHeaderAtomPresent = true;
    m_movieExtendsHeaderAtom        = movieExtendsHeaderAtom;
}

bool MovieExtendsAtom::IsMovieExtendsHeaderAtomPresent() const
{
    return m_movieExtendsHeaderAtomPresent;
}

const MovieExtendsHeaderAtom& MovieExtendsAtom::GetMovieExtendsHeaderAtom() const
{
    return m_movieExtendsHeaderAtom;
}

void MovieExtendsAtom::AddTrackExtendsAtom(UniquePtr<TrackExtendsAtom> trackExtendsAtom)
{
    m_trackExtends.push_back(std::move(trackExtendsAtom));
}

const std::vector<TrackExtendsAtom*> MovieExtendsAtom::GetTrackExtendsAtoms() const
{
    std::vector<TrackExtendsAtom*> trackExtendsAtoms;
    for (auto& trackExtends : m_trackExtends)
    {
        trackExtendsAtoms.push_back(trackExtends.get());
    }
    return trackExtendsAtoms;
}

void MovieExtendsAtom::ToStream(Stream& str)
{
    WriteAtomHeader(str);
    if (IsMovieExtendsHeaderAtomPresent() == true)
    {
        m_movieExtendsHeaderAtom.ToStream(str);
    }
    for (auto& trackExtends : m_trackExtends)
    {
        trackExtends->ToStream(str);
    }
    UpdateSize(str);
}

void MovieExtendsAtom::FromStream(Stream& str)
{
    ParseAtomHeader(str);
    bool foundTrex = false;
    while (str.BytesRemain() > 0)
    {
        FourCCInt AtomType;
        Stream subBitstr = str.ReadSubAtomStream(AtomType);

        if (AtomType == "mehd")
        {
            m_movieExtendsHeaderAtomPresent = true;
            m_movieExtendsHeaderAtom.FromStream(subBitstr);
        }
        else if (AtomType == "trex")
        {
            UniquePtr<TrackExtendsAtom> trackExtendsAtom(new TrackExtendsAtom());
            trackExtendsAtom->FromStream(subBitstr);
            m_trackExtends.push_back(std::move(trackExtendsAtom));
            foundTrex = true;
        }
        else
        {
			char type[4];
            AtomType.GetString().copy(type, 4, 0);
            ISO_LOG(LOG_WARNING, "Skipping an unsupported Atom '%s' inside MovieExtendsAtom.\n", type);
        }
    }

    if (!foundTrex)
    {
        ISO_LOG(LOG_ERROR, "FromStreamAtom cannot find mandatory TrackExtendsAtom Atom\n");
        throw Exception();
    }
}

MovieAtom::MovieAtom()
    : Atom("moov")
    , m_movieHeaderAtom()
    , m_tracks()
    , m_movieExtendsAtom()
{
}

void MovieAtom::Clear()
{
    m_movieHeaderAtom = {};
    m_tracks.clear();
    m_movieExtendsAtom = {};
}

MovieHeaderAtom& MovieAtom::GetMovieHeaderAtom()
{
    return m_movieHeaderAtom;
}

const MovieHeaderAtom& MovieAtom::GetMovieHeaderAtom() const
{
    return m_movieHeaderAtom;
}

std::vector<TrackAtom*> MovieAtom::GetTrackAtoms()
{
    std::vector<TrackAtom*> trackAtoms;
    for (auto& track : m_tracks)
    {
        trackAtoms.push_back(track.get());
    }
    return trackAtoms;
}

TrackAtom* MovieAtom::GetTrackAtom(uint32_t trackId)
{
    for (auto& track : m_tracks)
    {
        if (track.get()->GetTrackHeaderAtom().GetTrackID() == trackId)
        {
            return track.get();
        }
    }
    return nullptr;
}

void MovieAtom::AddTrackAtom(UniquePtr<TrackAtom> trackAtom)
{
    m_tracks.push_back(std::move(trackAtom));
}

bool MovieAtom::IsMovieExtendsAtomPresent() const
{
    return !!m_movieExtendsAtom;
}

const MovieExtendsAtom* MovieAtom::GetMovieExtendsAtom() const
{
    return m_movieExtendsAtom.get();
}

void MovieAtom::AddMovieExtendsAtom(UniquePtr<MovieExtendsAtom> movieExtendsAtom)
{
    m_movieExtendsAtom = std::move(movieExtendsAtom);
}

// @todo Implement support for MovieAtom-level MetaAtom
bool MovieAtom::IsMetaAtomPresent() const
{
    return false;
}

void MovieAtom::ToStream(Stream& str)
{
    WriteAtomHeader(str);

    m_movieHeaderAtom.ToStream(str);

    for (auto& track : m_tracks)
    {
        track->ToStream(str);
    }

    if (m_movieExtendsAtom)
    {
        m_movieExtendsAtom->ToStream(str);
    }

    UpdateSize(str);
}

void MovieAtom::FromStream(Stream& str)
{
    ParseAtomHeader(str);

    while (str.BytesRemain() > 0)
    {
        FourCCInt AtomType;
        Stream subBitstr = str.ReadSubAtomStream(AtomType);

        if (AtomType == "mvhd")
        {
            m_movieHeaderAtom.FromStream(subBitstr);
        }
        else if (AtomType == "trak")
        {
            UniquePtr<TrackAtom> trackAtom(new TrackAtom());
            trackAtom->FromStream(subBitstr);
            // Ignore Atom if the handler type is not video, audio or metadata
            FourCCInt handlerType = trackAtom->GetMediaAtom().GetHandlerAtom().GetHandlerType();
            if (handlerType == "vide" || handlerType == "soun" || handlerType == "meta")
            {
                m_tracks.push_back(move(trackAtom));
            }
        }
        else if (AtomType == "mvex")
        {
            m_movieExtendsAtom = MakeUnique<MovieExtendsAtom, MovieExtendsAtom>();
            m_movieExtendsAtom->FromStream(subBitstr);
        }
        else
        {
			char type[4];
            AtomType.GetString().copy(type, 4, 0);
            ISO_LOG(LOG_WARNING, "Skipping an unsupported Atom '%s' inside movie Atom.\n", type);
        }
    }
}

VCD_MP4_END
