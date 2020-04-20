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
//! \file:   MovieAtom.h
//! \brief:  Movie Atom class
//! \detail: 'moov' Atom
//!
//! Created on October 16, 2019, 13:39 PM
//!

#ifndef _MOVIEATOM_H_
#define _MOVIEATOM_H_

#include "Atom.h"
#include "FormAllocator.h"
#include "MovieHeaderAtom.h"
#include "TrackAtom.h"
#include "FullAtom.h"
#include "Stream.h"
#include "TrackExtAtom.h"

VCD_MP4_BEGIN

class MovieExtendsHeaderAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    MovieExtendsHeaderAtom(uint8_t version = 0);

    //!
    //! \brief Destructor
    //!
    virtual ~MovieExtendsHeaderAtom() = default;

    //!
    //! \brief    Set Fragment Duration
    //!
    //! \param    [in] const uint64_t
    //!           fragment Duration
    //!
    //! \return   void
    //!
    void SetFragmentDuration(const uint64_t fragmentDuration);

    //!
    //! \brief    Get Fragment Duration
    //!
    //! \return   uint64_t
    //!           fragment Duration
    //!
    uint64_t GetFragmentDuration() const;

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
    uint64_t m_fragmentDuration;    //!< fragment duration
};

class MovieExtendsAtom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    MovieExtendsAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~MovieExtendsAtom() = default;

    //!
    //! \brief    Add Movie Extends Header Atom
    //!
    //! \param    [in] const MovieExtendsHeaderAtom&
    //!           Movie Extends Header Atom
    //!
    //! \return   void
    //!
    void AddMovieExtendsHeaderAtom(const MovieExtendsHeaderAtom& movieExtendsHeaderAtom);

    //!
    //! \brief    Is Movie Extends Header Atom Present
    //!
    //! \return   bool
    //!           is present or not
    //!
    bool IsMovieExtendsHeaderAtomPresent() const;

    //!
    //! \brief    Get MovieExtendsHeader Atom
    //!
    //! \return   const MovieExtendsHeaderAtom&
    //!           MovieExtendsHeader Atom
    //!
    const MovieExtendsHeaderAtom& GetMovieExtendsHeaderAtom() const;

    //!
    //! \brief    Add TrackExtends Atom
    //!
    //! \param    [in] UniquePtr<TrackExtendsAtom>
    //!           trackExtendsAtom pointer
    //!
    //! \return   void
    //!
    void AddTrackExtendsAtom(UniquePtr<TrackExtendsAtom> trackExtendsAtom);

    //!
    //! \brief    Get TrackExtends Atoms
    //!
    //! \return   const std::vector<TrackExtendsAtom*>
    //!           TrackExtends Atoms pointers
    //!
    const std::vector<TrackExtendsAtom*> GetTrackExtendsAtoms() const;

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
    bool m_movieExtendsHeaderAtomPresent;                   //!< is movie Extends Header Atom Present
    MovieExtendsHeaderAtom m_movieExtendsHeaderAtom;        //!< Movie Extends Header Atom
    std::vector<UniquePtr<TrackExtendsAtom>> m_trackExtends;//!< Contained TrackExtendsAtoms
};

class MovieAtom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    MovieAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~MovieAtom() = default;

    //!
    //! \brief Clear track atom
    //!
    void Clear();

    //!
    //! \brief    Get MovieHeader Atom
    //!
    //! \return   MovieHeaderAtom&
    //!           MovieHeader Atom
    //!
    MovieHeaderAtom& GetMovieHeaderAtom();

    //!
    //! \brief    Get MovieHeader Atom
    //!
    //! \return   const MovieHeaderAtom&
    //!           MovieHeader Atom
    //!
    const MovieHeaderAtom& GetMovieHeaderAtom() const;

    //!
    //! \brief    Get Track Atoms
    //!
    //! \return   std::vector<TrackAtom*>
    //!           Track Atoms
    //!
    std::vector<TrackAtom*> GetTrackAtoms();

    //!
    //! \brief    Get Track Atom
    //!
    //! \param    [in] uint32_t
    //!           track Id
    //! \return   TrackAtom*
    //!           Track Atom
    //!
    TrackAtom* GetTrackAtom(uint32_t trackId);

    //!
    //! \brief    Get Movie Extends Atom
    //!
    //! \return   const MovieExtendsAtom*
    //!           Movie Extends Atom
    //!
    const MovieExtendsAtom* GetMovieExtendsAtom() const;

    //!
    //! \brief    Add Movie Extends Atom
    //!
    //! \param    [in] UniquePtr<MovieExtendsAtom>
    //!           movie Extends Atom pointer
    //! \return   void
    //!
    void AddMovieExtendsAtom(UniquePtr<MovieExtendsAtom> movieExtendsAtom);

    //!
    //! \brief    Is Meta Atom Present
    //!
    //! \return   bool
    //!           is or not
    //!
    bool IsMetaAtomPresent() const;

    //!
    //! \brief    Is Movie Extends Atom Present
    //!
    //! \return   bool
    //!           is or not
    //!
    bool IsMovieExtendsAtomPresent() const;

    //!
    //! \brief    Add Track Atom
    //!
    //! \param    [in] UniquePtr<TrackAtom>
    //!           track Atom pointer
    //! \return   void
    //!
    void AddTrackAtom(UniquePtr<TrackAtom> trackAtom);

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
    MovieHeaderAtom m_movieHeaderAtom;               //!< The mandatory MovieHeaderAtom
    std::vector<UniquePtr<TrackAtom>> m_tracks;      //!< Contained TrackAtoms
    UniquePtr<MovieExtendsAtom> m_movieExtendsAtom;  //!< Optional Movie Extends Atom
};

VCD_MP4_END;
#endif /* _MOVIEATOM_H_ */
