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
//! \file:   MediaAtom.h
//! \brief:  Media Atom class.
//! \detail: 'mdia' Atom
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef MEDIAATOM_H
#define MEDIAATOM_H

#include "Atom.h"
#include "Stream.h"
#include "FormAllocator.h"
#include "HandlerAtom.h"
#include "MediaHeaderAtom.h"
#include "MediaInfoAtom.h"

VCD_MP4_BEGIN

class MediaAtom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    MediaAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~MediaAtom() = default;

    //!
    //! \brief    Get Media Header Atom
    //!
    //! \return   MediaHeaderAtom&
    //!           header atom
    //!
    MediaHeaderAtom& GetMediaHeaderAtom();

    //!
    //! \brief    Get Media Header Atom
    //!
    //! \return   const MediaHeaderAtom&
    //!           header atom
    //!
    const MediaHeaderAtom& GetMediaHeaderAtom() const;

    //!
    //! \brief    Get Handler Atom
    //!
    //! \return   HandlerAtom&
    //!           handler atom
    //!
    HandlerAtom& GetHandlerAtom();

    //!
    //! \brief    Get Handler Atom
    //!
    //! \return   const HandlerAtom&
    //!           handler atom
    //!
    const HandlerAtom& GetHandlerAtom() const;

    //!
    //! \brief    Get Media Information Atom
    //!
    //! \return   MediaInformationAtom&
    //!           Media Information Atom
    //!
    MediaInformationAtom& GetMediaInformationAtom();

    //!
    //! \brief    Get Media Information Atom
    //!
    //! \return   const MediaInformationAtom&
    //!           Media Information Atom
    //!
    const MediaInformationAtom& GetMediaInformationAtom() const;

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
    MediaHeaderAtom m_mediaHeaderAtom;            //!< Media Header Atom
    HandlerAtom m_handlerAtom;                    //!< Media Handler Atom
    MediaInformationAtom m_mediaInformationAtom;  //!< Media Information Atom
};

VCD_MP4_END;
#endif /* MEDIAATOM_H */
