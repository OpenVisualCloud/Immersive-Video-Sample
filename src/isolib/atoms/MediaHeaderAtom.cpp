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
//! \file:   MediaHeaderAtom.cpp
//! \brief:  MediaHeaderAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "MediaHeaderAtom.h"


VCD_MP4_BEGIN

MediaHeaderAtom::MediaHeaderAtom()
    : FullAtom("mdhd", 0, 0)
    , m_creationTime(0)
    , m_modificationTime(0)
    , m_timeScale(0)
    , m_duration(0)
    , m_language(0)
{
}

void MediaHeaderAtom::ToStream(Stream& str)
{
    // Write Atom headers
    WriteFullAtomHeader(str);

    if (GetVersion() == 0)
    {
        str.Write32(static_cast<uint32_t>(m_creationTime));
        str.Write32(static_cast<uint32_t>(m_modificationTime));
        str.Write32(m_timeScale);
        str.Write32(static_cast<uint32_t>(m_duration));
    }
    else if (GetVersion() == 1)
    {
        str.Write64(m_creationTime);
        str.Write64(m_modificationTime);
        str.Write32(m_timeScale);
        str.Write64(m_duration);
    }
    else
    {
        ISO_LOG(LOG_ERROR, "MediaHeaderAtom::ToStream() supports only 'mdhd' version 0 and version 1\n");
        throw Exception();
    }

    str.Write16(0);  // Pad, Langauge
    str.Write16(0);  // Predefined

    // Update the size of the movie Atom
    UpdateSize(str);
}

void MediaHeaderAtom::FromStream(Stream& str)
{
    Stream subBitstr;

    //  First parse the Atom header
    ParseFullAtomHeader(str);
    if ((GetVersion() != 0) && (GetVersion() != 1))
    {
        ISO_LOG(LOG_ERROR, "MediaHeaderAtom::FromStream() supports only 'mdhd' version 0 and version 1\n");
        throw Exception();
    }
    uint8_t pVersion = GetVersion();
    if (pVersion == 0)
    {
        m_creationTime     = str.Read32();
        m_modificationTime = str.Read32();
    }
    else
    {
        m_creationTime     = str.Read64();
        m_modificationTime = str.Read64();
    }
    m_timeScale = str.Read32();
    if (pVersion == 0)
    {
        m_duration = str.Read32();
    }
    else
    {
        m_duration = str.Read64();
    }

    str.Read16();  // Pad, Langauge
    str.Read16();  // Predefined
}

VCD_MP4_END