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
//! \file:   CompOffsetAtom.h
//! \brief:  Composition Time to Sample Atom class
//! \detail: 'sttc' Atom implementation.
//!
//! Created on October 14, 2019, 13:39 PM
//!
#ifndef COMPOSITIONOFFSETATOM_H
#define COMPOSITIONOFFSETATOM_H

#include "FormAllocator.h"
#include "FullAtom.h"

VCD_MP4_BEGIN

class CompositionOffsetAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    CompositionOffsetAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~CompositionOffsetAtom() = default;

    struct EntryVersion0    //!< entry version v0
    {
        std::uint32_t m_sampleNum;
        std::uint32_t m_sampleOffset;
    };

    struct EntryVersion1    //!< entry version v1
    {
        std::uint32_t m_sampleNum;
        std::int32_t m_sampleOffset;
    };

    //!
    //! \brief    Add Entry Version v0
    //!
    //! \param    [in] const EntryVersion0&
    //!           entry
    //!
    //! \return   void
    //!
    void AddEntryVersion0(const EntryVersion0& entry);

    //!
    //! \brief    Add Entry Version v1
    //!
    //! \param    [in] const EntryVersion1&
    //!           entry
    //!
    //! \return   void
    //!
    void AddEntryVersion1(const EntryVersion1& entry);

    //!
    //! \brief    Get Sample Number
    //!
    //! \return   uint32_t
    //!           Sample Number
    //!
    uint32_t GetSampleNum();

    //!
    //! \brief    Get Sample Composition Offsets
    //!
    //! \return   std::vector<int>
    //!           Sample Composition Offsets
    //!
    std::vector<int> GetSampleCompositionOffsets() const;

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
    std::vector<EntryVersion0> m_entryVersion0; //!< entry version v0
    std::vector<EntryVersion1> m_entryVersion1; //!< entry version v1
};

VCD_MP4_END;
#endif /* COMPOSITIONOFFSETATOM_H */
