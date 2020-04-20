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
//! \file:   Mp4Segment.cpp
//! \brief:  DashSegGroup related class implementation
//!

#include "Mp4Segment.h"
#include "Mp4ReaderImpl.h"

VCD_MP4_BEGIN

DashSegGroup::DashSegGroup(Mp4Reader& impl, InitSegmentId initSegmentId)
    : m_impl(impl)
    , m_initSegId(initSegmentId)
{
}

ConstDashSegGroup::ConstDashSegGroup(const Mp4Reader& impl, InitSegmentId initSegmentId)
    : m_impl(impl)
    , m_initSegId(initSegmentId)
{
}

DashSegGroup::SegmentIter DashSegGroup::begin()
{
    auto& seqToSeg = m_impl.m_initSegProps.at(m_initSegId).seqToSeg;
    return SegmentIter(m_impl, m_initSegId, seqToSeg, seqToSeg.begin());
}

DashSegGroup::SegmentIter DashSegGroup::end()
{
    auto& seqToSeg = m_impl.m_initSegProps.at(m_initSegId).seqToSeg;
    return SegmentIter(m_impl, m_initSegId, seqToSeg, seqToSeg.end());
}

DashSegGroup::SegmentConstIter DashSegGroup::begin() const
{
    auto& seqToSeg = m_impl.m_initSegProps.at(m_initSegId).seqToSeg;
    return SegmentConstIter(m_impl, m_initSegId, seqToSeg, seqToSeg.begin());
}

DashSegGroup::SegmentConstIter DashSegGroup::end() const
{
    auto& seqToSeg = m_impl.m_initSegProps.at(m_initSegId).seqToSeg;
    return SegmentConstIter(m_impl, m_initSegId, seqToSeg, seqToSeg.end());
}

ConstDashSegGroup::ConstIter ConstDashSegGroup::begin() const
{
    auto& seqToSeg = m_impl.m_initSegProps.at(m_initSegId).seqToSeg;
    return ConstIter(m_impl, m_initSegId, seqToSeg, seqToSeg.begin());
}

ConstDashSegGroup::ConstIter ConstDashSegGroup::end() const
{
    auto& seqToSeg = m_impl.m_initSegProps.at(m_initSegId).seqToSeg;
    return ConstIter(m_impl, m_initSegId, seqToSeg, seqToSeg.end());
}

DashSegGroup::SegmentIter::SegmentIter(Mp4Reader& impl,
                             InitSegmentId initSegmentId,
                             SeqToSegMap& seqToSeg,
                             SeqToSegMap::iterator theiterator)
    : m_impl(impl)
    , m_initSegId(initSegmentId)
    , m_seqToSeg(seqToSeg)
    , m_iter(theiterator)
{
}

SegmentProperties& DashSegGroup::SegmentIter::operator*() const
{
    return m_impl.m_initSegProps.at(m_initSegId).segPropMap.at(m_iter->second);
}

SegmentProperties* DashSegGroup::SegmentIter::operator->() const
{
    return &m_impl.m_initSegProps.at(m_initSegId).segPropMap.at(m_iter->second);
}

bool DashSegGroup::SegmentIter::operator!=(const DashSegGroup::SegmentIter& other) const
{
    return &m_impl != &other.m_impl || m_iter != other.m_iter;
}

bool DashSegGroup::SegmentConstIter::operator!=(const DashSegGroup::SegmentConstIter& other) const
{
    return &m_impl != &other.m_impl || m_iter != other.m_iter;
}

DashSegGroup::SegmentIter& DashSegGroup::SegmentIter::operator++()
{
    SegmentId curSegmentId = m_iter->second;
    do
    {
        ++m_iter;
    } while (m_iter != m_seqToSeg.end() && m_iter->second == curSegmentId);
    return *this;
}

DashSegGroup::SegmentConstIter::SegmentConstIter(const Mp4Reader& impl,
                                         const InitSegmentId initSegmentId,
                                         const SeqToSegMap& seqToSeg,
                                         SeqToSegMap::const_iterator iterator)
    : m_impl(impl)
    , m_initSegId(initSegmentId)
    , m_seqToSeg(seqToSeg)
    , m_iter(iterator)
{
}

const SegmentProperties& DashSegGroup::SegmentConstIter::operator*() const
{
    return m_impl.m_initSegProps.at(m_initSegId).segPropMap.at(m_iter->second);
}

const SegmentProperties* DashSegGroup::SegmentConstIter::operator->() const
{
    return &m_impl.m_initSegProps.at(m_initSegId).segPropMap.at(m_iter->second);
}

DashSegGroup::SegmentConstIter& DashSegGroup::SegmentConstIter::operator++()
{
    SegmentId curSegmentId = m_iter->second;
    do
    {
        ++m_iter;
    } while (m_iter != m_seqToSeg.end() && m_iter->second == curSegmentId);
    return *this;
}

VCD_MP4_END
