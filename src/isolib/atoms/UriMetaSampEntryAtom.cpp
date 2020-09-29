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
//! \file:   UriMetaSampEntryAtom.cpp
//! \brief:  UriMetaSampEntryAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "UriMetaSampEntryAtom.h"

VCD_MP4_BEGIN

UriAtom::UriAtom()
    : FullAtom("uri ", 0, 0)
    , m_uri()
{
}

void UriAtom::SetUri(const std::string& uri)
{
    m_uri = uri;
}

const std::string& UriAtom::GetUri() const
{
    return m_uri;
}

void UriAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);
    str.WriteZeroEndString(m_uri);
    UpdateSize(str);
}

void UriAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);
    str.ReadZeroEndString(m_uri);
}

UriInitAtom::UriInitAtom()
    : FullAtom("uriI", 0, 0)
    , m_initAtomType(UriInitAtom::InitAtomMode::UNKNOWN)
    , m_uriInitializationData()
{
}

UriInitAtom::InitAtomMode UriInitAtom::GetInitAtomMode() const
{
    return m_initAtomType;
}

void UriInitAtom::SetInitAtomMode(UriInitAtom::InitAtomMode dataType)
{
    m_initAtomType = dataType;
}

void UriInitAtom::SetUriInitializationData(const std::vector<std::uint8_t>& uriInitData)
{
    m_initAtomType           = InitAtomMode::UNKNOWN;
    m_uriInitializationData = uriInitData;
}

std::vector<std::uint8_t> UriInitAtom::GetUriInitializationData() const
{
    return m_uriInitializationData;
}

void UriInitAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);
    str.WriteArray(m_uriInitializationData, static_cast<unsigned int>(m_uriInitializationData.size()));

    UpdateSize(str);
}

void UriInitAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);

    const uint64_t BytesRemain = str.BytesRemain();
    if (BytesRemain >= 8)
    {
        FourCCInt AtomType;
        Stream subBitstr = str.ReadSubAtomStream(AtomType);
        m_uriInitializationData.clear();
        m_uriInitializationData.reserve(BytesRemain);
        subBitstr.ReadArray(m_uriInitializationData, BytesRemain);
    }
}

UriMetaSampleEntryAtom::UriMetaSampleEntryAtom()
    : MetaDataSampleEntryAtom("urim")
    , m_uriAtom()
    , m_vRMetaDataType(VRTMDType::UNKNOWN)
    , m_hasUriInitAtom(false)
    , m_uriInitAtom()
{
}

UriAtom& UriMetaSampleEntryAtom::GetUriAtom()
{
    return m_uriAtom;
}

bool UriMetaSampleEntryAtom::HasUriInitAtom()
{
    return m_hasUriInitAtom;
}

UriInitAtom& UriMetaSampleEntryAtom::GetUriInitAtom()
{
    return m_uriInitAtom;
}

UriMetaSampleEntryAtom::VRTMDType UriMetaSampleEntryAtom::GetVRTMDType() const
{
    return m_vRMetaDataType;
}

void UriMetaSampleEntryAtom::ToStream(Stream& str)
{
    MetaDataSampleEntryAtom::ToStream(str);
    m_uriAtom.ToStream(str);

    if (m_hasUriInitAtom)
    {
        m_uriInitAtom.ToStream(str);
    }

    UpdateSize(str);
}

void UriMetaSampleEntryAtom::FromStream(Stream& str)
{
    MetaDataSampleEntryAtom::FromStream(str);
    UriInitAtom::InitAtomMode initAtomType = UriInitAtom::InitAtomMode::UNKNOWN;

    bool uriFound = false;
    while (str.BytesRemain() > 0)
    {
        FourCCInt AtomType;
        Stream subBitstr = str.ReadSubAtomStream(AtomType);

        if (AtomType == "uri ")
        {
            m_uriAtom.FromStream(subBitstr);
            uriFound        = true;
            m_vRMetaDataType = VRTMDType::UNKNOWN;
        }
        else if (AtomType == "uriI")
        {
            m_hasUriInitAtom = true;
            m_uriInitAtom.SetInitAtomMode(initAtomType);
            m_uriInitAtom.FromStream(subBitstr);
        }
        // unsupported Atoms are skipped
    }
    if (!uriFound)
    {
        ISO_LOG(LOG_ERROR, "UriMetaSampleEntryAtom couldn't found URI Atom\n");
        throw Exception();
    }
}

UriMetaSampleEntryAtom* UriMetaSampleEntryAtom::Clone() const
{
    UriMetaSampleEntryAtom* Atom = new UriMetaSampleEntryAtom();

    auto mutableThis = const_cast<UriMetaSampleEntryAtom*>(this);

    {
        Stream bs;
        mutableThis->ToStream(bs);
        bs.Reset();
        Atom->FromStream(bs);
    }

    return Atom;
}

const Atom* UriMetaSampleEntryAtom::GetConfigurationAtom() const
{
    ISO_LOG(LOG_ERROR, "UriMetaSampleEntryAtom::GetConfigurationAtom() not impelmented \n");
    return nullptr;
}

const DecoderConfigurationRecord* UriMetaSampleEntryAtom::GetConfigurationRecord() const
{
    ISO_LOG(LOG_ERROR, "UriMetaSampleEntryAtom::GetConfigurationRecord() not impelmented\n");
    return nullptr;
}

VCD_MP4_END