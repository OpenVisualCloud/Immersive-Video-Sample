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
//! \file:   SampGroupDescAtom.h
//! \brief:  Sample Group Description Atom class.
//! \detail: Contains Sample Group Description Atom data structure
//!
//! Created on October 16, 2019, 13:39 PM
//!

#ifndef _SAMPLEGROUPDESCRIPTIONATOM_H_
#define _SAMPLEGROUPDESCRIPTIONATOM_H_

#include "Stream.h"
#include "FormAllocator.h"
#include "FullAtom.h"
#include "SampGroupEntry.h"

VCD_MP4_BEGIN

class SampleGroupDescriptionAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    SampleGroupDescriptionAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~SampleGroupDescriptionAtom() = default;

    //!
    //! \brief    Set version
    //!
    //! \param    [in] std::uint8_t
    //!           version value
    //!
    //! \return   void
    //!
    void SetVersion(std::uint8_t version);

    //!
    //! \brief    Set and Get function for m_groupType member
    //!
    //! \param    [in] FourCCInt
    //!           value to set
    //! \param    [in] m_groupType
    //!           m_groupType member in class
    //! \param    [in] GroupingType
    //!           m_groupType name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(FourCCInt, m_groupType, GroupingType, const);

    //!
    //! \brief    Get Entry Index Of Sample Id
    //!
    //! \param    [in] std::uint32_t
    //!           sample Id value
    //!
    //! \return   std::uint32_t
    //!           return entry index
    //!
    std::uint32_t GetEntryIndexOfSampleId(std::uint32_t sampleId) const;

    //!
    //! \brief    Set and Get function for m_defaultLength member
    //!
    //! \param    [in] std::uint32_t
    //!           value to set
    //! \param    [in] m_defaultLength
    //!           m_defaultLength member in class
    //! \param    [in] DefaultLength
    //!           m_defaultLength name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint32_t, m_defaultLength, DefaultLength, const);

    //!
    //! \brief    add entry
    //!
    //! \param    [in] UniquePtr<SampleGroupEntry>
    //!           sample Group Entry pointer
    //!
    //! \return   void
    //!
    void AddEntry(UniquePtr<SampleGroupEntry> sampleGroupEntry);

    //!
    //! \brief    Get entry
    //!
    //! \param    [in] std::uint32_t
    //!           index
    //!
    //! \return   const SampleGroupEntry*
    //!           SampleGroupEntry pointer
    //!
    const SampleGroupEntry* GetEntry(std::uint32_t index) const;

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
    FourCCInt m_groupType;          //!< Grouping type
    std::uint32_t m_defaultLength;  //!< Default byte size of the description
    std::vector<UniquePtr<SampleGroupEntry>> m_sampleGroupEntry;  //!< std::vector of sample group entries
};

VCD_MP4_END;
#endif /* _SAMPLEGROUPDESCRIPTIONBOX_H_ */
