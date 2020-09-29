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
//! \file:   DataRefAtom.cpp
//! \brief:  DataRefAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "DataRefAtom.h"


#include <stdexcept>

VCD_MP4_BEGIN

DataEntryAtom::DataEntryAtom(FourCCInt AtomType, const std::uint8_t version, const std::uint32_t flags)
    : FullAtom(AtomType, version, flags)
    , m_location()
{
}

void DataEntryAtom::SetLocation(const std::string& location)
{
    m_location = location;
}

const std::string DataEntryAtom::GetLocation() const
{
    return m_location;
}

DataEntryUrlAtom::DataEntryUrlAtom(IsContained isContained)
    : DataEntryAtom("url ", 0, isContained == NotContained ? 0 : 1)
{
}

void DataEntryUrlAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);

    if (!(GetFlags() & 1))
    {
        str.WriteZeroEndString(GetLocation());
    }

    UpdateSize(str);
}

void DataEntryUrlAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);

    if (!(GetFlags() & 1))
    {
        std::string location;
        str.ReadZeroEndString(location);
        SetLocation(location);
    }
}

DataEntryUrnAtom::DataEntryUrnAtom()
    : DataEntryAtom("urn ", 0, 0)
    , m_name()
{
}

void DataEntryUrnAtom::SetName(const std::string& name)
{
    m_name = name;
}

const std::string DataEntryUrnAtom::GetName() const
{
    return m_name;
}

void DataEntryUrnAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);
    str.WriteZeroEndString(m_name);
    str.WriteZeroEndString(GetLocation());
    UpdateSize(str);
}

void DataEntryUrnAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);
    str.ReadZeroEndString(m_name);
    std::string location;
    str.ReadZeroEndString(location);
    SetLocation(location);
}

DataReferenceAtom::DataReferenceAtom()
    : FullAtom("dref", 0, 0)
    , m_dataEntries()
{
}

unsigned int DataReferenceAtom::AddEntry(std::shared_ptr<DataEntryAtom> dataEntryAtom)
{
    m_dataEntries.push_back(dataEntryAtom);
    unsigned int ret = static_cast<unsigned int>(m_dataEntries.size());
    return ret;
}

void DataReferenceAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);

    str.Write32(static_cast<std::uint32_t>(m_dataEntries.size()));
    for (auto& entry : m_dataEntries)
    {
        entry->ToStream(str);
    }

    UpdateSize(str);
}

void DataReferenceAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);

    const unsigned int entryCount = str.Read32();
    for (unsigned int i = 0; i < entryCount; ++i)
    {
        FourCCInt AtomType;
        Stream subStream = str.ReadSubAtomStream(AtomType);

        std::shared_ptr<DataEntryAtom> dataEntryAtom;
        if (AtomType == "urn ")
        {
            dataEntryAtom = MakeShared<DataEntryUrnAtom>();
            if (!dataEntryAtom)
            {
                ISO_LOG(LOG_ERROR, "NULL pointer !\n");
                throw Exception();
            }
            dataEntryAtom->FromStream(subStream);
        }
        else if (AtomType == "url ")
        {
            dataEntryAtom = MakeShared<DataEntryUrlAtom>();
            if (!dataEntryAtom)
            {
                ISO_LOG(LOG_ERROR, "NULL pointer !\n");
                throw Exception();
            }
            dataEntryAtom->FromStream(subStream);
        }
        else
        {
            ISO_LOG(LOG_ERROR, "An unknown Atom inside dref\n");
            throw Exception();
        }
        m_dataEntries.push_back(dataEntryAtom);
    }
}

VCD_MP4_END
