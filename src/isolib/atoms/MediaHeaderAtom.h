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
//! \file:   MediaHeaderAtom.h
//! \brief:  Media Header Atom class.
//! \detail: 'mdhd' Atom contains basic inforation about the media data
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _MEDIAHEADERATOM_H_
#define _MEDIAHEADERATOM_H_

#include "Stream.h"
#include "FormAllocator.h"
#include "FullAtom.h"

#include <cstdint>

VCD_MP4_BEGIN

class MediaHeaderAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    MediaHeaderAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~MediaHeaderAtom() = default;

    //!
    //! \brief    Set and Get function for m_creationTime member
    //!
    //! \param    [in] std::uint64_t
    //!           value to set
    //! \param    [in] m_creationTime
    //!           m_creationTime member in class
    //! \param    [in] CreationTime
    //!           m_creationTime name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint64_t, m_creationTime, CreationTime, );

    //!
    //! \brief    Set and Get function for m_modificationTime member
    //!
    //! \param    [in] std::uint64_t
    //!           value to set
    //! \param    [in] m_modificationTime
    //!           m_modificationTime member in class
    //! \param    [in] ModificationTime
    //!           m_modificationTime name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint64_t, m_modificationTime, ModificationTime, );

    //!
    //! \brief    Set and Get function for m_timeScale member
    //!
    //! \param    [in] std::uint32_t
    //!           value to set
    //! \param    [in] m_timeScale
    //!           m_timeScale member in class
    //! \param    [in] TimeScale
    //!           m_timeScale name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint32_t, m_timeScale, TimeScale, const);

    //!
    //! \brief    Set and Get function for m_duration member
    //!
    //! \param    [in] std::uint64_t
    //!           value to set
    //! \param    [in] m_duration
    //!           m_duration member in class
    //! \param    [in] Duration
    //!           m_duration name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint64_t, m_duration, Duration, const);

    //!
    //! \brief    Set and Get function for m_language member
    //!
    //! \param    [in] std::uint32_t
    //!           value to set
    //! \param    [in] m_language
    //!           m_language member in class
    //! \param    [in] Language
    //!           m_language name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint32_t, m_language, Language, const);

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
    std::uint64_t m_creationTime;      //!< Creation time
    std::uint64_t m_modificationTime;  //!< Modification time
    std::uint32_t m_timeScale;         //!< Timescale
    std::uint64_t m_duration;          //!< Duration
    std::uint16_t m_language;          //!< Language
};

VCD_MP4_END;
#endif /* _MEDIAHEADERATOM_H_ */
