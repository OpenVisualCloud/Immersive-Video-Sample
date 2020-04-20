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
//! \file:   SampGroupEntry.h
//! \brief:  SampGroupEntry class.
//! \detail: Provides abstract methods to generate a sample group entry.
//!
//! Created on October 16, 2019, 13:39 PM
//!

#ifndef _SAMPLEGROUPENTRY_H_
#define _SAMPLEGROUPENTRY_H_

#include "Stream.h"
#include "FormAllocator.h"

VCD_MP4_BEGIN

class SampleGroupEntry
{
public:

    //!
    //! \brief Constructor
    //!
    SampleGroupEntry()          = default;

    //!
    //! \brief Destructor
    //!
    virtual ~SampleGroupEntry() = default;

    //!
    //! \brief    Get size
    //!
    //! \return   std::uint32_t
    //!           size
    //!
    virtual std::uint32_t GetSize() const = 0;

    //!
    //! \brief    Write entry content
    //!
    //! \param    [in] Stream&
    //!           bitstream
    //!
    //! \return   void
    //!
    virtual void WriteEntry(Stream& str) = 0;

    //!
    //! \brief    Parse entry content
    //!
    //! \param    [in] Stream&
    //!           bitstream
    //!
    //! \return   void
    //!
    virtual void ParseEntry(Stream& str) = 0;
};

class DirectReferenceSampleListEntry : public SampleGroupEntry
{
public:

    //!
    //! \brief Constructor
    //!
    DirectReferenceSampleListEntry();

    //!
    //! \brief Constructor
    //!
    virtual ~DirectReferenceSampleListEntry() = default;

    //!
    //! \brief    Set and Get function for m_sampleId member
    //!
    //! \param    [in] std::uint32_t
    //!           value to set
    //! \param    [in] m_sampleId
    //!           m_sampleId member in class
    //! \param    [in] SampleId
    //!           m_sampleId name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint32_t, m_sampleId, SampleId, const);

    void SetDirectRefSampleIds(const std::vector<std::uint32_t>& refSampleId);

    //!
    //! \brief    Get Direct Ref Sample Ids
    //!
    //! \return   std::vector<std::uint32_t>
    //!           return Direct Ref Sample Ids
    //!
    std::vector<std::uint32_t> GetDirectRefSampleIds() const;

    //!
    //! \brief    Get size
    //!
    //! \return   std::uint32_t
    //!           size
    //!
    virtual std::uint32_t GetSize() const;

    //!
    //! \brief    Write entry content
    //!
    //! \param    [in] Stream&
    //!           bitstream
    //!
    //! \return   void
    //!
    virtual void WriteEntry(Stream& str);

    //!
    //! \brief    Parse entry content
    //!
    //! \param    [in] Stream&
    //!           bitstream
    //!
    //! \return   void
    //!
    virtual void ParseEntry(Stream& str);

private:
    std::uint32_t m_sampleId;                       //!< Sample Id whose referenced sample Id will be listed
    std::vector<std::uint32_t> m_directRefSampIds;  //!< std::vector of direct reference sample Ids
};

VCD_MP4_END;
#endif /* _SAMPLEGROUPENTRY_H_ */
