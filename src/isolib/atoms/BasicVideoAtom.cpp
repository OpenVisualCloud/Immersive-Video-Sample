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
//! \file:   BasicVideoAtom.cpp
//! \brief:  BasicVideoAtom class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include "BasicVideoAtom.h"
#include <algorithm>
#include <string>

VCD_MP4_BEGIN

SphericalVideoHeader::SphericalVideoHeader()
    : FullAtom("svhd", 0, 0)
    , m_metadataSource("MP4 Tool " + std::string(MP4_BUILD_VERSION) + "\0")
{
}

void SphericalVideoHeader::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);
    str.WriteZeroEndString(m_metadataSource);
    UpdateSize(str);
}

void SphericalVideoHeader::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);
    str.ReadZeroEndString(m_metadataSource);
}

SphericalVideoV1Atom::SphericalVideoV1Atom()
    : Atom("uuid")
    , m_xmlMetadata()
    , m_globalMetadata{}
{
    std::vector<uint8_t> uuid = SPHERICAL_VIDEOV1_GENERAL_UUID;
    SetUserType(uuid);
}

template <typename T>
void SphericalVideoV1Atom::ReadTag(const std::string& tag, T& data)
{
    std::string xmltag = "<" + tag + ">";
    size_t offset = m_xmlMetadata.find(xmltag);
    if (offset != std::string::npos)
    {
        size_t end = m_xmlMetadata.find("</" + tag + ">");
        if (end != std::string::npos)
        {
            if (end > offset && (end - offset - xmltag.length()) > 0)
            {
                std::string valueStr =
                    m_xmlMetadata.substr(offset + xmltag.length(), end - offset - xmltag.length());
                remove_if(valueStr.begin(), valueStr.end(), isspace);
                data = static_cast<T>(std::stoi(valueStr.c_str(), nullptr));
            }
        }
    }
    return;
}

template <>
void SphericalVideoV1Atom::ReadTag(const std::string& tag, bool& data)
{
    std::string xmltag = "<" + tag + ">";
    size_t offset = m_xmlMetadata.find(xmltag);
    if (offset != std::string::npos)
    {
        size_t end = m_xmlMetadata.find("</" + tag + ">");
        if (end != std::string::npos)
        {
            if (end > offset && (end - offset - xmltag.length()) > 0)
            {
                std::string valueStr =
                    m_xmlMetadata.substr(offset + xmltag.length(), end - offset - xmltag.length());
                if (valueStr.find("true") != std::string::npos || valueStr.find("1") != std::string::npos)
                {
                    data = true;
                }
                else if (valueStr.find("false") != std::string::npos || valueStr.find("0") != std::string::npos)
                {
                    data = false;
                }
                else
                {
                    ISO_LOG(LOG_WARNING, "Parsing Error in SphericalVideoV1Atom/%s data %s\n", tag.c_str(), valueStr.c_str());
                }
            }
        }
    }
}

template <typename T>
void SphericalVideoV1Atom::WriteTag(Stream& str, const std::string& tag, const T data)
{
    std::string xml = "<" + tag + ">";
    str.WriteString(xml);
    xml = std::to_string(data).c_str();
    str.WriteString(xml);
    xml = "</" + tag + ">\n";
    str.WriteString(xml);
}

template <>
void SphericalVideoV1Atom::WriteTag(Stream& str, const std::string& tag, const bool data)
{
    std::string xml = "<" + tag + ">";
    str.WriteString(xml);
    xml = data ? "true" : "false";
    str.WriteString(xml);
    xml = "</" + tag + ">\n";
    str.WriteString(xml);
}

template <>
void SphericalVideoV1Atom::WriteTag(Stream& str, const std::string& tag, const std::string data)
{
    std::string xml = "<" + tag + ">";
    str.WriteString(xml);
    str.WriteString(data);
    xml = "</" + tag + ">\n";
    str.WriteString(xml);
}

// This is so that character literals can be used as the data argument
template <>
void SphericalVideoV1Atom::WriteTag(Stream& str, const std::string& tag, const char* data)
{
    WriteTag(str, tag, std::string(data));
}

template <>
void SphericalVideoV1Atom::ReadTag(const std::string& tag, std::string& data)
{
    std::string xmltag = "<" + tag + ">";
    size_t offset = m_xmlMetadata.find(xmltag);
    if (offset != std::string::npos)
    {
        size_t end = m_xmlMetadata.find("</" + tag + ">");
        if (end != std::string::npos)
        {
            if (end > offset && (end - offset - xmltag.length()) > 0)
            {
                data = m_xmlMetadata.substr(offset + xmltag.length(), end - offset - xmltag.length());
            }
        }
    }
}

void SphericalVideoV1Atom::ToStream(Stream& str)
{
    WriteAtomHeader(str);
    std::string header =
        "<?xml "
        "version=\"1.0\"?><rdf:SphericalVideo\nxmlns:rdf=\""
        "22-rdf-syntax-ns#\"\nxmlns:GSpherical=\">";
    str.WriteString(header);

    WriteTag(str, "GSpherical:Spherical", true);  // must be true on v1.0
    WriteTag(str, "GSpherical:Stitched", true);   // must be true on v1.0
    WriteTag(str, "GSpherical:StitchingSoftware", m_globalMetadata.stitchedSW);
    WriteTag(str, "GSpherical:ProjectFormat", "equirectangular");  // must be "equirectangular" on v1.0

    if (m_globalMetadata.stereoType != StereoTypeV1::UNDEFINED)
    {
        std::string stereoType;
        if (m_globalMetadata.stereoType == StereoTypeV1::STEREO_TOP_BOTTOM)
        {
            stereoType = "top-bottom";
        }
        else if (m_globalMetadata.stereoType == StereoTypeV1::STEREO_LEFT_RIGHT)
        {
            stereoType = "left-right";
        }
        else
        {
            stereoType = "mono";
        }
        WriteTag(str, "GSpherical:StereoMode", stereoType);
    }

    if (m_globalMetadata.sourceCount)
    {
        WriteTag(str, "GSpherical:SourceCount", m_globalMetadata.sourceCount);
    }
    if (m_globalMetadata.initViewHead)
    {
        WriteTag(str, "GSpherical:InitialViewHeadingDegrees", m_globalMetadata.initViewHead);
    }
    if (m_globalMetadata.initViewPitch)
    {
        WriteTag(str, "GSpherical:InitialViewPitchDegrees", m_globalMetadata.initViewPitch);
    }
    if (m_globalMetadata.initViewRoll)
    {
        WriteTag(str, "GSpherical:InitialViewRollDegrees", m_globalMetadata.initViewRoll);
    }
    if (m_globalMetadata.timestamp)
    {
        WriteTag(str, "GSpherical:Timestamp", m_globalMetadata.timestamp);
    }
    if (m_globalMetadata.fullPanoWidth)
    {
        WriteTag(str, "GSpherical:FullPanoWidthPixels", m_globalMetadata.fullPanoWidth);
    }
    if (m_globalMetadata.fullPanoHeight)
    {
        WriteTag(str, "GSpherical:FullPanoHeightPixels", m_globalMetadata.fullPanoHeight);
    }
    if (m_globalMetadata.croppedAreaImageWidth)
    {
        WriteTag(str, "GSpherical:CroppedAreaImageWidthPixels", m_globalMetadata.croppedAreaImageWidth);
    }
    if (m_globalMetadata.croppedAreaImageHeight)
    {
        WriteTag(str, "GSpherical:CroppedAreaImageHeightPixels", m_globalMetadata.croppedAreaImageHeight);
    }
    if (m_globalMetadata.croppedAreaLeft)
    {
        WriteTag(str, "GSpherical:CroppedAreaLeftPixels", m_globalMetadata.croppedAreaLeft);
    }
    if (m_globalMetadata.croppedAreaTop)
    {
        WriteTag(str, "GSpherical:CroppedAreaTopPixels", m_globalMetadata.croppedAreaTop);
    }

    std::string footer = "</rdf:SphericalVideo>";
    str.WriteString(footer);
    UpdateSize(str);
}

void SphericalVideoV1Atom::FromStream(Stream& str)
{
    ParseAtomHeader(str);
    str.ReadStringWithLen(m_xmlMetadata, static_cast<uint32_t>(str.BytesRemain()));

    std::string tag = "GSpherical:Spherical";
    ReadTag(tag, m_globalMetadata.isSpherical);

    tag = "GSpherical:Stitched";
    ReadTag(tag, m_globalMetadata.isStitched);

    tag = "GSpherical:StitchingSoftware";
    ReadTag(tag, m_globalMetadata.stitchedSW);

    tag = "GSpherical:ProjectFormat";
    std::string projection;
    ReadTag(tag, projection);
    std::transform(projection.begin(), projection.end(), projection.begin(), ::tolower);
    if (projection == "equirectangular")
    {
        m_globalMetadata.projectionFormat = ProjectFormat::ERP;
    }
    else
    {
        ISO_LOG(LOG_WARNING, "Parsing Error in SphericalVideoV1Atom/%s data %s\n", tag.c_str(), projection.c_str());
    }

    tag = "GSpherical:StereoMode";
    std::string stereoType;
    ReadTag(tag, stereoType);
    std::transform(stereoType.begin(), stereoType.end(), stereoType.begin(), ::tolower);
    if (stereoType == "mono")
    {
        m_globalMetadata.stereoType = StereoTypeV1::MONO_TYPE;
    }
    else if (stereoType == "top-bottom")
    {
        m_globalMetadata.stereoType = StereoTypeV1::STEREO_TOP_BOTTOM;
    }
    else if (stereoType == "left-right")
    {
        m_globalMetadata.stereoType = StereoTypeV1::STEREO_LEFT_RIGHT;
    }
    else
    {
        m_globalMetadata.stereoType = StereoTypeV1::UNDEFINED;
    }

    tag = "GSpherical:SourceCount";
    ReadTag(tag, m_globalMetadata.sourceCount);

    tag = "GSpherical:InitialViewHeadingDegrees";
    ReadTag(tag, m_globalMetadata.initViewHead);

    tag = "GSpherical:InitialViewPitchDegrees";
    ReadTag(tag, m_globalMetadata.initViewPitch);

    tag = "GSpherical:InitialViewRollDegrees";
    ReadTag(tag, m_globalMetadata.initViewRoll);

    tag = "GSpherical:Timestamp";
    ReadTag(tag, m_globalMetadata.timestamp);

    tag = "GSpherical:FullPanoWidthPixels";
    ReadTag(tag, m_globalMetadata.fullPanoWidth);

    tag = "GSpherical:FullPanoHeightPixels";
    ReadTag(tag, m_globalMetadata.fullPanoHeight);

    tag = "GSpherical:CroppedAreaImageWidthPixels";
    ReadTag(tag, m_globalMetadata.croppedAreaImageWidth);

    tag = "GSpherical:CroppedAreaImageHeightPixels";
    ReadTag(tag, m_globalMetadata.croppedAreaImageHeight);

    tag = "GSpherical:CroppedAreaLeftPixels";
    ReadTag(tag, m_globalMetadata.croppedAreaLeft);

    tag = "GSpherical:CroppedAreaTopPixels";
    ReadTag(tag, m_globalMetadata.croppedAreaTop);
}

SphericalVideoV2Atom::SphericalVideoV2Atom()
    : Atom("sv3d")
    , m_sphericalVideoHeaderAtom()
    , m_projectionAtom()
{
}

void SphericalVideoV2Atom::ToStream(Stream& str)
{
    // Write Atom headers
    WriteAtomHeader(str);

    m_sphericalVideoHeaderAtom.ToStream(str);
    m_projectionAtom.ToStream(str);

    UpdateSize(str);
}

void SphericalVideoV2Atom::FromStream(Stream& str)
{
    ParseAtomHeader(str);

    while (str.BytesRemain() >= 8)
    {
        FourCCInt AtomType;
        Stream subBitstr = str.ReadSubAtomStream(AtomType);
        if (AtomType == "svhd")
        {
            m_sphericalVideoHeaderAtom.FromStream(subBitstr);
        }
        else if (AtomType == "proj")
        {
            m_projectionAtom.FromStream(subBitstr);
        }
        // unknown Atoms are skipped.
    }
}

Stereoscopic3D::Stereoscopic3D()
    : FullAtom("st3d", 0, 0)
    , m_stereoMode(StereoTypeV2::MONO_TYPE)
{
}

void Stereoscopic3D::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);
    str.Write8(static_cast<uint8_t>(m_stereoMode));
    UpdateSize(str);
}

void Stereoscopic3D::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);
    m_stereoMode = static_cast<StereoTypeV2>(str.Read8());
}

VCD_MP4_END
