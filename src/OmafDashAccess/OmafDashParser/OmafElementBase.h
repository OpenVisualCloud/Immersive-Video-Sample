/*
 * Copyright (c) 2019, Intel Corporation
 *  * All rights reserved.
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
//! \file:   OmafElementBase.h
//! \brief:  OMAF element base class
//!

#ifndef OMAFELEMENTBASE_H
#define OMAFELEMENTBASE_H

#include "Common.h"
#include "OmafXMLElement.h"

VCD_OMAF_BEGIN

//!
//! \class:  OmafElementBase
//! \brief:  OMAF element base class
//!
class OmafElementBase
{
public:

    //!
    //! \brief Constructor
    //!
    OmafElementBase();

    //!
    //! \brief Destructor
    //!
    virtual ~OmafElementBase();

    //!
    //! \brief    Add child element
    //!
    //! \param    [in] element
    //!           child element
    //!
    //! \return   void
    //!
    virtual void AddChildElement(OmafXMLElement* element);

    //!
    //! \brief    Add original attributes
    //!
    //! \param    [in] originalAttributes
    //!           map of original attributes
    //!
    //! \return   void
    //!
    virtual void AddOriginalAttributes(map<string, string>& originalAttributes);

    //!
    //! \brief    Get child elements
    //!
    //! \return   vector<OmafXMLElement*>
    //!           vector of child XML elements
    //!
    virtual vector<OmafXMLElement*> GetChildElements();

    //!
    //! \brief    Get original attributes
    //!
    //! \return   map<string, string>
    //!           map of original attributes
    //!
    virtual map<string, string> GetOriginalAttributes();

private:

    vector<OmafXMLElement*>   m_XMLElements;        //!< the child XML elements of this element
    map<string, string>       m_originalAttributes; //!< the original attributes of this element
};

VCD_OMAF_END;

#endif //OMAFELEMENTBASE_H
