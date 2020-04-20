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
//! \file:   MetaAtom.cpp
//! \brief:  MetaAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!
#include "MetaAtom.h"

VCD_MP4_BEGIN

XmlAtom::XmlAtom()
    : FullAtom("uri ", 0, 0)
    , m_contents()
{
}

void XmlAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);
    str.WriteZeroEndString(m_contents);
    UpdateSize(str);
}

void XmlAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);
    str.ReadZeroEndString(m_contents);
}

MetaAtom::MetaAtom()
    : FullAtom("meta", 0, 0)
    , m_handlerAtom()
    , m_hasPrimaryItemAtom()
    , m_primaryItemAtom()
    , m_hasDataInfoAtom()
    , m_dataInfoAtom()
    , m_hasItemLocationAtom()
    , m_itemLocationAtom()
    , m_hasItemProtectionAtom()
    , m_itemProtectionAtom()
    , m_hasItemInfoAtom()
    , m_itemInfoAtom(2)
    , m_hasItemRefAtom()
    , m_itemRefAtom()
    , m_hasItemDataAtom()
    , m_itemDataAtom()
    , m_hasXmlAtom()
    , m_xmlAtom()
{
}

FourCCInt MetaAtom::GetHandlerType() const
{
    FourCCInt ret = m_handlerAtom.GetHandlerType();
    return ret;
}

void MetaAtom::SetHandlerType(FourCCInt handler)
{
    m_handlerAtom.SetHandlerType(handler);
}

const PrimaryItemAtom& MetaAtom::GetPrimaryItemAtom() const
{
    return m_primaryItemAtom;
}

void MetaAtom::SetPrimaryItem(const std::uint32_t itemId)
{
    m_hasPrimaryItemAtom = true;
    m_primaryItemAtom.SetItemId(itemId);

    if (m_itemLocationAtom.HasItemIdEntry(itemId))
    {
        auto urlAtom = MakeShared<DataEntryUrlAtom>();
        urlAtom->SetFlags(1);  // Flag 0x01 tells the data is in this file. DataEntryUrlAtom will write only its header.
        const std::uint16_t index = m_dataInfoAtom.AddDataEntryAtom(urlAtom);
        m_itemLocationAtom.SetItemDataReferenceIndex(itemId, index);
    }
}

const ItemInfoAtom& MetaAtom::GetItemInfoAtom() const
{
    return m_itemInfoAtom;
}

void MetaAtom::SetItemInfoAtom(const ItemInfoAtom& itemInfoAtom)
{
    m_hasItemInfoAtom = true;
    m_itemInfoAtom    = itemInfoAtom;
}

const ItemLocationAtom& MetaAtom::GetItemLocationAtom() const
{
    return m_itemLocationAtom;
}

const ItemReferenceAtom& MetaAtom::GetItemReferenceAtom() const
{
    return m_itemRefAtom;
}

const DataInformationAtom& MetaAtom::GetDataInformationAtom() const
{
    return m_dataInfoAtom;
}

void MetaAtom::AddItemReference(FourCCInt type, const std::uint32_t from_id, const std::uint32_t toId)
{
    m_hasItemRefAtom = true;
    m_itemRefAtom.AddItems(type, from_id, toId);
}

void MetaAtom::AddIloc(const std::uint32_t itemId,
                      const std::uint32_t offset,
                      const std::uint32_t length,
                      const std::uint32_t baseOffset)
{
    m_hasItemLocationAtom = true;

    ExtentParams locationExtent;
    locationExtent.m_extentOffset = offset;
    locationExtent.m_extentLen = length;

    ItemLocation itemLocation;
    itemLocation.SetItemID(itemId);
    itemLocation.SetBaseOffset(baseOffset);
    itemLocation.AddExtent(locationExtent);

    m_itemLocationAtom.AddLocation(itemLocation);
}

void MetaAtom::AddItem(const std::uint32_t itemId, FourCCInt type, const std::string& name, const bool hidden)
{
    m_hasItemInfoAtom = true;

    ItemInfoEntry infoAtom;
    infoAtom.SetVersion(2);
    infoAtom.SetItemType(type);
    infoAtom.SetItemID(itemId);
    infoAtom.SetItemName(name);

    if (hidden)
    {
        infoAtom.SetFlags(1);
    }

    m_itemInfoAtom.AddItemInfoEntry(infoAtom);
}

void MetaAtom::AddMdatItem(const std::uint32_t itemId,
                          FourCCInt type,
                          const std::string& name,
                          const std::uint32_t baseOffset)
{
    m_hasItemLocationAtom = true;

    AddItem(itemId, type, name);

    ItemLocation itemLocation;
    itemLocation.SetItemID(itemId);
    itemLocation.SetBaseOffset(baseOffset);
    itemLocation.SetConstructType(ItemLocation::ConstructType::FILE_OFFSET);
    m_itemLocationAtom.AddLocation(itemLocation);
}

void MetaAtom::AddItemExtent(const std::uint32_t itemId, const std::uint32_t offset, const std::uint32_t length)
{
    m_hasItemLocationAtom = true;

    ExtentParams locationExtent;
    locationExtent.m_extentOffset = offset;
    locationExtent.m_extentLen = length;
    m_itemLocationAtom.AddExtent(itemId, locationExtent);
}

void MetaAtom::AddIdatItem(const std::uint32_t itemId, FourCCInt type, const std::string& name, const std::vector<uint8_t>& data)
{
    m_hasItemLocationAtom = true;

    const unsigned int offset = m_itemDataAtom.AddData(data);
    AddItem(itemId, type, name);
    ExtentParams locationExtent;
    locationExtent.m_extentOffset = offset;
    locationExtent.m_extentLen = data.size();

    ItemLocation itemLocation;
    itemLocation.SetItemID(itemId);
    itemLocation.AddExtent(locationExtent);
    itemLocation.SetConstructType(ItemLocation::ConstructType::IDAT_OFFSET);
    m_itemLocationAtom.AddLocation(itemLocation);
}

void MetaAtom::AddItemIdatExtent(const std::uint32_t itemId, const std::vector<uint8_t>& data)
{
    m_hasItemLocationAtom = true;

    const unsigned int offset = m_itemDataAtom.AddData(data);
    ExtentParams locationExtent;
    locationExtent.m_extentOffset = offset;
    locationExtent.m_extentLen = data.size();
    m_itemLocationAtom.AddExtent(itemId, locationExtent);
}

const ItemDataAtom& MetaAtom::GetItemDataAtom() const
{
    return m_itemDataAtom;
}

const ProtectionSchemeInfoAtom& MetaAtom::GetProtectionSchemeInfoAtom(std::uint16_t index) const
{
    const ProtectionSchemeInfoAtom& atom = m_itemProtectionAtom.GetEntry(index);
    return atom;
}

const XmlAtom& MetaAtom::GetXmlAtom() const
{
    return m_xmlAtom;
}

void MetaAtom::SetXmlAtom(const XmlAtom& atom)
{
    m_hasXmlAtom = true;
    m_xmlAtom    = atom;
}

void MetaAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);

    m_handlerAtom.ToStream(str);
    if (m_hasPrimaryItemAtom)
    {
        m_primaryItemAtom.ToStream(str);
    }
    if (m_hasDataInfoAtom)
    {
        m_dataInfoAtom.ToStream(str);
    }
    if (m_hasItemLocationAtom)
    {
        m_itemLocationAtom.ToStream(str);
    }
    if (m_hasItemProtectionAtom)
    {
        m_itemProtectionAtom.ToStream(str);
    }
    if (m_hasItemInfoAtom)
    {
        m_itemInfoAtom.ToStream(str);
    }
    if (m_hasItemRefAtom)
    {
        m_itemRefAtom.ToStream(str);
    }
    if (m_hasItemDataAtom)
    {
        m_itemDataAtom.ToStream(str);
    }
    if (m_hasXmlAtom)
    {
        m_xmlAtom.ToStream(str);
    }

    UpdateSize(str);
}

void MetaAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);

    while (str.BytesRemain() > 0)
    {
        FourCCInt pAtomType;
        Stream subStr = str.ReadSubAtomStream(pAtomType);

        if (pAtomType == "hdlr")
        {
            m_handlerAtom.FromStream(subStr);
        }
        else if (pAtomType == "pitm")
        {
            m_hasPrimaryItemAtom = true;
            m_primaryItemAtom.FromStream(subStr);
        }
        else if (pAtomType == "iloc")
        {
            m_hasItemLocationAtom = true;
            m_itemLocationAtom.FromStream(subStr);
        }
        else if (pAtomType == "iinf")
        {
            m_hasItemInfoAtom = true;
            m_itemInfoAtom.FromStream(subStr);
        }
        else if (pAtomType == "iref")
        {
            m_hasItemRefAtom = true;
            m_itemRefAtom.FromStream(subStr);
        }
        else if (pAtomType == "dinf")
        {
            m_hasDataInfoAtom = true;
            m_dataInfoAtom.FromStream(subStr);
        }
        else if (pAtomType == "idat")
        {
            m_hasItemDataAtom = true;
            m_itemDataAtom.FromStream(subStr);
        }
        else if (pAtomType == "ipro")
        {
            m_hasItemProtectionAtom = true;
            m_itemProtectionAtom.FromStream(subStr);
        }
        else if (pAtomType == "xml ")
        {
            m_hasXmlAtom = true;
            m_xmlAtom.FromStream(subStr);
        }
    }
}
VCD_MP4_END