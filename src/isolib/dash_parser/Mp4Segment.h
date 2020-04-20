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
//! \file:   Mp4Segment.h
//! \brief:  DashSegGroup related class definition
//! \detail: Define segments related operation class
//!

#ifndef _MP4SEGMENTS_H_
#define _MP4SEGMENTS_H_

#include "Mp4DataTypes.h"

VCD_MP4_BEGIN

class Mp4Reader;

class DashSegGroup
{
public:
    DashSegGroup(Mp4Reader& impl, InitSegmentId initSegmentId);

    class SegmentIter
    {
    public:
        SegmentIter(Mp4Reader& impl,
                 InitSegmentId initSegmentId,
                 SeqToSegMap& sequenceToSegment,
                 SeqToSegMap::iterator);
        SegmentProperties& operator*() const;
        SegmentProperties* operator->() const;
        SegmentIter& operator++();
        Mp4Reader& m_impl;
        InitSegmentId m_initSegId;
        SeqToSegMap& m_seqToSeg;
        SeqToSegMap::iterator m_iter;

        bool operator!=(const SegmentIter& other) const;
    };

    class SegmentConstIter
    {
    public:
        SegmentConstIter(const Mp4Reader& impl,
                       const InitSegmentId initSegmentId,
                       const SeqToSegMap& seqToSeg,
                       SeqToSegMap::const_iterator);
        const SegmentProperties& operator*() const;
        const SegmentProperties* operator->() const;
        SegmentConstIter& operator++();
        const Mp4Reader& m_impl;
        const InitSegmentId m_initSegId;
        const SeqToSegMap& m_seqToSeg;
        SeqToSegMap::const_iterator m_iter;

        bool operator!=(const SegmentConstIter& other) const;
    };

    SegmentIter begin();
    SegmentConstIter begin() const;
    SegmentIter end();
    SegmentConstIter end() const;

private:
    Mp4Reader& m_impl;
    InitSegmentId m_initSegId;
};

class ConstDashSegGroup
{
public:
    ConstDashSegGroup(const Mp4Reader& impl, InitSegmentId initSegmentId);

    typedef DashSegGroup::SegmentConstIter Iter;
    typedef DashSegGroup::SegmentConstIter ConstIter;

    ConstIter begin() const;
    ConstIter end() const;

private:
    const Mp4Reader& m_impl;
    InitSegmentId m_initSegId;
};

VCD_MP4_END;
#endif  // _MP4SEGMENTS_H_
