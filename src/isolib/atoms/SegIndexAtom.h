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
//! \file:   SegIndexAtom.h
//! \brief:  Segment Index Atom class
//! \detail: 'sidx' Atom
//!
//! Created on October 16, 2019, 13:39 PM
//!

#ifndef _SEGMENTINDEXATOM_H_
#define _SEGMENTINDEXATOM_H_

#include "Stream.h"
#include "FormAllocator.h"
#include "FullAtom.h"

VCD_MP4_BEGIN

class SegmentIndexAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    SegmentIndexAtom(uint8_t version = 0);

    //!
    //! \brief Destructor
    //!
    virtual ~SegmentIndexAtom() = default;

    struct Reference    //!< reference
    {
        bool referenceType;
        uint32_t referencedSize;
        uint32_t subsegmentDuration;
        bool startsWithSAP;
        uint8_t sapType;
        uint32_t sapDeltaTime;
    };

    //!
    //! \brief    Set Space Reserve
    //!
    //! \param    [in] size_t
    //!           reserve Total
    //!
    //! \return   void
    //!
    void SetSpaceReserve(size_t reserveTotal);

    //!
    //! \brief    Set and Get function for m_referenceID member
    //!
    //! \param    [in] uint32_t
    //!           value to set
    //! \param    [in] m_referenceID
    //!           m_referenceID member in class
    //! \param    [in] ReferenceId
    //!           m_referenceID name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(uint32_t, m_referenceID, ReferenceId, const);

    //!
    //! \brief    Set and Get function for m_timescale member
    //!
    //! \param    [in] std::uint32_t
    //!           value to set
    //! \param    [in] m_timescale
    //!           m_timescale member in class
    //! \param    [in] Timescale
    //!           m_timescale name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(uint32_t, m_timescale, Timescale, const);

    //!
    //! \brief    Set and Get function for m_earliestPresentationTime member
    //!
    //! \param    [in] std::uint64_t
    //!           value to set
    //! \param    [in] m_earliestPresentationTime
    //!           m_earliestPresentationTime member in class
    //! \param    [in] EarliestPresentationTime
    //!           m_earliestPresentationTime name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(uint64_t, m_earliestPresentationTime, EarliestPresentationTime, const);

    //!
    //! \brief    Set and Get function for m_firstOffset member
    //!
    //! \param    [in] std::uint64_t
    //!           value to set
    //! \param    [in] m_firstOffset
    //!           m_firstOffset member in class
    //! \param    [in] FirstOffset
    //!           m_firstOffset name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(uint64_t, m_firstOffset, FirstOffset, const);

    //!
    //! \brief    Add Reference
    //!
    //! \param    [in] const Reference&
    //!           reference
    //!
    //! \return   void
    //!
    void AddReference(const Reference& reference);

    //!
    //! \brief    Get References
    //!
    //! \return   std::vector<Reference>
    //!           reference array
    //!
    std::vector<Reference> GetReferences() const;

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
    uint32_t m_referenceID;             //!< reference ID
    uint32_t m_timescale;               //!< time scale
    uint64_t m_earliestPresentationTime;//!< earliest Presentation Time
    uint64_t m_firstOffset;             //!< first Offset
    std::vector<Reference> m_references;//!< Reference array
    size_t m_reserveTotal;              //!< reserve Total
};

VCD_MP4_END;
#endif /* end of include guard: SEGMENTINDEXATOM_HPP */
