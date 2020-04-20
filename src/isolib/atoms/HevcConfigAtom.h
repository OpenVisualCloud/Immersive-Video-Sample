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
//! \file:   HevcConfigAtom.h
//! \brief:  HEVC Configuration Atom class
//! \detail: 'hvcC' Atom implementation.
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _HEVCCONFIGURATIONATOM_H_
#define _HEVCCONFIGURATIONATOM_H_

#include "Atom.h"
#include "FormAllocator.h"
#include "DecConfigRecord.h"
#include <cstdint>

VCD_MP4_BEGIN

class Stream;

enum class HevcNalDefs : std::uint8_t   //!< hevc nal unit definition
{
    SLICE_TRAIL_N = 0,  // 0
    SLICE_TRAIL_R,      // 1

    SLICE_TSA_N,  // 2
    SLICE_TSA_R,  // 3

    SLICE_STSA_N,  // 4
    SLICE_STSA_R,  // 5

    SLICE_RADL_N,  // 6
    SLICE_RADL_R,  // 7

    SLICE_RASL_N,  // 8
    SLICE_RASL_R,  // 9

    VCL_N10,
    VCL_R11,
    VCL_N12,
    VCL_R13,
    VCL_N14,
    VCL_R15,

    SLICE_BLA_W_LP,    // 16
    SLICE_BLA_W_RADL,  // 17
    SLICE_BLA_N_LP,    // 18
    SLICE_IDR_W_RADL,  // 19
    SLICE_IDR_N_LP,    // 20
    SLICE_CRA,         // 21
    IRAP_VCL22,
    IRAP_VCL23,

    VCL24,
    VCL25,
    VCL26,
    VCL27,
    VCL28,
    VCL29,
    VCL30,
    VCL31,

    VPS,                    // 32
    SPS,                    // 33
    PPS,                    // 34
    ACCESS_UNIT_DELIMITER,  // 35
    EOS,                    // 36
    EOB,                    // 37
    FILLER_DATA,            // 38
    PREFIX_SEI,             // 39
    SUFFIX_SEI,             // 40
    NVCL41,
    NVCL42,
    NVCL43,
    NVCL44,
    NVCL45,
    NVCL46,
    NVCL47,
    UNFORM48,
    UNFORM49,
    UNFORM50,
    UNFORM51,
    UNFORM52,
    UNFORM53,
    UNFORM54,
    UNFORM55,
    UNFORM56,
    UNFORM57,
    UNFORM58,
    UNFORM59,
    UNFORM60,
    UNFORM61,
    UNFORM62,
    UNFORM63,
    INVALID
};

class HevcDecoderConfigurationRecord : public DecoderConfigurationRecord
{
public:

    //!
    //! \brief Constructor
    //!
    HevcDecoderConfigurationRecord();

    //!
    //! \brief Destructor
    //!
    ~HevcDecoderConfigurationRecord() = default;

    //!
    //! \brief    Parse configuration information from a SPS NAL unit
    //!
    //! \param    [in] const std::vector<std::uint8_t>&
    //!           sps value
    //! \param    [in] float
    //!           frameRate
    //!
    //! \return   void
    //!
    void ConfigSPS(const std::vector<std::uint8_t> &sps, float frameRate);

    //!
    //! \brief    Add NAL unit to the NAL unit array
    //!
    //! \param    [in] const std::vector<std::uint8_t>&
    //!           sps value
    //! \param    [in] AvcNalDefs
    //!           nal unit type defs
    //! \param    [in] std::uint8_t
    //!           arrayCompleteness
    //!
    //! \return   void
    //!
    void AddNalUnit(const std::vector<std::uint8_t> &sps, HevcNalDefs nalUnitType, std::uint8_t arrayCompleteness);

    //!
    //! \brief    Write Decoder Configuration Record
    //!
    //! \param    [in] Stream&
    //!           bitstream
    //!
    //! \return   void
    //!
    void WriteDecConfigRec(Stream &str) const;

    //!
    //! \brief    Parse Decoder Configuration Record
    //!
    //! \param    [in] Stream&
    //!           bitstream
    //!
    //! \return   void
    //!
    void ParseConfig(Stream &str);

    //!
    //! \brief    get one parameters set
    //!
    //! \param    [in] std::vector<std::uint8_t>&
    //!           byte stream
    //! \param    [in] AvcNalDefs
    //!           nal unit type defs
    //!
    //! \return   void
    //!
    void GetOneParameterSet(std::vector<std::uint8_t> &byteStream, HevcNalDefs nalUnitType) const;

    //!
    //! \brief    Get Picture Width
    //!
    //! \return   std::uint16_t
    //!           Picture Width
    //!
    std::uint16_t GetPicWidth() const;

    //!
    //! \brief    Get Picture Height
    //!
    //! \return   std::uint16_t
    //!           Picture Height
    //!
    std::uint16_t GetPicHeight() const;

    //!
    //! \brief    Get Avg Frame Rate
    //!
    //! \return   std::uint16_t
    //!           Avg Frame Rate
    //!
    std::uint16_t GetAvgFrameRate() const;

    //!
    //! \brief    Get Length Size Minus 1
    //!
    //! \return   std::uint8_t
    //!           LengthSizeMinus1
    //!
    std::uint8_t GetLengthSizeMinus1() const;

    //!
    //! \brief    Parse Decoder Configuration Record
    //!
    //! \param    [in] Stream&
    //!           bitstream
    //!
    //! \return   void
    //!
    virtual void GetConfigurationMap(ConfigurationMap &aMap) const override;

private:

    struct NALs //!< nal array
    {
        std::uint8_t arrayCompleteness = 0;
        HevcNalDefs nalUnitType    = HevcNalDefs::INVALID;
        std::vector<std::vector<std::uint8_t>> nalList;
    };

    std::uint8_t m_configVersion;                       //!< configuration version
    std::uint8_t m_globalProfileSpace;                  //!< global Profile Space
    std::uint8_t m_globalTierFlag;                      //!< global Tier Flag
    std::uint8_t m_globalProfileIdc;                    //!< global Profile Idc
    std::uint32_t m_globalProfileCompatFlags;           //!< global Profile Compat Flags
    std::vector<std::uint8_t> m_globalConstrainIdcFlags;//!< global Constrain Idc Flags
    std::uint8_t m_globalLevelIdc;                      //!< global Level Idc
    std::uint16_t m_minSpatialSegIdc;                   //!< min Spatial Seg Idc
    std::uint8_t m_parallelismType;                     //!< parallelism Type
    std::uint8_t m_chromaFormat;                        //!< chroma Format
    std::uint16_t m_picWidthSamples;                    //!< picture Width Samples
    std::uint16_t m_picHeightSamples;                   //!< picture Height Samples
    std::uint16_t m_confWinLeftOffset;                  //!< conf Win Left Offset
    std::uint16_t m_confWinRightOffset;                 //!< conf Win Right Offset
    std::uint16_t m_confWinTopOffset;                   //!< conf Win Top Offset
    std::uint16_t m_confWinBottomOffset;                //!< conf Win Bottom Offset
    std::uint8_t m_bitDepthLuma;                        //!< bit Depth Luma
    std::uint8_t m_bitDepthChroma;                      //!< bit Depth Chroma
    std::uint16_t m_avgFrameRate;                       //!< avg FrameRate
    std::uint8_t m_constantFrameRate;                   //!< constant FrameRate
    std::uint8_t m_numTemporalLayers;                   //!< num Temporal Layers
    std::uint8_t m_temporalIdNested;                    //!< temporal Id Nested
    std::uint8_t m_lengthSizeMinus1;                    //!< length Size Minus 1
    std::vector<NALs> m_nalArray;                       //!< nal Array
};

class HevcConfigurationAtom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    HevcConfigurationAtom();
    HevcConfigurationAtom(const HevcConfigurationAtom& Atom);

    HevcConfigurationAtom& operator=(const HevcConfigurationAtom&) = default;

    //!
    //! \brief Destructor
    //!
    virtual ~HevcConfigurationAtom() = default;

    //!
    //! \brief    Set and Get function for m_hevcConfig member
    //!
    //! \param    [in] const HevcDecoderConfigurationRecord&
    //!           value to set
    //! \param    [in] m_hevcConfig
    //!           m_hevcConfig member in class
    //! \param    [in] Configuration
    //!           m_hevcConfig name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(const HevcDecoderConfigurationRecord&, m_hevcConfig, Configuration, const);

    //!
    //! \brief    Write atom information to stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void ToStream(Stream& str);

    //!
    //! \brief    Parse atom information from stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void FromStream(Stream& str);

private:
    HevcDecoderConfigurationRecord m_hevcConfig;  //!< HEVCConfigurationAtom field HEVCConfig
};

VCD_MP4_END;
#endif /* _HEVCCONFIGURATIONATOM_H_ */
