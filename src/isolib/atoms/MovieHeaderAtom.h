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
//! \file:   MovieHeaderAtom.h
//! \brief:  Movie Header Atom class
//! \detail: 'mvhd' Atom
//!
//! Created on October 16, 2019, 13:39 PM
//!

#ifndef _MOVIEHEADERATOM_H_
#define _MOVIEHEADERATOM_H_

#include "FormAllocator.h"
#include "FullAtom.h"

VCD_MP4_BEGIN

class MovieHeaderAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    MovieHeaderAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~MovieHeaderAtom() = default;

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
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint64_t, m_creationTime, CreationTime, const);

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
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint64_t, m_modificationTime, ModificationTime, const);

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
    //! \brief    Set and Get function for m_nextTrackID member
    //!
    //! \param    [in] std::uint32_t
    //!           value to set
    //! \param    [in] m_nextTrackID
    //!           m_nextTrackID member in class
    //! \param    [in] NextTrackID
    //!           m_nextTrackID name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint32_t, m_nextTrackID, NextTrackID, const);

    //!
    //! \brief    Set and Get function for m_matrix member
    //!
    //! \param    [in] const std::vector<int32_t>&
    //!           value to set
    //! \param    [in] m_matrix
    //!           m_matrix member in class
    //! \param    [in] Matrix
    //!           m_matrix name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(const std::vector<int32_t>&, m_matrix, Matrix, const);

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
    std::uint64_t m_creationTime;       //!< creation Time
    std::uint64_t m_modificationTime;   //!< modification Time
    std::uint32_t m_timeScale;          //!< time Scale
    std::uint64_t m_duration;           //!< duration
    std::vector<std::int32_t> m_matrix; //!< matrix
    std::uint32_t m_nextTrackID;        //!< next Track ID
};

VCD_MP4_END;
#endif /* _MOVIEHEADERATOM_H_ */
