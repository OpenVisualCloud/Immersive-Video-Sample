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
//! \file:   BasicProjAtom.h
//! \brief:  Basic Projection Atom class
//! \detail: Definition for Projection Atoms.
//!
//! Created on October 14, 2019, 13:39 PM
//!

#ifndef GOOGLEPROJECTIONATOM_H
#define GOOGLEPROJECTIONATOM_H

#include "Atom.h"
#include "Stream.h"
#include "FormAllocator.h"
#include "FullAtom.h"

VCD_MP4_BEGIN

class ProjectionHeader : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    ProjectionHeader();

    //!
    //! \brief Destructor
    //!
    virtual ~ProjectionHeader() = default;

    struct PoseInDegrees_16_16_FP   //!< head pose
    {
        int32_t yaw;
        int32_t pitch;
        int32_t roll;
    };

    //!
    //! \brief    Set and Get function for m_pose member
    //!
    //! \param    [in] std::uint16_t
    //!           value to set
    //! \param    [in] m_pose
    //!           m_pose member in class
    //! \param    [in] Pose
    //!           m_pose name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(const PoseInDegrees_16_16_FP&, m_pose, Pose, const);

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
    PoseInDegrees_16_16_FP m_pose;  //!< head Pose values
};

class ProjectionDataAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    ProjectionDataAtom(FourCCInt proj_type, uint8_t version, uint32_t flags);

    //!
    //! \brief Destructor
    //!
    virtual ~ProjectionDataAtom() = default;

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
};

class CubemapProjection : public ProjectionDataAtom
{
public:

    //!
    //! \brief Constructor
    //!
    CubemapProjection();

    //!
    //! \brief Destructor
    //!
    virtual ~CubemapProjection() = default;

    //!
    //! \brief    Set and Get function for m_layout member
    //!
    //! \param    [in] std::uint32_t
    //!           value to set
    //! \param    [in] m_layout
    //!           m_layout member in class
    //! \param    [in] Layout
    //!           m_layout name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(uint32_t, m_layout, Layout, const);

    //!
    //! \brief    Set and Get function for m_padding member
    //!
    //! \param    [in] std::uint32_t
    //!           value to set
    //! \param    [in] m_padding
    //!           m_padding member in class
    //! \param    [in] m_padding
    //!           m_padding name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(uint32_t, m_padding, Padding, const);

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
    uint32_t m_layout;  //!< layout
    uint32_t m_padding; //!< padding
};

class EquirectangularProjection : public ProjectionDataAtom
{
public:

    //!
    //! \brief Constructor
    //!
    EquirectangularProjection();

    //!
    //! \brief Destructor
    //!
    virtual ~EquirectangularProjection() = default;

    struct ProjectionBounds //!< projection bounds
    {
        uint32_t top_32FP;
        uint32_t bottom_32FP;
        uint32_t left_32FP;
        uint32_t right_32FP;
    };

    //!
    //! \brief    Set and Get function for m_projectionBounds member
    //!
    //! \param    [in] const ProjectionBounds&
    //!           value to set
    //! \param    [in] m_projectionBounds
    //!           m_projectionBounds member in class
    //! \param    [in] ProjectionBounds
    //!           m_projectionBounds name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(const ProjectionBounds&, m_projectionBounds, ProjectionBounds, const);

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
    ProjectionBounds m_projectionBounds;    //!< projection bounds
};

class Projection : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    Projection();

    //!
    //! \brief Destructor
    //!
    virtual ~Projection() = default;

    enum class ProjectFormat : uint8_t  //!< projection format
    {
        UNKNOWN         = 0,
        CUBEMAP         = 1,
        ERP             = 2,
        MESH            = 3
    };

    //!
    //! \brief    Set and Get function for m_projectionHeaderAtom member
    //!
    //! \param    [in] const ProjectionHeader&
    //!           value to set
    //! \param    [in] m_projectionHeaderAtom
    //!           m_projectionHeaderAtom member in class
    //! \param    [in] ProjectionHeaderAtom
    //!           m_projectionHeaderAtom name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(const ProjectionHeader&, m_projectionHeaderAtom, ProjectionHeaderAtom, const);

    //!
    //! \brief    Get ProjectFormat
    //!
    //! \return   ProjectFormat
    //!           ProjectFormat
    //!
    ProjectFormat GetProjectFormat() const;

    //!
    //! \brief    Get CubemapProjection
    //!
    //! \return   const CubemapProjection&
    //!           CubemapProjection
    //!
    const CubemapProjection& GetCubemapProjectionAtom() const;

    //!
    //! \brief    Set Cubemap Projection Atom
    //!
    //! \param    [in] const CubemapProjection&
    //!           projection value
    //!
    //! \return   void
    //!
    void SetCubemapProjectionAtom(const CubemapProjection& projection);

    //!
    //! \brief    Get Equirectangular Projection atom
    //!
    //! \return   const EquirectangularProjection&
    //!           Equirectangular Projection atom
    //!
    const EquirectangularProjection& GetEquirectangularProjectionAtom() const;

    //!
    //! \brief    Set Equirectangular Projection atom
    //!
    //! \param    [in] const EquirectangularProjection&
    //!           Equirectangular Projection atom value
    //!
    //! \return   void
    //!
    void SetEquirectangularProjectionAtom(const EquirectangularProjection& projection);

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
    ProjectionHeader m_projectionHeaderAtom;                    //!< projection Header Atom
    ProjectFormat m_projectionFormat;                           //!< projection Format
    CubemapProjection m_cubemapProjectionAtom;                  //!< cubemap Projection Atom
    EquirectangularProjection m_equirectangularProjectionAtom;  //!< equirectangular Projection Atom
};

VCD_MP4_END;
#endif /* GOOGLEPROJECTIONATOM_H */
