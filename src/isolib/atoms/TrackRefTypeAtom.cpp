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
//! \file:   TrackRefTypeAtom.cpp
//! \brief:  TrackRefTypeAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "TrackRefTypeAtom.h"


VCD_MP4_BEGIN

TrackReferenceTypeAtom::TrackReferenceTypeAtom(FourCCInt trefType)
    : Atom(trefType)
    , m_trackId()
{
}

void TrackReferenceTypeAtom::SetTrackIds(const VectorT<uint32_t>& trackId)
{
    m_trackId = trackId;
}

const VectorT<uint32_t>& TrackReferenceTypeAtom::GetTrackIds() const
{
    return m_trackId;
}

void TrackReferenceTypeAtom::ToStream(Stream& str)
{
    WriteAtomHeader(str);

    for (auto trackId : m_trackId)
    {
        str.Write32(trackId);
    }

    UpdateSize(str);
}

void TrackReferenceTypeAtom::FromStream(Stream& str)
{
    ParseAtomHeader(str);

    // if there a data available in the file
    while (str.BytesRemain() > 0)
    {
        m_trackId.push_back(str.Read32());
    }
}

VCD_MP4_END