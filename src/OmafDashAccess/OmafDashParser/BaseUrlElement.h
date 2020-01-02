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
//! \file:   BaseUrlElement.h
//! \brief:  BaseUrl element class
//!

#ifndef BASEURLELEMENT_H
#define BASEURLELEMENT_H
#include"OmafElementBase.h"

VCD_OMAF_BEGIN

class BaseUrlElement: public OmafElementBase
{
public:

    //!
    //! \brief Constructor
    //!
    BaseUrlElement();

    //!
    //! \brief Destructor
    //!
    virtual ~BaseUrlElement();

    //!
    //! \brief    Set function for m_path member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_path
    //!           m_path member in class
    //! \param    [in] Path
    //!           m_path name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_path, Path);

private:

    string  m_path; //!< the string to save URL
};

VCD_OMAF_END;

#endif //BASEURLELEMENT_H
