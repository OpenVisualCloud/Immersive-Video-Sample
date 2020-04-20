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
//! \file:   HandlerAtom.h
//! \brief:  Handler Atom class.
//! \detail: 'hdlr' Atom implementation
//!
//! Created on October 14, 2019, 13:39 PM
//!

#ifndef HANDLERATOM_H
#define HANDLERATOM_H

#include "Stream.h"
#include "FormAllocator.h"
#include "FullAtom.h"

VCD_MP4_BEGIN

class HandlerAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    HandlerAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~HandlerAtom() = default;

    //!
    //! \brief    Set and Get function for m_handlerType member
    //!
    //! \param    [in] FourCCInt
    //!           value to set
    //! \param    [in] m_handlerType
    //!           m_handlerType member in class
    //! \param    [in] HandlerType
    //!           m_handlerType name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(FourCCInt, m_handlerType, HandlerType, const);

    //!
    //! \brief    Set and Get function for m_name member
    //!
    //! \param    [in] const std::string&
    //!           value to set
    //! \param    [in] m_name
    //!           m_name member in class
    //! \param    [in] Name
    //!           m_name name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(const std::string&, m_name, Name, const);

    //!
    //! \brief    Write atom information to stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    void ToStream(Stream& str);

    //!
    //! \brief    Parse atom information from stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    void FromStream(Stream& str);

private:
    FourCCInt m_handlerType;       //!< type of the media handler
    std::string m_name;            //!< designated name of the media handler
};

VCD_MP4_END;
#endif /* HANDLERATOM_H */
