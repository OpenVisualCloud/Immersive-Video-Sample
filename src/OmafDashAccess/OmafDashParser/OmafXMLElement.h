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
//! \file:   OmafXMLElement.h
//! \brief:  XML element with OMAF DASH standard
//!

#ifndef OMAFXMLELEMENT_H
#define OMAFXMLELEMENT_H

#include "Common.h"

VCD_OMAF_BEGIN

//!
//! \class:  OmafXMLElement
//! \brief:  OMAF XML element
//!
class OmafXMLElement
{
public:

    //!
    //! \brief Constructor
    //!
    OmafXMLElement();

    //!
    //! \brief Destructor
    //!
    ~OmafXMLElement();

    //!
    //! \brief    Get name of this element
    //!
    //! \return   string
    //!           name in string
    //!
    string GetName();

    //!
    //! \brief    Get text of this element
    //!
    //! \return   string
    //!           text in string
    //!
    string GetText();

    //!
    //! \brief    Get path of this element
    //!
    //! \return   string
    //!           path in string
    //!
    string GetPath();

    //!
    //! \brief    Get child elements of this element
    //!
    //! \return   vector<OmafXMLElement*>
    //!           vector of XML elements
    //!
    vector<OmafXMLElement*>      GetChildElements();

    //!
    //! \brief    Get attributes of this element
    //!
    //! \return   map<string, string>
    //!           map of attributes
    //!
    map<string, string>          GetAttributes();

    //!
    //! \brief    Get attributes of this element with key
    //!
    //! \param    [in] attrKey
    //!           attribute key
    //!
    //! \return   string
    //!           attribute value
    //!
    string                       GetAttributeVal(string attrKey);

    //!
    //! \brief    Set name for this element
    //!
    //! \param    [in] name
    //!           name in string
    //!
    //! \return   void
    //!
    void SetName(string name);

    //!
    //! \brief    Set text for this element
    //!
    //! \param    [in] text
    //!           text in string
    //!
    //! \return   void
    //!
    void SetText(string text);

    //!
    //! \brief    Set path for this element
    //!
    //! \param    [in] path
    //!           path in string
    //!
    //! \return   void
    //!
    void SetPath(string path);

    //!
    //! \brief    Add child element
    //!
    //! \param    [in] element
    //!           child element
    //!
    //! \return   void
    //!
    void AddChildElement(OmafXMLElement* element);

    //!
    //! \brief    Add attribute for this element
    //!
    //! \param    [in] attrKey
    //!           key of attribute
    //! \param    [in] attrVal
    //!           value of attribute
    //!
    //! \return   void
    //!
    void AddAttribute(string attrKey, string attrVal);

private:

    string                      m_name;          //!< name of this element
    string                      m_text;          //!< text of this element
    string                      m_path;          //!< path of this element
    map<string, string>         m_attributes;    //!< attributes of this element
    vector<OmafXMLElement*>     m_childElements; //!< child elements of this element
};

VCD_OMAF_END;

#endif //OMAFXMLELEMENT_H
