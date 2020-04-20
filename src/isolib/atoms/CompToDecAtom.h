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
//! \file:   CompToDecodeAtom.h
//! \brief:  Composition To Decode Atom class
//! \detail: 'cslg' Atom implementation.
//!
//! Created on October 14, 2019, 13:39 PM
//!
#ifndef COMPOSITIONTODECODEATOM_H
#define COMPOSITIONTODECODEATOM_H

#include "Stream.h"
#include "FormAllocator.h"
#include "FullAtom.h"

VCD_MP4_BEGIN

class CompositionToDecodeAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    CompositionToDecodeAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~CompositionToDecodeAtom() = default;

    //!
    //! \brief    Set compositionTosDtsShift
    //!
    //! \param    [in] std::int64_t
    //!           DtsShift
    //!
    //! \return   void
    //!
    void SetDtsShift(std::int64_t compositionTosDtsShift);

    //!
    //! \brief    Get Dts Shift
    //!
    //! \return   std::int64_t
    //!           Dts Shift
    //!
    std::int64_t GetDtsShift() const;

    //!
    //! \brief    Set Least Display Delta
    //!
    //! \param    [in] std::int64_t
    //!           least Decode To Display Delta
    //!
    //! \return   void
    //!
    void SetLeastDisplayDelta(std::int64_t leastDecodeToDisplayDelta);

    //!
    //! \brief    Get Least Display Delta
    //!
    //! \return   std::int64_t
    //!           Least Display Delta
    //!
    std::int64_t GetLeastDisplayDelta() const;

    //!
    //! \brief    Set Greatest Display Delta
    //!
    //! \param    [in] std::int64_t
    //!           greatest Decode To Display Delta
    //!
    //! \return   void
    //!
    void SetGreatestDisplayDelta(std::int64_t greatestDecodeToDisplayDelta);

    //!
    //! \brief    Get Greatest Display Delta
    //!
    //! \return   std::int64_t
    //!           greatest Decode To Display Delta
    //!
    std::int64_t GetGreatestDisplayDelta() const;

    //!
    //! \brief    Set Start Time
    //!
    //! \param    [in] std::int64_t
    //!           Start Time
    //!
    //! \return   void
    //!
    void SetStartTime(std::int64_t compositionStartTime);

    //!
    //! \brief    Get Start Time
    //!
    //! \return   std::int64_t
    //!           Start Time
    //!
    std::int64_t GetStartTime() const;

    //!
    //! \brief    Set End Time
    //!
    //! \param    [in] std::int64_t
    //!           End Time
    //!
    //! \return   void
    //!
    void SetEndTime(std::int64_t compositionEndTime);

    //!
    //! \brief    Get End Time
    //!
    //! \return   std::int64_t
    //!           End Time
    //!
    std::int64_t GetEndTime() const;

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

    //!
    //! \brief    Update Version
    //!
    //! \return   void
    //!
    void UpdateVersion();

    std::int64_t m_dtsShift;                //!< dts Shift
    std::int64_t m_leastDisplayDelta;       //!< least Display Delta
    std::int64_t m_greatestDisplayDelta;    //!< greatest Display Delta
    std::int64_t m_startTime;               //!< start Time
    std::int64_t m_endTime;                 //!< end Time
};

VCD_MP4_END;
#endif /* COMPOSITIONTODECODEATOM_H */
