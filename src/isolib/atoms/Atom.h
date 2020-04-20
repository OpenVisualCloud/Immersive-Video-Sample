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
//! \file:   Atom.h
//! \brief:  Basic Atom class.
//! \detail: Basic ISOBMFF Atom definition.
//!
//! Created on October 14, 2019, 13:39 PM
//!
#ifndef _ATOM_H_
#define _ATOM_H_

#include "../include/Common.h"
#include <cstdint>
#include "FormAllocator.h"
#include "FourCCInt.h"

#define ABMAX_SAMP_CNT (1 << 22)

VCD_MP4_BEGIN

class Stream;

class Atom
{
public:

    //!
    //! \brief    Set and Get function for m_type member
    //!
    //! \param    [in] FourCCInt
    //!           value to set
    //! \param    [in] m_type
    //!           m_type member in class
    //! \param    [in] Type
    //!           m_type name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(FourCCInt, m_type, Type, const);

    //!
    //! \brief    Set 64-bit size
    //!
    //! \return   void
    //!
    void SetLargeSize();

    //!
    //! \brief    Get 64-bit size
    //!
    //! \return   bool
    //!
    bool GetLargeSize() const;

    //!
    //! \brief    Write atom information to stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void ToStream(Stream& str) = 0;

    //!
    //! \brief    Parse atom information from stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void FromStream(Stream& str) = 0;

protected:

    //!
    //! \brief Constructor
    //!
    Atom(FourCCInt boxType);

    //!
    //! \brief Destructor
    //!
    virtual ~Atom() = default;

    //!
    //! \brief    Set and Get function for m_size member
    //!
    //! \param    [in] uint64_t
    //!           value to set
    //! \param    [in] m_size
    //!           m_size member in class
    //! \param    [in] Size
    //!           m_size name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(uint64_t, m_size, Size, const);

    //!
    //! \brief    Set and Get function for m_size member
    //!
    //! \param    [in] std::vector<uint8_t>&
    //!           value to set
    //! \param    [in] m_userType
    //!           m_userType member in class
    //! \param    [in] UserType
    //!           m_userType name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::vector<uint8_t>&, m_userType, UserType, );

    //!
    //! \brief    Write atom header information
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    void WriteAtomHeader(Stream& str) const;

    //!
    //! \brief    Parse atom header information
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    void ParseAtomHeader(Stream& str);

    //!
    //! \brief    Update total atom size.
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    void UpdateSize(Stream& str) const;

private:

    mutable std::uint64_t m_size;          //!< atom size
    FourCCInt m_type;                      //!< atom type
    std::vector<std::uint8_t> m_userType;  //!< atom user type
    mutable std::uint64_t m_startLocation; //!< start position of bitstream
    bool m_largeSize;                      //!< Use large size(64 bit) or not
};

VCD_MP4_END;
#endif /* ATOM_H */
