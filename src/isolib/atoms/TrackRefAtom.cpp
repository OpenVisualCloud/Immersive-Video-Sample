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
//! \file:   TrackRefAtom.cpp
//! \brief:  TrackRefAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "TrackRefAtom.h"

VCD_MP4_BEGIN

TrackReferenceAtom::TrackReferenceAtom()
    : Atom("tref")
    , m_trefTypeAtoms()
{
}

void TrackReferenceAtom::ClearAtoms()
{
    m_trefTypeAtoms.clear();
}

void TrackReferenceAtom::AddAtom(TrackReferenceTypeAtom& trefTypeAtom)
{
    m_trefTypeAtoms.push_back(trefTypeAtom);
}

const std::vector<TrackReferenceTypeAtom>& TrackReferenceAtom::GetTypeAtoms() const
{
    return m_trefTypeAtoms;
}

bool TrackReferenceAtom::IsRefTypePresent(FourCCInt type) const
{
    for (const auto& trackReferenceTypeAtom : m_trefTypeAtoms)
    {
        if (trackReferenceTypeAtom.GetType() == type)
        {
            return true;
        }
    }

    return false;
}

void TrackReferenceAtom::ToStream(Stream& str)
{
    // Write Atom headers
    WriteAtomHeader(str);

    // For each track reference type call its writeAtom method
    for (auto& trefTypeAtom : m_trefTypeAtoms)
    {
        trefTypeAtom.ToStream(str);
    }

    UpdateSize(str);
}

void TrackReferenceAtom::FromStream(Stream& str)
{
    ParseAtomHeader(str);

    // Read sub-Atoms until no data is left in this Atom
    while (str.BytesRemain() > 0)
    {
        // Extract the bitstream content of this Atom
        FourCCInt AtomType;
        Stream subBitstr               = str.ReadSubAtomStream(AtomType);
        TrackReferenceTypeAtom trefTypeAtom = TrackReferenceTypeAtom(AtomType);
        trefTypeAtom.FromStream(subBitstr);

        m_trefTypeAtoms.push_back(trefTypeAtom);
    }
}

VCD_MP4_END