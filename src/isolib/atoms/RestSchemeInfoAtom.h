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
//! \file:   RestSchemeInfoAtom.h
//! \brief:  RestSchemeInfoAtom Atom class.
//! \detail: Restrict Scheme Information Atom
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _RESTRICTEDSCHEMEINFOATOM_H_
#define _RESTRICTEDSCHEMEINFOATOM_H_

#include <cstdint>
#include <functional>
#include "Atom.h"
#include "FormAllocator.h"
#include "FullAtom.h"
#include "ProjRelatedAtom.h"

VCD_MP4_BEGIN

class OriginalFormatAtom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    OriginalFormatAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~OriginalFormatAtom() = default;

    //!
    //! \brief    Set and Get function for m_originalFormat member
    //!
    //! \param    [in] FourCCInt
    //!           value to set
    //! \param    [in] m_originalFormat
    //!           m_originalFormat member in class
    //! \param    [in] OriginalFormat
    //!           m_originalFormat name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(FourCCInt, m_originalFormat, OriginalFormat, const);

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
    FourCCInt m_originalFormat; //!< original format
};

class ProjectedOmniVideoAtom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    ProjectedOmniVideoAtom();
    ProjectedOmniVideoAtom(const ProjectedOmniVideoAtom&);

    ProjectedOmniVideoAtom& operator=(const ProjectedOmniVideoAtom&) = default;

    //!
    //! \brief Destructor
    //!
    virtual ~ProjectedOmniVideoAtom() = default;

    //!
    //! \brief    Get ProjectionFormat Atom
    //!
    //! \return   ProjectionFormatAtom&
    //!           ProjectionFormat Atom
    //!
    ProjectionFormatAtom& GetProjectionFormatAtom();

    //!
    //! \brief    Get RegionWisePacking Atom
    //!
    //! \return   RegionWisePackingAtom&
    //!           RegionWisePacking Atom
    //!
    RegionWisePackingAtom& GetRegionWisePackingAtom();

    //!
    //! \brief    Set RegionWisePacking Atom
    //!
    //! \param    [in] UniquePtr<RegionWisePackingAtom>
    //!           RegionWisePacking Atom
    //!
    //! \return   void
    //!
    void SetRegionWisePackingAtom(UniquePtr<RegionWisePackingAtom>);

    //!
    //! \brief    Has RegionWisePacking Atom or not
    //!
    //! \return   bool
    //!           has or not
    //!
    bool HasRegionWisePackingAtom() const;

    //!
    //! \brief    Get CoverageInformation Atom
    //!
    //! \return   CoverageInformationAtom&
    //!           CoverageInformation Atom
    //!
    CoverageInformationAtom& GetCoverageInformationAtom();

    //!
    //! \brief    Set CoverageInformation Atom
    //!
    //! \param    [in] UniquePtr<CoverageInformationAtom>
    //!           CoverageInformation Atom
    //!
    //! \return   void
    //!
    void SetCoverageInformationAtom(UniquePtr<CoverageInformationAtom>);

    //!
    //! \brief    Has CoverageInformatio Atom or not
    //!
    //! \return   bool
    //!           has or not
    //!
    bool HasCoverageInformationAtom() const;

    //!
    //! \brief    Get Rotation Atom
    //!
    //! \return   RotationAtom&
    //!           Rotation Atom
    //!
    RotationAtom& GetRotationAtom();

    //!
    //! \brief    Set Rotation Atom
    //!
    //! \param    [in] UniquePtr<RotationAtom>
    //!           Rotation Atom
    //!
    //! \return   void
    //!
    void SetRotationAtom(UniquePtr<RotationAtom>);

    //!
    //! \brief    Has Rotation Atom or not
    //!
    //! \return   bool
    //!           has or not
    //!
    bool HasRotationAtom() const;

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
    void dump() const;

private:
    ProjectionFormatAtom m_projectionFormatAtom;                    //!< Projection Format Atom
    UniquePtr<RegionWisePackingAtom> m_regionWisePackingAtom;       //!< region Wise Packing Atom
    UniquePtr<CoverageInformationAtom> m_coverageInformationAtom;   //!< Coverage Information Atom
    UniquePtr<RotationAtom> m_rotationAtom;                         //!< Rotation Atom
};

class SchemeTypeAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    SchemeTypeAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~SchemeTypeAtom() = default;

    //!
    //! \brief    Set and Get function for m_schemeType member
    //!
    //! \param    [in] FourCCInt
    //!           value to set
    //! \param    [in] m_schemeType
    //!           m_schemeType member in class
    //! \param    [in] SchemeType
    //!           m_schemeType name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(FourCCInt, m_schemeType, SchemeType, const);

    //!
    //! \brief    Set and Get function for m_schemeVersion member
    //!
    //! \param    [in] std::uint32_t
    //!           value to set
    //! \param    [in] m_schemeVersion
    //!           m_schemeVersion member in class
    //! \param    [in] SchemeVersion
    //!           m_schemeVersion name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(std::uint32_t, m_schemeVersion, SchemeVersion, const);

    //!
    //! \brief    Set Scheme Uri
    //!
    //! \param    [in] const std::string&
    //!           Scheme Uri value
    //!
    //! \return   void
    //!
    void SetSchemeUri(const std::string& uri);

    //!
    //! \brief    Get Scheme Uri
    //!
    //! \return   const std::string&
    //!           Scheme Uri
    //!
    const std::string& GetSchemeUri() const;

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
    FourCCInt m_schemeType;     //!< scheme Type
    uint32_t m_schemeVersion;   //!< scheme Version
    std::string m_schemeUri;    //!< scheme Uri
};

class CompatibleSchemeTypeAtom : public SchemeTypeAtom
{
public:

    //!
    //! \brief Constructor
    //!
    CompatibleSchemeTypeAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~CompatibleSchemeTypeAtom() = default;
};

class StereoVideoAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    StereoVideoAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~StereoVideoAtom() = default;

    enum class SingleViewMode : std::uint8_t    //!< Single View Mode
    {
        NONE_MODE    = 0,
        RIGHT_MODE   = 1,
        LEFT_MODE    = 2,
        INVALID      = 3
    };

    enum class SchemeSpec : std::uint8_t    //!< Scheme Spec
    {
        SPEC14496 = 1,
        SPEC13818 = 2,
        SPEC23000 = 3,
        POVD      = 4

    };

    enum class ISO14496FPArrType : std::uint32_t    //!< ISO14496 FP Arr Type
    {
        INTERBOARD   = 0,
        INTERCOLUMN  = 1,
        INTERROW     = 2,
        SBSPACK      = 3,
        TPPACK       = 4,
        INTERTEMPER  = 5
    };

    enum class ISO13818ArrType : std::uint32_t  //!< ISO13818 Arr Type
    {
        SBSSTEREO   = 0b0000000000000011,
        TBSTEREO    = 0b0000000000000100,
        TWODVIDEO   = 0b0000000000001000
    };

    enum class ISO23000StereoCompType : std::uint8_t    //!< ISO23000 Stereo Comp Type
    {
        SBSHALF         = 0x00,
        INTERVERTICAL   = 0x01,
        FRAMESEQ        = 0x02,
        LRSEQ           = 0x03,
        TBHALF          = 0x04,
        SBSFULL         = 0x05,
        TBFULL          = 0x06
    };

    struct ISO23000ArrType  //!< ISO23000 Arr Type
    {
        ISO23000StereoCompType compositionType;
        bool isLeftFirst;
    };

    enum class POVDFrameCompType : std::uint8_t //!< POVD Frame Comp Type
    {
        TBPACK      = 3,
        SBSPACK     = 4,
        INTERTEMPER = 5
    };

    struct POVDArrangeType  //!< POVD Arrange Type
    {
        POVDFrameCompType compositionType;
        bool useQuincunxSampling;
    };

    union StereoIndicationType {    //!< Stereo Indication Type
        uint32_t valAsUint32;
        ISO14496FPArrType type14496;
        ISO13818ArrType type13818;
        ISO23000ArrType type23000;
        POVDArrangeType typePOVD;
    };

    //!
    //! \brief    Set and Get function for m_singleViewAllowed member
    //!
    //! \param    [in] SingleViewMode
    //!           value to set
    //! \param    [in] m_singleViewAllowed
    //!           m_singleViewAllowed member in class
    //! \param    [in] SingleViewAllowed
    //!           m_singleViewAllowed name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(SingleViewMode, m_singleViewAllowed, SingleViewAllowed, const);

    //!
    //! \brief    Set and Get function for m_stereoScheme member
    //!
    //! \param    [in] SchemeSpec
    //!           value to set
    //! \param    [in] m_stereoScheme
    //!           m_stereoScheme member in class
    //! \param    [in] StereoScheme
    //!           m_stereoScheme name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(SchemeSpec, m_stereoScheme, StereoScheme, const);

    //!
    //! \brief    Set and Get function for m_stereoIndType member
    //!
    //! \param    [in] StereoIndicationType
    //!           value to set
    //! \param    [in] m_stereoIndType
    //!           m_stereoIndType member in class
    //! \param    [in] StereoIndicationType
    //!           m_stereoIndType name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(StereoIndicationType, m_stereoIndType, StereoIndicationType, const);

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
    SingleViewMode m_singleViewAllowed;     //!< Single View Mode
    SchemeSpec m_stereoScheme;              //!< Scheme Spec
    StereoIndicationType m_stereoIndType;   //!< Stereo Indication Type
};

class RestrictedSchemeInfoAtom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    RestrictedSchemeInfoAtom();
    RestrictedSchemeInfoAtom(const RestrictedSchemeInfoAtom&);

    RestrictedSchemeInfoAtom& operator=(const RestrictedSchemeInfoAtom&) = default;

    //!
    //! \brief Destructor
    //!
    virtual ~RestrictedSchemeInfoAtom() = default;

    //!
    //! \brief    Write atom information to stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void ToStream(Stream&);

    //!
    //! \brief    Parse atom information from stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void FromStream(Stream&);

    //!
    //! \brief    Get OriginalFormat
    //!
    //! \return   FourCCInt
    //!           OriginalFormat
    //!
    virtual FourCCInt GetOriginalFormat() const;

    //!
    //! \brief    Set OriginalFormat
    //!
    //! \param    [in] FourCCInt
    //!           OriginalFormat value
    //!
    //! \return   void
    //!
    virtual void SetOriginalFormat(FourCCInt);

    //!
    //! \brief    Get SchemeType
    //!
    //! \return   FourCCInt
    //!           SchemeType
    //!
    virtual FourCCInt GetSchemeType() const;

    //!
    //! \brief    Get SchemeType Atom
    //!
    //! \return   SchemeTypeAtom&
    //!           SchemeType Atom
    //!
    SchemeTypeAtom& GetSchemeTypeAtom() const;

    //!
    //! \brief    Add Scheme Type Atom
    //!
    //! \param    [in] UniquePtr<SchemeTypeAtom>
    //!           Scheme Type Atom pointer
    //!
    //! \return   void
    //!
    void AddSchemeTypeAtom(UniquePtr<SchemeTypeAtom>);

    //!
    //! \brief    has Scheme Type Atom
    //!
    //! \return   bool
    //!           has or not
    //!
    bool HasSchemeTypeAtom() const;

    //!
    //! \brief    Get ProjectedOmniVideoAtom Atom
    //!
    //! \return   ProjectedOmniVideoAtom&
    //!           ProjectedOmniVideoAtom Atom
    //!
    ProjectedOmniVideoAtom& GetProjectedOmniVideoAtom() const;

    //!
    //! \brief    add ProjectedOmniVideoAtom Atom
    //!
    //! \param    [in] UniquePtr<ProjectedOmniVideoAtom>
    //!           ProjectedOmniVideoAtom Atom pointer
    //!
    //! \return   void
    //!
    void AddProjectedOmniVideoAtom(UniquePtr<ProjectedOmniVideoAtom>);

    //!
    //! \brief    Get StereoVideoAtom Atom
    //!
    //! \return   StereoVideoAtom&
    //!           StereoVideoAtom Atom
    //!
    StereoVideoAtom& GetStereoVideoAtom() const;

    //!
    //! \brief    add StereoVideoAtom Atom
    //!
    //! \param    [in] UniquePtr<StereoVideoAtom>
    //!           StereoVideoAtom Atom pointer
    //!
    //! \return   void
    //!
    void AddStereoVideoAtom(UniquePtr<StereoVideoAtom>);

    //!
    //! \brief    has StereoVideo Atom
    //!
    //! \return   bool
    //!           has or not
    //!
    bool HasStereoVideoAtom() const;

    //!
    //! \brief    Get CompatibleScheme Types
    //!
    //! \return   std::vector<CompatibleSchemeTypeAtom*>
    //!           CompatibleScheme Types
    //!
    std::vector<CompatibleSchemeTypeAtom*> GetCompatibleSchemeTypes() const;

    //!
    //! \brief    add CompatibleScheme Type atom
    //!
    //! \param    [in] UniquePtr<CompatibleSchemeTypeAtom>
    //!           CompatibleSchemeType Atom pointer
    //! \return   void
    //!
    void AddCompatibleSchemeTypeAtom(UniquePtr<CompatibleSchemeTypeAtom>);

private:
    UniquePtr<OriginalFormatAtom> m_originalFormatAtom;                         //!< Original Format Atom
    UniquePtr<SchemeTypeAtom> m_schemeTypeAtom;                                 //!< Scheme Type Atom
    UniquePtr<ProjectedOmniVideoAtom> m_projectedOmniVideoAtom;                 //!< Projected Omni Video Atom
    UniquePtr<StereoVideoAtom> m_stereoVideoAtom;                               //!< Stereo Video Atom
    std::vector<UniquePtr<CompatibleSchemeTypeAtom>> m_compatibleSchemeTypes;   //!< Compatible Scheme Type Atom
};

VCD_MP4_END;
#endif /* _RESTRICTEDSCHEMEINFOATOM_H_ */
