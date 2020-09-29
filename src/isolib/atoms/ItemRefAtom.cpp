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
//! \file:   ItemRefAtom.cpp
//! \brief:  ItemRefAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "ItemRefAtom.h"
#include "Stream.h"

#include <algorithm>
#include <limits>
#include <list>
#include <stdexcept>

VCD_MP4_BEGIN

SingleItemTypeReferenceAtom::SingleItemTypeReferenceAtom(bool isLarge)
    : Atom(FourCCInt())
    , m_fromItemId(0)
    , m_isLarge(isLarge)
{
}

void SingleItemTypeReferenceAtom::SetReferenceType(FourCCInt referenceType)
{
    Atom::SetType(referenceType);
}

void SingleItemTypeReferenceAtom::AddToItemID(const uint32_t itemID)
{
    m_toItemIds.push_back(itemID);
}

void SingleItemTypeReferenceAtom::ClearToItemIDs()
{
    m_toItemIds.clear();
}

void SingleItemTypeReferenceAtom::ToStream(Stream& str)
{
    WriteAtomHeader(str);  // parent Atom

    if (m_isLarge)
    {
        str.Write32(m_fromItemId);
    }
    else
    {
        str.Write16(static_cast<std::uint16_t>(m_fromItemId));
    }

    str.Write16(static_cast<std::uint16_t>(m_toItemIds.size()));
    for (const auto i : m_toItemIds)
    {
        if (m_isLarge)
        {
            str.Write32(i);
        }
        else
        {
            str.Write16(static_cast<std::uint16_t>(i));
        }
    }

    UpdateSize(str);
}

std::vector<uint32_t> SingleItemTypeReferenceAtom::GetToItemIds() const
{
    return m_toItemIds;
}

ItemReferenceAtom::ItemReferenceAtom()
    : FullAtom("iref", 0, 0)
    , m_refList()
{
}

void ItemReferenceAtom::AddItemRef(const SingleItemTypeReferenceAtom& ref)
{
    m_refList.push_back(ref);
}

void ItemReferenceAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);  // parent Atom

    for (auto& i : m_refList)
    {
        i.ToStream(str);
    }

    UpdateSize(str);
}

void ItemReferenceAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);

    const bool largeIds = GetVersion() ? true : false;

    while (str.BytesRemain() > 0)
    {
        SingleItemTypeReferenceAtom singleRef(largeIds);
        singleRef.FromStream(str);
        AddItemRef(singleRef);
    }
}

void SingleItemTypeReferenceAtom::FromStream(Stream& str)
{
    ParseAtomHeader(str);  // parent Atom

    if (m_isLarge)
    {
        m_fromItemId = str.Read32();
    }
    else
    {
        m_fromItemId = str.Read16();
    }
    const uint16_t referenceCount = str.Read16();
    for (unsigned int i = 0; i < referenceCount; ++i)
    {
        if (m_isLarge)
        {
            m_toItemIds.push_back(str.Read32());
        }
        else
        {
            m_toItemIds.push_back(str.Read16());
        }
    }
}

std::vector<SingleItemTypeReferenceAtom> ItemReferenceAtom::GetRefOfType(FourCCInt type) const
{
    std::vector<SingleItemTypeReferenceAtom> refList;
    for (const auto& ref : m_refList)
    {
        FourCCInt pType = ref.GetType();
        if (pType == type)
        {
            refList.push_back(ref);
        }
    }
    return refList;
}

void ItemReferenceAtom::AddItems(FourCCInt type, const std::uint32_t fromId, const std::uint32_t toId)
{
    const bool largeIds = GetVersion() ? true : false;
    if (((fromId > std::numeric_limits<std::uint16_t>::max()) || (toId > std::numeric_limits<std::uint16_t>::max())) &&
        !largeIds)
    {
        ISO_LOG(LOG_ERROR, "ItemReferenceAtom::Add can not add large item IDs to Atom version 0\n");
        throw Exception();
    }

    auto pRef =
        std::find_if(m_refList.begin(), m_refList.end(), [&](const SingleItemTypeReferenceAtom& entry) {
            return (entry.GetType() == type) && (entry.GetFromItemID() == fromId);
        });
    if (pRef != m_refList.end())
    {
        pRef->AddToItemID(toId);
    }
    else
    {
        // Add a new entry
        SingleItemTypeReferenceAtom ref(largeIds);
        ref.SetType(type);
        ref.SetFromItemID(fromId);
        ref.AddToItemID(toId);
        m_refList.push_back(ref);
    }
}

VCD_MP4_END