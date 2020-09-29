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
//! \file:   MPDElement.cpp
//! \brief:  MPD file element class
//!

#include "MPDElement.h"

VCD_OMAF_BEGIN

MPDElement::MPDElement()
{
}

MPDElement::~MPDElement()
{
    m_xmlns_omaf.clear();
    m_xmlns_xsi.clear();
    m_xmlns.clear();
    m_xmlns_xlink.clear();
    m_xsi_schemaLocation.clear();
    m_minBufferTime.clear();
    m_maxSegmentDuration.clear();
    m_profiles.clear();
    m_type.clear();
    m_availabilityStartTime.clear();
    m_timeShiftBufferDepth.clear();
    m_minimumUpdatePeriod.clear();
    m_publishTime.clear();

    if(m_essentialProperties.size())
    {
        for(auto essentialProperty : m_essentialProperties)
            SAFE_DELETE(essentialProperty);
        m_essentialProperties.clear();
    }

    if(m_baseUrls.size())
    {
        for(auto baseUrl : m_baseUrls)
            SAFE_DELETE(baseUrl);
        m_baseUrls.clear();
    }

    if(m_periods.size())
    {
        for(auto period : m_periods)
            SAFE_DELETE(period);
        m_periods.clear();
    }
}

void MPDElement::AddEssentialProperty(EssentialPropertyElement* essentialProperty)
{
    if(!essentialProperty)
    {
        OMAF_LOG(LOG_ERROR,"Fail to add essentialProperty in MPDElement.\n");
        return;
    }
    m_essentialProperties.push_back(essentialProperty);
}

void MPDElement::AddBaseUrl(BaseUrlElement* baseUrl)
{
    if(!baseUrl)
    {
        OMAF_LOG(LOG_ERROR,"Fail to add baseUrl in MPDElement.\n");
        return;
    }
    m_baseUrls.push_back(baseUrl);
}

void MPDElement::AddPeriod(PeriodElement* period)
{
    if(!period)
    {
        OMAF_LOG(LOG_ERROR,"Fail to add period in MPDElement.\n");
        return;
    }
    m_periods.push_back(period);
}

void MPDElement::AddProfile(string profile)
{
    if(!profile.length())
        return;

    m_profiles.push_back(profile);
}

ProjectionFormat MPDElement::GetProjectionFormat()
{
    ProjectionFormat pf = PF_UNKNOWN;
    if(!m_essentialProperties.size())
        return pf;

    for(auto e: m_essentialProperties)
    {
        pf = e->GetProjectionFormat();
        if(pf > PF_UNKNOWN && pf < PF_RESERVED)
            return pf;
    }

    return pf;
}


VCD_OMAF_END
