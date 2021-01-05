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
//! \file:   TwoDRegionQualityElement.h
//! \brief:  omaf:twoDRegionQuality element class
//!

#ifndef TWODREGIONQUALITYELEMENT_H
#define TWODREGIONQUALITYELEMENT_H
#include "QualityInfoElement.h"

VCD_OMAF_BEGIN

class TwoDRegionQualityElement: public OmafElementBase
{
public:

    //!
    //! \brief Constructor
    //!
    TwoDRegionQualityElement();

    //!
    //! \brief Destructor
    //!
    virtual ~TwoDRegionQualityElement();

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
    //! \return   map<int32_t, PlanarQuality>
    //!           a map of all planar qality info
    //!
    //vector<SrqrQualityInfo> GetQualityInfos();
    map<int32_t, TwoDQualityInfo> GetTwoDQualityInfos();

private:
    TwoDRegionQualityElement& operator=(const TwoDRegionQualityElement& other) { return *this; };
    TwoDRegionQualityElement(const TwoDRegionQualityElement& other) { /* do not create copies */ };

private:

    vector<QualityInfoElement*>      m_qualityInfo;                //!< the omaf:qualityInfo child elements

    map<int32_t, TwoDQualityInfo>    m_twoDQualityInfos;           //!< the map of <qualityRanking, TwoDQualityInfo> for all planar video sources
};

VCD_OMAF_END;

#endif //TWODREGIONQUALITYELEMENT_H
