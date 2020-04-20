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
//! \file:   MovieFragAtom.h
//! \brief:  Movie Fragment Atom class
//! \detail: 'moof' Atom
//!
//! Created on October 16, 2019, 13:39 PM
//!

#ifndef _MOVIEFRAGMENTATOM_H_
#define _MOVIEFRAGMENTATOM_H_

#include <memory>
#include "Atom.h"
#include "Stream.h"
#include "FormAllocator.h"
#include "MovieFragHeaderAtom.h"
#include "TrackExtAtom.h"
#include "TrackFragAtom.h"

VCD_MP4_BEGIN

class MovieFragmentAtom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    MovieFragmentAtom(std::vector<SampleDefaults>& sampleDefaults);

    //!
    //! \brief Destructor
    //!
    virtual ~MovieFragmentAtom() = default;

    //!
    //! \brief    Get Movie Fragment Header Atom
    //!
    //! \return   MovieFragmentHeaderAtom&
    //!           Movie Fragment Header Atom
    //!
    MovieFragmentHeaderAtom& GetMovieFragmentHeaderAtom();

    //!
    //! \brief    Add Track Fragment Atom
    //!
    //! \param    [in] UniquePtr<TrackFragmentAtom>
    //!           track Fragment Atom pointer
    //!
    //! \return   void
    //!
    void AddTrackFragmentAtom(UniquePtr<TrackFragmentAtom> trackFragmentAtom);

    //!
    //! \brief    Get Track Fragment Atoms
    //!
    //! \return   std::vector<TrackFragmentAtom*>
    //!           Track Fragment Atoms
    //!
    std::vector<TrackFragmentAtom*> GetTrackFragmentAtoms();

    //!
    //! \brief    Set Moof First Byte Offset
    //!
    //! \param    [in] std::uint64_t
    //!           Moof First Byte Offset
    //!
    //! \return   void
    //!
    void SetMoofFirstByteOffset(std::uint64_t moofFirstByteOffset);

    //!
    //! \brief    Get Moof First Byte Offset
    //!
    //! \return   std::uint64_t
    //!           Moof First Byte Offset
    //!
    std::uint64_t GetMoofFirstByteOffset();

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
    MovieFragmentHeaderAtom m_movieFragmentHeaderAtom;              //!< Movie Fragment Header Atom
    std::vector<UniquePtr<TrackFragmentAtom>> m_trackFragmentAtoms; //!< Contained TrackFragmentAtoms
    std::vector<SampleDefaults>& m_sampleDefaults;                  //!< sampleDefaults array
    std::uint64_t m_firstByteOffset;                                //!< Offset of 1st byte of this moof
};

VCD_MP4_END;
#endif /* _MOVIEFRAGMENTATOM_H_ */
