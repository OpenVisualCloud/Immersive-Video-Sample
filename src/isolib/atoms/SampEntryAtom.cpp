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
//! \file:   SampEntryAtom.cpp
//! \brief:  SampEntryAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!
#include "SampEntryAtom.h"
#include "Stream.h"


VCD_MP4_BEGIN

static const int RESERVED_BYTES = 6;

SampleEntryAtom::SampleEntryAtom(FourCCInt codingname)
    : Atom(codingname)
    , m_dataReferenceIndex(0)
    , m_restrictedSchemeInfoAtom(nullptr)
{
}

SampleEntryAtom::SampleEntryAtom(const SampleEntryAtom& atom)
    : Atom(atom.GetType())
    , m_dataReferenceIndex(atom.m_dataReferenceIndex)
    , m_restrictedSchemeInfoAtom(nullptr)
{
    if (atom.m_restrictedSchemeInfoAtom)
    {
        UniquePtr<RestrictedSchemeInfoAtom> box = MakeUnique<RestrictedSchemeInfoAtom, RestrictedSchemeInfoAtom>(*atom.m_restrictedSchemeInfoAtom);
        m_restrictedSchemeInfoAtom = move(box);
    }
}

std::uint16_t SampleEntryAtom::GetDataReferenceIndex() const
{
    return m_dataReferenceIndex;
}

void SampleEntryAtom::SetDataReferenceIndex(std::uint16_t dataReferenceIndex)
{
    m_dataReferenceIndex = dataReferenceIndex;
}

void SampleEntryAtom::ToStream(Stream& str)
{
    WriteAtomHeader(str);

    for (int i = 0; i < RESERVED_BYTES; ++i)
    {
        str.Write8(0);  // reserved = 0
    }

    str.Write16(m_dataReferenceIndex);

    // Update the size of the movie Atom
    UpdateSize(str);
}

void SampleEntryAtom::FromStream(Stream& str)
{
    ParseAtomHeader(str);

    for (int i = 0; i < RESERVED_BYTES; ++i)
    {
        str.Read8();  // reserved
    }

    m_dataReferenceIndex = str.Read16();
}

void SampleEntryAtom::AddRestrictedSchemeInfoAtom(UniquePtr<RestrictedSchemeInfoAtom> restrictedSchemeInfoAtom)
{
    m_restrictedSchemeInfoAtom = std::move(restrictedSchemeInfoAtom);
}

RestrictedSchemeInfoAtom* SampleEntryAtom::GetRestrictedSchemeInfoAtom() const
{
    return m_restrictedSchemeInfoAtom.get();
}

bool SampleEntryAtom::IsVisual() const
{
    return false;
}

VCD_MP4_END
