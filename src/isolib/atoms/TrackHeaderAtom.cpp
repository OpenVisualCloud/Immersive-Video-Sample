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
//! \file:   TrackHeaderAtom.cpp
//! \brief:  TrackHeaderAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "TrackHeaderAtom.h"


VCD_MP4_BEGIN

TrackHeaderAtom::TrackHeaderAtom()
    : FullAtom("tkhd", 0, 0)
    , m_creationTime(0)
    , m_modificationTime(0)
    , m_trackID(0)
    , m_duration(0)
    , m_width(0)
    , m_height(0)
    , m_alternateGroup(0)
    , m_volume(0)
    , m_matrix({0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000})
{
}

void TrackHeaderAtom::ToStream(Stream& str)
{
    // Write Atom headers
    WriteFullAtomHeader(str);

    if (GetVersion() == 0)
    {
        str.Write32(static_cast<uint32_t>(m_creationTime));
        str.Write32(static_cast<uint32_t>(m_modificationTime));
        str.Write32(m_trackID);
        str.Write32(0);
        str.Write32(static_cast<uint32_t>(m_duration));
    }
    else if (GetVersion() == 1)
    {
        str.Write64(m_creationTime);
        str.Write64(m_modificationTime);
        str.Write32(m_trackID);
        str.Write32(0);
        str.Write64(m_duration);
    }
    else
    {
        ISO_LOG(LOG_ERROR, "ToStream() supports only 'tkhd' version 0 and version 1\n");
        throw Exception();
    }

    str.Write32(0);  // Reserved
    str.Write32(0);

    str.Write16(0);                // Layer
    str.Write16(m_alternateGroup);  // Alternate Group
    str.Write16(m_volume);          // Volume
    str.Write16(0);                // Reserved

    for (auto value : m_matrix)
    {
        str.Write32(static_cast<uint32_t>(value));
    }

    str.Write32(m_width);
    str.Write32(m_height);

    UpdateSize(str);
}

void TrackHeaderAtom::FromStream(Stream& str)
{
    //  First parse the Atom header
    ParseFullAtomHeader(str);
    if ((GetVersion() != 0) && (GetVersion() != 1))
    {
        ISO_LOG(LOG_ERROR, "FromStream() supports only 'tkhd' version 0 and version 1\n");
        throw Exception();
    }

    if (GetVersion() == 0)
    {
        m_creationTime     = str.Read32();
        m_modificationTime = str.Read32();
    }
    else
    {
        m_creationTime     = str.Read64();
        m_modificationTime = str.Read64();
    }
    m_trackID = str.Read32();
    str.Read32();
    if (GetVersion() == 0)
    {
        m_duration = str.Read32();
    }
    else
    {
        m_duration = str.Read64();
    }

    str.Read32();  // Reserved
    str.Read32();

    str.Read16();                    // Layer
    m_alternateGroup = str.Read16();  // Alternate Group
    m_volume         = str.Read16();  // Volume
    str.Read16();                    // Reserved

    m_matrix.clear();
    for (int n = 9; n > 0; n--)  // Matrix[9]
    {
        m_matrix.push_back(static_cast<int32_t>(str.Read32()));
    }

    m_width  = str.Read32();
    m_height = str.Read32();
}

VCD_MP4_END