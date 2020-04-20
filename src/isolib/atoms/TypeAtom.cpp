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
//! \file:   TypeAtom.cpp
//! \brief:  TypeAtom class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!
#include "TypeAtom.h"
#include <algorithm>
#include "FullAtom.h"


VCD_MP4_BEGIN

template <>
BrandAtom<Atom>::BrandAtom(FourCCInt AtomType)
    : Atom(AtomType)
    , m_majorBrand()
    , m_minorVersion(0)
    , m_compatibleBrands()
{
}

template <>
BrandAtom<FullAtom>::BrandAtom(FourCCInt AtomType,
                                                                          std::uint8_t version,
                                                                          std::uint32_t flags)
    : FullAtom(AtomType, version, flags)
    , m_majorBrand()
    , m_minorVersion(0)
    , m_compatibleBrands()
{
}

template <class T>
void BrandAtom<T>::SetMajorBrand(const std::string& majorBrand)
{
    m_majorBrand = majorBrand;
}

template <class T>
const std::string& BrandAtom<T>::GetMajorBrand() const
{
    return m_majorBrand;
}

template <class T>
void BrandAtom<T>::SetMinorVersion(uint32_t minorVersion)
{
    m_minorVersion = minorVersion;
}

template <class T>
uint32_t BrandAtom<T>::GetMinorVersion() const
{
    return m_minorVersion;
}

template <class T>
void BrandAtom<T>::AddCompatibleBrand(const std::string& compatibleBrand)
{
    m_compatibleBrands.push_back(compatibleBrand);
}

template <class T>
std::vector<std::string> BrandAtom<T>::GetCompatibleBrands() const
{
    return m_compatibleBrands;
}

template <class T>
bool BrandAtom<T>::CheckCompatibleBrand(const std::string& brand) const
{
    if (find(m_compatibleBrands.begin(), m_compatibleBrands.end(), brand) != m_compatibleBrands.end())
    {
        return true;
    }
    else
    {
        return false;
    }
}

template <>
void BrandAtom<Atom>::WriteHeader(Stream& str)
{
    WriteAtomHeader(str);
}

template <>
void BrandAtom<FullAtom>::WriteHeader(Stream& str)
{
    WriteFullAtomHeader(str);
}

template <class T>
void BrandAtom<T>::ToStream(Stream& str)
{
    // Write the Atom header
    WriteHeader(str);  // write parent Atom

    // major_brand
    str.WriteString(m_majorBrand);
    // minor_version
    str.Write32(m_minorVersion);
    // compatible_brands[]
    for (auto const& brand : m_compatibleBrands)
    {
        str.WriteString(brand);
    }

    // Update the size field of this Atom in the bitstream
    T::UpdateSize(str);
}

template <>
void BrandAtom<Atom>::ParseHeader(Stream& str)
{
    ParseAtomHeader(str);
}

template <>
void BrandAtom<FullAtom>::ParseHeader(Stream& str)
{
    ParseFullAtomHeader(str);
}

template <class T>
void BrandAtom<T>::FromStream(Stream& str)
{
    // Parse the Atom or fullAtom header
    ParseHeader(str);

    // major_brand
    str.ReadStringWithLen(m_majorBrand, 4);
    // minor_version
    m_minorVersion = str.Read32();
    // compatible_brands[]
    while (str.BytesRemain() >= 4)
    {
        std::string compatibleBrand;
        str.ReadStringWithLen(compatibleBrand, 4);
        m_compatibleBrands.push_back(compatibleBrand);
    }
}

template class BrandAtom<Atom>;
template class BrandAtom<FullAtom>;

FileTypeAtom::FileTypeAtom()
: BrandAtom("ftyp")
{
}

TrackTypeAtom::TrackTypeAtom()
    : BrandAtom("ttyp", 0, 0)
{
}

SegmentTypeAtom::SegmentTypeAtom()
    : BrandAtom("styp")
{
}

VCD_MP4_END