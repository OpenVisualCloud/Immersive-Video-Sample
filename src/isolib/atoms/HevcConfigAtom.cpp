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
//! \file:   HevcConfigAtom.cpp
//! \brief:  HevcConfigAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "HevcConfigAtom.h"
#include "Stream.h"
#include "NalUtil.h"

VCD_MP4_BEGIN

HevcDecoderConfigurationRecord::HevcDecoderConfigurationRecord()
    : m_configVersion(1)
    , m_globalProfileSpace(0)
    , m_globalTierFlag(0)
    , m_globalProfileIdc(0)
    , m_globalProfileCompatFlags(0)
    , m_globalConstrainIdcFlags(6, 0)
    , m_globalLevelIdc(0)
    , m_minSpatialSegIdc(0)
    , m_parallelismType(0)
    , m_chromaFormat(0)
    , m_picWidthSamples(0)
    , m_picHeightSamples(0)
    , m_confWinLeftOffset(0)
    , m_confWinRightOffset(0)
    , m_confWinTopOffset(0)
    , m_confWinBottomOffset(0)
    , m_bitDepthLuma(0)
    , m_bitDepthChroma(0)
    , m_avgFrameRate(0)
    , m_constantFrameRate(0)
    , m_numTemporalLayers(0)
    , m_temporalIdNested(0)
    , m_lengthSizeMinus1(0)
    , m_nalArray()
{
}

void HevcDecoderConfigurationRecord::ConfigSPS(const std::vector<uint8_t> &srcSps, float frameRate)
{
    unsigned int maxNumMinus1;
    std::vector<bool> subLayerFlag(8, 0);
    std::vector<bool> subLayerLevelFlag(8, 0);
    std::vector<uint8_t> sps = TransferStreamToRBSP(srcSps);

    auto maxFR = ((float) 0xffff) / 256;
    if (frameRate > maxFR)
    {
        frameRate = maxFR;
    }
    m_avgFrameRate      = static_cast<uint16_t>(frameRate * 256 + 0.5);
    m_constantFrameRate = 0;
    m_lengthSizeMinus1  = 3;  // NAL length fields are 4 bytes long (3+1)

    Stream str(sps);

    // NALU header
    str.Read1(1);  // forbidden_zero_bit
    str.Read1(6);  // nal_unit_type
    str.Read1(6);  // nuh_layer_id
    str.Read1(3);  // nuh_temporal_id_plus1

    str.Read1(4);                          // sps_video_parametr_set_id  -> not needed
    maxNumMinus1 = str.Read1(3);  // sps_max_sub_layers_minus1
    m_numTemporalLayers    = static_cast<uint8_t>(maxNumMinus1 + 1);
    m_temporalIdNested     = static_cast<uint8_t>(str.Read1(1));  // sps_temporal_id_nesting_flag

    // start profile_tier_level parsing

    m_globalProfileSpace              = static_cast<uint8_t>(str.Read1(2));  // general_profile_space
    m_globalTierFlag                  = static_cast<uint8_t>(str.Read1(1));  // general_tier_flag
    m_globalProfileIdc                = static_cast<uint8_t>(str.Read1(5));  // general_profile_idc
    m_globalProfileCompatFlags = 0;
    // general_profile_compatibility_flags (32 flags)
    for (int i = 0; i < 32; i++)
    {
        m_globalProfileCompatFlags = (m_globalProfileCompatFlags << 1) | str.Read1(1);
    }
    // constrain_flags (48 flags)
    for (unsigned int i = 0; i < 6; i++)
    {
        m_globalConstrainIdcFlags.at(i) = static_cast<uint8_t>(str.Read1(8));
    }
    m_globalLevelIdc = static_cast<uint8_t>(str.Read1(8));  // general_level_idc
    for (unsigned int i = 0; i < maxNumMinus1; i++)
    {
        subLayerFlag.at(i) = str.Read1(1);  // sub_layer_profile_present_flag
        subLayerLevelFlag.at(i)   = str.Read1(1);  // sub_layer_level_present_flag
    }
    if (maxNumMinus1 > 0)
    {
        for (unsigned int i = maxNumMinus1; i < 8; i++)
        {
            str.Read1(2);  // reserved_zero_2bits
        }
    }
    // The following sub-layer syntax element are not needed in the decoder
    // configuration record
    for (unsigned int i = 0; i < maxNumMinus1; i++)
    {
        if (subLayerFlag.at(i))
        {
            str.Read1(2);  // sub_layer_profile_space[i]
            str.Read1(1);  // sub_layer_tier_flag[i]
            str.Read1(5);  // sub_layer_profile_idc[i]
            for (int j = 0; j < 32; j++)
            {
                str.Read1(1);  // sub_layer_profile_compatibility_flag[i][j]
            }
            for (int j = 0; j < 6; j++)
            {
                str.Read1(8);  // Constraint flags
            }
        }
        if (subLayerLevelFlag.at(i))
        {
            str.Read1(8);  // sub_level_idc[i]
        }
    }

    // end profile_tier_level parsing

    str.ReadExpGolombCode();                                        // sps_seq_parameter_set_id
    m_chromaFormat = static_cast<uint8_t>(str.ReadExpGolombCode());  // chroma_format_idc
    if (m_chromaFormat == 3)
    {
        str.Read1(1);  // separate_colour_plane_flag
    }
    m_picWidthSamples  = static_cast<uint16_t>(str.ReadExpGolombCode());  // pic_width_in_luma_samples
    m_picHeightSamples = static_cast<uint16_t>(str.ReadExpGolombCode());  // pic_height_in_luma_samples

    if (str.Read1(1))  // conformance_window_flag
    {
        m_confWinLeftOffset   = static_cast<uint16_t>(str.ReadExpGolombCode());  // conf_win_left_offset
        m_confWinRightOffset  = static_cast<uint16_t>(str.ReadExpGolombCode());  // conf_win_right_offset
        m_confWinTopOffset    = static_cast<uint16_t>(str.ReadExpGolombCode());  // conf_win_top_offset
        m_confWinBottomOffset = static_cast<uint16_t>(str.ReadExpGolombCode());  // conf_win_bottom_offset
    }
    else
    {
        m_confWinLeftOffset   = 0;
        m_confWinRightOffset  = 0;
        m_confWinTopOffset    = 0;
        m_confWinBottomOffset = 0;
    }

    m_bitDepthLuma   = static_cast<uint8_t>(str.ReadExpGolombCode());  // bit_depth_luma_minus8
    m_bitDepthChroma = static_cast<uint8_t>(str.ReadExpGolombCode());  // bit_depth_chroma_minus8
    str.ReadExpGolombCode();                                                // log2_max_pic_order_cnt_lsb_minus4

    m_minSpatialSegIdc = 0;
    m_parallelismType           = 0;
}

void HevcDecoderConfigurationRecord::AddNalUnit(const std::vector<uint8_t> &nalUnit,
                                                const HevcNalDefs nalUnitType,
                                                const uint8_t arrCom)
{
    NALs *nalArray = nullptr;
    std::vector<uint8_t> tmpNU;
    unsigned int startCodeLen;

    // find array for the given NAL unit type
    for (auto &i : m_nalArray)
    {
        if (static_cast<uint8_t>(nalUnitType) == static_cast<uint8_t>(i.nalUnitType))
        {
            nalArray = &i;
            break;
        }
    }

    // if an array is not present for the NAL unit type, create one
    if (nullptr == nalArray)
    {
        NALs ATmp;
        ATmp.arrayCompleteness = arrCom;
        ATmp.nalUnitType       = nalUnitType;
        m_nalArray.push_back(ATmp);
        nalArray = &m_nalArray.back();
    }

    startCodeLen = FindStartCodeLen(nalUnit);
    tmpNU.insert(tmpNU.begin(), nalUnit.cbegin() + static_cast<int>(startCodeLen),
                      nalUnit.cend());  // copy NAL data excluding potential start code

    // add NAL unit to the NAL unit array
    nalArray->nalList.push_back(tmpNU);
}

void HevcDecoderConfigurationRecord::WriteDecConfigRec(Stream &str) const
{
    str.Write1(m_configVersion, 8);
    str.Write1(m_globalProfileSpace, 2);
    str.Write1(m_globalTierFlag, 1);
    str.Write1(m_globalProfileIdc, 5);
    str.Write1(m_globalProfileCompatFlags, 32);
    for (unsigned int i = 0; i < 6; i++)
    {
        str.Write1(m_globalConstrainIdcFlags.at(i), 8);
    }
    str.Write1(m_globalLevelIdc, 8);
    str.Write1(0xf, 4);  // reserved = '1111'b
    str.Write1(m_minSpatialSegIdc, 12);
    str.Write1(0x3f, 6);  // reserved = '111111'b
    str.Write1(m_parallelismType, 2);
    str.Write1(0x3f, 6);  // reserved = '111111'b
    str.Write1(m_chromaFormat, 2);
    str.Write1(0x1f, 5);  // reserved = '11111'b
    str.Write1(m_bitDepthLuma, 3);
    str.Write1(0x1f, 5);  // reserved = '11111'b
    str.Write1(m_bitDepthChroma, 3);
    str.Write1(m_avgFrameRate, 16);
    str.Write1(m_constantFrameRate, 2);
    str.Write1(m_numTemporalLayers, 3);
    str.Write1(m_temporalIdNested, 1);
    str.Write1(m_lengthSizeMinus1, 2);

    str.Write1(m_nalArray.size(), 8);
    for (const auto &i : m_nalArray)
    {
        str.Write1(i.arrayCompleteness, 1);
        str.Write1(0, 1);  // reserved = 0
        str.Write1(static_cast<uint8_t>(i.nalUnitType), 6);
        str.Write1(static_cast<unsigned int>(i.nalList.size()), 16);
        for (const auto &j : i.nalList)
        {
            str.Write1(static_cast<unsigned int>(j.size()), 16);
            str.WriteArray(j, j.size());  // write parameter set NAL unit
        }
    }
}

void HevcDecoderConfigurationRecord::ParseConfig(Stream &str)
{
    unsigned int numOfArrays;

    m_configVersion             = static_cast<uint8_t>(str.Read1(8));
    m_globalProfileSpace              = static_cast<uint8_t>(str.Read1(2));
    m_globalTierFlag                  = static_cast<uint8_t>(str.Read1(1));
    m_globalProfileIdc                = static_cast<uint8_t>(str.Read1(5));
    m_globalProfileCompatFlags = str.Read1(32);
    for (unsigned int i = 0; i < 6; i++)
    {
        m_globalConstrainIdcFlags.at(i) = static_cast<uint8_t>(str.Read1(8));
    }
    m_globalLevelIdc = static_cast<uint8_t>(str.Read1(8));
    str.Read1(4);  // reserved = '1111'b
    m_minSpatialSegIdc = static_cast<uint16_t>(str.Read1(12));
    str.Read1(6);  // reserved = '111111'b
    m_parallelismType = static_cast<uint8_t>(str.Read1(2));
    str.Read1(6);  // reserved = '111111'b
    m_chromaFormat = static_cast<uint8_t>(str.Read1(2));
    str.Read1(5);  // reserved = '11111'b
    m_bitDepthLuma = static_cast<uint8_t>(str.Read1(3));
    str.Read1(5);  // reserved = '11111'b
    m_bitDepthChroma = static_cast<uint8_t>(str.Read1(3));
    m_avgFrameRate         = static_cast<uint16_t>(str.Read1(16));
    m_constantFrameRate    = static_cast<uint8_t>(str.Read1(2));
    m_numTemporalLayers    = static_cast<uint8_t>(str.Read1(3));
    m_temporalIdNested     = static_cast<uint8_t>(str.Read1(1));
    m_lengthSizeMinus1     = static_cast<uint8_t>(str.Read1(2));

    numOfArrays = str.Read1(8);
    for (unsigned int i = 0; i < numOfArrays; i++)
    {
        uint8_t arrCom;
        HevcNalDefs type;
        unsigned int numNalus;

        arrCom = static_cast<uint8_t>(str.Read1(1));
        str.Read1(1);  // reserved = 0
        type = (HevcNalDefs) str.Read1(6);
        numNalus    = str.Read1(16);
        for (unsigned int j = 0; j < numNalus; j++)
        {
            std::vector<uint8_t> nals;
            unsigned int size;

            size = str.Read1(16);
            nals.clear();
            str.ReadArray(nals, size);  // read parameter set NAL unit
            AddNalUnit(nals, type, arrCom);
        }
    }
}

void HevcDecoderConfigurationRecord::GetOneParameterSet(std::vector<uint8_t> &str,
                                                        const HevcNalDefs type) const
{
    for (const auto &arr : m_nalArray)
    {
        if (arr.nalUnitType == type && arr.nalList.size() > 0)
        {
            for (int i=0;i<3;i++)
            {
                str.push_back(0);
            }
            str.push_back(1);
            str.insert(str.end(), arr.nalList.at(0).cbegin(), arr.nalList.at(0).cend());
        }
    }
}

uint16_t HevcDecoderConfigurationRecord::GetPicWidth() const
{
    // static const std::vector<uint16_t> pWidth = {1, 2, 2, 1};
    std::vector<uint16_t> pWidth;
    pWidth.push_back(1);
    pWidth.push_back(2);
    pWidth.push_back(2);
    pWidth.push_back(1);
    uint16_t picWidthRet = m_picWidthSamples - pWidth.at(m_chromaFormat) * (m_confWinLeftOffset + m_confWinRightOffset);
    return picWidthRet;
}

uint16_t HevcDecoderConfigurationRecord::GetPicHeight() const
{
    // static const std::vector<uint16_t> pHeight = {1, 2, 1, 1};
    std::vector<uint16_t> pHeight;
    pHeight.push_back(1);
    pHeight.push_back(2);
    pHeight.push_back(1);
    pHeight.push_back(1);
    uint16_t picHeightRet = m_picHeightSamples - pHeight.at(m_chromaFormat) * (m_confWinTopOffset + m_confWinBottomOffset);
    return picHeightRet;
}

uint16_t HevcDecoderConfigurationRecord::GetAvgFrameRate() const
{
    return m_avgFrameRate;
}

std::uint8_t HevcDecoderConfigurationRecord::GetLengthSizeMinus1() const
{
    return m_lengthSizeMinus1;
}

void HevcDecoderConfigurationRecord::GetConfigurationMap(ConfigurationMap &aMap) const
{
    std::vector<std::uint8_t> sps;
    std::vector<std::uint8_t> pps;
    std::vector<std::uint8_t> vps;
    GetOneParameterSet(sps, HevcNalDefs::SPS);
    GetOneParameterSet(pps, HevcNalDefs::PPS);
    GetOneParameterSet(vps, HevcNalDefs::VPS);

    aMap.insert({DecParam::HEVC_SPS, move(sps)});
    aMap.insert({DecParam::HEVC_PPS, move(pps)});
    aMap.insert({DecParam::HEVC_VPS, move(vps)});
}

HevcConfigurationAtom::HevcConfigurationAtom()
    : Atom("hvcC")
    , m_hevcConfig()
{
}

HevcConfigurationAtom::HevcConfigurationAtom(const HevcConfigurationAtom& atom)
    : Atom(atom.GetType())
    , m_hevcConfig(atom.m_hevcConfig)
{
}

void HevcConfigurationAtom::ToStream(Stream& str)
{
    WriteAtomHeader(str);
    m_hevcConfig.WriteDecConfigRec(str);
    UpdateSize(str);
}

void HevcConfigurationAtom::FromStream(Stream& str)
{
    ParseAtomHeader(str);
    m_hevcConfig.ParseConfig(str);
}

VCD_MP4_END