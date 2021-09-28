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
//! \file:   ProducerReferenceTimeElement.h
//! \brief:  ProducerReferenceTime element class
//!

#ifndef PRODUCERREFERENCETIMEELEMENT_H
#define PRODUCERREFERENCETIMEELEMENT_H
#include <string>
#include "OmafElementBase.h"

VCD_OMAF_BEGIN

class ProducerReferenceTimeElement : public OmafElementBase{
 public:
  //!
  //! \brief Constructor
  //!
  ProducerReferenceTimeElement();

  //!
  //! \brief Destructor
  //!
  virtual ~ProducerReferenceTimeElement();

  //!
  //! \brief    Set function for m_id
  //!
  //! \param    [in] int32_t
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
  //! \brief    Set function for m_inband
  //!
  //! \param    [in] bool
  //!           value to set
  //! \param    [in] m_inband
  //!           m_inband member in class
  //! \param    [in] Inband
  //!           m_inband name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(string, m_inband, Inband);

  //!
  //! \brief    Set function for m_type
  //!
  //! \param    [in] string
  //!           value to set
  //! \param    [in] m_type
  //!           m_type member in class
  //! \param    [in] Type
  //!           m_type name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(string, m_type, Type);

  //!
  //! \brief    Set function for m_wallclockTime
  //!
  //! \param    [in] string
  //!           value to set
  //! \param    [in] m_wallclockTime
  //!           m_wallclockTime member in class
  //! \param    [in] WallclockTime
  //!           m_wallclockTime name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(string, m_wallclockTime, WallclockTime);

  //!
  //! \brief    Set function for m_presentationTime
  //!
  //! \param    [in] int32_t
  //!           value to set
  //! \param    [in] m_presentationTime
  //!           m_presentationTime member in class
  //! \param    [in] PresentationTime
  //!           m_presentationTime name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(string, m_presentationTime, PresentationTime);

 private:
  string m_id;           //!< the id attribute which is corresponding to media presentation
  string m_inband;          //!< the inband attribute
  string m_type;          //!< the type attribute
  string m_wallclockTime; //!< the wallclockTime attribute
  string m_presentationTime; //!< the presentationTime attribute
};

VCD_OMAF_END;

#endif  // PRODUCERREFERENCETIMEELEMENT_H
