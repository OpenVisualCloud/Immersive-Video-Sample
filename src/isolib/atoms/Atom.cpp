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
//! \file:   Atom.cpp
//! \brief:  Atom class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include "Atom.h"
#include <limits>
#include "Stream.h"

VCD_MP4_BEGIN

Atom::Atom(FourCCInt AtomType)
    : m_size(8)
    , m_type(AtomType)
    , m_userType()
    , m_startLocation(0)
    , m_largeSize(false)
{
}

void Atom::SetLargeSize()
{
    m_largeSize = true;
}

bool Atom::GetLargeSize() const
{
    return m_largeSize;
}

void Atom::WriteAtomHeader(Stream& str) const
{
    m_startLocation = str.GetSize();

    if (m_largeSize)
    {
        str.Write32(1);
    }
    else
    {
        str.Write32(static_cast<uint32_t>(m_size));
    }

    str.Write32(m_type.GetUInt32());

    // Note that serialized size values will be dummy values until UpdateSize() is called.
    if (m_largeSize)
    {
        str.Write64(m_size);
    }

    if (m_type == "uuid")
    {
        str.WriteArray(m_userType, 16);
    }
}

void Atom::UpdateSize(Stream& str) const
{
    m_size = str.GetSize() - m_startLocation;

    if ((m_size > std::numeric_limits<std::uint32_t>::max()) && (m_largeSize == false))
    {
        ISO_LOG(LOG_ERROR, "Atom::UpdateSize(): Atom size exceeds 4GB but large size for 64-bit size field was not set.\n");
        throw Exception();
    }

    // Write updated size to the bitstream.
    if (m_largeSize)
    {
        str.SetByte(m_startLocation + 8, (m_size >> 56) & 0xff);
        str.SetByte(m_startLocation + 9, (m_size >> 48) & 0xff);
        str.SetByte(m_startLocation + 10, (m_size >> 40) & 0xff);
        str.SetByte(m_startLocation + 11, (m_size >> 32) & 0xff);
        str.SetByte(m_startLocation + 12, (m_size >> 24) & 0xff);
        str.SetByte(m_startLocation + 13, (m_size >> 16) & 0xff);
        str.SetByte(m_startLocation + 14, (m_size >> 8) & 0xff);
        str.SetByte(m_startLocation + 15, m_size & 0xff);
    }
    else
    {
        str.SetByte(m_startLocation + 0, (m_size >> 24) & 0xff);
        str.SetByte(m_startLocation + 1, (m_size >> 16) & 0xff);
        str.SetByte(m_startLocation + 2, (m_size >> 8) & 0xff);
        str.SetByte(m_startLocation + 3, m_size & 0xff);
    }
}

void Atom::ParseAtomHeader(Stream& str)
{
    m_size = str.Read32();
    m_type = str.Read32();

    if (m_size == 1)
    {
        m_size      = str.Read64();
        m_largeSize = true;
    }

    if (m_type == "uuid")
    {
        m_userType.clear();
        for (uint8_t i = 0; i < 16; ++i)
        {
            m_userType.push_back(str.Read8());
        }
    }
}

VCD_MP4_END