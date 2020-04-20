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
//! \file:   SphereRegionSampEntryAtom.cpp
//! \brief:  SphereRegionSampEntryAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "SphereRegionSampEntryAtom.h"

VCD_MP4_BEGIN

SphereRegionConfigAtom::SphereRegionConfigAtom()
    : FullAtom("rosc", 0, 0)
    , m_numRegions(1)
{
    m_shapeType = ShapeMode::TwoAzimuthAndTwoElevationCircles;
    m_dynamicRangeFlag = false;
    m_staticAzimuthRange = 0;
    m_staticElevationRange = 0;
}

void SphereRegionConfigAtom::SetShapeMode(ShapeMode shapeType)
{
    m_shapeType = shapeType;
}

SphereRegionConfigAtom::ShapeMode SphereRegionConfigAtom::GetShapeMode()
{
    return m_shapeType;
}

void SphereRegionConfigAtom::SetDynamicRangeFlag(bool rangeFlag)
{
    m_dynamicRangeFlag = rangeFlag;
}

bool SphereRegionConfigAtom::GetDynamicRangeFlag()
{
    return m_dynamicRangeFlag;
}

void SphereRegionConfigAtom::SetStaticAzimuthRange(std::uint32_t range)
{
    m_staticAzimuthRange = range;
}

std::uint32_t SphereRegionConfigAtom::GetStaticAzimuthRange()
{
    return m_staticAzimuthRange;
}

void SphereRegionConfigAtom::SetStaticElevationRange(std::uint32_t range)
{
    m_staticElevationRange = range;
}

std::uint32_t SphereRegionConfigAtom::GetStaticElevationRange()
{
    return m_staticElevationRange;
}

void SphereRegionConfigAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);

    str.Write8((uint8_t) m_shapeType);
    str.Write8(m_dynamicRangeFlag ? 0x1 : 0x0);
    if (!m_dynamicRangeFlag)
    {
        str.Write32(m_staticAzimuthRange);
        str.Write32(m_staticElevationRange);
    }
    str.Write8(m_numRegions);

    UpdateSize(str);
}

void SphereRegionConfigAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);
    m_shapeType        = (ShapeMode) str.Read8();
    m_dynamicRangeFlag = str.Read8() & 0x1;
    if (!m_dynamicRangeFlag)
    {
        m_staticAzimuthRange   = str.Read32();
        m_staticElevationRange = str.Read32();
    }
    m_numRegions = str.Read8();
}

SphereRegionSampleEntryAtom::SphereRegionSampleEntryAtom(FourCCInt codingname)
    : MetaDataSampleEntryAtom(codingname)
{
}

SphereRegionConfigAtom& SphereRegionSampleEntryAtom::GetSphereRegionConfig()
{
    return m_sphereRegionConfig;
}

void SphereRegionSampleEntryAtom::ToStream(Stream& str)
{
    MetaDataSampleEntryAtom::ToStream(str);

    Stream subStream;
    m_sphereRegionConfig.ToStream(subStream);
    str.WriteStream(subStream);

    UpdateSize(str);
}

void SphereRegionSampleEntryAtom::FromStream(Stream& str)
{
    MetaDataSampleEntryAtom::FromStream(str);

    FourCCInt AtomType;
    auto sphrereRegionConfigAtomStream = str.ReadSubAtomStream(AtomType);
    m_sphereRegionConfig.ToStream(sphrereRegionConfigAtomStream);
}

VCD_MP4_END
