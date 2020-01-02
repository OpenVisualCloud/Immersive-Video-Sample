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
//! \file:   OmafXMLElement.cpp
//! \brief:  XML element with OMAF DASH standard
//!

#include "OmafXMLElement.h"

VCD_OMAF_BEGIN

OmafXMLElement::OmafXMLElement()
{
}

OmafXMLElement::~OmafXMLElement()
{
    m_name.clear();
    m_text.clear();
    if(m_attributes.size())
    {
        m_attributes.clear();
    }
    if(m_childElements.size())
    {
        for(auto child : m_childElements)
            SAFE_DELETE(child);
        m_childElements.clear();
    }
}

string OmafXMLElement::GetName()
{
    return m_name;
}

string OmafXMLElement::GetText()
{
    return m_text;
}

string OmafXMLElement::GetPath()
{
    return m_path;
}

vector<OmafXMLElement*> OmafXMLElement::GetChildElements()
{
    return m_childElements;
}

map<string, string> OmafXMLElement::GetAttributes()
{
    return m_attributes;
}

string OmafXMLElement::GetAttributeVal(string attrKey)
{
    if(m_attributes.find(attrKey) != m_attributes.end())
        return m_attributes[attrKey];

    return "";
}

void OmafXMLElement::SetName(string name)
{
    m_name = name;
}

void OmafXMLElement::SetText(string text)
{
    m_text = text;
}

void OmafXMLElement::SetPath(string path)
{
    m_path = path;
}

void OmafXMLElement::AddChildElement(OmafXMLElement* element)
{
    m_childElements.push_back(element);
}

void OmafXMLElement::AddAttribute(string attrKey, string attrVal)
{
    m_attributes.insert(pair<string, string>(attrKey, attrVal));
}

VCD_OMAF_END;
