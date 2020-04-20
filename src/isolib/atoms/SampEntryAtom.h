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
//! \file:   SampEntryAtom.h
//! \brief:  Sample Entry Atom class.
//! \detail: Defines Sample Entry data structure
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _SAMPLEENTRYATOM_H_
#define _SAMPLEENTRYATOM_H_

#include <cstdint>
#include "Atom.h"
#include "FormAllocator.h"
#include "DecConfigRecord.h"
#include "RestSchemeInfoAtom.h"

VCD_MP4_BEGIN

class SampleEntryAtom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    SampleEntryAtom(FourCCInt codingname);
    SampleEntryAtom(const SampleEntryAtom& Atom);

    SampleEntryAtom& operator=(const SampleEntryAtom&) = default;

    //!
    //! \brief Destructor
    //!
    virtual ~SampleEntryAtom() = default;

    //!
    //! \brief    Get Data Reference Index
    //!
    //! \return   std::uint16_t
    //!           Data Reference Index
    //!
    std::uint16_t GetDataReferenceIndex() const;

    //!
    //! \brief    Set Data Reference Index
    //!
    //! \param    [in] std::uint16_t
    //!           data Reference Index
    //!
    //! \return   void
    //!
    void SetDataReferenceIndex(std::uint16_t dataReferenceIndex);

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
    //! \brief    Get Copy of AudioSampleEntryAtom
    //!
    //! \return   AudioSampleEntryAtom*
    //!           AudioSampleEntry Atom
    //!
    virtual SampleEntryAtom* Clone() const = 0;

    //!
    //! \brief    Add Restricted Scheme Info Atom
    //!
    //! \param    [in] UniquePtr<RestrictedSchemeInfoAtom>
    //!           Restricted Scheme Info Atom pointer
    //!
    //! \return   void
    //!
    void AddRestrictedSchemeInfoAtom(UniquePtr<RestrictedSchemeInfoAtom>);

    //!
    //! \brief    Get Restricted Scheme Info Atom
    //!
    //! \return   RestrictedSchemeInfoAtom*
    //!           Restricted Scheme Info Atom
    //!
    RestrictedSchemeInfoAtom* GetRestrictedSchemeInfoAtom() const;

    //!
    //! \brief    Get ConfigurationRecord
    //!
    //! \return   const DecoderConfigurationRecord*
    //!           DecoderConfigurationRecord value
    //!
    virtual const DecoderConfigurationRecord* GetConfigurationRecord() const = 0;

    //!
    //! \brief    Get Configuration Atom
    //!
    //! \return   const Atom*
    //!           Configuration Atom
    //!
    virtual const Atom* GetConfigurationAtom() const = 0;

    //!
    //! \brief    Is Visual or not
    //!
    //! \return   bool
    //!           check is visual or not
    //!
    virtual bool IsVisual() const;

private:
    std::uint16_t m_dataReferenceIndex;  //!< data reference index value
    UniquePtr<RestrictedSchemeInfoAtom>
        m_restrictedSchemeInfoAtom;  //!< resv info
};

VCD_MP4_END;
#endif /* _SAMPLEENTRYATOM_H_ */
