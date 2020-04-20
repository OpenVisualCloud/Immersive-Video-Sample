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
//! \file:   HevcSampEntry.h
//! \brief:  HEVC Sample Entry class.
//! \detail: 'hvc1' Atom implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _HEVCSAMPLEENTRY_H_
#define _HEVCSAMPLEENTRY_H_

#include "Stream.h"
#include "FormAllocator.h"
#include "BasicVideoAtom.h"
#include "HevcConfigAtom.h"
#include "VisualSampEntryAtom.h"

VCD_MP4_BEGIN

class HevcSampleEntry : public VisualSampleEntryAtom
{
public:

    //!
    //! \brief Constructor
    //!
    HevcSampleEntry();
    HevcSampleEntry(const HevcSampleEntry& other);

    HevcSampleEntry& operator=(const HevcSampleEntry&) = default;

    //!
    //! \brief Destructor
    //!
    virtual ~HevcSampleEntry() = default;

    //!
    //! \brief    Get Configuration Record
    //!
    //! \return   HevcConfigurationAtom&
    //!           DecoderConfigurationRecord value
    //!
    HevcConfigurationAtom& GetHevcConfigurationAtom();

    //!
    //! \brief    Create Stereoscopic3D Atom
    //!
    //! \return   void
    //!
    void CreateStereoscopic3DAtom();

    //!
    //! \brief    Create SphericalVideo Atom
    //!
    //! \return   void
    //!
    void CreateSphericalVideoV2Atom();

    //!
    //! \brief    Get Stereoscopic3D Atom
    //!
    //! \return   const Stereoscopic3D*
    //!           Stereoscopic3D Atom
    //!
    virtual const Stereoscopic3D* GetStereoscopic3DAtom() const override;

    //!
    //! \brief    Get SphericalVideo Atom
    //!
    //! \return   const SphericalVideoV2Atom*
    //!           Stereoscopic3D Atom
    //!
    virtual const SphericalVideoV2Atom* GetSphericalVideoV2Atom() const override;

    //!
    //! \brief    Write atom information to stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void ToStream(Stream& str) override;

    //!
    //! \brief    Parse atom information from stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void FromStream(Stream& str) override;

    //!
    //! \brief    Get Copy of hevcSampleEntry
    //!
    //! \return   hevcSampleEntry*
    //!           hevcSampleEntry Atom
    //!
    virtual HevcSampleEntry* Clone() const override;

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
    HevcConfigurationAtom m_hevcConfigurationAtom;  //!< hevc configuration atom
    bool m_isStereoscopic3DPresent;                 //!< is Stereoscopic3D Present
    Stereoscopic3D m_stereoscopic3DAtom;            //!< stereoscopic3D Atom
    bool m_isSphericalVideoV2AtomPresent;           //!< is SphericalVideoV2Atom Present
    SphericalVideoV2Atom m_sphericalVideoV2Atom;    //!< spherical Video V2 Atom
};

VCD_MP4_END;
#endif /* _HEVCSAMPLEENTRY_H_ */
