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
//! \file:   VisualSampEntryAtom.h
//! \brief:  VisualSampEntryAtom class.
//! \detail: Visual Sample Entry Atom definition
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _VISUALSAMPLEENTRYATOM_H_
#define _VISUALSAMPLEENTRYATOM_H_

#include "Stream.h"
#include "FormAllocator.h"
#include "SampEntryAtom.h"

VCD_MP4_BEGIN

class CleanApertureAtom;
class Stereoscopic3D;
class SphericalVideoV2Atom;

class VisualSampleEntryAtom : public SampleEntryAtom
{
public:

    //!
    //! \brief Constructor
    //!
    VisualSampleEntryAtom(FourCCInt codingName, const std::string& compressorName);

    VisualSampleEntryAtom(const VisualSampleEntryAtom& Atom);

    VisualSampleEntryAtom& operator=(const VisualSampleEntryAtom&) = default;

    //!
    //! \brief Destructor
    //!
    virtual ~VisualSampleEntryAtom() = default;

    //!
    //! \brief    Set and Get function for m_width member
    //!
    //! \param    [in] std::uint32_t
    //!           value to set
    //! \param    [in] m_width
    //!           m_width member in class
    //! \param    [in] Width
    //!           m_width name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint32_t, m_width, Width, const);

    //!
    //! \brief    Set and Get function for m_height member
    //!
    //! \param    [in] std::uint32_t
    //!           value to set
    //! \param    [in] m_height
    //!           m_height member in class
    //! \param    [in] Height
    //!           m_height name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint32_t, m_height, Height, const);

    //!
    //! \brief Create Clap
    //!
    void CreateClap();

    //!
    //! \brief    Get Clap
    //!
    //! \return   const CleanApertureAtom*
    //!           Clap
    //!
    const CleanApertureAtom* GetClap() const;

    //!
    //! \brief    Get Clap
    //!
    //! \return   CleanApertureAtom*
    //!           Clap
    //!
    CleanApertureAtom* GetClap();

    //!
    //! \brief    Get Stereoscopic3DAtom
    //!
    //! \return   const Stereoscopic3D*
    //!           Stereoscopic3D
    //!
    virtual const Stereoscopic3D* GetStereoscopic3DAtom() const
    {
        return nullptr;
    }

    //!
    //! \brief    is Stereoscopic3DAtom Present or not
    //!
    //! \return   bool
    //!           is or not
    //!
    bool IsStereoscopic3DAtomPresent() const
    {
        bool value = (const_cast<VisualSampleEntryAtom*>(this)->GetStereoscopic3DAtom() != nullptr);
        return value;
    }

    //!
    //! \brief    Get SphericalVideoV2 atom
    //!
    //! \return   const SphericalVideoV2Atom*
    //!           SphericalVideoV2Atom
    //!
    virtual const SphericalVideoV2Atom* GetSphericalVideoV2Atom() const
    {
        return nullptr;
    }

    //!
    //! \brief    is SphericalVideoV2 atom Present or not
    //!
    //! \return   bool
    //!           is or not
    //!
    bool IsSphericalVideoV2AtomAtomPresent() const
    {
        bool value = (const_cast<VisualSampleEntryAtom*>(this)->GetSphericalVideoV2Atom() != nullptr);
        return value;
    }

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
    //! \brief    is virtual or not
    //!
    //! \return   bool
    //!           is or not
    //!
    virtual bool IsVisual() const override;

private:
    std::uint16_t m_width;                     //!< Sample display width
    std::uint16_t m_height;                    //!< Sample display height
    std::string m_compressorName;              //!< Compressor name used, e.g. "HEVC Coding"
    std::shared_ptr<CleanApertureAtom> m_clap; //!< Clean Aperture data structure
};

VCD_MP4_END;
#endif /* _VISUALSAMPLEENTRYATOM_H_ */
