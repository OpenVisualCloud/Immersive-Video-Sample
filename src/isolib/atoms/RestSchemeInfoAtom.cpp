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
//! \file:   RestSchemeInfoAtom.cpp
//! \brief:  RestSchemeInfoAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include <cassert>

#include "Stream.h"
#include "RestSchemeInfoAtom.h"

VCD_MP4_BEGIN

OriginalFormatAtom::OriginalFormatAtom()
    : Atom("frma")
{
}

void OriginalFormatAtom::ToStream(Stream& str)
{
    WriteAtomHeader(str);
    str.Write32(m_originalFormat.GetUInt32());
    UpdateSize(str);
}

void OriginalFormatAtom::FromStream(Stream& str)
{
    ParseAtomHeader(str);
    m_originalFormat = str.Read32();
}

ProjectedOmniVideoAtom::ProjectedOmniVideoAtom()
    : Atom("povd")
    , m_projectionFormatAtom()
    , m_regionWisePackingAtom()
    , m_coverageInformationAtom()
{
}

ProjectedOmniVideoAtom::ProjectedOmniVideoAtom(const ProjectedOmniVideoAtom& atom)
    : Atom(atom)
    , m_projectionFormatAtom(atom.m_projectionFormatAtom)
    , m_regionWisePackingAtom(atom.m_regionWisePackingAtom ? MakeUnique<RegionWisePackingAtom, RegionWisePackingAtom>(
                                                            *atom.m_regionWisePackingAtom)
                                                      : UniquePtr<RegionWisePackingAtom>())
    , m_coverageInformationAtom(
          atom.m_coverageInformationAtom
              ? MakeUnique<CoverageInformationAtom, CoverageInformationAtom>(*atom.m_coverageInformationAtom)
              : UniquePtr<CoverageInformationAtom>())
    , m_rotationAtom(atom.m_rotationAtom ? MakeUnique<RotationAtom, RotationAtom>(*atom.m_rotationAtom)
                                    : UniquePtr<RotationAtom>())
{
}

ProjectionFormatAtom& ProjectedOmniVideoAtom::GetProjectionFormatAtom()
{
    return m_projectionFormatAtom;
}

RegionWisePackingAtom& ProjectedOmniVideoAtom::GetRegionWisePackingAtom()
{
    return *m_regionWisePackingAtom;
}

void ProjectedOmniVideoAtom::SetRegionWisePackingAtom(UniquePtr<RegionWisePackingAtom> rwpkAtom)
{
    m_regionWisePackingAtom = std::move(rwpkAtom);
}

bool ProjectedOmniVideoAtom::HasRegionWisePackingAtom() const
{
    return !!m_regionWisePackingAtom;
}

CoverageInformationAtom& ProjectedOmniVideoAtom::GetCoverageInformationAtom()
{
    return *m_coverageInformationAtom;
}

void ProjectedOmniVideoAtom::SetCoverageInformationAtom(UniquePtr<CoverageInformationAtom> coviAtom)
{
    m_coverageInformationAtom = std::move(coviAtom);
}

bool ProjectedOmniVideoAtom::HasCoverageInformationAtom() const
{
    return !!m_coverageInformationAtom;
}

RotationAtom& ProjectedOmniVideoAtom::GetRotationAtom()
{
    return *m_rotationAtom;
}

void ProjectedOmniVideoAtom::SetRotationAtom(UniquePtr<RotationAtom> rotnAtom)
{
    m_rotationAtom = std::move(rotnAtom);
}

bool ProjectedOmniVideoAtom::HasRotationAtom() const
{
    return !!m_rotationAtom;
}

void ProjectedOmniVideoAtom::ToStream(Stream& str)
{
    WriteAtomHeader(str);
    m_projectionFormatAtom.ToStream(str);
    if (m_regionWisePackingAtom)
    {
        m_regionWisePackingAtom->ToStream(str);
    }
    if (m_coverageInformationAtom)
    {
        m_coverageInformationAtom->ToStream(str);
    }
    if (m_rotationAtom)
    {
        m_rotationAtom->ToStream(str);
    }
    UpdateSize(str);
}

void ProjectedOmniVideoAtom::FromStream(Stream& str)
{
    ParseAtomHeader(str);

    m_projectionFormatAtom.FromStream(str);
    if (m_projectionFormatAtom.GetType() != "prfr")
    {
        ISO_LOG(LOG_ERROR, "POVD Atom must start with prfr Atom\n");
        throw Exception();
    }

    while (str.BytesRemain() > 0)
    {
        // Extract contained Atom bitstream and type
        FourCCInt AtomType;
        Stream subStream = str.ReadSubAtomStream(AtomType);

        if (AtomType == "rwpk")
        {
            m_regionWisePackingAtom = MakeUnique<RegionWisePackingAtom, RegionWisePackingAtom>();
            m_regionWisePackingAtom->FromStream(subStream);
        }
        else if (AtomType == "covi")
        {
            m_coverageInformationAtom = MakeUnique<CoverageInformationAtom, CoverageInformationAtom>();
            m_coverageInformationAtom->FromStream(subStream);
        }
        else if (AtomType == "rotn")
        {
            m_rotationAtom = MakeUnique<RotationAtom, RotationAtom>();
            m_rotationAtom->FromStream(subStream);
        }
        else
        {
			char type[4];
            AtomType.GetString().copy(type, 4, 0);
            ISO_LOG(LOG_WARNING, "Ignoring unknown AtomType found from povd Atom: %s\n", type);
        }
    }
}

void ProjectedOmniVideoAtom::dump() const
{
    ISO_LOG(LOG_INFO, "---------------------------------- POVD ------------------------------\n");
    ISO_LOG(LOG_INFO, "m_projectionFormatAtom.GetProjectFormat: %d\n", (std::uint32_t) m_projectionFormatAtom.GetProjectFormat());

    if (m_regionWisePackingAtom)
    {
        m_regionWisePackingAtom->Dump();
    }

    if (m_coverageInformationAtom)
    {
        m_coverageInformationAtom->Dump();
    }

    if (m_rotationAtom)
    {
        ISO_LOG(LOG_INFO, "Also rotation Atom is present\n");
    }

    ISO_LOG(LOG_INFO, "-============================ End Of POVD ===========================-\n");
}

SchemeTypeAtom::SchemeTypeAtom()
    : FullAtom("schm", 0, 0)
    , m_schemeType(0)
    , m_schemeVersion(0)
    , m_schemeUri("")
{
}

void SchemeTypeAtom::SetSchemeUri(const std::string& uri)
{
    std::uint32_t hasUriFieldFlags = uri.empty() ? GetFlags() & (~0x1) : GetFlags() | 0x1;
    SetFlags(hasUriFieldFlags);
    m_schemeUri = uri;
}

const std::string& SchemeTypeAtom::GetSchemeUri() const
{
    return m_schemeUri;
}

void SchemeTypeAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);
    str.Write32(m_schemeType.GetUInt32());
    str.Write32(m_schemeVersion);
    if (GetFlags() & 0x1)
    {
        str.WriteZeroEndString(m_schemeUri);
    }
    UpdateSize(str);
}

void SchemeTypeAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);
    m_schemeType    = str.Read32();
    m_schemeVersion = str.Read32();
    if (GetFlags() & 0x1)
    {
        str.ReadZeroEndString(m_schemeUri);
    }
}

CompatibleSchemeTypeAtom::CompatibleSchemeTypeAtom()
    : SchemeTypeAtom()
{
    SetType("csch");
}

RestrictedSchemeInfoAtom::RestrictedSchemeInfoAtom()
    : Atom("rinf")
    , m_projectedOmniVideoAtom()
    , m_stereoVideoAtom()
{
}

RestrictedSchemeInfoAtom::RestrictedSchemeInfoAtom(const RestrictedSchemeInfoAtom& atom)
    : Atom(atom)
    , m_originalFormatAtom(atom.m_originalFormatAtom ? std::move(MakeUnique<OriginalFormatAtom, OriginalFormatAtom>(
                                                      *atom.m_originalFormatAtom))
                                                : nullptr)
    , m_schemeTypeAtom(atom.m_schemeTypeAtom ? std::move(MakeUnique<SchemeTypeAtom, SchemeTypeAtom>(*atom.m_schemeTypeAtom))
                                        : nullptr)
    , m_projectedOmniVideoAtom(
          atom.m_projectedOmniVideoAtom
              ? std::move(MakeUnique<ProjectedOmniVideoAtom, ProjectedOmniVideoAtom>(*atom.m_projectedOmniVideoAtom))
              : nullptr)
    , m_stereoVideoAtom(atom.m_stereoVideoAtom
                          ? std::move(MakeUnique<StereoVideoAtom, StereoVideoAtom>(*atom.m_stereoVideoAtom))
                          : nullptr)
{
    for (auto& schemeTypeAtom : atom.m_compatibleSchemeTypes)
    {
        m_compatibleSchemeTypes.push_back(
            MakeUnique<CompatibleSchemeTypeAtom, CompatibleSchemeTypeAtom>(*schemeTypeAtom));
    }
}


void RestrictedSchemeInfoAtom::ToStream(Stream& str)
{
    WriteAtomHeader(str);
    if (m_originalFormatAtom)
    {
        Stream subStream;
        m_originalFormatAtom->ToStream(str);
        str.WriteStream(subStream);
    }
    if (m_schemeTypeAtom)
    {
        Stream subStream;
        m_schemeTypeAtom->ToStream(str);
        str.WriteStream(subStream);
    }
    for (auto& compatibleScheme : m_compatibleSchemeTypes)
    {
        Stream subStream;
        compatibleScheme->ToStream(str);
        str.WriteStream(subStream);
    }

    if (m_schemeTypeAtom && (m_schemeTypeAtom->GetSchemeType() == "podv"))
    {
        Stream povdStream;
        m_projectedOmniVideoAtom->ToStream(povdStream);

        Stream stviStream;
        if (m_stereoVideoAtom)
        {
            m_stereoVideoAtom->ToStream(stviStream);
        }

        // write schi + povd
        Stream schiStream;
        schiStream.WriteHeaders("schi", povdStream.GetSize() + stviStream.GetSize());
        schiStream.WriteStream(povdStream);
        schiStream.WriteStream(stviStream);
        str.WriteStream(schiStream);
    }

    UpdateSize(str);
}

void RestrictedSchemeInfoAtom::FromStream(Stream& str)
{
    // rinf header
    ParseAtomHeader(str);

    // if there a data available in the file
    while (str.BytesRemain() > 0)
    {
        // Extract contained Atom bitstream and type
        FourCCInt AtomType;
        Stream subBitstr = str.ReadSubAtomStream(AtomType);

        if (AtomType == "frma")
        {
            m_originalFormatAtom = std::move(MakeUnique<OriginalFormatAtom, OriginalFormatAtom>());
            m_originalFormatAtom->FromStream(subBitstr);
        }
        else if (AtomType == "schm")
        {
            m_schemeTypeAtom = std::move(MakeUnique<SchemeTypeAtom, SchemeTypeAtom>());
            m_schemeTypeAtom->FromStream(subBitstr);
        }
        else if (AtomType == "csch")
        {
            auto compatibeSchemeTypeAtom = MakeUnique<CompatibleSchemeTypeAtom, CompatibleSchemeTypeAtom>();
            compatibeSchemeTypeAtom->FromStream(subBitstr);
            m_compatibleSchemeTypes.push_back(std::move(compatibeSchemeTypeAtom));
        }
        else if (AtomType == "schi")
        {
            if (!m_schemeTypeAtom)
            {
                ISO_LOG(LOG_ERROR, "Scheme type Atom was not found, before scheme info Atom\n");
                throw Exception();
            }

            // skip schi Atom headers
            subBitstr.ReadAtomHeaders(AtomType);

            FourCCInt subSchiAtomType;

            // try to parse internals only if there is enough bytes to contain Atom inside

            while (subBitstr.BytesRemain() > 16)
            {
                Stream subSchiBitstr = subBitstr.ReadSubAtomStream(subSchiAtomType);

                auto schemeType = m_schemeTypeAtom->GetSchemeType().GetString();

                if (schemeType == "podv")
                {
                    if (subSchiAtomType == "povd")
                    {
                        m_projectedOmniVideoAtom =
                            std::move(MakeUnique<ProjectedOmniVideoAtom, ProjectedOmniVideoAtom>());
                        m_projectedOmniVideoAtom->FromStream(subSchiBitstr);
                    }

                    if (subSchiAtomType == "stvi")
                    {
                        m_stereoVideoAtom = std::move(MakeUnique<StereoVideoAtom, StereoVideoAtom>());
                        m_stereoVideoAtom->FromStream(subSchiBitstr);
                    }
                }
                else
                {
                    ISO_LOG(LOG_WARNING, "Skipping unsupported scheme type '%s'\n", schemeType.c_str());
                    break;
                }
            }
        }
        else
        {
			char type[4];
            AtomType.GetString().copy(type, 4, 0);
            ISO_LOG(LOG_WARNING, "Skipping unsupported Atom in rinf '%s'\n", type);
        }
    }
}

FourCCInt RestrictedSchemeInfoAtom::GetOriginalFormat() const
{
    if (!m_originalFormatAtom)
    {
        ISO_LOG(LOG_ERROR, "Frma Atom was not found\n");
        throw Exception();
    }
    FourCCInt ret = m_originalFormatAtom->GetOriginalFormat();
    return ret;
}

void RestrictedSchemeInfoAtom::SetOriginalFormat(FourCCInt origFormat)
{
    if (!m_originalFormatAtom)
    {
        m_originalFormatAtom = move(MakeUnique<OriginalFormatAtom, OriginalFormatAtom>());
    }

    m_originalFormatAtom->SetOriginalFormat(origFormat);
}

FourCCInt RestrictedSchemeInfoAtom::GetSchemeType() const
{
    if (!m_schemeTypeAtom)
    {
        ISO_LOG(LOG_ERROR, "Schm Atom was not found\n");
        throw Exception();
    }
    FourCCInt ret = m_schemeTypeAtom->GetSchemeType();
    return ret;
}

SchemeTypeAtom& RestrictedSchemeInfoAtom::GetSchemeTypeAtom() const
{
    return *m_schemeTypeAtom;
}

void RestrictedSchemeInfoAtom::AddSchemeTypeAtom(UniquePtr<SchemeTypeAtom> schemeTypeAtom)
{
    m_schemeTypeAtom = std::move(schemeTypeAtom);
}

bool RestrictedSchemeInfoAtom::HasSchemeTypeAtom() const
{
    return !!m_schemeTypeAtom;
}

ProjectedOmniVideoAtom& RestrictedSchemeInfoAtom::GetProjectedOmniVideoAtom() const
{
    if (!m_projectedOmniVideoAtom)
    {
        ISO_LOG(LOG_ERROR, "POVD Atom was not found\n");
        throw Exception();
    }
    return *m_projectedOmniVideoAtom;
}

void RestrictedSchemeInfoAtom::AddProjectedOmniVideoAtom(UniquePtr<ProjectedOmniVideoAtom> povdAtom)
{
    m_projectedOmniVideoAtom = std::move(povdAtom);
}

StereoVideoAtom& RestrictedSchemeInfoAtom::GetStereoVideoAtom() const
{
    if (!m_stereoVideoAtom)
    {
        ISO_LOG(LOG_ERROR, "Stvi Atom was not found\n");
        throw Exception();
    }
    return *m_stereoVideoAtom;
}

void RestrictedSchemeInfoAtom::AddStereoVideoAtom(UniquePtr<StereoVideoAtom> stviAtom)
{
    m_stereoVideoAtom = std::move(stviAtom);
}

bool RestrictedSchemeInfoAtom::HasStereoVideoAtom() const
{
    return !!m_stereoVideoAtom;
}

std::vector<CompatibleSchemeTypeAtom*> RestrictedSchemeInfoAtom::GetCompatibleSchemeTypes() const
{
    std::vector<CompatibleSchemeTypeAtom*> schemeTypes;
    for (auto& schemeType : m_compatibleSchemeTypes)
    {
        schemeTypes.push_back(schemeType.get());
    }
    return schemeTypes;
}

void RestrictedSchemeInfoAtom::AddCompatibleSchemeTypeAtom(UniquePtr<CompatibleSchemeTypeAtom> compatibleSchemeType)
{
    m_compatibleSchemeTypes.push_back(std::move(compatibleSchemeType));
}

const uint32_t SINGLEVIEWMASK = 0x3;

StereoVideoAtom::StereoVideoAtom()
    : FullAtom("stvi", 0, 0)
    , m_singleViewAllowed(StereoVideoAtom::SingleViewMode::NONE_MODE)
    , m_stereoScheme(StereoVideoAtom::SchemeSpec::SPEC14496)
{
}

void StereoVideoAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);
    str.Write32((uint32_t) m_singleViewAllowed & SINGLEVIEWMASK);
    str.Write32((uint32_t) m_stereoScheme);

    switch (m_stereoScheme)
    {
    case SchemeSpec::SPEC13818:
    case SchemeSpec::SPEC14496:
        str.Write32(4);  // length 4
        str.Write32((uint32_t) m_stereoIndType.valAsUint32);
        break;
    case SchemeSpec::SPEC23000:
        str.Write32(2);
        str.Write8((uint8_t) m_stereoIndType.type23000.compositionType);
        str.Write8(m_stereoIndType.type23000.isLeftFirst ? 0x01 : 0x00);
        break;
    case SchemeSpec::POVD:
        str.Write32(2);
        str.Write8((uint8_t) m_stereoIndType.typePOVD.compositionType);
        str.Write8(m_stereoIndType.typePOVD.useQuincunxSampling ? 0x01 : 0x00);
        break;
    }

    UpdateSize(str);
}

void StereoVideoAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);

    m_singleViewAllowed                  = (SingleViewMode)(str.Read32() & SINGLEVIEWMASK);
    m_stereoScheme                       = (SchemeSpec)(str.Read32());
    uint32_t pLength = str.Read32();

    auto checkLen = [&](uint32_t len) {
        if (pLength != len)
        {
            ISO_LOG(LOG_INFO, "Invalid length ( %d, ) for stvi stereo_indication_type data.\n", pLength);
            ISO_LOG(LOG_INFO, "For scheme_type %d, expected length is %d\n", (uint32_t) m_stereoScheme, len);
        }
    };

    switch (m_stereoScheme)
    {
    case SchemeSpec::SPEC13818:
    case SchemeSpec::SPEC14496:
        m_stereoIndType.valAsUint32 = str.Read32();
        checkLen(4);
        break;
    case SchemeSpec::SPEC23000:
        m_stereoIndType.type23000.compositionType = (ISO23000StereoCompType) str.Read8();
        m_stereoIndType.type23000.isLeftFirst     = str.Read8() & 0x01;
        checkLen(2);
        break;
    case SchemeSpec::POVD:
        m_stereoIndType.typePOVD.compositionType     = (POVDFrameCompType) str.Read8();
        m_stereoIndType.typePOVD.useQuincunxSampling = str.Read8() & 0x01;
        checkLen(2);
        break;
    }
}

VCD_MP4_END
