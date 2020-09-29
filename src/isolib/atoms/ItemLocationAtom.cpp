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
//! \file:   ItemLocationAtom.cpp
//! \brief:  ItemLocationAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "ItemLocationAtom.h"

#include <algorithm>
#include <stdexcept>

VCD_MP4_BEGIN

ItemLocation::ItemLocation()
    : m_itemID(0)
    , m_constructionMethod(ConstructType::FILE_OFFSET)
    , m_dataReferenceIndex(0)
    , m_baseOffset(0)
    , m_extentList()
{
}

std::uint16_t ItemLocation::GetExtentCount() const
{
    return static_cast<std::uint16_t>(m_extentList.size());
}

void ItemLocation::AddExtent(const ExtentParams& extent)
{
    m_extentList.push_back(extent);
}

const ExtentList& ItemLocation::GetExtentList() const
{
    return m_extentList;
}

ItemLocationAtom::ItemLocationAtom()
    : FullAtom("iloc", 0, 0)
    , m_offSetSize(4)
    , m_lengthSize(4)
    , m_baseOffSetSize(4)
    , m_indexSize(0)
    , m_itemLocations()
{
}

std::uint32_t ItemLocationAtom::GetItemCount() const
{
    return static_cast<std::uint32_t>(m_itemLocations.size());
}

void ItemLocationAtom::AddLocation(const ItemLocation& itemLoc)
{
    // Use version to 1 if needed
    if (itemLoc.GetConstructType() != ItemLocation::ConstructType::FILE_OFFSET)
    {
        SetVersion(1);
    }
    m_itemLocations.push_back(itemLoc);
}

void ItemLocationAtom::AddExtent(const std::uint32_t itemId, const ExtentParams& extent)
{
    const auto iter = findItem(itemId);
    if (iter == m_itemLocations.end())
    {
        ISO_LOG(LOG_ERROR, "ItemLocationAtom::AddExtent() invalid item id\n");
        throw Exception();
    }

    iter->AddExtent(extent);
}

ItemLocationVector& ItemLocationAtom::GetItemLocations()
{
    return m_itemLocations;
}

bool ItemLocationAtom::HasItemIdEntry(std::uint32_t itemId) const
{
    if (findItem(itemId) != m_itemLocations.cend())
    {
        return true;
    }
    return false;
}

bool ItemLocationAtom::SetItemDataReferenceIndex(const std::uint32_t itemId, const std::uint16_t dataReferenceIndex)
{
    const auto iter = findItem(itemId);
    if (iter != m_itemLocations.end())
    {
        iter->SetDataReferenceIndex(dataReferenceIndex);
        return true;
    }

    return false;
}

void ItemLocationAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);

    str.Write1(m_offSetSize, 4);
    str.Write1(m_lengthSize, 4);
    str.Write1(m_baseOffSetSize, 4);
    if ((GetVersion() == 1) || (GetVersion() == 2))
    {
        str.Write1(m_indexSize, 4);
    }
    else
    {
        str.Write1(0, 4);  // reserved = 0
    }
    if (GetVersion() < 2)
    {
        str.Write16(static_cast<std::uint16_t>(m_itemLocations.size()));
    }
    else if (GetVersion() == 2)
    {
        str.Write32(static_cast<unsigned int>(m_itemLocations.size()));
    }

    for (const auto& itemLoc : m_itemLocations)
    {
        if (GetVersion() < 2)
        {
            str.Write16(static_cast<std::uint16_t>(itemLoc.GetItemID()));
        }
        else if (GetVersion() == 2)
        {
            str.Write32(itemLoc.GetItemID());
        }

        if ((GetVersion() == 1) || (GetVersion() == 2))
        {
            str.Write1(0, 12);  // reserved = 0
            str.Write1(static_cast<unsigned int>(itemLoc.GetConstructType()), 4);
        }
        str.Write16(itemLoc.GetDataReferenceIndex());
        str.Write1(itemLoc.GetBaseOffset(), static_cast<unsigned int>(m_baseOffSetSize * 8));
        str.Write16(itemLoc.GetExtentCount());

        const ExtentList& extents = itemLoc.GetExtentList();
        for (const auto& locExt : extents)
        {
            if (((GetVersion() == 1) || (GetVersion() == 2)) && (m_indexSize > 0))
            {
                str.Write1(locExt.m_extentIndex, static_cast<unsigned int>(m_indexSize * 8));
            }
            str.Write1(locExt.m_extentOffset, static_cast<unsigned int>(m_offSetSize * 8));
            str.Write1(locExt.m_extentLen, static_cast<unsigned int>(m_lengthSize * 8));
        }
    }

    UpdateSize(str);
}

void ItemLocationAtom::FromStream(Stream& str)
{
    unsigned int itemCount = 0;

    ParseFullAtomHeader(str);

    m_offSetSize     = static_cast<uint8_t>(str.Read1(4));
    m_lengthSize     = static_cast<uint8_t>(str.Read1(4));
    m_baseOffSetSize = static_cast<uint8_t>(str.Read1(4));
    if ((GetVersion() == 1) || (GetVersion() == 2))
    {
        m_indexSize = static_cast<uint8_t>(str.Read1(4));
    }
    else
    {
        str.Read1(4);  // reserved = 0
    }

    if (GetVersion() < 2)
    {
        itemCount = str.Read16();
    }
    else if (GetVersion() == 2)
    {
        itemCount = str.Read32();
    }

    for (unsigned int i = 0; i < itemCount; i++)
    {
        ItemLocation itemLoc;
        if (GetVersion() < 2)
        {
            itemLoc.SetItemID(str.Read16());
        }
        else if (GetVersion() == 2)
        {
            itemLoc.SetItemID(str.Read32());
        }

        if ((GetVersion() == 1) || (GetVersion() == 2))
        {
            str.Read1(12);  // reserved = 0
            itemLoc.SetConstructType(static_cast<ItemLocation::ConstructType>(str.Read1(4)));
        }
        itemLoc.SetDataReferenceIndex(str.Read16());
        itemLoc.SetBaseOffset(str.Read1(static_cast<std::uint8_t>(m_baseOffSetSize * 8)));
        const unsigned int extentCount = str.Read16();
        for (unsigned int j = 0; j < extentCount; j++)
        {
            ExtentParams locExt;
            if (((GetVersion() == 1) || (GetVersion() == 2)) && (m_indexSize > 0))
            {
                locExt.m_extentIndex = str.Read1(static_cast<std::uint8_t>(m_indexSize * 8));
            }
            locExt.m_extentOffset = str.Read1(static_cast<std::uint8_t>(m_offSetSize * 8));
            locExt.m_extentLen = str.Read1(static_cast<std::uint8_t>(m_lengthSize * 8));
            itemLoc.AddExtent(locExt);
        }
        AddLocation(itemLoc);
    }
}

const ItemLocation& ItemLocationAtom::GetItemLocationForID(const unsigned int itemID) const
{
    const auto iter = findItem(itemID);
    if (iter != m_itemLocations.cend())
    {
        return *iter;
    }
    ISO_LOG(LOG_ERROR, "ItemLocationAtom::GetItemLocationForID: invalid item ID\n");
    throw Exception();
}

const ExtentParams& ItemLocation::GetExtent(const unsigned int i) const
{
    if (i >= m_extentList.size())
    {
        ISO_LOG(LOG_ERROR, "ItemLocationAtom::GetExtent: invalid extent ID\n");
        throw Exception();
    }
    else
    {
        return m_extentList.at(i);
    }
}

ItemLocationVector::const_iterator ItemLocationAtom::findItem(const std::uint32_t itemId) const
{
    ItemLocationVector::const_iterator iter =
        std::find_if(m_itemLocations.cbegin(), m_itemLocations.cend(),
                     [itemId](const ItemLocation& itemLocation) { return itemLocation.GetItemID() == itemId; });
    return iter;
}

ItemLocationVector::iterator ItemLocationAtom::findItem(const std::uint32_t itemId)
{
    ItemLocationVector::iterator iter =
        std::find_if(m_itemLocations.begin(), m_itemLocations.end(),
                     [itemId](const ItemLocation& itemLocation) { return itemLocation.GetItemID() == itemId; });
    return iter;
}

VCD_MP4_END