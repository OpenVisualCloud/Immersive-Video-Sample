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
//! \file:   CompToDecodeAtom.cpp
//! \brief:  CompToDecodeAtom class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!
#include "CompToDecAtom.h"
#include <limits>
#include "Stream.h"
#include "FullAtom.h"

VCD_MP4_BEGIN

CompositionToDecodeAtom::CompositionToDecodeAtom()
    : FullAtom("cslg", 0, 0)
    , m_dtsShift(0)
    , m_leastDisplayDelta(0)
    , m_greatestDisplayDelta(0)
    , m_startTime(0)
    , m_endTime(0)
{
}

bool static requires64Bits(const int64_t value)
{
    if ((value > std::numeric_limits<std::int32_t>::max()) || (value < std::numeric_limits<std::int32_t>::min()))
    {
        return true;
    }
    return false;
}


void CompositionToDecodeAtom::SetDtsShift(const std::int64_t compositionToDtsShift)
{
    m_dtsShift = compositionToDtsShift;
    UpdateVersion();
}

std::int64_t CompositionToDecodeAtom::GetDtsShift() const
{
    return m_dtsShift;
}

void CompositionToDecodeAtom::SetLeastDisplayDelta(const std::int64_t leastDecodeToDisplayDelta)
{
    m_leastDisplayDelta = leastDecodeToDisplayDelta;
    UpdateVersion();
}

std::int64_t CompositionToDecodeAtom::GetLeastDisplayDelta() const
{
    return m_leastDisplayDelta;
}

void CompositionToDecodeAtom::SetGreatestDisplayDelta(const std::int64_t greatestDecodeToDisplayDelta)
{
    m_greatestDisplayDelta = greatestDecodeToDisplayDelta;
    UpdateVersion();
}

std::int64_t CompositionToDecodeAtom::GetGreatestDisplayDelta() const
{
    return m_greatestDisplayDelta;
}

void CompositionToDecodeAtom::SetStartTime(const std::int64_t compositionStartTime)
{
    m_startTime = compositionStartTime;
    UpdateVersion();
}

std::int64_t CompositionToDecodeAtom::GetStartTime() const
{
    return m_startTime;
}

void CompositionToDecodeAtom::SetEndTime(const std::int64_t compositionEndTime)
{
    m_endTime = compositionEndTime;
    UpdateVersion();
}

std::int64_t CompositionToDecodeAtom::GetEndTime() const
{
    return m_endTime;
}

void CompositionToDecodeAtom::UpdateVersion()
{
    if (requires64Bits(m_dtsShift) || requires64Bits(m_leastDisplayDelta) ||
        requires64Bits(m_greatestDisplayDelta) || requires64Bits(m_startTime) ||
        requires64Bits(m_endTime))
    {
        SetVersion(1);
    }
    else
    {
        SetVersion(0);
    }
}

void CompositionToDecodeAtom::ToStream(Stream& str)
{
    // Write Atom headers
    WriteFullAtomHeader(str);

    if (GetVersion() == 0)
    {
        str.Write32(static_cast<std::uint32_t>(m_dtsShift));
        str.Write32(static_cast<std::uint32_t>(m_leastDisplayDelta));
        str.Write32(static_cast<std::uint32_t>(m_greatestDisplayDelta));
        str.Write32(static_cast<std::uint32_t>(m_startTime));
        str.Write32(static_cast<std::uint32_t>(m_endTime));
    }
    else
    {
        str.Write64(static_cast<std::uint64_t>(m_dtsShift));
        str.Write64(static_cast<std::uint64_t>(m_leastDisplayDelta));
        str.Write64(static_cast<std::uint64_t>(m_greatestDisplayDelta));
        str.Write64(static_cast<std::uint64_t>(m_startTime));
        str.Write64(static_cast<std::uint64_t>(m_endTime));
    }

    // Update the size of the movie Atom
    UpdateSize(str);
}

void CompositionToDecodeAtom::FromStream(Stream& str)
{
    //  First parse the Atom header
    ParseFullAtomHeader(str);

    if (GetVersion() == 0)
    {
        m_dtsShift        = static_cast<std::int32_t>(str.Read32());
        m_leastDisplayDelta    = static_cast<std::int32_t>(str.Read32());
        m_greatestDisplayDelta = static_cast<std::int32_t>(str.Read32());
        m_startTime         = static_cast<std::int32_t>(str.Read32());
        m_endTime           = static_cast<std::int32_t>(str.Read32());
    }
    else
    {
        m_dtsShift        = static_cast<std::int64_t>(str.Read64());
        m_leastDisplayDelta    = static_cast<std::int64_t>(str.Read64());
        m_greatestDisplayDelta = static_cast<std::int64_t>(str.Read64());
        m_startTime         = static_cast<std::int64_t>(str.Read64());
        m_endTime           = static_cast<std::int64_t>(str.Read64());
    }
}

VCD_MP4_END