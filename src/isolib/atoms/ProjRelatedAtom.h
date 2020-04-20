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
//! \file:   ProjRelatedAtom.h
//! \brief:  ProjRelatedAtom class
//! \detail: defines Projection Related Atoms
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _PROJECTIONFORMATATOM_H_
#define _PROJECTIONFORMATATOM_H_

#include <cstdint>
#include "FullAtom.h"
#include <vector>
#include "CommonTypes.h"

VCD_MP4_BEGIN

using namespace std;

class ProjectionFormatAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    ProjectionFormatAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~ProjectionFormatAtom() = default;

    enum class ProjectFormat : uint8_t  //!< Projection Format
    {
        ERP = 0,
        CUBEMAP
    };

    //!
    //! \brief    Get Projection Format
    //!
    //! \return   ProjectFormat
    //!           Projection Format
    //!
    ProjectFormat GetProjectFormat() const;

    //!
    //! \brief    Set Projection Format
    //!
    //! \param    [in] ProjectFormat
    //!           Projection Format value
    //!
    //! \return   void
    //!
    void SetProjectFormat(ProjectFormat);

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
    uint8_t m_projectionFormat;  //!< projection format
};

class CoverageInformationAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    CoverageInformationAtom();
    CoverageInformationAtom(const CoverageInformationAtom&);

    CoverageInformationAtom& operator=(const CoverageInformationAtom&) = default;

    //!
    //! \brief Destructor
    //!
    virtual ~CoverageInformationAtom() = default;

    enum class CoverageShapeMode : uint8_t  //!< Coverage Shape Mode
    {
        FOUR_GCIRCLES = 0,
        TWO_AZIMUTH_ELEVATION_CIRCLES
    };

    struct CoverageSphereRegion //!< Coverage Sphere Region
    {
        ViewMode viewIdc;
        SphereRegion region;
    };

    //!
    //! \brief    Set and Get function for m_coverageShapeMode member
    //!
    //! \param    [in] CoverageShapeMode
    //!           value to set
    //! \param    [in] m_coverageShapeMode
    //!           m_coverageShapeMode member in class
    //! \param    [in] CoverageShapeMode
    //!           m_coverageShapeMode name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(CoverageShapeMode, m_coverageShapeMode, CoverageShapeMode, const);

    //!
    //! \brief    Set and Get function for m_viewIdcPresenceFlag member
    //!
    //! \param    [in] bool
    //!           value to set
    //! \param    [in] m_viewIdcPresenceFlag
    //!           m_viewIdcPresenceFlag member in class
    //! \param    [in] ViewIdcPresenceFlag
    //!           m_viewIdcPresenceFlag name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(bool, m_viewIdcPresenceFlag, ViewIdcPresenceFlag, const);

    //!
    //! \brief    Set and Get function for m_defaultViewIdc member
    //!
    //! \param    [in] ViewMode
    //!           value to set
    //! \param    [in] m_defaultViewIdc
    //!           m_defaultViewIdc member in class
    //! \param    [in] DefaultViewIdc
    //!           m_defaultViewIdc name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(ViewMode, m_defaultViewIdc, DefaultViewIdc, const);

    //!
    //! \brief    Get SphereRegions
    //!
    //! \return   vector<CoverageSphereRegion*>
    //!           SphereRegions
    //!
    vector<CoverageSphereRegion*> GetSphereRegions() const;

    //!
    //! \brief    Add Sphere Region
    //!
    //! \param    [in] UniquePtr<CoverageSphereRegion>
    //!           CoverageSphereRegion pointer
    //!
    //! \return   void
    //!
    void AddSphereRegion(UniquePtr<CoverageSphereRegion>);

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

    //!
    //! \brief    dump information
    //!
    void Dump() const;

private:
    CoverageShapeMode m_coverageShapeMode;                  //!< Coverage Shape Mode
    bool m_viewIdcPresenceFlag;                             //!< view Idc Presence Flag
    ViewMode m_defaultViewIdc;                              //!< default View Idc
    vector<UniquePtr<CoverageSphereRegion>> m_sphereRegions;//!< sphere Regions
};

class RegionWisePackingAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    RegionWisePackingAtom();
    RegionWisePackingAtom(const RegionWisePackingAtom&);

    RegionWisePackingAtom& operator=(const RegionWisePackingAtom&) = default;

    //!
    //! \brief Destructor
    //!
    virtual ~RegionWisePackingAtom() = default;

    struct RectangularRegionWisePacking //!< Rectangular Region Wise Packing
    {
        //projection parameters
        uint32_t projRegWidth;
        uint32_t projRegHeight;
        uint32_t projRegTop;
        uint32_t projRegLeft;
        //transform type
        uint8_t transformType;
        //packed parameters
        uint16_t packedRegWidth;
        uint16_t packedRegHeight;
        uint16_t packedRegTop;
        uint16_t packedRegLeft;
        //Flowing parameters take effect when guardBandFlag is true.
        bool    gbNotUsedForPredFlag;
        uint8_t leftGbWidth;
        uint8_t rightGbWidth;
        uint8_t topGbHeight;
        uint8_t bottomGbHeight;
        uint8_t gbType0;
        uint8_t gbType1;
        uint8_t gbType2;
        uint8_t gbType3;
    };

    //!
    //! \brief    Set and Get function for m_constituentPictureMatchingFlag member
    //!
    //! \param    [in] bool
    //!           value to set
    //! \param    [in] m_constituentPictureMatchingFlag
    //!           m_constituentPictureMatchingFlag member in class
    //! \param    [in] ConstituentPictureMatchingFlag
    //!           m_constituentPictureMatchingFlag name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(bool, m_constituentPictureMatchingFlag, ConstituentPictureMatchingFlag, const);

    //!
    //! \brief    Set and Get function for m_projPictureWidth member
    //!
    //! \param    [in] uint32_t
    //!           value to set
    //! \param    [in] m_projPictureWidth
    //!           m_projPictureWidth member in class
    //! \param    [in] ProjPictureWidth
    //!           m_projPictureWidth name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(uint32_t, m_projPictureWidth, ProjPictureWidth, const);

    //!
    //! \brief    Set and Get function for m_projPictureHeight member
    //!
    //! \param    [in] uint32_t
    //!           value to set
    //! \param    [in] m_projPictureHeight
    //!           m_projPictureHeight member in class
    //! \param    [in] ProjPictureHeight
    //!           m_projPictureHeight name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(uint32_t, m_projPictureHeight, ProjPictureHeight, const);

    //!
    //! \brief    Set and Get function for m_packedPictureWidth member
    //!
    //! \param    [in] uint16_t
    //!           value to set
    //! \param    [in] m_packedPictureWidth
    //!           m_packedPictureWidth member in class
    //! \param    [in] PackedPictureWidth
    //!           m_packedPictureWidth name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(uint16_t, m_packedPictureWidth, PackedPictureWidth, const);

    //!
    //! \brief    Set and Get function for m_packedPictureHeight member
    //!
    //! \param    [in] uint16_t
    //!           value to set
    //! \param    [in] m_packedPictureHeight
    //!           m_packedPictureHeight member in class
    //! \param    [in] PackedPictureHeight
    //!           m_packedPictureHeight name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(uint16_t, m_packedPictureHeight, PackedPictureHeight, const);

    enum class PackingType : uint8_t    //!< packing type
    {
        RECTANGULAR = 0
    };

    struct Region   //!< region information
    {
        Region()
        {
            guardBandFlag = 0;
            packingType = PackingType::RECTANGULAR;
        };
        Region(const Region&);
        Region& operator=(const Region&) = default;
        bool guardBandFlag;
        PackingType packingType;
        UniquePtr<RectangularRegionWisePacking> rectangularPacking;
    };

    //!
    //! \brief    Get regions
    //!
    //! \return   vector<RegionWisePackingAtom::Region*>
    //!           regions
    //!
    vector<RegionWisePackingAtom::Region*> GetRegions() const;

    //!
    //! \brief    add regions
    //!
    //! \param    [in] UniquePtr<RegionWisePackingAtom::Region>
    //!           regions value
    //!
    //! \return   void
    //!
    void AddRegion(UniquePtr<RegionWisePackingAtom::Region>);

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

    //!
    //! \brief    dump information
    //!
    void Dump() const;

private:
    bool m_constituentPictureMatchingFlag;                      //!< constituent Picture Matching Flag
    uint32_t m_projPictureWidth;                                //!< projection Picture Width
    uint32_t m_projPictureHeight;                               //!< projection Picture Height
    uint16_t m_packedPictureWidth;                              //!< packed Picture Width
    uint16_t m_packedPictureHeight;                             //!< packed Picture Height
    vector<UniquePtr<RegionWisePackingAtom::Region>> m_regions; //!< regions
};

class RotationAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    RotationAtom();
    RotationAtom(const RotationAtom&);

    RotationAtom& operator=(const RotationAtom&) = default;
    //!
    //! \brief Destructor
    //!
    virtual ~RotationAtom() = default;

    struct Rotation //!< rotation
    {
        int32_t yaw;
        int32_t pitch;
        int32_t roll;
    };

    //!
    //! \brief    Set and Get function for m_rotation member
    //!
    //! \param    [in] Rotation
    //!           value to set
    //! \param    [in] m_rotation
    //!           m_rotation member in class
    //! \param    [in] Rotation
    //!           m_rotation name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(Rotation, m_rotation, Rotation, const);

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
    Rotation m_rotation;    //!< rotation
};

VCD_MP4_END;
#endif /* _PROJECTIONFORMATATOM_H_ */
