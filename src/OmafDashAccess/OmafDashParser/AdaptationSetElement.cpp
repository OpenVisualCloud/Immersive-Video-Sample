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
//! \file:   AdaptationSetElement.cpp
//! \brief:  AdaptationSet element class
//!

#include "AdaptationSetElement.h"

VCD_OMAF_BEGIN

AdaptationSetElement::AdaptationSetElement()
{
}

AdaptationSetElement::~AdaptationSetElement()
{
    m_id.clear();
    m_mimeType.clear();
    m_codecs.clear();
    m_maxWidth.clear();
    m_maxHeight.clear();
    m_maxFrameRate.clear();
    m_audioSamplingRate.clear();
    m_segmentAlignment.clear();
    m_subsegmentAlignment.clear();

    if(m_viewport.size())
    {
        for(auto viewport : m_viewport)
            SAFE_DELETE(viewport);
        m_viewport.clear();
    }

    if(m_essentialProperties.size())
    {
        for(auto essentialProperty : m_essentialProperties)
            SAFE_DELETE(essentialProperty);
        m_essentialProperties.clear();
    }

    if(m_representations.size())
    {
        for( auto r : m_representations)
        {
            SAFE_DELETE(r);
        }
        m_representations.clear();
    }

    if(m_supplementalProperties.size())
    {
        for(auto sp : m_supplementalProperties)
        {
            SAFE_DELETE(sp);
        }
        m_supplementalProperties.clear();
    }
}

void AdaptationSetElement::AddViewport(ViewportElement* viewport)
{
    if(!viewport)
    {
        OMAF_LOG(LOG_ERROR,"Fail to add viewport in Element.\n");
        return;
    }
    m_viewport.push_back(viewport);
}

void AdaptationSetElement::AddEssentialProperty(EssentialPropertyElement* essentialProperty)
{
    if(!essentialProperty)
    {
        OMAF_LOG(LOG_ERROR,"Fail to add essentialProperty in Element.\n");
        return;
    }
    m_essentialProperties.push_back(essentialProperty);
}

void AdaptationSetElement::AddSupplementalProperty(SupplementalPropertyElement* supplementalProperty)
{
    if(!supplementalProperty)
    {
        OMAF_LOG(LOG_ERROR,"Fail to add supplementalProperty in Element.\n");
        return;
    }
    m_supplementalProperties.push_back(supplementalProperty);
}

void AdaptationSetElement::AddRepresentation(RepresentationElement* representation)
{
    if(!representation)
    {
        OMAF_LOG(LOG_ERROR,"Fail to add representation in Element.\n");
        return;
    }
    m_representations.push_back(representation);
}

ProjectionFormat AdaptationSetElement::GetProjectionFormat()
{
    ProjectionFormat pf = PF_UNKNOWN;

    if(!m_essentialProperties.size())
        return pf;
    
    for(auto p : m_essentialProperties)
    {
        auto projf = p->GetProjectionFormat();
        if(projf > PF_UNKNOWN && projf < PF_RESERVED)
            return projf;
    }

    return pf;
}

PreselValue* AdaptationSetElement::GetPreselection()
{
    if(!m_supplementalProperties.size())
        return nullptr;

    for(auto s : m_supplementalProperties)
    {
        auto presel = s->GetPreselection();
        if(presel) return presel;
    }

    return nullptr;
}

SphereQuality* AdaptationSetElement::GetSphereQuality()
{
    if(!m_supplementalProperties.size())
        return nullptr;

    for(auto s : m_supplementalProperties)
    {
        auto srqr = s->GetSRQR();
        if(srqr) return srqr;
    }

    return nullptr;
}

OmafSrd* AdaptationSetElement::GetSRD()
{
    // it can also be in essential property
    if(!m_supplementalProperties.size() && !m_essentialProperties.size())
        return nullptr;

    for(auto s : m_supplementalProperties)
    {
        auto srd = s->GetSRD();
        if(srd) return srd;
    }

    for(auto e : m_essentialProperties)
    {
        auto srd = e->GetSRD();
        if(srd) return srd;
    }

    return nullptr;
}

map<int32_t, TwoDQualityInfo> AdaptationSetElement::GetTwoDQuality()
{
    map<int32_t, TwoDQualityInfo> twoDQualityInfos;
    // it can also be in essential property
    if(!m_supplementalProperties.size())
        return twoDQualityInfos;

    for(auto s : m_supplementalProperties)
    {
        twoDQualityInfos = s->GetTwoDRegionQualityInfos();
        if(twoDQualityInfos.size()) return twoDQualityInfos;
    }

    return twoDQualityInfos;
}

RwpkType AdaptationSetElement::GetRwpkType()
{
    RwpkType rt = RWPK_UNKNOWN;

    if(!m_essentialProperties.size())
        return rt;

    for(auto e: m_essentialProperties)
    {
        rt = e->GetRwpkType();
        if(rt > RWPK_UNKNOWN && rt < RWPK_RESERVED)
            return rt;
    }

    return rt;
}

ContentCoverage* AdaptationSetElement::GetContentCoverage()
{
    if(!m_supplementalProperties.size())
        return nullptr;

    for(auto s : m_supplementalProperties)
    {
        auto cc = s->GetContentCoverage();
        if(cc) return cc;
    }

    return nullptr;
}


VCD_OMAF_END;
