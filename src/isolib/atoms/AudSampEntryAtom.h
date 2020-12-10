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
//! \file:   AudSampEntryAtom.h
//! \brief:  AudSampEntryAtom class.
//! \detail: atom sample contains audio information.
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _AUDIOSAMPLEENTRYATOM_H_
#define _AUDIOSAMPLEENTRYATOM_H_

#include "Stream.h"
#include "ChannelLayoutAtom.h"
#include "FormAllocator.h"
#include "SampEntryAtom.h"
#include "SampRateAtom.h"

VCD_MP4_BEGIN

class AudioSampleEntryAtom : public SampleEntryAtom
{
public:

    //!
    //! \brief Constructor
    //!
    AudioSampleEntryAtom(FourCCInt codingname);
    AudioSampleEntryAtom(const AudioSampleEntryAtom& Atom);

    AudioSampleEntryAtom& operator=(const AudioSampleEntryAtom&) = default;

    //!
    //! \brief Destructor
    //!
    virtual ~AudioSampleEntryAtom() = default;

    //!
    //! \brief    Set version
    //!
    //! \param    [in] std::uint16_t
    //!           version value
    //!
    //! \return   void
    //!
    void SetVersion(std::uint16_t version);

    //!
    //! \brief    Get version
    //!
    //! \return   std::uint16_t
    //!           version
    //!
    std::uint16_t GetVersion() const;

    //!
    //! \brief    Set and Get function for m_channelNumber member
    //!
    //! \param    [in] std::uint16_t
    //!           value to set
    //! \param    [in] m_channelNumber
    //!           m_channelNumber member in class
    //! \param    [in] ChannelNumber
    //!           m_channelNumber name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    //MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint16_t, m_channelNumber, ChannelNumber, const);

    //!
    //! \brief    Set and Get function for m_sampleSize member
    //!
    //! \param    [in] std::uint16_t
    //!           value to set
    //! \param    [in] m_sampleSize
    //!           m_sampleSize member in class
    //! \param    [in] SampleSize
    //!           m_sampleSize name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    //MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint16_t, m_sampleSize, SampleSize, const);

    //!
    //! \brief    Get ChannelCount
    //!
    //! \return   std::uint16_t
    //!           ChannelCount
    //!
    std::uint16_t GetChannelCount() const;

    //!
    //! \brief    Set ChannelCount
    //!
    //! \param    [in] std::uint16_t
    //!           ChannelCount value
    //!
    //! \return   void
    //!
    void SetChannelCount(std::uint16_t channelCnt);

    void SetSampleSize(std::uint16_t sampSize);

    std::uint16_t GetSampleSize() const;
    //!
    //! \brief    Get SampleRate
    //!
    //! \return   std::uint32_t
    //!           SampleRate
    //!
    std::uint32_t GetSampleRate() const;

    //!
    //! \brief    Set SampleRate
    //!
    //! \param    [in] std::uint32_t
    //!           SampleRate value
    //!
    //! \return   void
    //!
    void SetSampleRate(std::uint32_t height);

    //!
    //! \brief    Has ChannelLayout Atom or not
    //!
    //! \return   bool
    //!           has or not
    //!
    bool HasChannelLayoutAtom();

    //!
    //! \brief    Get ChannelLayout Atom
    //!
    //! \return   ChannelLayoutAtom&
    //!           ChannelLayout Atom
    //!
    ChannelLayoutAtom& GetChannelLayoutAtom();

    //!
    //! \brief    Set ChannelLayout Atom
    //!
    //! \param    [in] ChannelLayoutAtom&
    //!           ChannelLayout Atom
    //!
    //! \return   void
    //!
    void SetChannelLayoutAtom(ChannelLayoutAtom& channelLayoutAtom);

    //!
    //! \brief    Has SamplingRate Atom or not
    //!
    //! \return   bool
    //!           has or not
    //!
    bool HasSamplingRateAtom();

    //!
    //! \brief    Get SamplingRate Atom
    //!
    //! \return   SamplingRateAtom&
    //!           SamplingRate Atom
    //!
    SamplingRateAtom& GetSamplingRateAtom();

    //!
    //! \brief    Set SamplingRate Atom
    //!
    //! \param    [in] SamplingRateAtom&
    //!           SamplingRate Atom
    //!
    //! \return   void
    //!
    void SetSamplingRateAtom(SamplingRateAtom& samplingRateAtom);

    //!
    //! \brief    Write atom information to stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void ToStream(Stream& bitstr) override;

    //!
    //! \brief    Parse atom information from stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void FromStream(Stream& bitstr) override;

    //!
    //! \brief    Get Copy of AudioSampleEntryAtom
    //!
    //! \return   AudioSampleEntryAtom*
    //!           AudioSampleEntry Atom
    //!
    virtual AudioSampleEntryAtom* Clone() const override;

    //!
    //! \brief    Get ConfigurationRecord
    //!
    //! \return   const DecoderConfigurationRecord*
    //!           DecoderConfigurationRecord value
    //!
    virtual const DecoderConfigurationRecord* GetConfigurationRecord() const override;

    //!
    //! \brief    Get Configuration Atom
    //!
    //! \return   const Atom*
    //!           Configuration Atom
    //!
    virtual const Atom* GetConfigurationAtom() const override;

private:
    std::uint16_t m_version;                //!< atom version
    std::uint16_t m_channelNumber;          //!< number of channel
    std::uint16_t m_sampleSize;             //!< sampile size
    std::uint32_t m_sampleRate;             //!< sample rate
    bool m_hasChannelLayoutAtom;            //!< has ChannelLayoutAtom or not
    bool m_hasSamplingRateAtom;             //!< has SamplingRateAtom or not
    ChannelLayoutAtom m_channelLayoutAtom;  //!< ChannelLayoutAtom
    SamplingRateAtom m_samplingRateAtom;    //!< SamplingRateAtom
};

VCD_MP4_END;
#endif  /* _AUDIOSAMPLEENTRYATOM_H_ */
