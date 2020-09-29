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
//! \file:   EssentialPropertyElement.cpp
//! \brief:  EssentialProperty element class
//!

#include "EssentialPropertyElement.h"

VCD_OMAF_BEGIN

EssentialPropertyElement::EssentialPropertyElement()
{
    m_srd = nullptr;
    m_pf = PF_UNKNOWN;
    m_rwpkType = RWPK_UNKNOWN;
}

EssentialPropertyElement::~EssentialPropertyElement()
{
    GetSchemeIdUri().clear();
    GetValue().clear();
    SAFE_DELETE(m_srd);
    m_pf = PF_UNKNOWN;
    m_rwpkType = RWPK_UNKNOWN;
}

ODStatus EssentialPropertyElement::ParseSchemeIdUriAndValue()
{
    if(GetSchemeIdUri() == SCHEMEIDURI_SRD)
    {
        if(0 == GetValue().length())
            OMAF_LOG(LOG_WARNING,"SRD doesn't have value.\n");

        m_srd = new OmafSrd();
        CheckNullPtr_PrintLog_ReturnStatus(m_srd, "Failed to create OmafSrd.", LOG_ERROR, OD_STATUS_OPERATION_FAILED);

        m_srd->SetInfo((char*)GetValue().c_str());
    }
    else if(GetSchemeIdUri() == SCHEMEIDURI_PF)
    {
        int pf = StringToInt(GetProjectionType());
        ProjectionFormat format = static_cast<ProjectionFormat>(pf);
        if(format < PF_UNKNOWN || format > PF_RESERVED)
        {
            OMAF_LOG(LOG_WARNING,"the projection format is invalid.\n");
            return OD_STATUS_INVALID;
        }

        m_pf = format;
    }
    else if(GetSchemeIdUri() == SCHEMEIDURI_RWPK)
    {
        RwpkType rwpkPackingType = (RwpkType)StringToInt(GetRwpkPackingType());
        if(rwpkPackingType <= RWPK_UNKNOWN || rwpkPackingType >= RWPK_RESERVED)
        {
            OMAF_LOG(LOG_WARNING,"the RWPK type is invalid.\n");
            return OD_STATUS_INVALID;
        }

        m_rwpkType = rwpkPackingType;
    }


    return OD_STATUS_SUCCESS;
}

VCD_OMAF_END;
