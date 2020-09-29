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
//! \file:   MovieFragAtom.cpp
//! \brief:  MovieFragAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "MovieFragAtom.h"

VCD_MP4_BEGIN

MovieFragmentAtom::MovieFragmentAtom(std::vector<SampleDefaults>& sampleDefaults)
    : Atom("moof")
    , m_movieFragmentHeaderAtom()
    , m_trackFragmentAtoms()
    , m_sampleDefaults(sampleDefaults)
    , m_firstByteOffset(0)
{
}

MovieFragmentHeaderAtom& MovieFragmentAtom::GetMovieFragmentHeaderAtom()
{
    return m_movieFragmentHeaderAtom;
}

void MovieFragmentAtom::AddTrackFragmentAtom(UniquePtr<TrackFragmentAtom> trackFragmentAtom)
{
    m_trackFragmentAtoms.push_back(std::move(trackFragmentAtom));
}

std::vector<TrackFragmentAtom*> MovieFragmentAtom::GetTrackFragmentAtoms()
{
    std::vector<TrackFragmentAtom*> trackFragmentAtoms;
    for (auto& trackFragmentAtom : m_trackFragmentAtoms)
    {
        trackFragmentAtoms.push_back(trackFragmentAtom.get());
    }
    return trackFragmentAtoms;
}

void MovieFragmentAtom::SetMoofFirstByteOffset(std::uint64_t moofFirstByteOffset)
{
    m_firstByteOffset = moofFirstByteOffset;
}

std::uint64_t MovieFragmentAtom::GetMoofFirstByteOffset()
{
    return m_firstByteOffset;
}

void MovieFragmentAtom::ToStream(Stream& str)
{
    WriteAtomHeader(str);
    m_movieFragmentHeaderAtom.ToStream(str);
    for (auto& trackFragmentAtom : m_trackFragmentAtoms)
    {
        trackFragmentAtom->ToStream(str);
    }
    UpdateSize(str);
}

void MovieFragmentAtom::FromStream(Stream& str)
{
    ParseAtomHeader(str);

    while (str.BytesRemain() > 0)
    {
        FourCCInt AtomType;
        Stream subBitstr = str.ReadSubAtomStream(AtomType);

        if (AtomType == "mfhd")
        {
            m_movieFragmentHeaderAtom.FromStream(subBitstr);
        }
        else if (AtomType == "traf")
        {
            UniquePtr<TrackFragmentAtom> trackFragmentAtom(new TrackFragmentAtom(m_sampleDefaults));
            trackFragmentAtom->FromStream(subBitstr);
            m_trackFragmentAtoms.push_back(std::move(trackFragmentAtom));
        }
        else
        {
			char type[4];
            AtomType.GetString().copy(type, 4, 0);
            ISO_LOG(LOG_WARNING, "Skipping an unsupported Atom '%s' inside MovieFragmentAtom.\n", type);
        }
    }
}

VCD_MP4_END
