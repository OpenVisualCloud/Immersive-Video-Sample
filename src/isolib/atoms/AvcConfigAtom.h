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
//! \file:   AvcConfigAtom.h
//! \brief:  AVC Configuration Atom class
//! \detail: 'avcC' Atom implementation.
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _AVCCONFIGURATIONATOM_H_
#define _AVCCONFIGURATIONATOM_H_

#include "Atom.h"
#include "FormAllocator.h"
#include "DecConfigRecord.h"
#include "Stream.h"

VCD_MP4_BEGIN

class Stream;

/** @brief Common enumeration definitions for decoder configuration record */
enum class AvcNalDefs : std::uint8_t
{
    UNSPECIFIED_0 = 0,
    CODED_SLICE_NON_IDR,  // 1
    CODED_SLICE_DPAR_A,
    CODED_SLICE_DPAR_B,
    CODED_SLICE_DPAR_C,
    CODED_SLICE_IDR,  // 5
    SEI,              // 6
    SPS,              // 7
    PPS,              // 8
    ACCESS_UNIT_DELIMITER,
    EOS,  // 10
    EOB,  // 11
    FILLER_DATA,
    SPS_EXT,
    PREFIX_NALU,
    SUB_SPS,
    DPS,
    RESERVED_17,
    RESERVED_18,
    SLICE_AUX_NOPAR,
    SLICE_EXT,
    SLICE_EXT_3D,
    RESERVED_22,
    RESERVED_23,
    UNSPECIFIED_24,
    UNSPECIFIED_25,
    UNSPECIFIED_26,
    UNSPECIFIED_27,
    UNSPECIFIED_28,
    UNSPECIFIED_29,
    UNSPECIFIED_30,
    UNSPECIFIED_31,
    INVALID
};

class AvcDecoderConfigurationRecord : public DecoderConfigurationRecord
{
public:

    //!
    //! \brief Constructor
    //!
    AvcDecoderConfigurationRecord();

    //!
    //! \brief Destructor
    //!
    ~AvcDecoderConfigurationRecord() = default;

    //!
    //! \brief    Parse configuration information from a SPS NAL unit
    //!
    //! \param    [in] const std::vector<std::uint8_t>&
    //!           sps value
    //!
    //! \return   bool
    //!           read success or not
    //!
    bool ConfigSPS(const std::vector<std::uint8_t>& sps);

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
    void AddNalUnit(const std::vector<std::uint8_t>& sps, AvcNalDefs nalUnitType, std::uint8_t arrayCompleteness = 0);

    //!
    //! \brief    Write Decoder Configuration Record
    //!
    //! \param    [in] Stream&
    //!           bitstream
    //!
    //! \return   void
    //!
    void WriteDecConfigRec(Stream& str) const;

    //!
    //! \brief    Parse Decoder Configuration Record
    //!
    //! \param    [in] Stream&
    //!           bitstream
    //!
    //! \return   void
    //!
    void ParseConfig(Stream& str);

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
    void GetOneParameterSet(std::vector<std::uint8_t>& byteStream, AvcNalDefs nalUnitType) const;

    //!
    //! \brief    Set and Get function for m_picWidth member
    //!
    //! \param    [in] std::uint16_t
    //!           value to set
    //! \param    [in] m_picWidth
    //!           m_picWidth member in class
    //! \param    [in] PicWidth
    //!           m_picWidth name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint16_t, m_picWidth, PicWidth, const);

    //!
    //! \brief    Set and Get function for m_picHeight member
    //!
    //! \param    [in] std::uint16_t
    //!           value to set
    //! \param    [in] m_picHeight
    //!           m_picHeight member in class
    //! \param    [in] PicHeight
    //!           m_picHeight name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint16_t, m_picHeight, PicHeight, const);

    //!
    //! \brief    Set and Get function for m_configVersion member
    //!
    //! \param    [in] std::uint8_t
    //!           value to set
    //! \param    [in] m_configVersion
    //!           m_configVersion member in class
    //! \param    [in] ConfigurationVersion
    //!           m_configVersion name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint8_t, m_configVersion, ConfigurationVersion, const);

    //!
    //! \brief    Set and Get function for m_avcProfileIdc member
    //!
    //! \param    [in] std::uint8_t
    //!           value to set
    //! \param    [in] m_avcProfileIdc
    //!           m_avcProfileIdc member in class
    //! \param    [in] AvcProfileIndication
    //!           m_avcProfileIdc name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint8_t, m_avcProfileIdc, AvcProfileIndication, const);

    //!
    //! \brief    Set and Get function for m_profileCompat member
    //!
    //! \param    [in] std::uint8_t
    //!           value to set
    //! \param    [in] m_profileCompat
    //!           m_profileCompat member in class
    //! \param    [in] ProfileCompatibility
    //!           m_profileCompat name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint8_t, m_profileCompat, ProfileCompatibility, const);

    //!
    //! \brief    Set and Get function for m_avcLevelIdc member
    //!
    //! \param    [in] std::uint8_t
    //!           value to set
    //! \param    [in] m_avcLevelIdc
    //!           m_avcLevelIdc member in class
    //! \param    [in] AvcLevelIndication
    //!           m_avcLevelIdc name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint8_t, m_avcLevelIdc, AvcLevelIndication, const);

    //!
    //! \brief    Set and Get function for m_lengthSizeMinus1 member
    //!
    //! \param    [in] std::uint8_t
    //!           value to set
    //! \param    [in] m_lengthSizeMinus1
    //!           m_lengthSizeMinus1 member in class
    //! \param    [in] LengthSizeMinus1
    //!           m_lengthSizeMinus1 name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint8_t, m_lengthSizeMinus1, LengthSizeMinus1, const);

    //!
    //! \brief    Set and Get function for m_chromaFormat member
    //!
    //! \param    [in] std::uint8_t
    //!           value to set
    //! \param    [in] m_chromaFormat
    //!           m_chromaFormat member in class
    //! \param    [in] ChromaFormat
    //!           m_chromaFormat name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint8_t, m_chromaFormat, ChromaFormat, const);

    //!
    //! \brief    Set and Get function for m_bitDepthLuma member
    //!
    //! \param    [in] std::uint8_t
    //!           value to set
    //! \param    [in] m_bitDepthLuma
    //!           m_bitDepthLuma member in class
    //! \param    [in] BitDepthLumaMinus8
    //!           m_bitDepthLuma name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint8_t, m_bitDepthLuma, BitDepthLumaMinus8, const);

    //!
    //! \brief    Set and Get function for m_bitDepthChroma member
    //!
    //! \param    [in] std::uint8_t
    //!           value to set
    //! \param    [in] m_bitDepthChroma
    //!           m_bitDepthChroma member in class
    //! \param    [in] BitDepthChromaMinus8
    //!           m_bitDepthChroma name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint8_t, m_bitDepthChroma, BitDepthChromaMinus8, const);

    //!
    //! \brief    Parse Decoder Configuration Record
    //!
    //! \param    [in] Stream&
    //!           bitstream
    //!
    //! \return   void
    //!
    virtual void GetConfigurationMap(ConfigurationMap& aMap) const override;

private:

    struct NALs
    {
        std::uint8_t arrayCompleteness = 0;
        AvcNalDefs nalUnitType     = AvcNalDefs::INVALID;
        std::vector<std::vector<std::uint8_t>> nalList;
    };

    std::uint8_t m_configVersion;     //!< configuration version
    std::uint8_t m_avcProfileIdc;     //!< avc profile indication
    std::uint8_t m_profileCompat;     //!< profile compatibility
    std::uint8_t m_avcLevelIdc;       //!< avc level indication
    std::uint8_t m_lengthSizeMinus1;  //!< length size - 1

    std::uint8_t m_chromaFormat;      //!< chroma format
    std::uint8_t m_bitDepthLuma;      //!< bit depth luma
    std::uint8_t m_bitDepthChroma;    //!< bit depth chroma

    std::uint16_t m_picWidth;         //!< picture width
    std::uint16_t m_picHeight;        //!< picture height

    std::vector<NALs> m_nalArray;     //!< nal unit array

    //!
    //! \brief    Get Nal array according to nal unit type
    //!
    //! \param    [in] AvcNalDefs
    //!           nalUnitType
    //!
    //! \return   const NALs*
    //!           nal array
    //!
    const NALs* GetNALs(AvcNalDefs nalUnitType) const;
};

class AvcConfigurationAtom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    AvcConfigurationAtom();
    AvcConfigurationAtom(const AvcConfigurationAtom& Atom);

    AvcConfigurationAtom& operator=(const AvcConfigurationAtom&) = default;

    //!
    //! \brief Destructor
    //!
    virtual ~AvcConfigurationAtom() = default;

    //!
    //! \brief    Set and Get function for m_avcConfig member
    //!
    //! \param    [in] const AvcDecoderConfigurationRecord&
    //!           value to set
    //! \param    [in] m_avcConfig
    //!           m_avcConfig member in class
    //! \param    [in] Configuration
    //!           m_avcConfig name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(const AvcDecoderConfigurationRecord&, m_avcConfig, Configuration, const);

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
    AvcDecoderConfigurationRecord m_avcConfig; //!< avc configuration
};

VCD_MP4_END;
#endif /* _AVCCONFIGURATIONATOM_H_ */
