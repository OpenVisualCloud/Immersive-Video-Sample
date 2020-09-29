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
//! \file:   ProjRelatedAtom.cpp
//! \brief:  ProjRelatedAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include <cstdint>

#include "Stream.h"
#include "ProjRelatedAtom.h"

VCD_MP4_BEGIN

const uint8_t PROJTYPEMASK = 0x1f;

ProjectionFormatAtom::ProjectionFormatAtom()
    : FullAtom("prfr", 0, 0)
    , m_projectionFormat(0)
{
}

ProjectionFormatAtom::ProjectFormat ProjectionFormatAtom::GetProjectFormat() const
{
    ProjectFormat ret = (ProjectFormat) m_projectionFormat;
    return ret;
}

void ProjectionFormatAtom::SetProjectFormat(ProjectFormat projectionFormat)
{
    m_projectionFormat = (uint8_t) projectionFormat & PROJTYPEMASK;
}

void ProjectionFormatAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);
    str.Write8(m_projectionFormat & PROJTYPEMASK);
    UpdateSize(str);
}

void ProjectionFormatAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);
    m_projectionFormat = str.Read8() & PROJTYPEMASK;
}

CoverageInformationAtom::CoverageInformationAtom()
    : FullAtom("covi", 0, 0)
    , m_viewIdcPresenceFlag(false)
    , m_defaultViewIdc(ViewMode::INVALID)
    , m_sphereRegions()
{
    m_coverageShapeMode = CoverageShapeMode::TWO_AZIMUTH_ELEVATION_CIRCLES;

}

CoverageInformationAtom::CoverageInformationAtom(const CoverageInformationAtom& Atom)
    : FullAtom(Atom)
    , m_coverageShapeMode(Atom.m_coverageShapeMode)
    , m_viewIdcPresenceFlag(Atom.m_viewIdcPresenceFlag)
    , m_defaultViewIdc(Atom.m_defaultViewIdc)
    , m_sphereRegions()
{
    for (auto& region : Atom.m_sphereRegions)
    {
        m_sphereRegions.push_back(MakeUnique<CoverageSphereRegion, CoverageSphereRegion>(*region));
    }
}

std::vector<CoverageInformationAtom::CoverageSphereRegion*> CoverageInformationAtom::GetSphereRegions() const
{
    std::vector<CoverageSphereRegion*> regions;
    for (auto& region : m_sphereRegions)
    {
        regions.push_back(region.get());
    }
    return regions;
}

void CoverageInformationAtom::AddSphereRegion(UniquePtr<CoverageSphereRegion> region)
{
    m_sphereRegions.push_back(std::move(region));
}

void CoverageInformationAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);

    str.Write8((uint8_t) m_coverageShapeMode);
    str.Write8((uint8_t) m_sphereRegions.size());

    str.Write8(m_viewIdcPresenceFlag ? 0b10000000 : (((uint8_t) m_defaultViewIdc & 0b11) << 5));

    for (auto& reg : m_sphereRegions)
    {
        if (m_viewIdcPresenceFlag)
        {
            // viewIdc is in first 2 bits 0bXX000000
            str.Write8(((uint8_t) reg->viewIdc & 0b0011) << 6);
        }

        str.Write32(reg->region.centreAzimuth);
        str.Write32(reg->region.centreElevation);
        str.Write32(reg->region.centreTilt);
        str.Write32(reg->region.azimuthRange);
        str.Write32(reg->region.elevationRange);

        str.Write8(reg->region.interpolate ? 0b10000000 : 0x0);
    }

    UpdateSize(str);
}

void CoverageInformationAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);

    m_coverageShapeMode       = (CoverageShapeMode) str.Read8();
    std::uint8_t numRegions  = str.Read8();
    std::uint8_t packed8Bits = str.Read8();

    m_viewIdcPresenceFlag = (packed8Bits >> 7) == 0x01;

    // if not used set all bits to init it with impossible value
    m_defaultViewIdc = (ViewMode)(m_viewIdcPresenceFlag ? 0xff : ((packed8Bits >> 5) & 0b00000011));

    for (int i = 0; i < numRegions; ++i)
    {
        auto reg = MakeUnique<CoverageSphereRegion, CoverageSphereRegion>();

        if (m_viewIdcPresenceFlag)
        {
            packed8Bits     = str.Read8();
            reg->viewIdc = (ViewMode)((packed8Bits >> 6) & 0b00000011);
        }
        else
        {
            reg->viewIdc = ViewMode::INVALID;
        }

        reg->region.centreAzimuth   = str.Read32();
        reg->region.centreElevation = str.Read32();
        reg->region.centreTilt      = str.Read32();
        reg->region.azimuthRange    = str.Read32();
        reg->region.elevationRange  = str.Read32();

        reg->region.interpolate = (str.Read8() >> 7) == 0x01;

        m_sphereRegions.push_back(std::move(reg));
    }
}

void CoverageInformationAtom::Dump() const
{
    ISO_LOG(LOG_INFO, "---------------------------------- COVI ------------------------------\n");
    ISO_LOG(LOG_INFO, "m_coverageShapeMode: %d\n", (std::uint32_t) m_coverageShapeMode);
    ISO_LOG(LOG_INFO, "m_viewIdcPresenceFlag: %d\n", (std::uint32_t) m_viewIdcPresenceFlag);
    ISO_LOG(LOG_INFO, "m_defaultViewIdc: %d\n", (std::uint32_t) m_defaultViewIdc);
    ISO_LOG(LOG_INFO, "m_numRegions: %d\n",(std::uint32_t) m_sphereRegions.size());

    int pCnt = 0;
    for (auto& pReg : m_sphereRegions)
    {
        pCnt++;

        ISO_LOG(LOG_INFO, "---------- Region - %d\n", pCnt);
        ISO_LOG(LOG_INFO, "viewIdc: %d\n", (std::uint32_t) pReg->viewIdc);
        ISO_LOG(LOG_INFO, "centreAzimuth: %d\n", pReg->region.centreAzimuth);
        ISO_LOG(LOG_INFO, "centreElevation: %d\n", pReg->region.centreElevation);
        ISO_LOG(LOG_INFO, "centreTilt: %d\n", pReg->region.centreTilt);
        ISO_LOG(LOG_INFO, "azimuthRange: %d\n", pReg->region.azimuthRange);
        ISO_LOG(LOG_INFO, "elevationRange: %d\n", pReg->region.elevationRange);
        ISO_LOG(LOG_INFO, "interpolate: %d\n", pReg->region.interpolate);
    }
    ISO_LOG(LOG_INFO, "-============================ End Of COVI ===========================-\n");
}

RegionWisePackingAtom::RegionWisePackingAtom()
    : FullAtom("rwpk", 0, 0)
    , m_constituentPictureMatchingFlag(false)
    , m_projPictureWidth(0)
    , m_projPictureHeight(0)
    , m_packedPictureWidth(0)
    , m_packedPictureHeight(0)
    , m_regions()
{
}

RegionWisePackingAtom::RegionWisePackingAtom(const RegionWisePackingAtom& Atom)
    : FullAtom(Atom)
    , m_constituentPictureMatchingFlag(Atom.m_constituentPictureMatchingFlag)
    , m_projPictureWidth(Atom.m_projPictureWidth)
    , m_projPictureHeight(Atom.m_projPictureHeight)
    , m_packedPictureWidth(Atom.m_packedPictureWidth)
    , m_packedPictureHeight(Atom.m_packedPictureHeight)
    , m_regions()
{
    for (auto& region : Atom.m_regions)
    {
        m_regions.push_back(MakeUnique<Region, Region>(*region));
    }
}

std::vector<RegionWisePackingAtom::Region*> RegionWisePackingAtom::GetRegions() const
{
    std::vector<Region*> regions;
    for (auto& region : m_regions)
    {
        regions.push_back(region.get());
    }
    return regions;
}


void RegionWisePackingAtom::AddRegion(UniquePtr<RegionWisePackingAtom::Region> region)
{
    m_regions.push_back(std::move(region));
}

void RegionWisePackingAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);

    str.Write8(m_constituentPictureMatchingFlag ? 0b10000000 : 0x0);
    str.Write8(m_regions.size());

    str.Write32(m_projPictureWidth);
    str.Write32(m_projPictureHeight);
    str.Write16(m_packedPictureWidth);
    str.Write16(m_packedPictureHeight);

    for (auto& pReg : m_regions)
    {
        // 4th bit is guardband flag and last 4 bits are packing type
        str.Write8((pReg->guardBandFlag ? 0b00010000 : 0x0) | ((uint8_t) pReg->packingType & 0x0f));

        if (pReg->packingType == PackingType::RECTANGULAR)
        {
            auto& packing = pReg->rectangularPacking;
            str.Write32(packing->projRegWidth);
            str.Write32(packing->projRegHeight);
            str.Write32(packing->projRegTop);
            str.Write32(packing->projRegLeft);
            // type in bits 0bXXX00000
            str.Write8(packing->transformType << 5);
            str.Write16(packing->packedRegWidth);
            str.Write16(packing->packedRegHeight);
            str.Write16(packing->packedRegTop);
            str.Write16(packing->packedRegLeft);

            if (pReg->guardBandFlag)
            {
                str.Write8(packing->leftGbWidth);
                str.Write8(packing->rightGbWidth);
                str.Write8(packing->topGbHeight);
                str.Write8(packing->bottomGbHeight);

                std::uint16_t packed16Bits = packing->gbNotUsedForPredFlag ? (0x1 << 15) : 0x0;
                packed16Bits |= (((std::uint16_t)(packing->gbType0) & 0b111) << 12) | (((std::uint16_t)(packing->gbType1) & 0b111) << 9) |
                                (((std::uint16_t)(packing->gbType2) & 0b111) << 6)  | (((std::uint16_t)(packing->gbType3) & 0b111) << 3);
                str.Write16(packed16Bits);
            }
        }
    }

    UpdateSize(str);
}

void RegionWisePackingAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);

    // read region wise packing struct
    m_constituentPictureMatchingFlag = (str.Read8() >> 7) & 0x1;
    std::uint8_t numRegions          = str.Read8();
    m_projPictureWidth               = str.Read32();
    m_projPictureHeight              = str.Read32();
    m_packedPictureWidth             = str.Read16();
    m_packedPictureHeight            = str.Read16();

    for (int i = 0; i < numRegions; ++i)
    {
        auto pReg = MakeUnique<Region, Region>();

        std::uint8_t packed8Bits = str.Read8();
        pReg->guardBandFlag    = (packed8Bits >> 4) & 0x01;
        pReg->packingType      = (PackingType)(packed8Bits & 0x0f);

        if (pReg->packingType == PackingType::RECTANGULAR)
        {
            auto pPacking = MakeUnique<RectangularRegionWisePacking, RectangularRegionWisePacking>();

            // read RectRegionPacking
            pPacking->projRegWidth    = str.Read32();
            pPacking->projRegHeight   = str.Read32();
            pPacking->projRegTop      = str.Read32();
            pPacking->projRegLeft     = str.Read32();
            pPacking->transformType   = str.Read8() >> 5;
            pPacking->packedRegWidth  = str.Read16();
            pPacking->packedRegHeight = str.Read16();
            pPacking->packedRegTop    = str.Read16();
            pPacking->packedRegLeft   = str.Read16();

            if (pReg->guardBandFlag)
            {
                // read GuardBand
                pPacking->leftGbWidth    = str.Read8();
                pPacking->rightGbWidth   = str.Read8();
                pPacking->topGbHeight    = str.Read8();
                pPacking->bottomGbHeight = str.Read8();

                std::uint16_t packed16Bits               = str.Read16();
                pPacking->gbNotUsedForPredFlag = packed16Bits >> 15 == 1;
                pPacking->gbType0              = (packed16Bits >> 12) & 0x07;
                pPacking->gbType1              = (packed16Bits >> 9) & 0x07;
                pPacking->gbType2              = (packed16Bits >> 6) & 0x07;
                pPacking->gbType3              = (packed16Bits >> 3) & 0x07;
            }

            pReg->rectangularPacking = std::move(pPacking);
        }

        m_regions.push_back(std::move(pReg));
    }
}

void RegionWisePackingAtom::Dump() const
{
    ISO_LOG(LOG_INFO, "---------------------------------- RWPK ------------------------------\n");
    ISO_LOG(LOG_INFO, "ConstituentPictureMatchingFlag is : %d\n", (std::uint32_t) m_constituentPictureMatchingFlag);
    ISO_LOG(LOG_INFO, "Projection Picture Width is : %d\n", (std::uint32_t) m_projPictureWidth);
    ISO_LOG(LOG_INFO, "Projection Picture Height is : %d\n", (std::uint32_t) m_projPictureHeight);
    ISO_LOG(LOG_INFO, "Packed Picture Width is : %d\n", (std::uint32_t) m_packedPictureWidth);
    ISO_LOG(LOG_INFO, "Packed Picture Height is : %d\n", (std::uint32_t) m_packedPictureHeight);
    ISO_LOG(LOG_INFO, "Num of Regions is : %d\n", (std::uint32_t) m_regions.size());
    int pCnt = 0;
    for (auto& pReg : m_regions)
    {
        pCnt++;

        ISO_LOG(LOG_INFO, "- Region - %d\n", pCnt);
        ISO_LOG(LOG_INFO, "GuarBandFlag is : %d\n", (std::uint32_t) pReg->guardBandFlag);
        ISO_LOG(LOG_INFO, "PackingType is : %d\n", (std::uint32_t) pReg->packingType);

        if (pReg->packingType == PackingType::RECTANGULAR)
        {
            ISO_LOG(LOG_INFO, "transformType is : %d\n", (std::uint32_t) pReg->rectangularPacking->transformType);
            ISO_LOG(LOG_INFO, "projection Region Width is: %d\n", (std::uint32_t) pReg->rectangularPacking->projRegWidth);
            ISO_LOG(LOG_INFO, "projection Region Height is : %d\n", (std::uint32_t) pReg->rectangularPacking->projRegHeight);
            ISO_LOG(LOG_INFO, "projection Region Top is : %d\n", (std::uint32_t) pReg->rectangularPacking->projRegTop);
            ISO_LOG(LOG_INFO, "projection Region Left is : %d\n", (std::uint32_t) pReg->rectangularPacking->projRegLeft);
            ISO_LOG(LOG_INFO, "packed Region Width is : %d\n", (std::uint32_t) pReg->rectangularPacking->packedRegWidth);
            ISO_LOG(LOG_INFO, "packed Region Height is : %d\n", (std::uint32_t) pReg->rectangularPacking->packedRegHeight);
            ISO_LOG(LOG_INFO, "packed Region Top is : %d\n", (std::uint32_t) pReg->rectangularPacking->packedRegTop);
            ISO_LOG(LOG_INFO, "packed Region Left is : %d\n", (std::uint32_t) pReg->rectangularPacking->packedRegLeft);

            if (!pReg->guardBandFlag)
            {
                ISO_LOG(LOG_INFO, "No guard band flag !!\n");
            }
            else
            {
                ISO_LOG(LOG_INFO, "left Gb Width is : %d\n", (std::uint32_t) pReg->rectangularPacking->leftGbWidth);
                ISO_LOG(LOG_INFO, "right Gb Width is : %d\n", (std::uint32_t) pReg->rectangularPacking->rightGbWidth);
                ISO_LOG(LOG_INFO, "top Gb Height is : %d\n", (std::uint32_t) pReg->rectangularPacking->topGbHeight);
                ISO_LOG(LOG_INFO, "bottom Gb Height is : %d\n", (std::uint32_t) pReg->rectangularPacking->bottomGbHeight);
                ISO_LOG(LOG_INFO, "gbNotUsedForPredFlag is : %d\n", (std::uint32_t) pReg->rectangularPacking->gbNotUsedForPredFlag);
                ISO_LOG(LOG_INFO, "gbType0 is : %d\n", (std::uint32_t) pReg->rectangularPacking->gbType0);
                ISO_LOG(LOG_INFO, "gbType1 is : %d\n", (std::uint32_t) pReg->rectangularPacking->gbType1);
                ISO_LOG(LOG_INFO, "gbType2 is : %d\n", (std::uint32_t) pReg->rectangularPacking->gbType2);
                ISO_LOG(LOG_INFO, "gbType3 is : %d\n", (std::uint32_t) pReg->rectangularPacking->gbType3);
            }
        }
    }
}

RegionWisePackingAtom::Region::Region(const Region& region)
    : guardBandFlag(region.guardBandFlag)
    , packingType(region.packingType)
    , rectangularPacking(nullptr)
{
    if (packingType == PackingType::RECTANGULAR)
    {
        UniquePtr<RectangularRegionWisePacking> box = MakeUnique<RectangularRegionWisePacking, RectangularRegionWisePacking>(*region.rectangularPacking);
        rectangularPacking = move(box);
    }
}

RotationAtom::RotationAtom()
    : FullAtom("rotn", 0, 0)
    , m_rotation({})
{
}

RotationAtom::RotationAtom(const RotationAtom& Atom)
    : FullAtom(Atom)
    , m_rotation(Atom.m_rotation)
{
}

void RotationAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);

    str.Write32(m_rotation.yaw);
    str.Write32(m_rotation.pitch);
    str.Write32(m_rotation.roll);

    UpdateSize(str);
}

void RotationAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);

    m_rotation.yaw   = str.Read32();
    m_rotation.pitch = str.Read32();
    m_rotation.roll  = str.Read32();
}

VCD_MP4_END
