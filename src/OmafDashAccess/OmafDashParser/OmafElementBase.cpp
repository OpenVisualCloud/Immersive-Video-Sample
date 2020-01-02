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
//! \file:   OmafElementBase.cpp
//! \brief:  OMAF element base class
//!

#include "OmafElementBase.h"

VCD_OMAF_BEGIN

OmafElementBase::OmafElementBase()
{
}

OmafElementBase::~OmafElementBase()
{
    if(m_XMLElements.size())
    {
        m_XMLElements.clear(); // no need to delete the element pointers here
    }

    if(m_originalAttributes.size())
        m_originalAttributes.clear();
}

void OmafElementBase::AddChildElement(OmafXMLElement* element)
{
    m_XMLElements.push_back(element);
}

void OmafElementBase::AddOriginalAttributes(map<string, string>& originalAttributes)
{
    m_originalAttributes.insert(originalAttributes.begin(), originalAttributes.end());
}

vector<OmafXMLElement*> OmafElementBase::GetChildElements()
{
    return m_XMLElements;
}

map<string, string> OmafElementBase::GetOriginalAttributes()
{
    return m_originalAttributes;
}

VCD_OMAF_END;
