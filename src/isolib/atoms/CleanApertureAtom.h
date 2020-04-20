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
//! \file:   CleanApertureAtom.h
//! \brief:  Implementation of CleanApertureAtom
//! \detail: CleanAperture Atom is an item property
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _CLEANAPERTURE_H_
#define _CLEANAPERTURE_H_

#include <cstdint>
#include "Atom.h"
#include "FormAllocator.h"

VCD_MP4_BEGIN

class CleanApertureAtom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    CleanApertureAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~CleanApertureAtom() = default;

    struct Rational //<! factor
    {
        std::uint32_t numerator   = 0;
        std::uint32_t denominator = 1;
    };

    //!
    //! \brief    Set and Get function for m_width member
    //!
    //! \param    [in] std::uint16_t
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
    MEMBER_SETANDGET_FUNC_WITH_OPTION(Rational, m_width, Width, const);

    //!
    //! \brief    Set and Get function for m_height member
    //!
    //! \param    [in] std::uint16_t
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
    MEMBER_SETANDGET_FUNC_WITH_OPTION(Rational, m_height, Height, const);

    //!
    //! \brief    Set and Get function for m_horizOffset member
    //!
    //! \param    [in] std::uint16_t
    //!           value to set
    //! \param    [in] m_horizOffset
    //!           m_horizOffset member in class
    //! \param    [in] HorizOffset
    //!           m_horizOffset name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(Rational, m_horizOffset, HorizOffset, const);

    //!
    //! \brief    Set and Get function for m_vertOffset member
    //!
    //! \param    [in] std::uint16_t
    //!           value to set
    //! \param    [in] m_vertOffset
    //!           m_vertOffset member in class
    //! \param    [in] VertOffset
    //!           m_vertOffset name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(Rational, m_vertOffset, VertOffset, const);

    //!
    //! \brief    Write atom information to stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void ToStream(Stream& output);

    //!
    //! \brief    Parse atom information from stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void FromStream(Stream& input);

private:
    Rational m_width;        //!< Clean aperture width
    Rational m_height;       //!< Clean aperture height
    Rational m_horizOffset;  //!< Clean aperture horizontal offset
    Rational m_vertOffset;   //!< Clean aperture vertical offset
};

VCD_MP4_END;
#endif /* _CLEANAPERTURE_H_ */
