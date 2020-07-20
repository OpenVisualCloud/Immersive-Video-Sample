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
//! \file:   Segmentlement.h
//! \brief:  SegmentTemplate element class
//!

#ifndef SEGMENTELEMENT_H
#define SEGMENTELEMENT_H
#include <string>

#include "../OmafDashDownload/OmafDownloader.h"
#include "BaseUrlElement.h"
#include "OmafElementBase.h"

VCD_OMAF_BEGIN

class SegmentElement : public OmafElementBase {
 public:
  //!
  //! \brief Constructor
  //!
  SegmentElement();

  //!
  //! \brief Destructor
  //!
  virtual ~SegmentElement();

  //!
  //! \brief    Set function for m_media member
  //!
  //! \param    [in] string
  //!           value to set
  //! \param    [in] m_media
  //!           m_media member in class
  //! \param    [in] Media
  //!           m_media name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(string, m_media, Media);

  //!
  //! \brief    Set function for m_initialization member
  //!
  //! \param    [in] string
  //!           value to set
  //! \param    [in] m_initialization
  //!           m_initialization member in class
  //! \param    [in] Initialization
  //!           m_initialization name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(string, m_initialization, Initialization);

  //!
  //! \brief    Set function for m_duration member
  //!
  //! \param    [in] int32_t
  //!           value to set
  //! \param    [in] m_duration
  //!           m_duration member in class
  //! \param    [in] Duration
  //!           m_duration name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(int32_t, m_duration, Duration);

  //!
  //! \brief    Set function for m_startNumber member
  //!
  //! \param    [in] int32_t
  //!           value to set
  //! \param    [in] m_startNumber
  //!           m_startNumber member in class
  //! \param    [in] StartNumber
  //!           m_startNumber name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(int32_t, m_startNumber, StartNumber);

  //!
  //! \brief    Set function for m_timescale member
  //!
  //! \param    [in] int32_t
  //!           value to set
  //! \param    [in] m_timescale
  //!           m_timescale member in class
  //! \param    [in] Timescale
  //!           m_timescale name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(int32_t, m_timescale, Timescale);

  //!
  //! \brief    Generate the complete URL
  //!
  //! \param    [in] baseURL
  //!           A vector of BaseUrlElement pointer
  //! \param    [in] representation
  //!           A string of representationID address
  //! \param    [in] number
  //!           The index of this element
  //! \param    [in] bandwidth
  //!           Which bandwidth does this element belong to
  //! \param    [in] time
  //!           An int of current time
  //!
  //! \return   string
  //!           The string of complete URL
  //!
  std::string GenerateCompleteURL(const vector<BaseUrlElement*>& baseURL, string& representationID, int32_t number,
                                  int32_t bandwidth = 0, int32_t time = 0);

 private:
  string m_media;           //!< the media attribute
  string m_initialization;  //!< the initialization attribute
  int32_t m_duration;       //!< the duration attribute
  int32_t m_startNumber;    //!< the startNumber attribute
  int32_t m_timescale;      //!< the timescale attribute
};

VCD_OMAF_END;

#endif  // SEGMENTELEMENT_H
