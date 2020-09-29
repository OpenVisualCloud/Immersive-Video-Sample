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
//! \file:   VisualSampEntryAtom.cpp
//! \brief:  VisualSampEntryAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "VisualSampEntryAtom.h"
#include "CleanApertureAtom.h"


#include <cassert>
#include <cstring>
#include <stdexcept>

VCD_MP4_BEGIN

/// Length of compressorname string in VisualSampleEntry class
static const unsigned int COMPRESSORNAME_STRING_LENGTH = 31;

VisualSampleEntryAtom::VisualSampleEntryAtom(FourCCInt codingName, const std::string& compressorName)
    : SampleEntryAtom(codingName)
    , m_width(0)
    , m_height(0)
    , m_compressorName(compressorName)
    , m_clap(nullptr)
{
    assert(m_compressorName.length() <= COMPRESSORNAME_STRING_LENGTH);
    m_compressorName.resize(COMPRESSORNAME_STRING_LENGTH, '\0');  // Make fixed length
}

VisualSampleEntryAtom::VisualSampleEntryAtom(const VisualSampleEntryAtom& Atom)
    : SampleEntryAtom(Atom)
    , m_width(Atom.m_width)
    , m_height(Atom.m_height)
    , m_compressorName(Atom.m_compressorName)
    , m_clap(Atom.m_clap)
{
    assert(m_compressorName.length() <= COMPRESSORNAME_STRING_LENGTH);
    m_compressorName.resize(COMPRESSORNAME_STRING_LENGTH, '\0');  // Make fixed length
}

void VisualSampleEntryAtom::CreateClap()
{
    if (m_clap.get() == nullptr)
    {
        m_clap = MakeShared<CleanApertureAtom>();
    }
}

const CleanApertureAtom* VisualSampleEntryAtom::GetClap() const
{
    return m_clap.get();
}

CleanApertureAtom* VisualSampleEntryAtom::GetClap()
{
    return m_clap.get();
}

void VisualSampleEntryAtom::ToStream(Stream& str)
{
    SampleEntryAtom::ToStream(str);

    str.Write16(0);           // pre_defined = 0
    str.Write16(0);           // reserved = 0
    str.Write32(0);           // reserved = 0
    str.Write32(0);           // reserved = 0
    str.Write32(0);           // reserved = 0
    str.Write16(m_width);      // width
    str.Write16(m_height);     // height
    str.Write32(0x00480000);  // horizresolution 72 dpi
    str.Write32(0x00480000);  // vertresolution 72 dpi
    str.Write32(0);           // reserved = 0
    str.Write16(1);           // frame_count = 1

    assert(m_compressorName.length() <= COMPRESSORNAME_STRING_LENGTH);

    // Length-byte prefixed compressor name padded to 32 bytes total.
    str.Write8(static_cast<std::uint8_t>(m_compressorName.length()));
    m_compressorName.resize(COMPRESSORNAME_STRING_LENGTH, '\0');  // Make fixed length
    str.WriteString(m_compressorName);                         // Write entire string buffer, including padding zeros

    str.Write16(0x0018);                              // depth
    str.Write16(static_cast<uint16_t>(int16_t(-1)));  // pre_defined

    auto rinfAtom = GetRestrictedSchemeInfoAtom();
    if (rinfAtom != nullptr)
    {
        str.SetByte(4, 'r');
        str.SetByte(5, 'e');
        str.SetByte(6, 's');
        str.SetByte(7, 'v');

        rinfAtom->ToStream(str);
    }

    // Update the size of the movie Atom
    UpdateSize(str);
}

void VisualSampleEntryAtom::FromStream(Stream& str)
{
    SampleEntryAtom::FromStream(str);

    str.Read16();            // pre_defined
    str.Read16();            // reserved
    str.Read32();            // predefined
    str.Read32();            // predefined
    str.Read32();            // predefined
    m_width  = str.Read16();  // width
    m_height = str.Read16();  // height
    str.Read32();            // horizontal resolution
    str.Read32();            // vertical resolution
    str.Read32();            // reserved
    str.Read16();            // frame_count

    const uint8_t compressorNameLength = str.Read8();
    if (compressorNameLength > COMPRESSORNAME_STRING_LENGTH)
    {
        ISO_LOG(LOG_ERROR, "Too long compressorname string length read from VisualSampleEntry (>31 bytes).\n");
        throw Exception();
    }
    std::string codecName;
    str.ReadStringWithLen(codecName, compressorNameLength);  // compressor name
    for (unsigned int i = compressorNameLength; i < COMPRESSORNAME_STRING_LENGTH; ++i)
    {
        str.Read8();  // discard padding
    }

    str.Read16();  // depth
    str.Read16();  // pre_defined

    // Read the optional clap Atom, if present
    if (str.BytesRemain() > 8)
    {
        const uint64_t startOffset = str.GetPos();
        FourCCInt AtomType;
        Stream subStream = str.ReadSubAtomStream(AtomType);
        if (AtomType == "clap")
        {
            const auto clap = MakeShared<CleanApertureAtom>();
            clap->FromStream(subStream);
            m_clap = clap;
        }
        else
        {
            // It was not 'clap', so the contained Atom probably belongs to the Atom Atom extending VisualSampleEntryAtom.
            // Reset bitstream position so it will be possible to read the whole extending Atom.
            str.SetPos(startOffset);
        }
    }
}

bool VisualSampleEntryAtom::IsVisual() const
{
    return true;
}

VCD_MP4_END
