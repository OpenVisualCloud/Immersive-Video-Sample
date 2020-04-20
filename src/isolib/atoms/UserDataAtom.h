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
//! \file:   UserDataAtom.h
//! \brief:  User Data Atom class
//! \detail: User Data Atom definition
//!
//! Created on October 16, 2019, 13:39 PM
//!

#ifndef _USERDATAATOM_H_
#define _USERDATAATOM_H_

#include <cstdint>
#include <functional>
#include "Atom.h"
#include "FormAllocator.h"

VCD_MP4_BEGIN

class UserDataAtom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    UserDataAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~UserDataAtom() = default;

    //!
    //! \brief    Get Atom
    //!
    //! \param    [in] Atom&
    //!           atom
    //!
    //! \return   bool
    //!           success or not
    //!
    bool GetAtom(Atom& Atom) const;

    //!
    //! \brief    Add Atom
    //!
    //! \param    [in] Atom&
    //!           atom
    //!
    //! \return   void
    //!
    void AddAtom(Atom& Atom);

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
    std::map<FourCCInt, Stream> m_bitStreams;   //!< bitStreams
};

VCD_MP4_END;
#endif /* _USERDATAATOM_H_ */
