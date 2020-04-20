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
//! \file:   DecPts.h
//! \brief:  DecPts class
//! \detail: Used for decode presentation time
//!
//! Created on October 14, 2019, 13:39 PM
//!
#ifndef DECODEPTS_H
#define DECODEPTS_H

#include <algorithm>
#include <map>
#include "CompOffsetAtom.h"
#include "CompToDecAtom.h"
#include "FormAllocator.h"
#include "EditAtom.h"
#include "TimeToSampAtom.h"
#include "TrackRunAtom.h"

VCD_MP4_BEGIN

class DecodePts
{
public:
    typedef std::int64_t PresentTime;    //!< Sample presentation time
    typedef std::int64_t PresentTimeTS;  //!< Sample presentation time
    typedef std::uint64_t SampleIndex;   //!< sample index

    using PMap = std::map<PresentTime, SampleIndex>;

    using PMapTS = std::map<PresentTimeTS, SampleIndex>;

public:

    //!
    //! \brief Constructor
    //!
    DecodePts();

    //!
    //! \brief Destructor
    //!
    virtual ~DecodePts() = default;

    //!
    //! \brief    Set Atom
    //!
    //! \param    [in] const TimeToSampleAtom*
    //!           time To Sample Atom value
    //!
    //! \return   void
    //!
    void SetAtom(const TimeToSampleAtom* timeToSampleAtom);
    void SetAtom(const CompositionOffsetAtom* compositionOffsetAtom);
    void SetAtom(const CompositionToDecodeAtom* compositionToDecodeAtom);
    void SetAtom(const EditListAtom* editListAtom, std::uint32_t movieTimescale, std::uint32_t mediaTimescale);
    void SetAtom(const TrackRunAtom* trackRunAtom);

    //!
    //! \brief    Generate presentation timestamps
    //!
    //! \return   bool
    //!           unravel success or not
    //!
    bool Unravel();

    //!
    //! \brief    Generate presentation timestamps in track run atom
    //!
    //! \return   void
    //!
    void UnravelTrackRun();

    //!
    //! \brief    Get Span
    //!
    //! \return   std::uint64_t
    //!           Span value
    //!
    std::uint64_t GetSpan() const;

    //!
    //! \brief    Set Local Time
    //!
    //! \param    [in] std::uint64_t
    //!           Local Time value
    //!
    //! \return   void
    //!
    void SetLocalTime(std::uint64_t ptsOffset);

    //!
    //! \brief    Get Time
    //!
    //! \param    [in] std::uint16_t
    //!           Time value
    //!
    //! \return   PMap
    //!           Presentation timestamps
    //!
    PMap GetTime(std::uint32_t timeScale) const;

    //!
    //! \brief    Get Time TS
    //!
    //! \return   PMapTS
    //!           Time TS
    //!
    PMapTS GetTimeTS() const;

    //!
    //! \brief    Get Time in Track Run atom
    //!
    //! \param    [in] std::uint32_t
    //!           time Scale
    //! \param    [out] PMap&
    //!           Presentation timestamps
    //!
    //! \return   void
    //!
    void GetTimeTrackRun(std::uint32_t timeScale, PMap& oldPMap) const;

    //!
    //! \brief    Get Time in Track Run atom TS
    //!
    //! \param    [out] PMapTS&
    //!           Presentation timestamps
    //!
    //! \return   void
    //!
    void GetTimeTrackRunTS(PMapTS& oldPMapTS) const;

private:
    const EditListAtom* m_editListAtom;                 //!< edit List Atom
    std::uint32_t m_movieTimescale;                     //!< movie Time scale
    std::uint32_t m_mediaTimescale;                     //!< media Time scale
    const TimeToSampleAtom* m_timeToSampAtom;           //!< time To Sample Atom
    const CompositionOffsetAtom* m_compOffsetAtom;      //!< comp Offset Atom
    const CompositionToDecodeAtom* m_compToDecodeAtom;  //!< comp To Decode Atom
    const TrackRunAtom* m_trackRunAtom;                 //!< track Run Atom
    std::uint64_t m_movieOffset;                        //!< movie Offset
    std::int64_t m_mediaOffset = 0;                     //!< media Offset
    PMapTS m_mediaPts;                                  //!< media Pts
    PMapTS m_moviePts;                                  //!< movie Pts

    //!
    //! \brief    Determine the duration of the last sample
    //!
    //! \return   std::uint64_t
    //!           duration
    //!
    std::uint64_t LastSampleDuration() const;

    //!
    //! \brief    Set Edit
    //!
    //! \param    [in] Entry&
    //!           Edit value
    //!
    //! \return   void
    //!
    template <typename Entry>
    void SetEdit(Entry& entry);

    //!
    //! \brief    Set Empty Edit
    //!
    //! \param    [in] Entry&
    //!           Empty Edit value
    //!
    //! \return   void
    //!
    template <typename Entry>
    void SetEmptyEdit(Entry& entry);

    //!
    //! \brief    Set Dwell Edit
    //!
    //! \param    [in] Entry&
    //!           entry value
    //!
    //! \return   void
    //!
    template <typename Entry>
    void SetDwellEdit(Entry& entry);

    //!
    //! \brief    Set Shift Edit
    //!
    //! \param    [in] Entry&
    //!           entry
    //!
    //! \return   void
    //!
    template <typename Entry>
    void SetShiftEdit(Entry& entry);

    //!
    //! \brief    From Movie To Media TS
    //!
    //! \param    [in] Time
    //!           movieTS
    //!
    //! \return   std::uint64_t
    //!           presentation time
    //!
    template <typename Time>
    std::uint64_t FromMovieToMediaTS(Time movieTS) const;

    //!
    //! \brief    Set Edit List
    //!
    //! \return   void
    //!
    void SetEditList();
};

VCD_MP4_END;
#endif /* DECODEPTS_H */
