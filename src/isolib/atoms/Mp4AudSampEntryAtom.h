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
//! \file:   Mp4AudSampEntryAtom.h
//! \brief:  Mp4AudSampEntryAtom class.
//! \detail: This Atom contains information related to the mp4 audio samples of the track
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _MP4AUDIOSAMPLEENTRYATOM_H_
#define _MP4AUDIOSAMPLEENTRYATOM_H_

#include "AudSampEntryAtom.h"
#include "Stream.h"
#include "FormAllocator.h"
#include "ElemStreamDescAtom.h"
#include "BasicAudAtom.h"
#include "Mp4AudDecConfigRecord.h"

VCD_MP4_BEGIN

class MP4AudioSampleEntryAtom : public AudioSampleEntryAtom
{
public:

    //!
    //! \brief Constructor
    //!
    MP4AudioSampleEntryAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~MP4AudioSampleEntryAtom() = default;

    //!
    //! \brief    Get ESD Atom
    //!
    //! \return   ElementaryStreamDescriptorAtom&
    //!           ESD Atom
    //!
    ElementaryStreamDescriptorAtom& GetESDAtom();

    //!
    //! \brief    Get ESD Atom
    //!
    //! \return   const ElementaryStreamDescriptorAtom&
    //!           ESD Atom
    //!
    const ElementaryStreamDescriptorAtom& GetESDAtom() const;

    //!
    //! \brief    Has Spatial Audio Atom or not
    //!
    //! \return   bool
    //!           has or not
    //!
    bool HasSpatialAudioAtom() const;

    //!
    //! \brief    Get Spatial Audio Atom
    //!
    //! \return   const SpatialAudioAtom&
    //!           Spatial Audio Atom
    //!
    const SpatialAudioAtom& GetSpatialAudioAtom() const;

    //!
    //! \brief    Set Spatial Audio Atom
    //!
    //! \param    [in] const SpatialAudioAtom&
    //!           Spatial Audio Atom
    //!
    //! \return   void
    //!
    void SetSpatialAudioAtom(const SpatialAudioAtom&);

    //!
    //! \brief    Has Non Diegetic Audio Atom or not
    //!
    //! \return   bool
    //!           has or not
    //!
    bool HasNonDiegeticAudioAtom() const;

    //!
    //! \brief    Get Non Diegetic Audio Atom
    //!
    //! \return   const NonDiegeticAudioAtom&
    //!           Non Diegetic Audio Atom
    //!
    const NonDiegeticAudioAtom& GetNonDiegeticAudioAtom() const;

    //!
    //! \brief    Set Non Diegetic Audio Atom
    //!
    //! \param    [in] const NonDiegeticAudioAtom&
    //!           Non Diegetic Audio Atom
    //!
    //! \return   void
    //!
    void SetNonDiegeticAudioAtom(const NonDiegeticAudioAtom&);

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

    //!
    //! \brief    Get Copy of MP4AudioSampleEntryAtom
    //!
    //! \return   MP4AudioSampleEntryAtom*
    //!           MP4AudioSampleEntryAtom Atom
    //!
    virtual MP4AudioSampleEntryAtom* Clone() const;

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
    ElementaryStreamDescriptorAtom m_ESDAtom;   //!< Elementary Stream Descriptor Atom
    bool m_hasSpatialAudioAtom;                 //!< has Spatial Audio Atom or not
    SpatialAudioAtom m_spatialAudioAtom;        //!< Spatial Audio Atom
    bool m_hasNonDiegeticAudioAtom;             //!< has Non Diegetic Audio Atom or not
    NonDiegeticAudioAtom m_nonDiegeticAudioAtom;//!< Non Diegetic Audio Atom
    MP4AudioDecoderConfigurationRecord m_record;//!< MP4 Audio Decoder Configuration Record
};

VCD_MP4_END;
#endif  /* _MP4AUDIOSAMPLEENTRYATOM_H_ */
