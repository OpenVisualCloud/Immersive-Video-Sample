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
//! \file:   ItemProtAtom.cpp
//! \brief:  ItemProtAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "ItemProtAtom.h"

VCD_MP4_BEGIN

std::vector<std::uint8_t> ProtectionSchemeInfoAtom::GetData() const
{
    return m_data;
}

void ProtectionSchemeInfoAtom::SetData(const std::vector<std::uint8_t>& data)
{
    m_data = data;
}

void ProtectionSchemeInfoAtom::ToStream(Stream& str)
{
    str.WriteArray(m_data, m_data.size());
}

void ProtectionSchemeInfoAtom::FromStream(Stream& str)
{
    str.ReadArray(m_data, str.GetSize());
}

ItemProtectionAtom::ItemProtectionAtom()
    : FullAtom("ipro", 0, 0)
{
}

std::uint16_t ItemProtectionAtom::GetSize() const
{
    return static_cast<std::uint16_t>(m_protectionInformation.size());
}

const ProtectionSchemeInfoAtom& ItemProtectionAtom::GetEntry(const std::uint16_t index) const
{
    return m_protectionInformation.at(index);
}

std::uint16_t ItemProtectionAtom::AddEntry(const ProtectionSchemeInfoAtom& sinf)
{
    m_protectionInformation.push_back(sinf);
    return static_cast<std::uint16_t>(m_protectionInformation.size() - 1);
}

void ItemProtectionAtom::ToStream(Stream& str)
{
    if (m_protectionInformation.size() == 0)
    {
        return;
    }

    WriteFullAtomHeader(str);
    str.Write16(static_cast<std::uint16_t>(m_protectionInformation.size()));
    for (auto& Atom : m_protectionInformation)
    {
        Atom.ToStream(str);
    }
    UpdateSize(str);
}

void ItemProtectionAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);
    const unsigned int Atoms = str.Read16();
    for (unsigned int i = 0; i < Atoms; ++i)
    {
        FourCCInt AtomType;
        Stream subStream = str.ReadSubAtomStream(AtomType);
        ProtectionSchemeInfoAtom sinf;
        sinf.FromStream(subStream);
        m_protectionInformation.push_back(sinf);
    }
}

VCD_MP4_END