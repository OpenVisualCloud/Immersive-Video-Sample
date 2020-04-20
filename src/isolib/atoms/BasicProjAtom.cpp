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
//! \file:   BasicProjAtom.cpp
//! \brief:  BasicProjAtom class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include "BasicProjAtom.h"
#include <cassert>

VCD_MP4_BEGIN

ProjectionHeader::ProjectionHeader()
    : FullAtom("prhd", 0, 0)
    , m_pose{}
{
}

void ProjectionHeader::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);
    str.Write32(static_cast<uint32_t>(m_pose.yaw));
    str.Write32(static_cast<uint32_t>(m_pose.pitch));
    str.Write32(static_cast<uint32_t>(m_pose.roll));
    UpdateSize(str);
}

void ProjectionHeader::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);
    m_pose.yaw   = static_cast<int32_t>(str.Read32());
    m_pose.pitch = static_cast<int32_t>(str.Read32());
    m_pose.roll  = static_cast<int32_t>(str.Read32());
}

ProjectionDataAtom::ProjectionDataAtom(FourCCInt proj_type, uint8_t version, uint32_t flags)
    : FullAtom(proj_type, version, flags)
{
}

void ProjectionDataAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);
    UpdateSize(str);
}

void ProjectionDataAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);
}

CubemapProjection::CubemapProjection()
    : ProjectionDataAtom("cbmp", 0, 0)
    , m_layout(0)
    , m_padding(0)
{
}

void CubemapProjection::ToStream(Stream& str)
{
    ProjectionDataAtom::ToStream(str);
    str.Write32(m_layout);
    str.Write32(m_padding);
    UpdateSize(str);
}

void CubemapProjection::FromStream(Stream& str)
{
    ProjectionDataAtom::FromStream(str);
    m_layout  = str.Read32();
    m_padding = str.Read32();
}

EquirectangularProjection::EquirectangularProjection()
    : ProjectionDataAtom("equi", 0, 0)
    , m_projectionBounds{}
{
}

void EquirectangularProjection::ToStream(Stream& str)
{
    ProjectionDataAtom::ToStream(str);
    str.Write32(m_projectionBounds.top_32FP);
    str.Write32(m_projectionBounds.bottom_32FP);
    str.Write32(m_projectionBounds.left_32FP);
    str.Write32(m_projectionBounds.right_32FP);
    UpdateSize(str);
}

void EquirectangularProjection::FromStream(Stream& str)
{
    ProjectionDataAtom::FromStream(str);
    m_projectionBounds.top_32FP    = str.Read32();
    m_projectionBounds.bottom_32FP = str.Read32();
    m_projectionBounds.left_32FP   = str.Read32();
    m_projectionBounds.right_32FP  = str.Read32();
}

Projection::Projection()
    : Atom("proj")
    , m_projectionHeaderAtom()
    , m_projectionFormat(ProjectFormat::UNKNOWN)
    , m_cubemapProjectionAtom()
    , m_equirectangularProjectionAtom()
{
}

Projection::ProjectFormat Projection::GetProjectFormat() const
{
    return m_projectionFormat;
}

const CubemapProjection& Projection::GetCubemapProjectionAtom() const
{
    return m_cubemapProjectionAtom;
}

void Projection::SetCubemapProjectionAtom(const CubemapProjection& projection)
{
    m_projectionFormat       = ProjectFormat::CUBEMAP;
    m_cubemapProjectionAtom = projection;
}

const EquirectangularProjection& Projection::GetEquirectangularProjectionAtom() const
{
    return m_equirectangularProjectionAtom;
}

void Projection::SetEquirectangularProjectionAtom(const EquirectangularProjection& projection)
{
    m_projectionFormat               = ProjectFormat::ERP;
    m_equirectangularProjectionAtom = projection;
}

void Projection::ToStream(Stream& str)
{
    // Write Atom headers
    WriteAtomHeader(str);

    m_projectionHeaderAtom.ToStream(str);
    if (m_projectionFormat == ProjectFormat::CUBEMAP)
    {
        m_cubemapProjectionAtom.ToStream(str);
    }
    else if (m_projectionFormat == ProjectFormat::ERP)
    {
        m_equirectangularProjectionAtom.ToStream(str);
    }
    else
    {
        // MESH PROJECTION NOT SUPPORTED
        assert(false);
    }

    UpdateSize(str);
}

void Projection::FromStream(Stream& str)
{
    ParseAtomHeader(str);

    while (str.BytesRemain() >= 8)
    {
        FourCCInt AtomType;
        Stream subBitstr = str.ReadSubAtomStream(AtomType);
        if (AtomType == "prhd")
        {
            m_projectionHeaderAtom.FromStream(subBitstr);
        }
        else if (AtomType == "cbmp")
        {
            m_projectionFormat = ProjectFormat::CUBEMAP;
            m_cubemapProjectionAtom.FromStream(subBitstr);
        }
        else if (AtomType == "equi")
        {
            m_projectionFormat = ProjectFormat::ERP;
            m_equirectangularProjectionAtom.FromStream(subBitstr);
        }
        else if (AtomType == "mshp")
        {
            m_projectionFormat = ProjectFormat::MESH;
            // this projection type Atom parsing not supported so skip.
        }
        // unknown Atoms are skipped.
    }
}

VCD_MP4_END