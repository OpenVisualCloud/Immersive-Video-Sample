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
//! \file:   SegIndexAtom.cpp
//! \brief:  SegIndexAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "SegIndexAtom.h"
#include <cassert>
#include <iostream>
#include <stdexcept>

VCD_MP4_BEGIN

SegmentIndexAtom::SegmentIndexAtom(uint8_t version)
    : FullAtom("sidx", version, 0)
    , m_referenceID(0)
    , m_timescale(0)
    , m_earliestPresentationTime(0)
    , m_firstOffset(0)
    , m_references()
    , m_reserveTotal(0)
{
}

void SegmentIndexAtom::SetSpaceReserve(size_t reserveTotal)
{
    m_reserveTotal = reserveTotal;
}

void SegmentIndexAtom::AddReference(const SegmentIndexAtom::Reference& reference)
{
    m_references.push_back(reference);
    assert(m_reserveTotal == 0 || m_references.size() < m_reserveTotal);
}

std::vector<SegmentIndexAtom::Reference> SegmentIndexAtom::GetReferences() const
{
    return m_references;
}

void SegmentIndexAtom::ToStream(Stream& str)
{
    const uint32_t referenceSize = 3 * 4;
    const uint32_t reserveBytes  = static_cast<uint32_t>((m_reserveTotal - m_references.size()) * referenceSize);

    WriteFullAtomHeader(str);
    str.Write32(m_referenceID);
    str.Write32(m_timescale);
    if (GetVersion() == 0)
    {
        str.Write32(static_cast<uint32_t>(m_earliestPresentationTime));
        str.Write32(static_cast<uint32_t>(m_firstOffset + reserveBytes));
    }
    else if (GetVersion() == 1)
    {
        str.Write64(m_earliestPresentationTime);
        str.Write64(m_firstOffset + reserveBytes);
    }
    else
    {
        ISO_LOG(LOG_ERROR, "SegmentIndexAtom::ToStream() supports only 'sidx' version 0 or 1\n");
        throw Exception();
    }
    str.Write16(0);                                          // reserved = 0
    str.Write16(static_cast<uint16_t>(m_references.size()));  // reference_count

    for (uint32_t i = 0; i < m_references.size(); i++)
    {
        str.Write1(m_references.at(i).referenceType ? uint64_t(1) : uint64_t(0), 1);  // bit (1) reference_type
        str.Write1(uint64_t(0) | m_references.at(i).referencedSize, 31);  // unsigned int(31) referenced_size
        str.Write32(m_references.at(i).subsegmentDuration);              // unsigned int(32) subsegment_duration
        str.Write1(m_references.at(i).startsWithSAP ? uint64_t(1) : uint64_t(0), 1);  // bit (1) starts_with_SAP
        str.Write1(uint64_t(0) | m_references.at(i).sapType, 3);                      // unsigned int(3) SAP_type
        str.Write1(uint64_t(0) | m_references.at(i).sapDeltaTime, 28);  // unsigned int(28) SAP_delta_time
    }

    UpdateSize(str);

    if (m_reserveTotal != 0)
    {
        str.Write32(reserveBytes);
        str.Write32(FourCCInt("free").GetUInt32());
        str.Write32(0);

        for (uint32_t i = 1; i < m_reserveTotal - m_references.size(); i++)
        {
            str.Write32(0);
            str.Write32(0);
            str.Write32(0);
        }
    }
}

void SegmentIndexAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);
    m_referenceID = str.Read32();
    m_timescale   = str.Read32();
    if (GetVersion() == 0)
    {
        m_earliestPresentationTime = str.Read32();
        m_firstOffset              = str.Read32();
    }
    else if (GetVersion() == 1)
    {
        m_earliestPresentationTime = str.Read64();
        m_firstOffset              = str.Read64();
    }
    else
    {
        ISO_LOG(LOG_ERROR, "SegmentIndexAtom::FromStream() supports only 'sidx' version 0 or 1\n");
        throw Exception();
    }

    str.Read16();                            // reserved = 0
    uint16_t referenceCount = str.Read16();  // reference_count
    m_references.clear();
    m_references.reserve(referenceCount);
    for (uint16_t i = 0; i < referenceCount; i++)
    {
        Reference ref;
        ref.referenceType      = (str.Read1(1) != 0);                 // bit (1) reference_type
        ref.referencedSize     = str.Read1(31);                       // unsigned int(31) referenced_size
        ref.subsegmentDuration = str.Read32();                       // unsigned int(32) subsegment_duration
        ref.startsWithSAP      = (str.Read1(1) != 0);                 // bit (1) starts_with_SAP
        ref.sapType            = static_cast<uint8_t>(str.Read1(3));  // unsigned int(3) SAP_type
        ref.sapDeltaTime       = str.Read1(28);                       // unsigned int(28) SAP_delta_time
        m_references.push_back(ref);
    }
}

VCD_MP4_END