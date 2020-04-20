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
//! \file:   FullAtom.cpp
//! \brief:  FullAtom class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!
#include "FullAtom.h"

VCD_MP4_BEGIN

FullAtom::FullAtom(FourCCInt AtomType, uint8_t version, uint32_t flags)
    : Atom(AtomType)
    , m_version(version)
    , m_flags(flags)
{
}

void FullAtom::SetVersion(uint8_t version)
{
    m_version = version;
}

uint8_t FullAtom::GetVersion() const
{
    return m_version;
}

void FullAtom::SetFlags(uint32_t flags)
{
    m_flags = flags;
}

uint32_t FullAtom::GetFlags() const
{
    return m_flags;
}

void FullAtom::WriteFullAtomHeader(Stream& str)
{
    WriteAtomHeader(str);

    str.Write8(m_version);
    str.Write24(m_flags);
}

void FullAtom::ParseFullAtomHeader(Stream& str)
{
    ParseAtomHeader(str);

    m_version = str.Read8();
    m_flags   = str.Read24();
}

VCD_MP4_END