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
//! \file:   SupplementalPropertyElement.cpp
//! \brief:  SupplementalProperty element class
//!

#include "SupplementalPropertyElement.h"

VCD_OMAF_BEGIN

SupplementalPropertyElement::SupplementalPropertyElement()
{
    m_srd = nullptr;
    m_srqr = nullptr;
    m_preselection = nullptr;
    m_sphRegionQuality = nullptr;
    m_twoDRegionQuality = nullptr;
}

SupplementalPropertyElement::~SupplementalPropertyElement()
{
    SAFE_DELETE(m_srd);
    SAFE_DELETE(m_srqr);
    if(m_preselection)
    {
        m_preselection->SelAsIDs.clear();
        SAFE_DELETE(m_preselection);
    }
    SAFE_DELETE(m_sphRegionQuality);
    SAFE_DELETE(m_twoDRegionQuality);
    m_twoDQualityInfos.clear();
}


ODStatus SupplementalPropertyElement::ParseSchemeIdUriAndValue()
{
    if(GetSchemeIdUri() == SCHEMEIDURI_SRD)
    {
        if(0 == GetValue().length())
            OMAF_LOG(LOG_WARNING, "SRD doesn't have value.\n");

        m_srd = new OmafSrd();
        CheckNullPtr_PrintLog_ReturnStatus(m_srd, "Failed to create OmafSrd.\n", LOG_ERROR, OD_STATUS_OPERATION_FAILED);

        m_srd->SetInfo((char*)GetValue().c_str());
    }
    else if(GetSchemeIdUri() == SCHEMEIDURI_SRQR)
    {
        m_srqr = new SphereQuality();
        CheckNullPtr_PrintLog_ReturnStatus(m_srqr, "Failed to create SphereQuality.\n", LOG_ERROR, OD_STATUS_OPERATION_FAILED);
    }
    else if(GetSchemeIdUri() == SCHEMEIDURI_PRESELECTION)
    {
        m_preselection = new PreselValue();
        CheckNullPtr_PrintLog_ReturnStatus(m_preselection, "Failed to create PreselValue.\n", LOG_ERROR, OD_STATUS_OPERATION_FAILED);

        string preselVal = GetValue();
        std::vector<std::string> splitTag;
        SplitString( preselVal, splitTag, "," );

        m_preselection->PreselTag = splitTag[0];

        std::vector<std::string> splitIDs;
        SplitString( splitTag[1], splitIDs, " ");

        for( uint32_t i = 1; i < splitIDs.size() ; i++)
        {
            string s = splitIDs[i];
            if(s.length())
                m_preselection->SelAsIDs.push_back(StringToInt(s));
            else
                OMAF_LOG(LOG_WARNING, "this adaptation set ID is invalid.\n");
        }

    }

    return OD_STATUS_SUCCESS;
}

ODStatus SupplementalPropertyElement::SetSphereRegionQuality(SphRegionQualityElement* sphRegionQuality)
{
    if(!sphRegionQuality)
        return OD_STATUS_INVALID;

    m_sphRegionQuality = sphRegionQuality;

    CheckNullPtr_PrintLog_ReturnStatus(m_srqr, "The input SphereQuality is null.\n", LOG_ERROR, OD_STATUS_INVALID);

    memset(m_srqr, 0 , sizeof(SphereQuality));
    m_srqr->shape_type = sphRegionQuality->GetShapeType();
    m_srqr->remaining_area_flag = sphRegionQuality->GetRemainingAreaFlag();
    m_srqr->quality_ranking_local = sphRegionQuality->GetQualityRankingLocalFlag();
    m_srqr->quality_type = sphRegionQuality->GetQualityType();
    m_srqr->default_view_idc = sphRegionQuality->GetDefaultIdc();
    m_srqr->view_idc_presence = sphRegionQuality->GetViewIdcPresence();

    auto quaInfos = m_sphRegionQuality->GetQualityInfos();
    m_srqr->srqr_quality_infos.insert(m_srqr->srqr_quality_infos.end(), quaInfos.begin(), quaInfos.end());

    return OD_STATUS_SUCCESS;
}

ODStatus SupplementalPropertyElement::SetTwoDRegionQuality(TwoDRegionQualityElement* twoDRegionQuality)
{
    if (!twoDRegionQuality)
        return OD_STATUS_INVALID;

    m_twoDRegionQuality = twoDRegionQuality;

    m_twoDQualityInfos = m_twoDRegionQuality->GetTwoDQualityInfos();

    return OD_STATUS_SUCCESS;
}

VCD_OMAF_END;
