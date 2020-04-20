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
//! \file:   SampRateAtom.h
//! \brief:  Sampling Rate Atom class
//! \detail: 'srat' Atom
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _SAMPLINGRATEATOM_H_
#define _SAMPLINGRATEATOM_H_

#include <vector>
#include "FormAllocator.h"
#include "FullAtom.h"

VCD_MP4_BEGIN

class SamplingRateAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    SamplingRateAtom();
    SamplingRateAtom(const SamplingRateAtom& Atom);

    SamplingRateAtom& operator=(const SamplingRateAtom&) = default;

    //!
    //! \brief Destructor
    //!
    virtual ~SamplingRateAtom() = default;

    //!
    //! \brief    Get Sampling Rate
    //!
    //! \return   std::uint32_t
    //!           SampleRate
    //!
    std::uint32_t GetSamplingRate() const;

    //!
    //! \brief    Set Sample Rate
    //!
    //! \param    [in] std::uint32_t
    //!           SampleRate value
    //!
    //! \return   void
    //!
    void SetSamplingRate(std::uint32_t samplingRate);

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
    std::uint32_t m_samplingRate;   //!< sampling rate
};

VCD_MP4_END;
#endif /* _SAMPLINGRATEATOM_H_ */
