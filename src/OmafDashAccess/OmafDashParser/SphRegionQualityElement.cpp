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
//! \file:   SphRegionQualityElement.cpp
//! \brief:  omaf:sphRegionQuality element class
//!

#include "SphRegionQualityElement.h"

VCD_OMAF_BEGIN

SphRegionQualityElement::SphRegionQualityElement()
{
    m_shape_type = 0;
    m_remaining_area_flag = 0;
    m_quality_ranking_local_flag = 0;
    m_quality_type = 0;

    m_contentCoverage = nullptr;
    m_defaultIdc      = 0;
    m_viewIdcPresence = 0;
}

SphRegionQualityElement::~SphRegionQualityElement()
{
    m_shape_type = 0;
    m_remaining_area_flag = 0;
    m_quality_ranking_local_flag = 0;
    m_quality_type = 0;
    m_srqrQualityInfo.clear();
    if(m_qualityInfo.size())
    {
        for(auto qi : m_qualityInfo)
        {
            SAFE_DELETE(qi);
        }
        m_qualityInfo.clear();
    }

    SAFE_DELETE(m_contentCoverage);
}

ODStatus SphRegionQualityElement::AddQualityInfo(QualityInfoElement* qualityInfo)
{
    CheckNullPtr_PrintLog_ReturnStatus(qualityInfo, "The input qualityInfo is null.\n", LOG_ERROR, OD_STATUS_INVALID);

    m_qualityInfo.push_back(qualityInfo);

    SrqrQualityInfo srqrQualInfo;
    memset(&srqrQualInfo, 0, sizeof(SrqrQualityInfo));
    srqrQualInfo.azimuth_range = qualityInfo->GetAzimuthRange();
    srqrQualInfo.centre_azimuth = qualityInfo->GetCentreAzimuth();
    srqrQualInfo.centre_elevation = qualityInfo->GetCentreElevation();
    srqrQualInfo.centre_tilt = qualityInfo->GetCentreTilt();
    srqrQualInfo.elevation_range = qualityInfo->GetElevationRange();
    srqrQualInfo.orig_height = qualityInfo->GetOrigHeight();
    srqrQualInfo.orig_width = qualityInfo->GetOrigWidth();
    srqrQualInfo.quality_ranking = qualityInfo->GetQualityRanking();

    m_srqrQualityInfo.push_back(srqrQualInfo);

    return OD_STATUS_SUCCESS;
}

vector<SrqrQualityInfo> SphRegionQualityElement::GetQualityInfos()
{
    return m_srqrQualityInfo;
}

ContentCoverage* SphRegionQualityElement::GetContentCoverage()
{
    if(m_contentCoverage)
        return m_contentCoverage;

    m_contentCoverage = new ContentCoverage();
    CheckNullPtr_PrintLog_ReturnNullPtr(m_contentCoverage, "Failed to create content coverage for SphRegionQualityElement\n", LOG_ERROR);

    m_contentCoverage->shape_type = m_shape_type;
    m_contentCoverage->view_idc_presence = m_viewIdcPresence;
    m_contentCoverage->default_view_idc = m_defaultIdc;
    for(auto qInfo:m_qualityInfo)
    {
        CoverageInfo cInfo;
        memset(&cInfo, 0, sizeof(CoverageInfo));

        cInfo.azimuth_range = qInfo->GetAzimuthRange();
        cInfo.centre_azimuth = qInfo->GetCentreAzimuth();
        cInfo.centre_elevation = qInfo->GetCentreElevation();
        cInfo.centre_tilt = qInfo->GetCentreElevation();
        cInfo.elevation_range = qInfo->GetElevationRange();

        m_contentCoverage->coverage_infos.push_back(cInfo);
    }

    return m_contentCoverage;
}

VCD_OMAF_END;
