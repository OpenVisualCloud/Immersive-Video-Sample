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
//! \file:   SampSizeAtom.h
//! \brief:  SampSize Atom.
//! \detail: 'stsz' Atom
//!
//! Created on October 16, 2019, 13:39 PM
//!

#ifndef _SAMPLESIZEATOM_H_
#define _SAMPLESIZEATOM_H_

#include "Stream.h"
#include "FormAllocator.h"
#include "FullAtom.h"

#include <iostream>
#include <list>

VCD_MP4_BEGIN

class SampleSizeAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    SampleSizeAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~SampleSizeAtom() = default;

    //!
    //! \brief    Set and Get function for m_sampleSize member
    //!
    //! \param    [in] std::uint32_t
    //!           value to set
    //! \param    [in] m_sampleSize
    //!           m_sampleSize member in class
    //! \param    [in] SampleSize
    //!           m_sampleSize name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint32_t, m_sampleSize, SampleSize, );

    //!
    //! \brief    Set and Get function for m_sampleNum member
    //!
    //! \param    [in] std::uint32_t
    //!           value to set
    //! \param    [in] m_sampleNum
    //!           m_sampleNum member in class
    //! \param    [in] SampleNum
    //!           m_sampleNum name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint32_t, m_sampleNum, SampleNum, const);

    //!
    //! \brief    Set Entry Size
    //!
    //! \param    [in] std::vector<uint32_t>
    //!           sample sizes
    //!
    //! \return   void
    //!
    void SetEntrySize(std::vector<uint32_t> sample_sizes);

    //!
    //! \brief    Get Entry Size
    //!
    //! \return   std::vector<uint32_t>
    //!           Entry Size
    //!
    std::vector<uint32_t> GetEntrySize() const;

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
    std::uint32_t m_sampleSize;                      //!< Default sample size.
    std::uint32_t m_sampleNum;                       //!< Number of samples to be listed
    mutable std::vector<std::uint32_t> m_entrySize;  //!< Sample sizes of each sample.
};

VCD_MP4_END;
#endif /* _SAMPLESIZEATOM_H_ */
