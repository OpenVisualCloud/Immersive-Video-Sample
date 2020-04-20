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
//! \file:   SyncSampAtom.h
//! \brief:  SyncSampAtom class.
//! \detail: 'stss' Atom
//!
//! Created on October 16, 2019, 13:39 PM
//!

#ifndef _SYNCSAMPLEATOM_H_
#define _SYNCSAMPLEATOM_H_

#include "FormAllocator.h"
#include "FullAtom.h"

VCD_MP4_BEGIN

class SyncSampleAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    SyncSampleAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~SyncSampleAtom() = default;

    //!
    //! \brief    Add Sample function
    //!
    //! \param    [in] std::uint32_t
    //!           sample Number
    //!
    //! \return   void
    //!
    void AddSample(std::uint32_t sampleNumber);

    //!
    //! \brief    Get Sync Sample Ids
    //!
    //! \return   const std::vector<std::uint32_t>
    //!           Sync Sample Ids
    //!
    const std::vector<std::uint32_t> GetSyncSampleIds() const;

    //!
    //! \brief    Set Sample Num Max Safety
    //!
    //! \param    [in] int64_t
    //!           max sample Num
    //!
    //! \return   void
    //!
    void SetSampleNumMaxSafety(int64_t sampleNumMax);

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
    std::vector<std::uint32_t> m_sampleNumber;  //!< std::vector of sync sample Ids
    int64_t m_sampleNumMax;                     //!< max sample num
};

VCD_MP4_END;
#endif /* _SYNCSAMPLEATOM_H_ */
