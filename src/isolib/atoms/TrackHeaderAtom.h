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
//! \file:   TrackHeaderAtom.h
//! \brief:  TrackHeaderAtom class.
//! \detail: 'tkhd' Atom
//!
//! Created on October 16, 2019, 13:39 PM
//!

#ifndef _TRACKHEADERATOM_H_CWQQ8ZSB_
#define _TRACKHEADERATOM_H_CWQQ8ZSB_

#include "Stream.h"
#include "FormAllocator.h"
#include "FullAtom.h"

#include <cstdint>
#include <iostream>

VCD_MP4_BEGIN

class TrackHeaderAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    TrackHeaderAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~TrackHeaderAtom() = default;

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
    //! \brief    Set and Get function for m_trackID member
    //!
    //! \param    [in] std::uint32_t
    //!           value to set
    //! \param    [in] m_trackID
    //!           m_trackID member in class
    //! \param    [in] TrackID
    //!           m_trackID name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint32_t, m_trackID, TrackID, const);

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
    //! \brief    Set and Get function for m_alternateGroup member
    //!
    //! \param    [in] std::uint16_t
    //!           value to set
    //! \param    [in] m_alternateGroup
    //!           m_alternateGroup member in class
    //! \param    [in] AlternateGroup
    //!           m_alternateGroup name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint16_t, m_alternateGroup, AlternateGroup, const);

    //!
    //! \brief    Set and Get function for m_volume member
    //!
    //! \param    [in] std::uint16_t
    //!           value to set
    //! \param    [in] m_volume
    //!           m_volume member in class
    //! \param    [in] Volume
    //!           m_volume name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint16_t, m_volume, Volume, const);

    //!
    //! \brief    Set and Get function for m_matrix member
    //!
    //! \param    [in] std::vector<int32_t>
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
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::vector<int32_t>, m_matrix, Matrix, const);

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
    std::uint64_t m_creationTime;           //!< Creation time
    std::uint64_t m_modificationTime;       //!< Modificaiton time
    std::uint32_t m_trackID;                //!< Track ID
    std::uint64_t m_duration;               //!< Track's duration
    std::uint32_t m_width;                  //!< Track display width
    std::uint32_t m_height;                 //!< Track display height
    std::uint16_t m_alternateGroup;         //!< Alternate group Id of the track
    std::uint16_t m_volume;                 //!< Volume (for audio tracks)
    std::vector<std::int32_t> m_matrix;     //!< Matrix
};

VCD_MP4_END;
#endif /* _TRACKHEADERATOM_H_CWQQ8ZSB_ */
