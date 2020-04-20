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
//! \file:   ChunkOffsetAtom.cpp
//! \brief:  ChunkOffsetAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "ChunkOffsetAtom.h"
#include <algorithm>
#include <limits>
#include "Stream.h"

VCD_MP4_BEGIN

ChunkOffsetAtom::ChunkOffsetAtom()
    : FullAtom("stco", 0, 0)
    , m_chunkOffsets()
{
}

void ChunkOffsetAtom::SetChunkOffsets(const std::vector<uint64_t>& chunkOffsets)
{
    m_chunkOffsets = chunkOffsets;
    if (*std::max_element(m_chunkOffsets.cbegin(), m_chunkOffsets.cend()) > std::numeric_limits<std::uint32_t>::max())
    {
        SetType("co64");
    }
    else
    {
        SetType("stco");
    }
}

std::vector<uint64_t> ChunkOffsetAtom::GetChunkOffsets()
{
    return m_chunkOffsets;
}

const std::vector<uint64_t> ChunkOffsetAtom::GetChunkOffsets() const
{
    return m_chunkOffsets;
}

void ChunkOffsetAtom::ToStream(Stream& str)
{
    // Write Atom headers
    WriteFullAtomHeader(str);

    str.Write32(static_cast<uint32_t>(m_chunkOffsets.size()));
    if (GetType() == "stco")
    {
        for (uint32_t i = 0; i < m_chunkOffsets.size(); ++i)
        {
            str.Write32(static_cast<uint32_t>(m_chunkOffsets.at(i)));
        }
    }
    else
    {
        // This is a ChunkLargeOffsetAtom 'co64' with unsigned int (64) chunk_offsets.
        for (uint32_t i = 0; i < m_chunkOffsets.size(); ++i)
        {
            str.Write64(m_chunkOffsets.at(i));
        }
    }

    // Update the size of the movie Atom
    UpdateSize(str);
}

void ChunkOffsetAtom::FromStream(Stream& str)
{
    //  First parse the Atom header
    ParseFullAtomHeader(str);

    const std::uint32_t entryCount = str.Read32();
    if (GetType() == "stco")
    {
        for (uint32_t i = 0; i < entryCount; ++i)
        {
            m_chunkOffsets.push_back(str.Read32());
        }
    }
    else  // This is a ChunkLargeOffsetAtom 'co64' with unsigned int (64) chunk_offsets.
    {
        for (uint32_t i = 0; i < entryCount; ++i)
        {
            m_chunkOffsets.push_back(str.Read64());
        }
    }
}

VCD_MP4_END