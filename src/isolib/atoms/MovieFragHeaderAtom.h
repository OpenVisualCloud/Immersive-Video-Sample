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
//! \file:   MovieFragHeaderAtom.h
//! \brief:  Movie Fragment Header Atom class
//! \detail: 'mfhd' Atom
//!
//! Created on October 16, 2019, 13:39 PM
//!

#ifndef _MOVIEFRAGMENTHEADERATOM_H_
#define _MOVIEFRAGMENTHEADERATOM_H_

#include "Stream.h"
#include "FormAllocator.h"
#include "FullAtom.h"

VCD_MP4_BEGIN

class MovieFragmentHeaderAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    MovieFragmentHeaderAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~MovieFragmentHeaderAtom() = default;

    //!
    //! \brief    Set Sequence Number
    //!
    //! \param    [in] std::uint32_t
    //!           Sequence Number
    //!
    //! \return   void
    //!
    void SetSequenceNumber(const uint32_t sequencyNumber);

    //!
    //! \brief    Get Sequence Number
    //!
    //! \return   std::uint32_t
    //!           Sequence Number
    //!
    uint32_t GetSequenceNumber() const;

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
    uint32_t m_sequenceNumber;  //!< Sequence Number
};

VCD_MP4_END;
#endif /* _MOVIEFRAGMENTHEADERATOM_H_ */
