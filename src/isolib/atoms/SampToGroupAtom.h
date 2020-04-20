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
//! \file:   SampToGroupAtom.h
//! \brief:  SampToGroupAtom class.
//! \detail: 'sbgp' Atom
//!
//! Created on October 16, 2019, 13:39 PM
//!

#ifndef _SAMPLETOGROUPATOM_H_
#define _SAMPLETOGROUPATOM_H_

#include "Stream.h"
#include "FormAllocator.h"
#include "FullAtom.h"

VCD_MP4_BEGIN

class SampleToGroupAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    SampleToGroupAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~SampleToGroupAtom() = default;

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
    //! \brief    Set and Get function for m_groupTypeParameter member
    //!
    //! \param    [in] std::uint32_t
    //!           value to set
    //! \param    [in] m_groupTypeParameter
    //!           m_groupTypeParameter member in class
    //! \param    [in] GroupingTypeParameter
    //!           m_groupTypeParameter name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint32_t, m_groupTypeParameter, GroupingTypeParameter, const);

    //!
    //! \brief    Set Entry Count
    //!
    //! \param    [in] std::uint32_t
    //!           Entry Count value
    //!
    //! \return   void
    //!
    void SetEntryCount(std::uint32_t count);

    //!
    //! \brief    Get Entry Count
    //!
    //! \return   std::uint32_t
    //!           Entry Count
    //!
    std::uint32_t GetEntryCount() const;

    //!
    //! \brief    Add Sample Run
    //!
    //! \param    [in] std::uint32_t
    //!           sampleNum
    //! \param    [in] std::uint32_t
    //!           index value
    //!
    //! \return   void
    //!
    void AddSampleRun(std::uint32_t sampleNum, std::uint32_t idx);

    //!
    //! \brief    Get Sample Group Description Index
    //!
    //! \param    [in] std::uint32_t
    //!           index value
    //!
    //! \return   std::uint32_t
    //!           Sample Group Description Index
    //!
    std::uint32_t GetSampleGroupDescriptionIndex(std::uint32_t idx) const;

    //!
    //! \brief    Get Sample id
    //!
    //! \param    [in] std::uint32_t
    //!           index value
    //!
    //! \return   std::uint32_t
    //!           Sample id
    //!
    std::uint32_t GetSampleId(std::uint32_t idx) const;

    //!
    //! \brief    Get Number Of Samples
    //!
    //! \return   unsigned int
    //!           Number Of Samples
    //!
    unsigned int GetNumberOfSamples() const;

private:
    FourCCInt m_groupType;                              //!< group Type
    std::uint32_t m_entryCount;                         //!< entry Count
    std::uint32_t m_groupTypeParameter;                 //!< group Type Parameter
    struct SampleRun                                    //!< Sample Run
    {
        std::uint32_t sampleNum;
        std::uint32_t groupDescriptionIndex;
    };
    std::vector<SampleRun> m_runOfSamples;              //!< run Of Samples
    std::vector<std::uint32_t> m_sampleToGroupIndex;    //!< sample To Group Index

    //!
    //! \brief Update Internal Index
    //!
    void UpdateInternalIndex();
};

VCD_MP4_END;
#endif /* _SAMPLETOGROUPATOM_H_ */
