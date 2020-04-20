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
//! \file:   TrackGroupTypeAtom.cpp
//! \brief:  TrackGroupTypeAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "TrackGroupTypeAtom.h"

VCD_MP4_BEGIN

TrackGroupTypeAtom::TrackGroupTypeAtom(FourCCInt AtomType, std::uint32_t trackGroupId)
    : FullAtom(AtomType, 0, 0)
    , m_trackGroupId(trackGroupId)
{
}

std::uint32_t TrackGroupTypeAtom::GetTrackGroupId() const
{
    return m_trackGroupId;
}

void TrackGroupTypeAtom::SetTrackGroupId(std::uint32_t trackGroupId)
{
    m_trackGroupId = trackGroupId;
}

void TrackGroupTypeAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);
    str.Write32(m_trackGroupId);
    UpdateSize(str);
}

void TrackGroupTypeAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);
    m_trackGroupId = str.Read32();
    if (str.BytesRemain() && GetType() == "obsp")
    {
        if (str.BytesRemain() >= 4)
        {
            FourCCInt AtomType;
            Stream subStream = str.ReadSubAtomStream(AtomType);
        }
    }
}

VCD_MP4_END