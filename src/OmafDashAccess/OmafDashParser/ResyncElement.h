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
//! \file:   ResyncElement.h
//! \brief:  Resync element class
//!

#ifndef RESYNCELEMENT_H
#define RESYNCELEMENT_H
#include <string>
#include "OmafElementBase.h"

VCD_OMAF_BEGIN

class ResyncElement : public OmafElementBase{
 public:
  //!
  //! \brief Constructor
  //!
  ResyncElement();

  //!
  //! \brief Destructor
  //!
  virtual ~ResyncElement();

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
  //! \brief    Set function for m_chunkDuration
  //!
  //! \param    [in] string
  //!           value to set
  //! \param    [in] m_chunkDuration
  //!           m_chunkDuration member in class
  //! \param    [in] ChunkDuration
  //!           m_chunkDuration name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(string, m_chunkDuration, ChunkDuration);

 private:
  string m_type;           //!< the type attribute
  string m_chunkDuration;  //!< the dT attribute
};

VCD_OMAF_END;

#endif  // RESYNCELEMENT_H
