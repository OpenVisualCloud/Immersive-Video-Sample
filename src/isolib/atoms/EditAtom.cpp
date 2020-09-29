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
//! \file:   EditAtom.cpp
//! \brief:  EditAtom class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!
#include "EditAtom.h"

#include <stdexcept>

VCD_MP4_BEGIN

EditAtom::EditAtom()
    : Atom("edts")
    , m_editListAtom(nullptr)
{
}

void EditAtom::ToStream(Stream& str)
{
    WriteAtomHeader(str);
    if (m_editListAtom != nullptr)
    {
        m_editListAtom->ToStream(str);
    }
    UpdateSize(str);
}

void EditAtom::FromStream(Stream& str)
{
    ParseAtomHeader(str);

    // if there a data available in the file
    while (str.BytesRemain() > 0)
    {
        // Extract contained Atom bitstream and type
        FourCCInt AtomType;
        Stream subBitstr = str.ReadSubAtomStream(AtomType);

        // Handle this Atom based on the type
        if (AtomType == "elst")
        {
            m_editListAtom = MakeShared<EditListAtom>();
            m_editListAtom->FromStream(subBitstr);
        }
    }
}

void EditAtom::SetEditListAtom(std::shared_ptr<EditListAtom> editListAtom)
{
    m_editListAtom = std::move(editListAtom);
}

const EditListAtom* EditAtom::GetEditListAtom() const
{
    return m_editListAtom.get();
}

EditListAtom::EditListAtom()
    : FullAtom("elst", 0, 0)
{
}

void EditListAtom::AddEntry(const EntryVersion0& entry)
{
    if (m_entryVersion1.size() != 0 || GetVersion() != 0)
    {
        ISO_LOG(LOG_ERROR, "Invalid attempt to add version0 EditListAtom entries.\n");
        throw Exception();
    }
    m_entryVersion0.push_back(entry);
}

void EditListAtom::AddEntry(const EntryVersion1& entry)
{
    if (m_entryVersion0.size() != 0 || GetVersion() != 1)
    {
        ISO_LOG(LOG_ERROR, "Invalid attempt to add version1 EditListAtom entries.\n");
        throw Exception();
    }
    m_entryVersion1.push_back(entry);
}

std::uint32_t EditListAtom::numEntry() const
{
    size_t size = 0;
    switch (GetVersion())
    {
    case 0:
        size = m_entryVersion0.size();
        break;
    case 1:
        size = m_entryVersion1.size();
        break;
    default:
        ISO_LOG(LOG_ERROR, "Not supported EditListAtom entry version (only 0 and 1 are supported).\n");
        throw Exception();
    }
    return static_cast<std::uint32_t>(size);
}

void EditListAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);
    if (m_entryVersion0.empty() == false)
    {
        str.Write32(static_cast<std::uint32_t>(m_entryVersion0.size()));
        for (const auto& entry : m_entryVersion0)
        {
            str.Write32(entry.m_segDuration);
            str.Write32(static_cast<unsigned int>(entry.m_mediaTime));
            str.Write16(entry.m_mediaRateInt);
            str.Write16(entry.m_mediaRateFraction);
        }
    }
    else if (m_entryVersion1.empty() == false)
    {
        str.Write32(static_cast<std::uint32_t>(m_entryVersion1.size()));
        for (const auto& entry : m_entryVersion0)
        {
            str.Write64(entry.m_segDuration);
            str.Write64(static_cast<std::uint64_t>(entry.m_mediaTime));
            str.Write16(entry.m_mediaRateInt);
            str.Write16(entry.m_mediaRateFraction);
        }
    }
    UpdateSize(str);
}

void EditListAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);

    const std::uint32_t entryCount = str.Read32();

    if (GetVersion() == 0)
    {
        for (std::uint32_t i = 0; i < entryCount; ++i)
        {
            EntryVersion0 entryVersion0;
            entryVersion0.m_segDuration   = str.Read32();
            entryVersion0.m_mediaTime     = static_cast<std::int32_t>(str.Read32());
            entryVersion0.m_mediaRateInt  = str.Read16();
            entryVersion0.m_mediaRateFraction = str.Read16();
            m_entryVersion0.push_back(entryVersion0);
        }
    }
    else if (GetVersion() == 1)
    {
        for (uint32_t i = 0; i < entryCount; ++i)
        {
            EntryVersion1 entryVersion1;
            entryVersion1.m_segDuration   = str.Read64();
            entryVersion1.m_mediaTime         = static_cast<std::int64_t>(str.Read64());
            entryVersion1.m_mediaRateInt  = str.Read16();
            entryVersion1.m_mediaRateFraction = str.Read16();
            m_entryVersion1.push_back(entryVersion1);
        }
    }
}

VCD_MP4_END