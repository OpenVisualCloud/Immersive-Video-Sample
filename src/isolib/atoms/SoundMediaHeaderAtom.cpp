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
//! \file:   SoundMediaHeaderAtom.cpp
//! \brief:  SoundMediaHeaderAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "SoundMediaHeaderAtom.h"

VCD_MP4_BEGIN

SoundMediaHeaderAtom::SoundMediaHeaderAtom()
    : FullAtom("smhd", 0, 0)
    , m_balance(0)
{
}

void SoundMediaHeaderAtom::SetBalance(const std::uint16_t balance)
{
    m_balance = balance;
}

std::uint16_t SoundMediaHeaderAtom::GetBalance() const
{
    return m_balance;
}

void SoundMediaHeaderAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);

    str.Write16(m_balance);  // Balance
    str.Write16(0);         // Reserved

    UpdateSize(str);
}

void SoundMediaHeaderAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);

    m_balance = str.Read16();  // Balance
    str.Read16();             // Reserved
}

VCD_MP4_END