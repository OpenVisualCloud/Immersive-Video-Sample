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
//! \file:   BasicVideoAtom.h
//! \brief:  Basic Video Atom class
//! \detail: Basic Video atoms definitions.
//!
//! Created on October 14, 2019, 13:39 PM
//!

#ifndef _GOOGLEVIDEOATOM_H_
#define _GOOGLEVIDEOATOM_H_

#include "Atom.h"
#include "Stream.h"
#include "FormAllocator.h"
#include "BasicProjAtom.h"
#include "FullAtom.h"

VCD_MP4_BEGIN

class SphericalVideoHeader : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    SphericalVideoHeader();

    //!
    //! \brief Destructor
    //!
    virtual ~SphericalVideoHeader() = default;

    //!
    //! \brief    Set and Get function for m_metadataSource member
    //!
    //! \param    [in] const std::string&
    //!           value to set
    //! \param    [in] m_metadataSource
    //!           m_metadataSource member in class
    //! \param    [in] MetadataSource
    //!           m_metadataSource name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(const std::string&, m_metadataSource, MetadataSource, const);

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
    std::string m_metadataSource;   //!< meta data source
};

#define SPHERICAL_VIDEOV1_GENERAL_UUID                                                          \
    {                                                                                                  \
        0xff, 0xcc, 0x82, 0x63, 0xf8, 0x55, 0x4a, 0x93, 0x88, 0x14, 0x58, 0x7a, 0x02, 0x52, 0x1f, 0xdd \
    }

class SphericalVideoV1Atom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    SphericalVideoV1Atom();

    //!
    //! \brief Destructor
    //!
    virtual ~SphericalVideoV1Atom() = default;

    enum class StereoTypeV1 : uint8_t   //!< stereo type
    {
        UNDEFINED         = 0,
        MONO_TYPE         = 1,
        STEREO_TOP_BOTTOM = 2,
        STEREO_LEFT_RIGHT = 3
    };

    enum class ProjectFormat : uint8_t  //!< projection format
    {
        UNKNOWN         = 0,
        ERP             = 1
    };

    struct GeneralMetaData  //!< general meta data
    {
        bool isSpherical;
        bool isStitched;
        std::string stitchedSW;
        ProjectFormat projectionFormat;
        StereoTypeV1 stereoType;
        uint32_t sourceCount;
        int32_t initViewHead;
        int32_t initViewPitch;
        int32_t initViewRoll;
        uint64_t timestamp;
        uint32_t fullPanoWidth;
        uint32_t fullPanoHeight;
        uint32_t croppedAreaImageWidth;
        uint32_t croppedAreaImageHeight;
        uint32_t croppedAreaLeft;
        uint32_t croppedAreaTop;
    };

    //!
    //! \brief    Set and Get function for m_globalMetadata member
    //!
    //! \param    [in] const GeneralMetaData&
    //!           value to set
    //! \param    [in] m_globalMetadata
    //!           m_globalMetadata member in class
    //! \param    [in] GeneralMetaData
    //!           m_globalMetadata name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(const GeneralMetaData&, m_globalMetadata, GeneralMetaData, const);

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

    //!
    //! \brief    Read Tag
    //!
    //! \param    [in] const std::string&
    //!           tag
    //! \param    [out] T&
    //!           value
    //!
    //! \return   void
    //!
    template <typename T>
    void ReadTag(const std::string& tag, T& value);

    //!
    //! \brief    Write Tag
    //!
    //! \param    [in] Stream& str
    //!           bitstream
    //! \param    [in] const std::string&
    //!           tag
    //! \param    [in] const T
    //!           value
    //!
    //! \return   void
    //!
    template <typename T>
    void WriteTag(Stream& str, const std::string& tag, const T value);

private:
    std::string m_xmlMetadata;          //!< xml meta data
    GeneralMetaData m_globalMetadata;   //! global meta data
};

class SphericalVideoV2Atom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    SphericalVideoV2Atom();

    //!
    //! \brief Destructor
    //!
    virtual ~SphericalVideoV2Atom() = default;

    //!
    //! \brief    Set and Get function for m_sphericalVideoHeaderAtom member
    //!
    //! \param    [in] const SphericalVideoHeader&
    //!           value to set
    //! \param    [in] m_sphericalVideoHeaderAtom
    //!           m_sphericalVideoHeaderAtom member in class
    //! \param    [in] SphericalVideoHeaderAtom
    //!           m_sphericalVideoHeaderAtom name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(const SphericalVideoHeader&, m_sphericalVideoHeaderAtom, SphericalVideoHeaderAtom, const);

    //!
    //! \brief    Set and Get function for m_projectionAtom member
    //!
    //! \param    [in] const Projection&
    //!           value to set
    //! \param    [in] m_projectionAtom
    //!           m_projectionAtom member in class
    //! \param    [in] ProjectionAtom
    //!           m_projectionAtom name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(const Projection&, m_projectionAtom, ProjectionAtom, const);

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
    SphericalVideoHeader m_sphericalVideoHeaderAtom;    //!< spherical Video Header Atom
    Projection m_projectionAtom;                        //!< projection atom
};

class Stereoscopic3D : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    Stereoscopic3D();

    //!
    //! \brief Destructor
    //!
    virtual ~Stereoscopic3D() = default;

    enum class StereoTypeV2 : uint8_t   //!< stereo type
    {
        MONO_TYPE           = 0,
        STEREO_TOPBOTTOM    = 1,
        STEREO_LEFTRIGHT    = 2,
        STEREO_STEREOCUSTOM = 3
    };

    //!
    //! \brief    Set and Get function for m_stereoMode member
    //!
    //! \param    [in] const StereoTypeV2&
    //!           value to set
    //! \param    [in] m_stereoMode
    //!           m_stereoMode member in class
    //! \param    [in] StereoMode
    //!           m_stereoMode name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(const StereoTypeV2&, m_stereoMode, StereoMode, const);

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
    StereoTypeV2 m_stereoMode;  //!< stereo mode
};

VCD_MP4_END;
#endif /* GOOGLESPHERICALVIDEOV1ATOM_H */
