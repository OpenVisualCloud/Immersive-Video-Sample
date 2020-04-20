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
//! \file:   SphereRegionSampEntryAtom.h
//! \brief:  OMAF SphereRegionSampleEntryAtom class.
//! \detail: Sphere Region Sample Entry Atom
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _SPHEREREGIONSAMPLEENTRYATOM_H_
#define _SPHEREREGIONSAMPLEENTRYATOM_H_

#include "Stream.h"
#include "CommonTypes.h"
#include "FormAllocator.h"
#include "MetaDataSampEntryAtom.h"
#include "FullAtom.h"

VCD_MP4_BEGIN

class SphereRegionConfigAtom : public FullAtom
{
public:
    enum class ShapeMode : std::uint8_t     //!< Shape Mode
    {
        FourGreatCircles                 = 0,
        TwoAzimuthAndTwoElevationCircles = 1
    };

    //!
    //! \brief Constructor
    //!
    SphereRegionConfigAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~SphereRegionConfigAtom() = default;

    //!
    //! \brief    Set Shape Mode
    //!
    //! \param    [in] ShapeMode
    //!           Shape Mode
    //!
    //! \return   void
    //!
    void SetShapeMode(ShapeMode shapeType);

    //!
    //! \brief    Get Shape Mode
    //!
    //! \return   ShapeMode
    //!           Shape Mode
    //!
    ShapeMode GetShapeMode();

    //!
    //! \brief    Set Dynamic Range Flag
    //!
    //! \param    [in] bool
    //!           shape Type
    //!
    //! \return   void
    //!
    void SetDynamicRangeFlag(bool shapeType);

    //!
    //! \brief    Get Dynamic Range Flag
    //!
    //! \return   bool
    //!           Dynamic Range Flag
    //!
    bool GetDynamicRangeFlag();

    //!
    //! \brief    Set Static Azimuth Range
    //!
    //! \param    [in] std::uint32_t
    //!           range
    //!
    //! \return   void
    //!
    void SetStaticAzimuthRange(std::uint32_t range);

    //!
    //! \brief    Get Static Azimuth Range
    //!
    //! \return   std::uint32_t
    //!           range
    //!
    std::uint32_t GetStaticAzimuthRange();

    //!
    //! \brief    Set Static Elevation Range
    //!
    //! \param    [in] std::uint32_t
    //!           range
    //!
    //! \return   void
    //!
    void SetStaticElevationRange(std::uint32_t range);

    //!
    //! \brief    Get Static Elevation Range
    //!
    //! \return   std::uint32_t
    //!           range
    //!
    std::uint32_t GetStaticElevationRange();

    //!
    //! \brief    Write atom information to stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void ToStream(Stream& str);

    //!
    //! \brief    Parse atom information from stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void FromStream(Stream& str);


private:
    ShapeMode m_shapeType;                  //!< Shape Mode
    bool m_dynamicRangeFlag;                //!< dynamic Range Flag
    std::uint32_t m_staticAzimuthRange;     //!< static Azimuth Range
    std::uint32_t m_staticElevationRange;   //!< static Elevation Range
    std::uint8_t m_numRegions;              //!< num of Regions
};

class SphereRegionSampleEntryAtom : public MetaDataSampleEntryAtom
{
public:
    struct SphereRegionSample   //!< Sphere Region Sample
    {
        std::vector<SphereRegion> regions;
    };

    //!
    //! \brief Constructor
    //!
    SphereRegionSampleEntryAtom(FourCCInt codingname);

    //!
    //! \brief Destructor
    //!
    virtual ~SphereRegionSampleEntryAtom() = default;

    //!
    //! \brief    Get Sphere Region Config Atom
    //!
    //! \return   SphereRegionConfigAtom&
    //!           Sphere Region Config Atom
    //!
    SphereRegionConfigAtom& GetSphereRegionConfig();

    //!
    //! \brief    Write atom information to stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void ToStream(Stream& str);

    //!
    //! \brief    Parse atom information from stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void FromStream(Stream& str);


private:
    SphereRegionConfigAtom m_sphereRegionConfig;    //!< Sphere Region Config Atom
};

VCD_MP4_END;
#endif  /* _SPHEREREGIONSAMPLEENTRYATOM_H_ */
