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
//! \file:   ItemInfoAtom.cpp
//! \brief:  ItemInfoAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "ItemInfoAtom.h"

#include <stdexcept>

VCD_MP4_BEGIN

ItemInfoAtom::ItemInfoAtom()
    : ItemInfoAtom(0)
{
}

ItemInfoAtom::ItemInfoAtom(const uint8_t version)
    : FullAtom("iinf", version, 0)
    , m_itemInfoList()
{
}

uint32_t ItemInfoAtom::GetEntryCount() const
{
    return static_cast<uint32_t>(m_itemInfoList.size());
}

std::vector<std::uint32_t> ItemInfoAtom::GetItemIds() const
{
    std::vector<std::uint32_t> itemIds;
    for (const auto& entry : m_itemInfoList)
    {
        itemIds.push_back(entry.GetItemID());
    }
    return itemIds;
}

void ItemInfoAtom::AddItemInfoEntry(const ItemInfoEntry& infoEntry)
{
    m_itemInfoList.push_back(infoEntry);
}

const ItemInfoEntry& ItemInfoAtom::GetItemInfoEntry(const std::uint32_t idx) const
{
    return m_itemInfoList.at(idx);
}

ItemInfoEntry ItemInfoAtom::GetItemById(const uint32_t itemId) const
{
    for (const auto& item : m_itemInfoList)
    {
        if (item.GetItemID() == itemId)
        {
            return item;
        }
    }
    ISO_LOG(LOG_ERROR, "Requested ItemInfoEntry not found.\n");
    throw Exception();
}

void ItemInfoAtom::Clear()
{
    m_itemInfoList.clear();
}

void ItemInfoAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);

    if (GetVersion() == 0)
    {
        str.Write16(static_cast<std::uint16_t>(m_itemInfoList.size()));
    }
    else
    {
        str.Write32(static_cast<unsigned int>(m_itemInfoList.size()));
    }

    for (auto& entry : m_itemInfoList)
    {
        entry.ToStream(str);
    }

    UpdateSize(str);
}

void ItemInfoAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);

    size_t cnt = 0;
    uint8_t version = GetVersion();
    if (version == 0)
    {
        cnt = str.Read16();
    }
    else
    {
        cnt = str.Read32();
    }
    for (size_t i = 0; i < cnt; ++i)
    {
        ItemInfoEntry infoEntry;
        infoEntry.FromStream(str);
        AddItemInfoEntry(infoEntry);
    }
}

unsigned int ItemInfoAtom::CountNumberOfItems(FourCCInt itemType)
{
    unsigned int cnt = 0;
    for (const auto& entry : m_itemInfoList)
    {
        FourCCInt type = entry.GetItemType();
        if (type == itemType)
        {
            ++cnt;
        }
    }
    return cnt;
}

// return item and its index for the specified itemType and itemID
ItemInfoEntry* ItemInfoAtom::FindItemWithTypeAndID(FourCCInt type, const unsigned int itemID, unsigned int& index)
{
    ItemInfoEntry* entry   = nullptr;
    unsigned int idx = 0;

    for (auto i = m_itemInfoList.begin(); i != m_itemInfoList.end(); ++i)
    {
        if (i->GetItemType() == type)
        {
            if (i->GetItemID() == itemID)
            {
                entry = &(*i);
                index = idx;
                break;
            }
            else
            {
                ++idx;
            }
        }
    }
    return entry;
}

ItemInfoEntry::ItemInfoEntry()
    : FullAtom("infe", 0, 0)
    , m_itemID(0)
    , m_itemProtectionIndex(0)
    , m_itemName()
    , m_contentType()
    , m_contentEncoding()
    , m_extensionType()
    , m_itemType()
    , m_itemUriType()
{
}

ItemInfoEntry::~ItemInfoEntry()
{
}

void ItemInfoEntry::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);

    if (GetVersion() == 0 || GetVersion() == 1)
    {
        str.Write16(static_cast<std::uint16_t>(m_itemID));
        str.Write16(m_itemProtectionIndex);
        str.WriteZeroEndString(m_itemName);
        str.WriteZeroEndString(m_contentType);
        str.WriteZeroEndString(m_contentEncoding);
    }
    if (GetVersion() == 1)
    {
        str.WriteString(m_extensionType);
        m_itemInfoExtension->ToStream(str);
    }
    if (GetVersion() >= 2)
    {
        if (GetVersion() == 2)
        {
            str.Write16(static_cast<std::uint16_t>(m_itemID));
        }
        else if (GetVersion() == 3)
        {
            str.Write32(m_itemID);
        }
        str.Write16(m_itemProtectionIndex);
        str.Write32(m_itemType.GetUInt32());
        str.WriteZeroEndString(m_itemName);
        if (m_itemType == "mime")
        {
            str.WriteZeroEndString(m_contentType);
            str.WriteZeroEndString(m_contentEncoding);
        }
        else if (m_itemType == "uri ")
        {
            str.WriteZeroEndString(m_itemUriType);
        }
    }

    UpdateSize(str);
}

void FDItemInfoExtension::ToStream(Stream& str)
{
    str.WriteZeroEndString(m_contentLocation);
    str.WriteZeroEndString(m_contentMD5);
    str.Write32((uint32_t)((m_contentLength >> 32) & 0xffffffff));
    str.Write32((uint32_t)(m_contentLength & 0xffffffff));
    str.Write32((uint32_t)((m_transferLength >> 32) & 0xffffffff));
    str.Write32((uint32_t)(m_transferLength & 0xffffffff));
    str.Write8(m_entryCount);
    for (unsigned int i = 0; i < m_entryCount; i++)
    {
        str.Write32(m_groupID.at(i));
    }
}

void ItemInfoEntry::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);

    if (GetVersion() == 0 || GetVersion() == 1)
    {
        m_itemID              = str.Read16();
        m_itemProtectionIndex = str.Read16();
        str.ReadZeroEndString(m_itemName);
        str.ReadZeroEndString(m_contentType);
        if (str.BytesRemain() > 0)  // This is an optional field
        {
            str.ReadZeroEndString(m_contentEncoding);
        }
    }
    if (GetVersion() == 1)
    {
        if (str.BytesRemain() > 0)  // This is an optional field
        {
            str.ReadStringWithLen(m_extensionType, 4);
        }
        if (str.BytesRemain() > 0)  // This is an optional field
        {
            FDItemInfoExtension* itemInfoExt = new FDItemInfoExtension();
            m_itemInfoExtension.reset(itemInfoExt);
            itemInfoExt->FromStream(str);
        }
    }
    if (GetVersion() >= 2)
    {
        if (GetVersion() == 2)
        {
            m_itemID = str.Read16();
        }
        else if (GetVersion() == 3)
        {
            m_itemID = str.Read32();
        }
        m_itemProtectionIndex = str.Read16();
        m_itemType            = str.Read32();
        str.ReadZeroEndString(m_itemName);
        if (m_itemType == "mime")
        {
            str.ReadZeroEndString(m_contentType);
            if (str.BytesRemain() > 0)  // This is an optional field
            {
                str.ReadZeroEndString(m_contentEncoding);
            }
        }
        else if (m_itemType == "uri ")
        {
            str.ReadZeroEndString(m_itemUriType);
        }
    }
}

void FDItemInfoExtension::FromStream(Stream& str)
{
    str.ReadZeroEndString(m_contentLocation);
    str.ReadZeroEndString(m_contentMD5);
    m_contentLength = ((uint64_t) str.Read32()) << 32;
    m_contentLength += str.Read32();
    m_transferLength = ((uint64_t) str.Read32()) << 32;
    m_transferLength += str.Read32();
    m_entryCount = str.Read8();
    for (unsigned int i = 0; i < m_entryCount; i++)
    {
        m_groupID.at(i) = str.Read32();
    }
}

ItemInfoEntry* ItemInfoAtom::GetItemsNumber(FourCCInt itemType, const unsigned int index)
{
    ItemInfoEntry* entry   = nullptr;
    unsigned int currIndex = 0;

    for (auto i = m_itemInfoList.begin(); i != m_itemInfoList.end(); ++i)
    {
        if (i->GetItemType() == itemType)
        {
            if (index == currIndex)
            {
                entry = &(*i);
                break;
            }
            else
            {
                ++currIndex;
            }
        }
    }

    return entry;
}

std::vector<ItemInfoEntry> ItemInfoAtom::GetItemsByType(FourCCInt itemType) const
{
    std::vector<ItemInfoEntry> items;
    for (const auto& i : m_itemInfoList)
    {
        if (i.GetItemType() == itemType)
        {
            items.push_back(i);
        }
    }
    return items;
}

FDItemInfoExtension::FDItemInfoExtension()
    : m_contentLocation()
    , m_contentMD5()
    , m_contentLength(0)
    , m_transferLength(0)
    , m_entryCount(0)
    , m_groupID(256, 0)
{
}

void FDItemInfoExtension::SetGroupID(const std::uint32_t idx, const uint32_t id)
{
    m_groupID.at(idx) = id;
}

uint32_t FDItemInfoExtension::GetGroupID(const std::uint32_t idx)
{
    return m_groupID.at(idx);
}

VCD_MP4_END