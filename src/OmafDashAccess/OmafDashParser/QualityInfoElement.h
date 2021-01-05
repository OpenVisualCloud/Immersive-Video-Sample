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
//! \file:   QualityInfoElement.h
//! \brief:  omaf:qualityInfo element class
//!

#ifndef QUALITYINFOELEMENT_H
#define QUALITYINFOELEMENT_H
#include "OmafElementBase.h"

VCD_OMAF_BEGIN

class QualityInfoElement : public OmafElementBase {
 public:
  //!
  //! \brief Constructor
  //!
  QualityInfoElement();

  //!
  //! \brief Destructor
  //!
  virtual ~QualityInfoElement();

  //!
  //! \brief    Set function for m_quality_ranking member
  //!
  //! \param    [in] int32_t
  //!           value to set
  //! \param    [in] m_quality_ranking
  //!           m_quality_ranking member in class
  //! \param    [in] QualityRanking
  //!           m_quality_ranking name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(int32_t, m_quality_ranking, QualityRanking);

  //!
  //! \brief    Set function for m_orig_width member
  //!
  //! \param    [in] int32_t
  //!           value to set
  //! \param    [in] m_orig_width
  //!           m_orig_width member in class
  //! \param    [in] OrigWidth
  //!           m_orig_width name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(int32_t, m_orig_width, OrigWidth);

  //!
  //! \brief    Set function for m_orig_height member
  //!
  //! \param    [in] int32_t
  //!           value to set
  //! \param    [in] m_orig_height
  //!           m_orig_height member in class
  //! \param    [in] OrigHeight
  //!           m_orig_height name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(int32_t, m_orig_height, OrigHeight);

  //!
  //! \brief    Set function for m_centre_azimuth member
  //!
  //! \param    [in] int32_t
  //!           value to set
  //! \param    [in] m_centre_azimuth
  //!           m_centre_azimuth member in class
  //! \param    [in] CentreAzimuth
  //!           m_centre_azimuth name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(int32_t, m_centre_azimuth, CentreAzimuth);

  //!
  //! \brief    Set function for m_centre_elevation member
  //!
  //! \param    [in] int32_t
  //!           value to set
  //! \param    [in] m_centre_elevation
  //!           m_centre_elevation member in class
  //! \param    [in] CentreElevation
  //!           m_centre_elevation name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(int32_t, m_centre_elevation, CentreElevation);

  //!
  //! \brief    Set function for m_centre_tilt member
  //!
  //! \param    [in] int32_t
  //!           value to set
  //! \param    [in] m_centre_tilt
  //!           m_centre_tilt member in class
  //! \param    [in] CentreTilt
  //!           m_centre_tilt name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(int32_t, m_centre_tilt, CentreTilt);

  //!
  //! \brief    Set function for m_azimuth_range member
  //!
  //! \param    [in] int32_t
  //!           value to set
  //! \param    [in] m_azimuth_range
  //!           m_azimuth_range member in class
  //! \param    [in] AzimuthRange
  //!           m_azimuth_range name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(int32_t, m_azimuth_range, AzimuthRange);

  //!
  //! \brief    Set function for m_elevation_range member
  //!
  //! \param    [in] int32_t
  //!           value to set
  //! \param    [in] m_elevation_range
  //!           m_elevation_range member in class
  //! \param    [in] ElevationRange
  //!           m_elevation_range name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(int32_t, m_elevation_range, ElevationRange);

  //!
  //! \brief    Set and Get function for m_region_width member
  //!
  //! \param    [in] int32_t
  //!           value to set
  //! \param    [in] m_region_width
  //!           m_region_width member in class
  //! \param    [in] RegionWidth
  //!           m_region_width name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(int32_t, m_region_width, RegionWidth);

  //!
  //! \brief    Set and Get function for m_region_height member
  //!
  //! \param    [in] int32_t
  //!           value to set
  //! \param    [in] m_region_height
  //!           m_region_height member in class
  //! \param    [in] RegionHeight
  //!           m_region_height name in class
  //!
  //! \return   void
  //!
  MEMBER_SET_AND_GET_FUNC(int32_t, m_region_height, RegionHeight);

 private:
  int32_t m_quality_ranking;   //!< the quality_ranking attribute
  int32_t m_orig_width;        //!< the orig_width attribute
  int32_t m_orig_height;       //!< the orig_height attribute
  int32_t m_centre_azimuth;    //!< the centre_azimuth attribute
  int32_t m_centre_elevation;  //!< the centre_elevation attribute
  int32_t m_centre_tilt;       //!< the centre_tilt attribute
  int32_t m_azimuth_range;     //!< the azimuth_range attribute
  int32_t m_elevation_range;   //!< the elevation_range attribute
  int32_t m_region_width;      //!< the region_width attribute
  int32_t m_region_height;     //!< the region_height attribute
};

VCD_OMAF_END;

#endif  // QUALITYINFOELEMENT_H
