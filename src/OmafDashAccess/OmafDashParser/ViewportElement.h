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
//! \file:   ViewportElement.h
//! \brief:  Viewport element class
//!

#ifndef VIEWPORTELEMENT_H
#define VIEWPORTELEMENT_H
#include "OmafElementBase.h"
#include "DescriptorElement.h"
#include "OmafStructure.h"

VCD_OMAF_BEGIN

class ViewportElement: public DescriptorElement, public OmafElementBase
{
public:

    //!
    //! \brief Constructor
    //!
    ViewportElement();

    //!
    //! \brief Destructor
    //!
    virtual ~ViewportElement();

    //!
    //! \brief    Parse SchemeIdUri and it's value
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    virtual ODStatus ParseSchemeIdUriAndValue();

    ViewportProperty* GetViewport() { return m_viewport; }

private:
    ViewportElement& operator=(const ViewportElement& other) { return *this; };
    ViewportElement(const ViewportElement& other) { /* do not create copies */ };

private:
    ViewportProperty *m_viewport;
};

VCD_OMAF_END;

#endif //VIEWPORTELEMENT_H
