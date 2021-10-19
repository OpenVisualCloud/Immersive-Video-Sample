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
//! \file:   AdaptationSetElement.h
//! \brief:  AdaptationSet element class
//!

#ifndef ADAPTATIONSETELEMENT_H
#define ADAPTATIONSETELEMENT_H

#include "OmafElementBase.h"
#include "DescriptorElement.h"
#include "ViewportElement.h"
#include "EssentialPropertyElement.h"
#include "RepresentationElement.h"
#include "SupplementalPropertyElement.h"

VCD_OMAF_BEGIN

class AdaptationSetElement: public OmafElementBase
{
public:

    //!
    //! \brief Constructor
    //!
    AdaptationSetElement();

    //!
    //! \brief Destructor
    //!
    virtual ~AdaptationSetElement();

    //!
    //! \brief    Set function for m_id member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_id
    //!           m_id member in class
    //! \param    [in] Id
    //!           m_id name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_id, Id);

    //!
    //! \brief    Set function for m_mimeType member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_mimeType
    //!           m_mimeType member in class
    //! \param    [in] MimeType
    //!           m_mimeType name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_mimeType, MimeType);

    //!
    //! \brief    Set function for m_maxWidth member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_maxWidth
    //!           m_maxWidth member in class
    //! \param    [in] MaxWidth
    //!           m_maxWidth name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_maxWidth, MaxWidth);

    //!
    //! \brief    Set function for m_maxHeight member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_maxHeight
    //!           m_maxHeight member in class
    //! \param    [in] MaxHeight
    //!           m_maxHeight name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_maxHeight, MaxHeight);

    //!
    //! \brief    Set function for m_gopSize member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_gopSize
    //!           m_gopSize member in class
    //! \param    [in] GopSize
    //!           m_gopSize name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_gopSize, GopSize);

    //!
    //! \brief    Set function for m_maxFrameRate member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_maxFrameRate
    //!           m_maxFrameRate member in class
    //! \param    [in] MaxFrameRate
    //!           m_maxFrameRate name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_maxFrameRate, MaxFrameRate);

    MEMBER_SET_AND_GET_FUNC(string, m_audioSamplingRate, AudioSamplingRate);
    //!
    //! \brief    Set function for m_segmentAlignment member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_segmentAlignment
    //!           m_segmentAlignment member in class
    //! \param    [in] SegmentAlignment
    //!           m_segmentAlignment name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_segmentAlignment, SegmentAlignment);

    //!
    //! \brief    Set function for m_subsegmentAlignment member
    //!
    //! \param    [in] string
    //!           value to set
    //! \param    [in] m_subsegmentAlignment
    //!           m_subsegmentAlignment member in class
    //! \param    [in] SubsegmentAlignment
    //!           m_subsegmentAlignment name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(string, m_subsegmentAlignment, SubsegmentAlignment);

    //!
    //! \brief    Set value for member m_codecs
    //!
    //! \param    [in] v
    //!           A string value to set
    //!
    //! \return   void
    void SetCodecs(string v) { SplitString(v, m_codecs, ","); }

    //!
    //! \brief    Get value of member m_codecs
    //!
    //! \return   vector<string>
    //!           A vector of string
    vector<string> GetCodecs() { return m_codecs; }

    //!
    //! \brief    Add an instance of Viewport element
    //!
    //! \param    [in] viewport
    //!           An Instance of Viewport element class
    //!
    //! \return   void
    void AddViewport(ViewportElement* viewport);

    //!
    //! \brief    Add an instance of EssentialProperty element
    //!
    //! \param    [in] essentialProperty
    //!           An Instance of EssentialProperty element class
    //!
    //! \return   void
    void AddEssentialProperty(EssentialPropertyElement* essentialProperty);

    //!
    //! \brief    Add an instance of SupplementalProperty element
    //!
    //! \param    [in] supplementalProperty
    //!           An Instance of SupplementalProperty element class
    //!
    //! \return   void
    void AddSupplementalProperty(SupplementalPropertyElement* supplementalProperty);

    //!
    //! \brief    Add an instance of Representation element
    //!
    //! \param    [in] representation
    //!           An Instance of Representation element class
    //!
    //! \return   void
    void AddRepresentation(RepresentationElement* representation);

    //!
    //! \brief    Get content converage from member m_supplementalProperties
    //!
    //! \return   PreselValue*
    //!           A pointer of PreselValue class
    PreselValue* GetPreselection();

    //!
    //! \brief    Get content converage from member m_supplementalProperties
    //!
    //! \return   SphereQuality*
    //!           A pointer of SphereQuality class
    SphereQuality* GetSphereQuality();

    //!
    //! \brief    Get 2D quality information from member m_supplementalProperties
    //!
    //! \return   map<int32_t, TwoDQualityInfo>
    //!           map of <qualityRanking, TwoDQualityInfo> for input planar
    //!           video sources
    map<int32_t, TwoDQualityInfo> GetTwoDQuality();

    //!
    //! \brief    Get content converage from member m_supplementalProperties
    //!
    //! \return   OmafSrd*
    //!           A pointer of OmafSrd class
    OmafSrd* GetSRD();

    //!
    //! \brief    Get value of member m_essentialProperties
    //!
    //! \return   ProjectionFormat
    //!           An instance of ProjectionFormat class
    ProjectionFormat GetProjectionFormat();

    //!
    //! \brief    Get rwpk type from member m_essentialProperties
    //!
    //! \return   RwpkTyp
    //!           An instance of RwpkTyp class
    RwpkType GetRwpkType();

    //!
    //! \brief    Get content converage from member m_supplementalProperties
    //!
    //! \return   ContentCoverage*
    //!           A pointer of ContentCoverage class
    ContentCoverage* GetContentCoverage();

    //!
    //! \brief    Get all Viewport elements
    //!
    //! \return   vector<ViewportElement*>
    //!           vector of Viewport Element
    //!
    vector<ViewportElement*>             GetViewports() {return m_viewport;}

    //!
    //! \brief    Get all EssentialProperty elements
    //!
    //! \return   vector<EssentialPropertyElement*>
    //!           vector of EssentialProperty Element
    //!
    vector<EssentialPropertyElement*>    GetEssentialProperties() {return m_essentialProperties;}

    //!
    //! \brief    Get all SupplementalProperty elements
    //!
    //! \return   vector<SupplementalPropertyElement*>
    //!           vector of SupplementalProperty Element
    //!
    vector<SupplementalPropertyElement*> GetSupplementalProperties() {return m_supplementalProperties;}

    //!
    //! \brief    Get all Representation elements
    //!
    //! \return   vector<RepresentationElement*>
    //!           vector of Representation Element
    //!
    vector<RepresentationElement*>       GetRepresentations() {return m_representations;}

private:

    string                               m_id;                     //!< the id attribute
    string                               m_mimeType;               //!< the mimeType attribute
    vector<string>                       m_codecs;                 //!< the codecs attribute
    string                               m_maxWidth;               //!< the maxWidth attribute
    string                               m_maxHeight;              //!< the maxHeight attribute
    string                               m_maxFrameRate;           //!< the maxFramerate attribute
    string                               m_audioSamplingRate;
    string                               m_gopSize;                //!< the gop size attribute

    string                               m_segmentAlignment;       //!< the segmentAlignment attribute
    string                               m_subsegmentAlignment;    //!< the subsegmentAlignment attribute
    vector<ViewportElement*>             m_viewport;               //!< the Viewport elements
    vector<EssentialPropertyElement*>    m_essentialProperties;    //!< the EssentialProperties elements
    vector<SupplementalPropertyElement*> m_supplementalProperties; //!< the SupplementalProperty elements
    vector<RepresentationElement*>       m_representations;        //!< the Representations elements
};

VCD_OMAF_END

#endif //ADAPTATIONSETELEMENT_H
