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
//! \file:   TrackGroupAtom.cpp
//! \brief:  TrackGroupAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "TrackGroupAtom.h"

VCD_MP4_BEGIN

TrackGroupAtom::TrackGroupAtom()
    : Atom("trgr")
{
}

const std::vector<TrackGroupTypeAtom>& TrackGroupAtom::GetTrackGroupTypeAtoms() const
{
    return m_trackGroupTypeAtoms;
}

void TrackGroupAtom::AddTrackGroupTypeAtom(const TrackGroupTypeAtom& trackGroupTypeAtom)
{
    m_trackGroupTypeAtoms.push_back(trackGroupTypeAtom);
}

void TrackGroupAtom::ToStream(Stream& str)
{
    WriteAtomHeader(str);

    for (unsigned int i = 0; i < m_trackGroupTypeAtoms.size(); i++)
    {
        m_trackGroupTypeAtoms.at(i).ToStream(str);
    }

    // Update the size of the movie Atom
    UpdateSize(str);
}

void TrackGroupAtom::FromStream(Stream& str)
{
    ParseAtomHeader(str);
    while (str.BytesRemain() > 0)
    {
        FourCCInt AtomType;
        Stream subBitstr = str.ReadSubAtomStream(AtomType);

        TrackGroupTypeAtom tracktypeAtom = TrackGroupTypeAtom(AtomType);
        tracktypeAtom.FromStream(subBitstr);
        m_trackGroupTypeAtoms.push_back(tracktypeAtom);
    }
}

VCD_MP4_END