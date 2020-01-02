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
//! \file:   DescriptorElement.h
//! \brief:  descriptor element class
//!

#ifndef DESCRIPTORELEMENT_H
#define DESCRIPTORELEMENT_H
#include "OmafXMLElement.h"
#include "OmafElementBase.h"

VCD_OMAF_BEGIN

class DescriptorElement
{
public:

    //!
    //! \brief Constructor
    //!
    DescriptorElement();

    //!
    //! \brief Destructor
    //!
    virtual ~DescriptorElement();

    //!
    //! \brief    Set function for m_schemeIdUri member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_schemeIdUri
    //!           m_schemeIdUri member in class
    //! \param    [in] SchemeIdUri
    //!           m_schemeIdUri name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_schemeIdUri, SchemeIdUri);

    //!
    //! \brief    Set function for m_value member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_value
    //!           m_value member in class
    //! \param    [in] Value
    //!           m_value name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_value, Value);

    //!
    //! \brief    Parse SchemeIdUri attribute and it's Value
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    virtual ODStatus ParseSchemeIdUriAndValue() = 0;



    string m_schemeIdUri; //!< the schemeIdUri attribute
    string m_value;       //!< the value attribute
};

VCD_OMAF_END;

#endif //DESCRIPTORELEMENT_H
