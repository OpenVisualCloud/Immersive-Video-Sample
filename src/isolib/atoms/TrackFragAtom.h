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
//! \file:   TrackFragAtom.h
//! \brief:  Track Fragment Atom class
//! \detail: 'traf' Atom
//!
//! Created on October 16, 2019, 13:39 PM
//!

#ifndef _TRACKFRAGMENTATOM_H_
#define _TRACKFRAGMENTATOM_H_

#include <memory>
#include "Atom.h"
#include "Stream.h"
#include "FormAllocator.h"
#include "TrackExtAtom.h"
#include "TrackRunAtom.h"
#include "FullAtom.h"
#include "MovieFragDataTypes.h"

VCD_MP4_BEGIN

class TrackFragmentHeaderAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    TrackFragmentHeaderAtom(std::uint32_t tr_flags = 0);

    //!
    //! \brief Destructor
    //!
    virtual ~TrackFragmentHeaderAtom() = default;

    enum TrackFragHeaderFlags   //!< Track Frag Header Flags
    {
        pDataOffset       = 0x000001,
        pSampleDescrIndex = 0x000002,
        pSampleDuration   = 0x000008,
        pSampleSize       = 0x000010,
        pSampleFlags      = 0x000020,
        IsDurationEmpty   = 0x010000,
        IsBaseMoof        = 0x020000
    };

    //!
    //! \brief    Set and Get function for m_trackId member
    //!
    //! \param    [in] std::uint32_t
    //!           value to set
    //! \param    [in] m_trackId
    //!           m_trackId member in class
    //! \param    [in] TrackId
    //!           m_trackId name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(uint32_t, m_trackId, TrackId, const);

    //!
    //! \brief    Set Base Data Offset
    //!
    //! \param    [in] const uint64_t
    //!           base Data Offset value
    //!
    //! \return   void
    //!
    void SetBaseDataOffset(const uint64_t baseDataOffset);

    //!
    //! \brief    Get Base Data Offset
    //!
    //! \return   std::uint64_t
    //!           Base Data Offset
    //!
    uint64_t GetBaseDataOffset() const;

    //!
    //! \brief    Set and Get function for m_sampleDescrIndex member
    //!
    //! \param    [in] std::uint32_t
    //!           value to set
    //! \param    [in] m_sampleDescrIndex
    //!           m_sampleDescrIndex member in class
    //! \param    [in] SampleDescrIndex
    //!           m_sampleDescrIndex name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(uint32_t, m_sampleDescrIndex, SampleDescrIndex, const);

    //!
    //! \brief    Set Default Sample Duration
    //!
    //! \param    [in] const uint32_t
    //!           Default Sample Duration
    //!
    //! \return   void
    //!
    void SetDefaultSampleDuration(const uint32_t defaultSampleDuration);

    //!
    //! \brief    Get Default Sample Duration
    //!
    //! \return   std::uint32_t
    //!           Default Sample Duration
    //!
    uint32_t GetDefaultSampleDuration() const;

    //!
    //! \brief    Set Default Sample Size
    //!
    //! \param    [in] const uint32_t
    //!           Default Sample Size
    //!
    //! \return   void
    //!
    void SetDefaultSampleSize(const uint32_t defaultSampleSize);

    //!
    //! \brief    Get Default Sample Size
    //!
    //! \return   std::uint32_t
    //!           Default Sample Size
    //!
    uint32_t GetDefaultSampleSize() const;

    //!
    //! \brief    Set Default Sample Flags
    //!
    //! \param    [in] const Flags
    //!           Default Sample Flags
    //!
    //! \return   void
    //!
    void SetDefaultSampleFlags(const SampleFlags defaultSampleFlags);

    //!
    //! \brief    Get Default Sample Flags
    //!
    //! \return   SampleFlags
    //!           Default Sample Flags
    //!
    SampleFlags GetDefaultSampleFlags() const;

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
    uint32_t m_trackId;                 //!< track Id
    uint64_t m_baseDataOffset;          //!< base Data Offset
    uint32_t m_sampleDescrIndex;        //!< sample Descr Index
    uint32_t m_defaultSampleDuration;   //!< default Sample Duration
    uint32_t m_defaultSampleSize;       //!< default Sample Size
    SampleFlags m_defaultSampleFlags;   //!< default Sample Flags
};

class TrackFragmentBaseMediaDecodeTimeAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    TrackFragmentBaseMediaDecodeTimeAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~TrackFragmentBaseMediaDecodeTimeAtom() = default;

    //!
    //! \brief    Set Base Media Decode Time
    //!
    //! \param    [in] const uint64_t
    //!           Base Media Decode Time
    //!
    //! \return   void
    //!
    void SetBaseMediaDecodeTime(const uint64_t baseMediaDecodeTime);

    //!
    //! \brief    Get Base Media Decode Time
    //!
    //! \return   std::uint64_t
    //!           Base Media Decode Time
    //!
    uint64_t GetBaseMediaDecodeTime() const;

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
    uint64_t m_baseMediaDecodeTime; //!< base Media Decode Time
};

class TrackFragmentAtom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    TrackFragmentAtom(std::vector<SampleDefaults>& sampleDefaults);

    //!
    //! \brief Destructor
    //!
    virtual ~TrackFragmentAtom() = default;

    //!
    //! \brief    Get TrackFragmentHeader Atom
    //!
    //! \return   TrackFragmentHeaderAtom&
    //!           TrackFragmentHeader Atom
    //!
    TrackFragmentHeaderAtom& GetTrackFragmentHeaderAtom();

    //!
    //! \brief    Add TrackRun Atom
    //!
    //! \param    [in] UniquePtr<TrackRunAtom>
    //!           TrackRun Atom
    //!
    //! \return   void
    //!
    void AddTrackRunAtom(UniquePtr<TrackRunAtom> trackRunAtom);

    //!
    //! \brief    Get TrackRun Atoms
    //!
    //! \return   std::vector<TrackRunAtom*>
    //!           TrackRun Atoms
    //!
    std::vector<TrackRunAtom*> GetTrackRunAtoms();

    //!
    //! \brief    Set TrackFragmentDecodeTime Atom
    //!
    //! \param    [in] UniquePtr<TrackFragmentBaseMediaDecodeTimeAtom>
    //!           TrackFragmentDecodeTime Atom
    //!
    //! \return   void
    //!
    void SetTrackFragmentDecodeTimeAtom(UniquePtr<TrackFragmentBaseMediaDecodeTimeAtom> trackFragmentDecodeTimeAtom);

    //!
    //! \brief    Get TrackFragmentBaseMediaDecodeTime Atoms
    //!
    //! \return   TrackFragmentBaseMediaDecodeTimeAtom*
    //!           TrackFragmentBaseMediaDecodeTime Atoms
    //!
    TrackFragmentBaseMediaDecodeTimeAtom* GetTrackFragmentBaseMediaDecodeTimeAtom();

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
    TrackFragmentHeaderAtom m_trackFragmentHeaderAtom;      //!< Track Fragment Header Atom
    std::vector<SampleDefaults>& m_sampleDefaults;          //!< Sample Default array
    std::vector<UniquePtr<TrackRunAtom>> m_trackRunAtoms;   //!< Contains TrackRunAtoms
    UniquePtr<TrackFragmentBaseMediaDecodeTimeAtom> m_trackFragmentDecodeTimeAtom;  //!< Track Fragment Base Media Decode Time Atom
};

VCD_MP4_END;
#endif /* _TRACKFRAGMENTATOM_H_ */
