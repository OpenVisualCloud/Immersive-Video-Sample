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
//! \file:   ItemDataAtom.cpp
//! \brief:  ItemDataAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "ItemDataAtom.h"

#include <cstring>

VCD_MP4_BEGIN

ItemDataAtom::ItemDataAtom()
    : Atom("idat")
    , m_data()
{
}

bool ItemDataAtom::Read(std::vector<std::uint8_t>& destination, const std::uint64_t offset, const std::uint64_t length) const
{
    if ((offset + length) > m_data.size())
    {
        return false;
    }

    destination.insert(destination.end(), m_data.cbegin() + static_cast<int64_t>(offset),
                       m_data.cbegin() + static_cast<int64_t>(offset + length));
    return true;
}

bool ItemDataAtom::Read(uint8_t* destination, const std::uint64_t offset, const std::uint64_t length) const
{
    if ((offset + length) > m_data.size() || destination == nullptr)
    {
        return false;
    }

    std::memcpy(destination, m_data.data() + static_cast<int64_t>(offset), length);
    return true;
}

std::uint64_t ItemDataAtom::AddData(const std::vector<std::uint8_t>& data)
{
    const std::uint64_t offset = m_data.size();
    m_data.insert(m_data.end(), data.cbegin(), data.cend());

    return offset;
}

void ItemDataAtom::ToStream(Stream& str)
{
    // Do not write an empty Atom at all
    if (m_data.size() == 0)
    {
        return;
    }

    WriteAtomHeader(str);
    str.WriteArray(m_data, m_data.size());
    UpdateSize(str);
}

void ItemDataAtom::FromStream(Stream& str)
{
    ParseAtomHeader(str);
    str.ReadArray(m_data, str.BytesRemain());
}

VCD_MP4_END