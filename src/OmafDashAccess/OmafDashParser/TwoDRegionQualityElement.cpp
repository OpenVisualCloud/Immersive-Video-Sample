/*
 * Copyright (c) 2020, Intel Corporation
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
//! \file:   TwoDRegionQualityElement.cpp
//! \brief:  urn:mpeg:mpegI:omaf:2017:2dqr element class
//!

#include "TwoDRegionQualityElement.h"

VCD_OMAF_BEGIN

TwoDRegionQualityElement::TwoDRegionQualityElement()
{
}

TwoDRegionQualityElement::~TwoDRegionQualityElement()
{
    m_twoDQualityInfos.clear();
    if(m_qualityInfo.size())
    {
        for(auto qi : m_qualityInfo)
        {
            SAFE_DELETE(qi);
        }
        m_qualityInfo.clear();
    }
}

ODStatus TwoDRegionQualityElement::AddQualityInfo(QualityInfoElement* qualityInfo)
{
    CheckNullPtr_PrintLog_ReturnStatus(qualityInfo, "The input qualityInfo is null.\n", LOG_ERROR, OD_STATUS_INVALID);

    m_qualityInfo.push_back(qualityInfo);

    TwoDQualityInfo planarQualityInfo;
    memset(&planarQualityInfo, 0, sizeof(TwoDQualityInfo));
    planarQualityInfo.orig_height = qualityInfo->GetOrigHeight();
    planarQualityInfo.orig_width = qualityInfo->GetOrigWidth();
    planarQualityInfo.quality_ranking = qualityInfo->GetQualityRanking();
    planarQualityInfo.region_width = qualityInfo->GetRegionWidth();
    planarQualityInfo.region_height = qualityInfo->GetRegionHeight();

    m_twoDQualityInfos.insert(make_pair(planarQualityInfo.quality_ranking, planarQualityInfo));

    return OD_STATUS_SUCCESS;
}

map<int32_t, TwoDQualityInfo> TwoDRegionQualityElement::GetTwoDQualityInfos()
{
    return m_twoDQualityInfos;
}

VCD_OMAF_END;
