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
//! \file:   MovieHeaderAtom.cpp
//! \brief:  MovieHeaderAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "MovieHeaderAtom.h"
#include <stdexcept>

VCD_MP4_BEGIN

static const int MATRIX_LENGTH = 9;

MovieHeaderAtom::MovieHeaderAtom()
    : FullAtom("mvhd", 0, 0)
    , m_creationTime(0)
    , m_modificationTime(0)
    , m_timeScale(0)
    , m_duration(0)
    , m_matrix({0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000})
    , m_nextTrackID(0)
{
}

void MovieHeaderAtom::ToStream(Stream& str)
{
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
        ISO_LOG(LOG_ERROR, "ToStream() supports only 'mvhd' version 0 and version 1\n");
        throw Exception();
    }
    str.Write32(0x00010000);  // Rate
    str.Write16(0x0100);      // Volume
    str.Write16(0);           // Reserved

    str.Write32(0);  // Reserved
    str.Write32(0);

    for (unsigned int i = 0; i < MATRIX_LENGTH; ++i)
    {
        str.Write32(static_cast<std::uint32_t>(m_matrix.at(i)));  // Matrix[9]
    }

    str.Write32(0);  // Predefined[6]
    str.Write32(0);
    str.Write32(0);
    str.Write32(0);
    str.Write32(0);
    str.Write32(0);

    str.Write32(m_nextTrackID);

    UpdateSize(str);
}

void MovieHeaderAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);
    if ((GetVersion() != 0) && (GetVersion() != 1))
    {
        ISO_LOG(LOG_ERROR, "FromStream() supports only 'mvhd' version 0 and version 1\n");
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
    m_timeScale = str.Read32();
    if (GetVersion() == 0)
    {
        m_duration = str.Read32();
    }
    else
    {
        m_duration = str.Read64();
    }
    str.Read32();  // Rate
    str.Read16();  // Volume
    str.Read16();  // Reserved

    str.Read32();  // Reserved
    str.Read32();

    m_matrix.clear();
    for (int i = 0; i < MATRIX_LENGTH; ++i)
    {
        m_matrix.push_back(static_cast<std::int32_t>(str.Read32()));  // Matrix[9]
    }

    str.Read32();  // Predefined[6]
    str.Read32();
    str.Read32();
    str.Read32();
    str.Read32();
    str.Read32();

    m_nextTrackID = str.Read32();
}

VCD_MP4_END