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
//! \file:   TrackRunAtom.h
//! \brief:  Track Run Atom class
//! \detail: 'trun' Atom
//!
//! Created on October 14, 2019, 13:39 PM
//!
#ifndef TRACKRUNATOM_H
#define TRACKRUNATOM_H

#include "Stream.h"
#include "FormAllocator.h"
#include "FullAtom.h"
#include "MovieFragDataTypes.h"

VCD_MP4_BEGIN

class TrackRunAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    TrackRunAtom(uint8_t version = 0, std::uint32_t tr_flags = 0);

    //!
    //! \brief Destructor
    //!
    virtual ~TrackRunAtom() = default;

    struct SampleConfig0    //!< Sample Configuratin v0
    {
        uint32_t pDuration;
        uint32_t pSize;
        SampleFlags pFlags;
        uint32_t pCompTimeOffset;
    };

    struct SampleConfig1    //!< Sample Configuratin v1
    {
        uint32_t pDuration;
        uint32_t pSize;
        SampleFlags pFlags;
        int32_t pCompTimeOffset;
    };

    union SampleDetails {   //!< Sample Details
        SampleConfig0 version0;
        SampleConfig1 version1;
    };

    enum TrackRunFlags      //!< Track Run Flags
    {
        pDataOffset                   = 0x000001,
        pFirstSampleFlags             = 0x000004,
        pSampleDuration               = 0x000100,
        pSampleSize                   = 0x000200,
        pSampleFlags                  = 0x000400,
        pSampleCompTimeOffsets        = 0x000800
    };

    //!
    //! \brief    Set and Get function for m_sampleNum member
    //!
    //! \param    [in] std::uint32_t
    //!           value to set
    //! \param    [in] m_sampleNum
    //!           m_sampleNum member in class
    //! \param    [in] SampleNum
    //!           m_sampleNum name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(uint32_t, m_sampleNum, SampleNum, const);

    //!
    //! \brief    Set Data Offset
    //!
    //! \param    [in] const int32_t
    //!           Data Offset value
    //!
    //! \return   void
    //!
    void SetDataOffset(const int32_t dataOffset);

    //!
    //! \brief    Get Data Offset
    //!
    //! \return   int32_t
    //!           Data Offset
    //!
    int32_t GetDataOffset() const;

    //!
    //! \brief    Set First Sample Flags
    //!
    //! \param    [in] const SampleFlags
    //!           First Sample Flags
    //!
    //! \return   void
    //!
    void SetFirstSampleFlags(const SampleFlags firstSampleFlags);

    //!
    //! \brief    Get First Sample Flags
    //!
    //! \return   SampleFlags
    //!           First Sample Flags
    //!
    SampleFlags GetFirstSampleFlags() const;

    //!
    //! \brief    Add Sample Details
    //!
    //! \param    [in] SampleDetails
    //!           Sample Details
    //!
    //! \return   void
    //!
    void AddSampleDetails(SampleDetails sampleDetails);

    //!
    //! \brief    Get First Sample Details
    //!
    //! \return   const std::vector<SampleDetails>&
    //!           First Sample Details
    //!
    const std::vector<SampleDetails>& GetSampleDetails() const;

    //!
    //! \brief    Set Sample Defaults
    //!
    //! \param    [in] SampleDefaults
    //!           Sample Defaults
    //!
    //! \return   void
    //!
    void SetSampleDefaults(SampleDefaults& sampleDefaults);

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
    bool m_sampleDefaultsSet;                   //!< sample Defaults Set
    SampleDefaults m_sampleDefaults;            //!< Sample Defaults
    uint32_t m_sampleNum;                       //!< sample Num
    int32_t m_dataOffset;                       //!< data Offset
    SampleFlags m_firstSampleFlags;             //!< first Sample Flags
    std::vector<SampleDetails> m_sampleDetails; //!< sample Details
};

VCD_MP4_END;
#endif /* TRACKRUNATOM_H */
