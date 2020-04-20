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
//! \file:   PrimaryItemAtom.cpp
//! \brief:  PrimaryItemAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "PrimaryItemAtom.h"

VCD_MP4_BEGIN

PrimaryItemAtom::PrimaryItemAtom()
    : FullAtom("pitm", 0, 0)
    , m_itemId(0)
{
}

void PrimaryItemAtom::SetItemId(uint32_t itemId)
{
    m_itemId = itemId;
}

uint32_t PrimaryItemAtom::GetItemId() const
{
    return m_itemId;
}

void PrimaryItemAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);
    if (GetVersion() == 0)
    {
        str.Write16(static_cast<std::uint16_t>(m_itemId));
    }
    else
    {
        str.Write32(m_itemId);
    }
    UpdateSize(str);
}

void PrimaryItemAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);
    if (GetVersion() == 0)
    {
        m_itemId = str.Read16();
    }
    else
    {
        m_itemId = str.Read32();
    }
}

VCD_MP4_END