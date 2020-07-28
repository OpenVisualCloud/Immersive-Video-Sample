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
//! \file:   SphRegionQualityElement.h
//! \brief:  omaf:sphRegionQuality element class
//!

#ifndef SPHREGIONQUALITYELEMENT_H
#define SPHREGIONQUALITYELEMENT_H
#include "QualityInfoElement.h"

VCD_OMAF_BEGIN

class SphRegionQualityElement: public OmafElementBase
{
public:

    //!
    //! \brief Constructor
    //!
    SphRegionQualityElement();

    //!
    //! \brief Destructor
    //!
    virtual ~SphRegionQualityElement();

    //!
    //! \brief    Set function for m_shape_type member
    //!
    //! \param    [in] int32_t
    //!           value to set
    //! \param    [in] m_shape_type
    //!           m_shape_type member in class
    //! \param    [in] ShapeType
    //!           m_shape_type name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(int32_t, m_shape_type, ShapeType);

    //!
    //! \brief    Set function for m_remaining_area_flag member
    //!
    //! \param    [in] int32_t
    //!           value to set
    //! \param    [in] m_remaining_area_flag
    //!           m_remaining_area_flag member in class
    //! \param    [in] RemainingAreaFlag
    //!           m_remaining_area_flag name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(int32_t, m_remaining_area_flag, RemainingAreaFlag);

    //!
    //! \brief    Set function for m_quality_ranking_local_flag member
    //!
    //! \param    [in] int32_t
    //!           value to set
    //! \param    [in] m_quality_ranking_local_flag
    //!           m_quality_ranking_local_flag member in class
    //! \param    [in] QualityRankingLocalFlag
    //!           m_quality_ranking_local_flag name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(int32_t, m_quality_ranking_local_flag, QualityRankingLocalFlag);

    //!
    //! \brief    Set function for m_quality_type member
    //!
    //! \param    [in] int32_t
    //!           value to set
    //! \param    [in] m_quality_type
    //!           m_quality_type member in class
    //! \param    [in] QualityType
    //!           m_quality_type name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(int32_t, m_quality_type, QualityType);

    //!
    //! \brief    Set function for m_defaultIdc member
    //!
    //! \param    [in] int32_t
    //!           value to set
    //! \param    [in] m_defaultIdc
    //!           m_defaultIdc member in class
    //! \param    [in] DefaultIdc
    //!           m_defaultIdc name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(int32_t, m_defaultIdc, DefaultIdc);

    //!
    //! \brief    Set function for m_viewIdcPresence member
    //!
    //! \param    [in] int32_t
    //!           value to set
    //! \param    [in] m_viewIdcPresence
    //!           m_viewIdcPresence member in class
    //! \param    [in] ViewIdcPresence
    //!           m_viewIdcPresence name in class
    //!
    //! \return   void
    //!
    MEMBER_SET_AND_GET_FUNC(int32_t, m_viewIdcPresence, ViewIdcPresence);

    //!
    //! \brief    Add an pointer of QualityInfo element
    //!
    //! \param    [in] qualityInfo
    //!           An pointer of QualityInfo element class
    //!
    //! \return   ODStatus
    //!           OD_STATUS_SUCCESS if success, else fail reason
    //!
    ODStatus AddQualityInfo(QualityInfoElement* qualityInfo);

    //!
    //! \brief    Get Quality Information
    //!
    //! \return   vector<SrqrQualityInfo>
    //!           a vector of SrqrQualityInfo class
    //!
    vector<SrqrQualityInfo> GetQualityInfos();

    //!
    //! \brief    Get member m_contentCoverage
    //!
    //! \return   ContentCoverage*
    //!           a pointer of ContentCoverage class
    //!
    ContentCoverage* GetContentCoverage();

private:
    SphRegionQualityElement& operator=(const SphRegionQualityElement& other) { return *this; };
    SphRegionQualityElement(const SphRegionQualityElement& other) { /* do not create copies */ };

private:

    int32_t                          m_shape_type;                 //!< the shape_type attribute
    int32_t                          m_remaining_area_flag;        //!< the remaining_area_flag attribute
    int32_t                          m_quality_ranking_local_flag; //!< the quality_ranking_local_flag attribute
    int32_t                          m_quality_type;               //!< the quality_type attribute
    int32_t                          m_defaultIdc;                 //!< the quality_ranking_local_flag attribute
    int32_t                          m_viewIdcPresence;            //!< the quality_ranking_local_flag attribute
    vector<QualityInfoElement*>      m_qualityInfo;                //!< the omaf:qualityInfo child elements
    vector<SrqrQualityInfo>          m_srqrQualityInfo;            //!< the srqrQualityInfo child elements
    ContentCoverage                  *m_contentCoverage;           //!< the ContentCoverage child elements
};

VCD_OMAF_END;

#endif //SPHREGIONQUALITYELEMENT_H
