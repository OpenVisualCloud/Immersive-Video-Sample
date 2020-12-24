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
//! \file:   OmafReaderBase.h
//! \brief:  OMAF reader base
//!

#ifndef OMAFREADERBASE_H
#define OMAFREADERBASE_H

#include "Common.h"
#include "OmafXMLElement.h"
#include "BaseUrlElement.h"
#include "MPDElement.h"
#include "PeriodElement.h"
#include "AdaptationSetElement.h"
#include "ViewportElement.h"
#include "EssentialPropertyElement.h"
#include "RepresentationElement.h"
#include "SegmentElement.h"
#include "SupplementalPropertyElement.h"
#include "SphRegionQualityElement.h"
#include "QualityInfoElement.h"

VCD_OMAF_BEGIN

//!
//! \class:  OmafReaderBase
//! \brief:  OMAF reader base class
//!
class OmafReaderBase
{
public:

    //!
    //! \brief Constructor
    //!
    OmafReaderBase(){};

    //!
    //! \brief Destructor
    //!
    virtual ~OmafReaderBase(){};

    //!
    //! \brief    Initialize reader
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    virtual ODStatus Init() = 0;

    //!
    //! \brief    Close reader
    //!
    //! \return   void
    //!
    virtual void Close() = 0;

    //!
    //! \brief    BUild MPD tree
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    virtual ODStatus BuildMPD() = 0;

    //!
    //! \brief    Get MPD element
    //!
    //! \return   MPDElement
    //!           OMAF MPD Element
    //!
    virtual MPDElement* GetMPD() = 0;

    //!
    //! \brief    Build Essential Property Element according to XML element
    //!
    //! \param    [in] xmlEssentialProperty
    //!           Essential Property XML Element
    //!
    //! \return   EssentialPropertyElement
    //!           OMAF Essential Property Element
    //!
    virtual EssentialPropertyElement* BuildEssentialProperty(OmafXMLElement* xmlEssentialProperty) = 0;

    //!
    //! \brief    Build Essential Property Element according to XML element
    //!
    //! \param    [in] xmlEssentialProperty
    //!           Essential Property XML Element
    //!
    //! \return   EssentialPropertyElement
    //!           OMAF Essential Property Element
    //!
    virtual BaseUrlElement* BuildBaseURL(OmafXMLElement* xmlBaseURL) = 0;

    //!
    //! \brief    Build Period Element according to XML element
    //!
    //! \param    [in] xmlPeriod
    //!           Period XML Element
    //!
    //! \return   PeriodElement
    //!           OMAF Period Element
    //!
    virtual PeriodElement* BuildPeriod(OmafXMLElement* xmlPeriod) = 0;

    //!
    //! \brief    Build AdaptationSet Element according to XML element
    //!
    //! \param    [in] xml
    //!           AdaptationSet XML Element
    //!
    //! \return   AdaptationSetElement
    //!           OMAF AdaptationSet Element
    //!
    virtual AdaptationSetElement* BuildAdaptationSet(OmafXMLElement* xml) = 0;

    //!
    //! \brief    Build Viewport Element according to XML element
    //!
    //! \param    [in] xmlViewport
    //!           Viewport XML Element
    //!
    //! \return   ViewportElement
    //!           OMAF Viewport Element
    //!
    virtual ViewportElement* BuildViewport(OmafXMLElement* xmlViewport) = 0;

    //!
    //! \brief    Build Representation Element according to XML element
    //!
    //! \param    [in] xmlRepresentation
    //!           Representation XML Element
    //!
    //! \return   RepresentationElement
    //!           OMAF Representation Element
    //!
    virtual RepresentationElement* BuildRepresentation(OmafXMLElement* xmlRepresentation) = 0;

    virtual AudioChannelConfigurationElement* BuildAudioChannelConfiguration(OmafXMLElement* xmlAudioChlCfg) = 0;
    //!
    //! \brief    Build Segment Element according to XML element
    //!
    //! \param    [in] xmlSegment
    //!           Segment XML Element
    //!
    //! \return   SegmentElement
    //!           OMAF Segment Element
    //!
    virtual SegmentElement* BuildSegment(OmafXMLElement* xmlSegment) = 0;

    //!
    //! \brief    Build Supplemental Property Element according to XML element
    //!
    //! \param    [in] xmlSupplementalProperty
    //!           Supplemental Property XML Element
    //!
    //! \return   SupplementalPropertyElement
    //!           OMAF Supplemental Property Element
    //!
    virtual SupplementalPropertyElement*  BuildSupplementalProperty(OmafXMLElement* xmlSupplementalProperty) = 0;

    //!
    //! \brief    Build Sphere Region Quality Element according to XML element
    //!
    //! \param    [in] xmlSphRegionQuality
    //!           Sphere Region Quality XML Element
    //!
    //! \return   SphRegionQualityElement
    //!           OMAF Sphere Region Quality Element
    //!
    virtual SphRegionQualityElement* BuildSphRegionQuality(OmafXMLElement* xmlSphRegionQuality) = 0;

    //!
    //! \brief    Build Quality Info Element according to XML element
    //!
    //! \param    [in] xmlQualityInfo
    //!           Quality Info XML Element
    //!
    //! \return   QualityInfoElement
    //!           OMAF Quality Info Element
    //!
    virtual QualityInfoElement* BuildQualityInfo(OmafXMLElement* xmlQualityInfo) = 0;
};

VCD_OMAF_END;

#endif //OMAFREADERBASE_H
