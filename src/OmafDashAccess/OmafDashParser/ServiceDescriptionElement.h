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
//! \file:   ServiceDescriptionElement.h
//! \brief:  ServiceDescription element class
//!

#ifndef SERVICEDESCRIPTIONELEMENT_H
#define SERVICEDESCRIPTIONELEMENT_H

#include "LatencyElement.h"

VCD_OMAF_BEGIN

class ServiceDescriptionElement: public OmafElementBase
{
public:

    //!
    //! \brief Constructor
    //!
    ServiceDescriptionElement();

    //!
    //! \brief Destructor
    //!
    virtual ~ServiceDescriptionElement();

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
  //! \brief    Set function for m_latency member
  //!
  //! \param    [in] LatencyElement
  //!           value to set
  //! \param    [in] m_latency
  //!           m_latency member in class
  //! \param    [in] Latency
  //!           m_latency name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(LatencyElement*, m_latency, Latency);

private:

    string  m_id;             //!< the id attribute

    LatencyElement* m_latency; //!< the AdaptationSet elements
};

VCD_OMAF_END;

#endif //SERVICEDESCRIPTIONELEMENT_H
