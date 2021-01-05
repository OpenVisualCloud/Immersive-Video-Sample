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
//! \file:   OmafMPDReader.h
//! \brief:  parse XML tree to get MPD tree with OMAF DASH standard
//!

#ifndef OMAFMPDREADER_H
#define OMAFMPDREADER_H

#include "OmafReaderBase.h"
#include "OmafXMLElement.h"
#include "MPDElement.h"

VCD_OMAF_BEGIN

//!
//! \class:  OmafMPDReader
//! \brief:  OMAF MPD reader
//!
class OmafMPDReader: public OmafReaderBase
{
public:

    //!
    //! \brief Constructor
    //!
    OmafMPDReader();

    //!
    //! \brief Constructor with parameter
    //!
    OmafMPDReader(OmafXMLElement *root);

    //!
    //! \brief Destructor
    //!
    virtual ~OmafMPDReader();

    //!
    //! \brief    Initialize reader
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    virtual ODStatus Init();

    //!
    //! \brief    Close reader
    //!
    //! \return   void
    //!
    virtual void Close();

    //!
    //! \brief    BUild MPD tree
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    virtual ODStatus BuildMPD();

    //!
    //! \brief    Build Essential Property Element according to XML element
    //!
    //! \param    [in] xmlEssentialProperty
    //!           Essential Property XML Element
    //!
    //! \return   EssentialPropertyElement
    //!           OMAF Essential Property Element
    //!
    virtual EssentialPropertyElement* BuildEssentialProperty(OmafXMLElement* xmlEssentialProperty);

    //!
    //! \brief    Build Essential Property Element according to XML element
    //!
    //! \param    [in] xmlEssentialProperty
    //!           Essential Property XML Element
    //!
    //! \return   EssentialPropertyElement
    //!           OMAF Essential Property Element
    //!
    virtual BaseUrlElement* BuildBaseURL(OmafXMLElement* xmlBaseURL);

    //!
    //! \brief    Build Period Element according to XML element
    //!
    //! \param    [in] xmlPeriod
    //!           Period XML Element
    //!
    //! \return   PeriodElement
    //!           OMAF Period Element
    //!
    virtual PeriodElement* BuildPeriod(OmafXMLElement* xmlPeriod);

    //!
    //! \brief    Build AdaptationSet Element according to XML element
    //!
    //! \param    [in] xml
    //!           AdaptationSet XML Element
    //!
    //! \return   AdaptationSetElement
    //!           OMAF AdaptationSet Element
    //!
    virtual AdaptationSetElement* BuildAdaptationSet(OmafXMLElement* xml);

    //!
    //! \brief    Build Viewport Element according to XML element
    //!
    //! \param    [in] xmlViewport
    //!           Viewport XML Element
    //!
    //! \return   ViewportElement
    //!           OMAF Viewport Element
    //!
    virtual ViewportElement* BuildViewport(OmafXMLElement* xmlViewport);

    //!
    //! \brief    Build Representation Element according to XML element
    //!
    //! \param    [in] xmlRepresentation
    //!           Representation XML Element
    //!
    //! \return   RepresentationElement
    //!           OMAF Representation Element
    //!
    virtual RepresentationElement* BuildRepresentation(OmafXMLElement* xmlRepresentation);

    virtual AudioChannelConfigurationElement* BuildAudioChannelConfiguration(OmafXMLElement* xmlAudioChlCfg);
    //!
    //! \brief    Build Segment Element according to XML element
    //!
    //! \param    [in] xmlSegment
    //!           Segment XML Element
    //!
    //! \return   SegmentElement
    //!           OMAF Segment Element
    //!
    virtual SegmentElement* BuildSegment(OmafXMLElement* xmlSegment);

    //!
    //! \brief    Build Supplemental Property Element according to XML element
    //!
    //! \param    [in] xmlSupplementalProperty
    //!           Supplemental Property XML Element
    //!
    //! \return   SupplementalPropertyElement
    //!           OMAF Supplemental Property Element
    //!
    virtual SupplementalPropertyElement* BuildSupplementalProperty(OmafXMLElement* xmlSupplementalProperty);

    //!
    //! \brief    Build Sphere Region Quality Element according to XML element
    //!
    //! \param    [in] xmlSphRegionQuality
    //!           Sphere Region Quality XML Element
    //!
    //! \return   SphRegionQualityElement
    //!           OMAF Sphere Region Quality Element
    //!
    virtual SphRegionQualityElement* BuildSphRegionQuality(OmafXMLElement* xmlSphRegionQuality);

    //!
    //! \brief    Build 2D Region Quality Element according to XML element
    //!
    //! \param    [in] xmlTwoDRegionQuality
    //!           2D Region Quality XML Element
    //!
    //! \return   TwoDRegionQualityElement
    //!           OMAF 2D Region Quality Element
    //!
    virtual TwoDRegionQualityElement* BuildTwoDRegionQuality(OmafXMLElement* xmlTwoDRegionQuality);

    //!
    //! \brief    Build Quality Info Element according to XML element
    //!
    //! \param    [in] xmlQualityInfo
    //!           Quality Info XML Element
    //!
    //! \return   QualityInfoElement
    //!           OMAF Quality Info Element
    //!
    virtual QualityInfoElement* BuildQualityInfo(OmafXMLElement* xmlQualityInfo);

    //!
    //! \brief    Get MPD element
    //!
    //! \return   MPDElement
    //!           OMAF MPD Element
    //!
    virtual MPDElement* GetMPD() {return m_mpd;}

private:
    OmafMPDReader& operator=(const OmafMPDReader& other) { return *this; };
    OmafMPDReader(const OmafMPDReader& other) { /* do not create copies */ };

private:

    OmafXMLElement      *m_rootXMLElement; //!< root XML element
    MPDElement          *m_mpd;            //!< root MPD element
};

VCD_OMAF_END

#endif //OMAFMPDREADER_H
