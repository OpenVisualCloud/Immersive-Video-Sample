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
//! \file:   PeriodElement.h
//! \brief:  Period element class
//!

#ifndef PERIODELEMENT_H
#define PERIODELEMENT_H

#include "AdaptationSetElement.h"

VCD_OMAF_BEGIN

class PeriodElement: public OmafElementBase
{
public:

    //!
    //! \brief Constructor
    //!
    PeriodElement();

    //!
    //! \brief Destructor
    //!
    virtual ~PeriodElement();

    //!
    //! \brief    Set function for m_start member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_start
    //!           m_start member in class
    //! \param    [in] Start
    //!           m_start name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_start, Start);

    //!
    //! \brief    Set function for m_id member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_id
    //!           m_id member in class
    //! \param    [in] Id
    //!           m_id name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_id, Id);

    //!
    //! \brief    Add an instance of AdaptationSet element
    //!
    //! \param    [in] adaptionSet
    //!           An Instance of AdaptationSet element class
    //!
    //! \return   void
    //!
    void AddAdaptationSet(AdaptationSetElement* adaptionSet);

    //!
    //! \brief    Get all AdaptationSet elemenst
    //!
    //! \return   vector<AdaptationSetElement*>
    //!           vector of AdaptationSet Element
    //!
    vector<AdaptationSetElement*> GetAdaptationSets() {return m_adaptionSets;}

private:

    string                      m_start;          //!< the start attribute
    string                      m_id;             //!< the id attribute

    vector<AdaptationSetElement*> m_adaptionSets; //!< the AdaptationSet elements
};

VCD_OMAF_END;

#endif //PERIODELEMENT_H
