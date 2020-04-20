
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
//! \file:   TimeToSampAtom.h
//! \brief:  TimeToSampAtom Atom class
//! \detail: 'stts' Atom
//!
//! Created on October 14, 2019, 13:39 PM
//!
#ifndef TIMETOSAMPLEATOM_H
#define TIMETOSAMPLEATOM_H

#include "Stream.h"
#include "FormAllocator.h"
#include "FullAtom.h"

VCD_MP4_BEGIN

class TimeToSampleAtom : public FullAtom
{
public:

    struct EntryVersion0    //!< Entry Version v0
    {
        std::uint32_t m_sampleNum;
        std::uint32_t m_sampleDelta;
    };

    //!
    //! \brief Constructor
    //!
    TimeToSampleAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~TimeToSampleAtom() = default;

    //!
    //! \brief    Get Sample Times
    //!
    //! \return   std::vector<std::uint32_t>
    //!           Sample Time array
    //!
    std::vector<std::uint32_t> GetSampleTimes() const;

    //!
    //! \brief    Get Sample Deltas
    //!
    //! \return   std::vector<std::uint32_t>
    //!           Sample Time Deltas
    //!
    std::vector<std::uint32_t> GetSampleDeltas() const;

    //!
    //! \brief    Get Sample number
    //!
    //! \return   std::uint32_t
    //!           Sample number
    //!
    std::uint32_t GetSampleNum() const;

    //!
    //! \brief    Get Decode Delta Entry
    //!
    //! \return   EntryVersion0&
    //!           Decode Delta Entry
    //!
    EntryVersion0& GetDecodeDeltaEntry();

    //!
    //! \brief    Add Sample Delta
    //!
    //! \param    [in] std::uint32_t
    //!           sample Delta
    //!
    //! \return   void
    //!
    void AddSampleDelta(std::uint32_t m_sampleDelta);

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
    std::vector<EntryVersion0> m_entryVersion0; //!< Entry Version v0
};

VCD_MP4_END;
#endif /* TIMETOSAMPLEATOM_H */
