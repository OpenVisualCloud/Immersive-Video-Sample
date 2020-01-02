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
//! \file:   MPDElement.h
//! \brief:  MPD file element class
//!

#ifndef MPDELEMENT_H
#define MPDELEMENT_H

#include "OmafElementBase.h"
#include "EssentialPropertyElement.h"
#include "BaseUrlElement.h"
#include "PeriodElement.h"

VCD_OMAF_BEGIN

class MPDElement: public OmafElementBase
{
public:

    //!
    //! \brief Constructor
    //!
    MPDElement();

    //!
    //! \brief Destructor
    //!
    virtual ~MPDElement();

    //!
    //! \brief    Set function for m_xmlns_omaf member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_xmlns_omaf
    //!           m_xmlns_omaf member in class
    //! \param    [in] XmlnsOmaf
    //!           m_xmlns_omaf name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_xmlns_omaf, XmlnsOmaf);

    //!
    //! \brief    Set function for m_xmlns_xsi member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_xmlns_xsi
    //!           m_xmlns_xsi member in class
    //! \param    [in] XmlnsXsi
    //!           m_xmlns_xsi name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_xmlns_xsi, XmlnsXsi);

    //!
    //! \brief    Set function for m_xmlns member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_xmlns
    //!           m_xmlns member in class
    //! \param    [in] Xmlns
    //!           m_xmlns member name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_xmlns, Xmlns);

    //!
    //! \brief    Set function for m_xmlns_xlink member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_xmlns_xlink
    //!           m_xmlns_xlink member in class
    //! \param    [in] XmlnsXlink
    //!           m_xmlns_xlink member name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_xmlns_xlink, XmlnsXlink);

    //!
    //! \brief    Set function for m_xsi_schemaLocation member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_xsi_schemaLocation
    //!           m_xsi_schemaLocation member in class
    //! \param    [in] XsiSchemaLocation
    //!           m_xsi_schemaLocation member name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_xsi_schemaLocation, XsiSchemaLocation);

    //!
    //! \brief    Set function for m_minBufferTime member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_minBufferTime
    //!           m_minBufferTime member in class
    //! \param    [in] MinBufferTime
    //!           m_minBufferTime member name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_minBufferTime, MinBufferTime);

    //!
    //! \brief    Set function for m_maxSegmentDuration member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_maxSegmentDuration
    //!           m_maxSegmentDuration member in class
    //! \param    [in] MaxSegmentDuration
    //!           m_maxSegmentDuration member name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_maxSegmentDuration, MaxSegmentDuration);

    //!
    //! \brief    Set function for m_type member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_type
    //!           m_type member in class
    //! \param    [in] Type
    //!           m_type member name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_type, Type);

    //!
    //! \brief    Set function for m_availabilityStartTime member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_availabilityStartTime
    //!           m_availabilityStartTime member in class
    //! \param    [in] AvailabilityStartTime
    //!           m_availabilityStartTime member name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_availabilityStartTime, AvailabilityStartTime);

    //!
    //! \brief    Set function for m_availabilityEndTime member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_availabilityEndTime
    //!           m_availabilityEndTime member in class
    //! \param    [in] AvailabilityEndTime
    //!           m_availabilityEndTime member name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_availabilityEndTime, AvailabilityEndTime);

    //!
    //! \brief    Set function for m_timeShiftBufferDepth member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_timeShiftBufferDepth
    //!           m_timeShiftBufferDepth member in class
    //! \param    [in] TimeShiftBufferDepth
    //!           m_timeShiftBufferDepth member name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_timeShiftBufferDepth, TimeShiftBufferDepth);

    //!
    //! \brief    Set function for m_minimumUpdatePeriod member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_minimumUpdatePeriod
    //!           m_minimumUpdatePeriod member in class
    //! \param    [in] MinimumUpdatePeriod
    //!           m_minimumUpdatePeriodmember name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_minimumUpdatePeriod, MinimumUpdatePeriod);

    //!
    //! \brief    Set function for m_publishTime member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_publishTime
    //!           m_publishTime member in class
    //! \param    [in] PublishTime
    //!           m_publishTime member name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_publishTime, PublishTime);

    //!
    //! \brief    Set function for m_mediaPresentationDuration member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_mediaPresentationDuration
    //!           m_mediaPresentationDuration member in class
    //! \param    [in] MediaPresentationDuration
    //!           m_mediaPresentationDuration member name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_mediaPresentationDuration, MediaPresentationDuration);

    //!
    //! \brief    Set function for m_suggestedPresentationDelay member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_suggestedPresentationDelay
    //!           m_suggestedPresentationDelay member in class
    //! \param    [in] SuggestedPresentationDelay
    //!           m_suggestedPresentationDelay member name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_suggestedPresentationDelay, SuggestedPresentationDelay);

    //!
    //! \brief    Add an instance EssentialProperty element
    //!
    //! \param    [in] essentialProperty
    //!           An Instance of EssentialProperty element class
    //!
    //! \return   void
    //!
    void AddEssentialProperty(EssentialPropertyElement* essentialProperty);

    //!
    //! \brief    Add an instance of BaseUrl element
    //!
    //! \param    [in] baseUrl
    //!           An Instance of BaseUrl element class
    //!
    //! \return   void
    //!
    void AddBaseUrl(BaseUrlElement* baseUrl);

    //!
    //! \brief    Add an instance of Period element
    //!
    //! \param    [in] period
    //!           An Instance of Period element class
    //!
    //! \return   void
    //!
    void AddPeriod(PeriodElement* period);

    //!
    //! \brief    Add an string item of Period attribute
    //!
    //! \param    [in] period
    //!           A string item of Period vector
    //!
    //! \return   void
    //!
    void AddProfile(string profile);

    //!
    //! \brief    Get Projection Format
    //!
    //! \return   ProjectionFormat
    //!           Projection Format
    //!
    ProjectionFormat GetProjectionFormat();

    //!
    //! \brief    Get all EssentialProperty elements
    //!
    //! \return   vector<EssentialPropertyElement*>
    //!           vector of EssentialProperty Element
    //!
    vector<EssentialPropertyElement*> GetEssentialProperties() {return m_essentialProperties;}

    //!
    //! \brief    Get all BaseUrl elements
    //!
    //! \return   vector<BaseUrlElement*>
    //!           vector of BaseUrl Element
    //!
    vector<BaseUrlElement*>           GetBaseUrls() {return m_baseUrls;}

    //!
    //! \brief    Get all Period elements
    //!
    //! \return   vector<PeriodElement*>
    //!           vector of Period Element
    //!
    vector<PeriodElement*>            GetPeriods() {return m_periods;}

    //!
    //! \brief    Get all items in vector m_profiles
    //!
    //! \return   vector<string>
    //!           vector of string
    //!
    vector<string>                    GetProfiles() {return m_profiles;}

private:

    string                            m_xmlns_omaf;                 //!< the xmlns:omaf attribute
    string                            m_xmlns_xsi;                  //!< the xmlns:xsi attribute
    string                            m_xmlns;                      //!< the xmlns attribute
    string                            m_xmlns_xlink;                //!< the xmlns:xlink attribute
    string                            m_xsi_schemaLocation;         //!< the xsi:schemaLocation attribute
    string                            m_minBufferTime;              //!< the minBufferTime attribute
    string                            m_maxSegmentDuration;         //!< the maxSegmentDuration attribute
    vector<string>                    m_profiles;                   //!< the profiles attribute
    string                            m_type;                       //!< the type attribute
    string                            m_availabilityStartTime;      //!< the availabilityStartTime attribute
    string                            m_availabilityEndTime;
    string                            m_timeShiftBufferDepth;       //!< the timeShiftBufferDepth attribute
    string                            m_minimumUpdatePeriod;        //!< the minimumUpdatePeriod attribute
    string                            m_publishTime;                //!< the publishTime attribute
    string                            m_mediaPresentationDuration;  //!< the mediaPresentationDuration attribute
    string                            m_suggestedPresentationDelay; //!< the suggestedPresentationDelay attribute

    vector<EssentialPropertyElement*> m_essentialProperties;        //!< the EssentialProperty child elements
    vector<BaseUrlElement*>           m_baseUrls;                   //!< the BaseUrl child elements
    vector<PeriodElement*>            m_periods;                    //!< the Period child elements
};

VCD_OMAF_END

#endif //MPDELEMENT_H
