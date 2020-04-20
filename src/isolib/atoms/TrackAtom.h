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
//! \file:   TrackAtom.h
//! \brief:  TrackAtom class.
//! \detail: 'trak' Atom
//!
//! Created on October 14, 2019, 13:39 PM
//!

#ifndef TRACKATOM_H
#define TRACKATOM_H

#include "Atom.h"
#include "Stream.h"
#include "FormAllocator.h"
#include "EditAtom.h"
#include "BasicVideoAtom.h"
#include "MediaAtom.h"
#include "TrackGroupAtom.h"
#include "TrackHeaderAtom.h"
#include "TrackRefAtom.h"
#include "TypeAtom.h"

VCD_MP4_BEGIN

class TrackAtom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    TrackAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~TrackAtom() = default;

    //!
    //! \brief    Set Has Track References
    //!
    //! \param    [in] bool
    //!           value
    //!
    //! \return   void
    //!
    void SetHasTrackReferences(bool value = true);

    //!
    //! \brief    Get Has Track References
    //!
    //! \return   bool
    //!           value
    //!
    bool GetHasTrackReferences() const;

    //!
    //! \brief    Get Track Header Atom
    //!
    //! \return   TrackHeaderAtom&
    //!           Track Header Atom
    //!
    TrackHeaderAtom& GetTrackHeaderAtom();

    //!
    //! \brief    Get Track Header Atom
    //!
    //! \return   const TrackHeaderAtom&
    //!           Track Header Atom
    //!
    const TrackHeaderAtom& GetTrackHeaderAtom() const;

    //!
    //! \brief    Get Media Atom
    //!
    //! \return   MediaAtom&
    //!           Media Atom
    //!
    MediaAtom& GetMediaAtom();

    //!
    //! \brief    Get Media Atom
    //!
    //! \return   const MediaAtom&
    //!           Media Atom
    //!
    const MediaAtom& GetMediaAtom() const;

    //!
    //! \brief    Get TrackReference Atom
    //!
    //! \return   TrackReferenceAtom&
    //!           TrackReference Atom
    //!
    TrackReferenceAtom& GetTrackReferenceAtom();

    //!
    //! \brief    Get TrackReference Atom
    //!
    //! \return   const TrackReferenceAtom&
    //!           TrackReference Atom
    //!
    const TrackReferenceAtom& GetTrackReferenceAtom() const;

    //!
    //! \brief    Get TrackGroup Atom
    //!
    //! \return   TrackGroupAtom&
    //!           TrackGroup Atom
    //!
    TrackGroupAtom& GetTrackGroupAtom();

    //!
    //! \brief    Get TrackGroup Atom
    //!
    //! \return   const TrackGroupAtom&
    //!           TrackGroup Atom
    //!
    const TrackGroupAtom& GetTrackGroupAtom() const;

    //!
    //! \brief    Set and Get function for m_hasTrackGroupAtom member
    //!
    //! \param    [in] bool
    //!           value to set
    //! \param    [in] m_hasTrackGroupAtom
    //!           m_hasTrackGroupAtom member in class
    //! \param    [in] HasTrackGroup
    //!           m_hasTrackGroupAtom name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(bool, m_hasTrackGroupAtom, HasTrackGroup, const);

    //!
    //! \brief    Get TrackType Atom
    //!
    //! \return   TrackTypeAtom&
    //!           TrackType Atom
    //!
    TrackTypeAtom& GetTrackTypeAtom();

    //!
    //! \brief    Get TrackType Atom
    //!
    //! \return   const TrackTypeAtom&
    //!           TrackType Atom
    //!
    const TrackTypeAtom& GetTrackTypeAtom() const;

    //!
    //! \brief    Set and Get function for m_hasTrackTypeAtom member
    //!
    //! \param    [in] bool
    //!           value to set
    //! \param    [in] m_hasTrackTypeAtom
    //!           m_hasTrackTypeAtom member in class
    //! \param    [in] HasTrackTypeAtom
    //!           m_hasTrackTypeAtom name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(bool, m_hasTrackTypeAtom, HasTrackTypeAtom, const);

    //!
    //! \brief    Set Edit Atom
    //!
    //! \param    [in] const EditAtom&
    //!           Edit Atom value
    //!
    //! \return   void
    //!
    void SetEditAtom(const EditAtom& EditAtom);

    //!
    //! \brief    Get Edit Atom
    //!
    //! \return   std::shared_ptr<const EditAtom>
    //!           Edit Atom
    //!
    std::shared_ptr<const EditAtom> GetEditAtom() const;

    //!
    //! \brief    Get SphericalVideoV1 Atom
    //!
    //! \return   SphericalVideoV1Atom&
    //!           SphericalVideoV1 Atom
    //!
    SphericalVideoV1Atom& GetSphericalVideoV1Atom();

    //!
    //! \brief    Get SphericalVideoV1 Atom
    //!
    //! \return   const SphericalVideoV1Atom&
    //!           SphericalVideoV1 Atom
    //!
    const SphericalVideoV1Atom& GetSphericalVideoV1Atom() const;

    //!
    //! \brief    Set and Get function for m_hasSphericalVideoV1Atom member
    //!
    //! \param    [in] bool
    //!           value to set
    //! \param    [in] m_hasSphericalVideoV1Atom
    //!           m_hasSphericalVideoV1Atom member in class
    //! \param    [in] HasSphericalVideoV1Atom
    //!           m_hasSphericalVideoV1Atom name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(bool, m_hasSphericalVideoV1Atom, HasSphericalVideoV1Atom, const);

    //!
    //! \brief    Write atom information to stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    void ToStream(Stream& str);

    //!
    //! \brief    Parse atom information from stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    void FromStream(Stream& str);

private:
    TrackHeaderAtom m_trackHeaderAtom;  //!< Track Header Atom
    MediaAtom m_mediaAtom;              //!< Media Atom related to this track
    TrackReferenceAtom m_trackRefAtom;  //!< Track Reference Atom
    bool m_hasTrackRef;                 //!< Flag that shows whether the track has references from other tracks
    TrackGroupAtom m_trackGroupAtom;    //!< Track Group Atom
    bool m_hasTrackGroupAtom;           //!< Flag that shows whether the track has a track group Atom
    TrackTypeAtom m_trackTypeAtom;      //!< Track Type Atom
    bool m_hasTrackTypeAtom;            //!< Flag that shows whether the track has a track type Atom
    std::shared_ptr<EditAtom> m_editAtom;//!< Edit Atom (optional)
    bool m_hasSphericalVideoV1Atom;     //!< has SphericalVideoV1 Atom
    SphericalVideoV1Atom m_sphericalVideoV1Atom; //!< spherical Video V1 Atom
};

VCD_MP4_END;
#endif /* TRACKATOM_H */
