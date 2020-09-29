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
//! \file:   AvcConfigAtom.cpp
//! \brief:  AvcConfigAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "AvcConfigAtom.h"
#include "Stream.h"
#include "AvcParser.h"
#include "NalUtil.h"

#include <cassert>

VCD_MP4_BEGIN

AvcConfigurationAtom::AvcConfigurationAtom()
    : Atom("avcC")
    , m_avcConfig()
{
}

AvcConfigurationAtom::AvcConfigurationAtom(const AvcConfigurationAtom& atom)
    : Atom(atom.GetType())
    , m_avcConfig(atom.m_avcConfig)
{
}

void AvcConfigurationAtom::ToStream(Stream& str)
{
    WriteAtomHeader(str);
    m_avcConfig.WriteDecConfigRec(str);
    UpdateSize(str);
}

void AvcConfigurationAtom::FromStream(Stream& str)
{
    ParseAtomHeader(str);
    m_avcConfig.ParseConfig(str);
}

AvcDecoderConfigurationRecord::AvcDecoderConfigurationRecord()
    : m_configVersion(1)
    , m_avcProfileIdc(66)
    , m_profileCompat(128)
    , m_avcLevelIdc(30)
    , m_lengthSizeMinus1(3)
    , m_chromaFormat(0)
    , m_bitDepthLuma(0)
    , m_bitDepthChroma(0)
    , m_picWidth(0)
    , m_picHeight(0)
    , m_nalArray()
{
}

bool AvcDecoderConfigurationRecord::ConfigSPS(const std::vector<uint8_t>& sps)
{
    const std::vector<uint8_t> rbsp = TransferStreamToRBSP(sps);
    Stream str(rbsp);
    SPSCfgs pConfig;

    str.Read8();
    if (!parseSPS(str, pConfig))
    {
        return false;
    }

    m_avcProfileIdc = pConfig.profileIdc;
    m_profileCompat = pConfig.profileCompatibility;
    m_avcLevelIdc   = pConfig.levelIdc;
    m_chromaFormat  = static_cast<uint8_t>(pConfig.chromaFormatIdc);
    m_bitDepthLuma   = static_cast<uint8_t>(pConfig.bitDepthLumaMinus8);
    m_bitDepthChroma = static_cast<uint8_t>(pConfig.bitDepthChromaMinus8);

    m_picWidth = (uint16_t)(pConfig.picWidthInMbsMinus1 + 1) * 16 -
                (pConfig.frameCropLeftOffset + pConfig.frameCropRightOffset) * 2;
    m_picHeight = (uint16_t)(2 - static_cast<uint32_t>(pConfig.frameMbsOnlyFlag)) *
                     (pConfig.picHeightInMapUnitsMinus1 + 1) * 16 -
                 (pConfig.frameCropTopOffset + pConfig.frameCropBottomOffset) * 2;

    return true;
}

void AvcDecoderConfigurationRecord::AddNalUnit(const std::vector<uint8_t>& nalUnit,
                                               const AvcNalDefs nalUnitType,
                                               const uint8_t arrayCompleteness)
{
    NALs* nals = nullptr;
    std::vector<uint8_t> tmpNalUnit;
    unsigned int pLen;

    for (auto& i : m_nalArray)
    {
        if (static_cast<uint8_t>(nalUnitType) == static_cast<uint8_t>(i.nalUnitType))
        {
            nals = &i;
            ISO_LOG(LOG_INFO, "find nal array existed!\n");
            break;
        }
    }

    if (nals == nullptr)
    {
        NALs pNalarray;
        pNalarray.arrayCompleteness = arrayCompleteness;
        pNalarray.nalUnitType       = nalUnitType;
        m_nalArray.push_back(pNalarray);
        nals = &m_nalArray.back();
    }

    pLen = FindStartCodeLen(nalUnit);
    tmpNalUnit.insert(tmpNalUnit.begin(), nalUnit.cbegin() + static_cast<int>(pLen),
                      nalUnit.cend());

    nals->nalList.push_back(tmpNalUnit);
}

void AvcDecoderConfigurationRecord::WriteDecConfigRec(Stream& str) const
{
    str.Write1(m_configVersion, 8);
    str.Write1(m_avcProfileIdc, 8);
    str.Write1(m_profileCompat, 8);
    str.Write1(m_avcLevelIdc, 8);

    str.Write1(0xff, 6);
    str.Write1(m_lengthSizeMinus1, 2);

    str.Write1(0xff, 3);
    const NALs* nalArray = GetNALs(AvcNalDefs::SPS);
    unsigned int cnt       = static_cast<unsigned int>(nalArray ? nalArray->nalList.size() : 0);

    if (!(cnt < (1 << 6)))
    {
        ISO_LOG(LOG_ERROR, "count invalid\n");
        return;
    }
    str.Write1(cnt, 5);

    if (cnt)
    {
        for (const auto& nal : nalArray->nalList)
        {
            str.Write1(static_cast<unsigned int>(nal.size()), 16);
            str.WriteArray(nal, static_cast<unsigned int>(nal.size()));
        }
    }

    // PPS NALS
    nalArray = GetNALs(AvcNalDefs::PPS);
    cnt    = static_cast<unsigned int>(nalArray ? nalArray->nalList.size() : 0);

    if (!(cnt < (1 << 9)))
    {
        ISO_LOG(LOG_ERROR, "count invalid\n");
        return;
    }
    str.Write1(cnt, 8);

    if (cnt)
    {
        for (const auto& nal : nalArray->nalList)
        {
            str.Write1(static_cast<unsigned int>(nal.size()), 16);
            str.WriteArray(nal, static_cast<unsigned int>(nal.size()));
        }
    }

    if (m_avcProfileIdc == 100 || m_avcProfileIdc == 110 || m_avcProfileIdc == 122 ||
        m_avcProfileIdc == 144)
    {
        str.Write1(0xff, 6);
        str.Write1(m_chromaFormat, 2);
        str.Write1(0xff, 5);
        str.Write1(m_bitDepthLuma, 3);
        str.Write1(0xff, 5);
        str.Write1(m_bitDepthChroma, 3);

        nalArray = GetNALs(AvcNalDefs::SPS_EXT);
        cnt    = static_cast<unsigned int>(nalArray ? nalArray->nalList.size() : 0);

        if (!(cnt < (1 << 9)))
        {
            ISO_LOG(LOG_ERROR, "count invalid\n");
            return;
        }
        str.Write1(cnt, 8);

        if (cnt)
        {
            for (const auto& nal : nalArray->nalList)
            {
                str.Write1(static_cast<unsigned int>(nal.size()), 16);
                str.WriteArray(nal, static_cast<unsigned int>(nal.size()));
            }
        }
    }
}

void AvcDecoderConfigurationRecord::ParseConfig(Stream& str)
{
    m_configVersion = static_cast<uint8_t>(str.Read1(8));
    m_avcProfileIdc = static_cast<uint8_t>(str.Read1(8));
    m_profileCompat = static_cast<uint8_t>(str.Read1(8));
    m_avcLevelIdc   = static_cast<uint8_t>(str.Read1(8));

    str.Read1(6);  // reserved = '111111'b
    m_lengthSizeMinus1 = static_cast<uint8_t>(str.Read1(2));

    // SPS NALS
    str.Read1(3);  // reserved = '111'b
    unsigned int cnt = static_cast<uint8_t>(str.Read1(5));

    for (unsigned int nal = 0; nal < cnt; ++nal)
    {
        unsigned int nalSize = str.Read1(16);

        std::vector<uint8_t> nalData;
        nalData.clear();
        str.ReadArray(nalData, nalSize);  // read parameter set NAL unit
        AddNalUnit(nalData, AvcNalDefs::SPS);
    }

    // PPS NALS
    cnt = static_cast<uint8_t>(str.Read1(8));

    for (unsigned int nal = 0; nal < cnt; ++nal)
    {
        unsigned int nalSize = str.Read1(16);

        std::vector<uint8_t> nalData;
        nalData.clear();
        str.ReadArray(nalData, nalSize);  // read parameter set NAL unit
        AddNalUnit(nalData, AvcNalDefs::PPS);
    }

    if (str.GetSize() == str.GetPos())
    {
        ISO_LOG(LOG_INFO, "Stop reading if there is no more data\n");
        return;
    }

    if (m_avcProfileIdc == 100 || m_avcProfileIdc == 110 || m_avcProfileIdc == 122 ||
        m_avcProfileIdc == 144)
    {
        str.Read1(6);  // reserved = '111111'b
        m_chromaFormat = static_cast<uint8_t>(str.Read1(2));
        str.Read1(5);  // reserved = '11111'b
        m_bitDepthLuma = static_cast<uint8_t>(str.Read1(3));
        str.Read1(5);  // reserved = '11111'b
        m_bitDepthChroma = static_cast<uint8_t>(str.Read1(3));

        // SPS EXT NALS
        cnt = static_cast<uint8_t>(str.Read1(8));

        for (unsigned int nal = 0; nal < cnt; ++nal)
        {
            unsigned int nalSize = str.Read1(16);

            std::vector<uint8_t> nalData;
            nalData.clear();
            str.ReadArray(nalData, nalSize);  // Read parameter set NAL unit.
            AddNalUnit(nalData, AvcNalDefs::SPS_EXT);
        }
    }
}

const AvcDecoderConfigurationRecord::NALs*
AvcDecoderConfigurationRecord::GetNALs(AvcNalDefs nalUnitType) const
{
    for (const auto& array : m_nalArray)
    {
        if (array.nalUnitType == nalUnitType)
        {
            return &array;  // Found
        }
    }

    return nullptr;  // Not found
}

void AvcDecoderConfigurationRecord::GetOneParameterSet(std::vector<uint8_t>& byteStream,
                                                       const AvcNalDefs nalUnitType) const
{
    const NALs* nalArray = GetNALs(nalUnitType);

    if (nalArray && nalArray->nalList.size() > 0)
    {
        // Add start code (0x00000001) before the NAL unit.
        byteStream.push_back(0);
        byteStream.push_back(0);
        byteStream.push_back(0);
        byteStream.push_back(1);
        byteStream.insert(byteStream.end(), nalArray->nalList.at(0).cbegin(), nalArray->nalList.at(0).cend());
    }
}

void AvcDecoderConfigurationRecord::GetConfigurationMap(ConfigurationMap& aMap) const
{
    std::vector<std::uint8_t> sps;
    std::vector<std::uint8_t> pps;
    GetOneParameterSet(sps, AvcNalDefs::SPS);
    GetOneParameterSet(pps, AvcNalDefs::PPS);

    aMap.clear();
    aMap.insert({DecParam::AVC_SPS, move(sps)});
    aMap.insert({DecParam::AVC_PPS, move(pps)});
}

VCD_MP4_END