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
//! \file:   ChunkLocationAtom.cpp
//! \brief:  ChunkLocationAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "ChunkLocationAtom.h"
#include "Stream.h"
#include <assert.h>

VCD_MP4_BEGIN

ChunkLocationAtom::ChunkLocationAtom(uint8_t version)
    : FullAtom("cloc", version, 0)
{
    m_chunksNum = 0;
    m_reserveTotal = 0;
    m_chunksLocation.clear();
}

void ChunkLocationAtom::SetSpaceReserve(size_t reserveTotal)
{
    m_reserveTotal = reserveTotal;
}

void ChunkLocationAtom::AddChunkLocation(const ChunkLocation& chunkLocation)
{
    m_chunksLocation.push_back(chunkLocation);
    //assert(m_chunksNum == 0 || m_chunksLocation.size() < m_chunksNum);
    //assert(m_reserveTotal == 0 || m_chunksLocation.size() < m_reserveTotal);
}

std::vector<ChunkLocationAtom::ChunkLocation> ChunkLocationAtom::GetChunksLocation() const
{
    return m_chunksLocation;
}

void ChunkLocationAtom::ToStream(Stream& str)
{
    const uint16_t locationSize = 2 + 4 + 4;
    const uint16_t reserveBytes = static_cast<uint16_t>((m_reserveTotal - m_chunksLocation.size()) * locationSize);

    // Write Atom headers
    WriteFullAtomHeader(str);
    str.Write16(m_chunksNum);
    for (uint32_t i = 0; i < m_chunksNum; i++)
    {
        str.Write16(m_chunksLocation[i].chunkIndex);
        str.Write32(m_chunksLocation[i].chunkOffset);
        str.Write32(m_chunksLocation[i].chunkSize);
    }
    // Update the size of the cloc Atom
    UpdateSize(str);

    if (reserveBytes != 0)
    {
        str.Write16(reserveBytes);
        str.Write32(FourCCInt("free").GetUInt32());
        str.Write32(0);

        for (uint32_t i = 1; i < m_reserveTotal - m_chunksLocation.size(); i++)
        {
            str.Write16(0);
            str.Write32(0);
            str.Write32(0);
        }
    }
}

void ChunkLocationAtom::FromStream(Stream& str)
{
    //  First parse the Atom header
    ParseFullAtomHeader(str);

    m_chunksNum = str.Read16();
    m_chunksLocation.clear();
    m_chunksLocation.reserve(m_chunksNum);
    for (uint32_t i = 0; i < m_chunksNum; i++)
    {
        ChunkLocation chunkLocation;
        chunkLocation.chunkIndex = str.Read16();
        chunkLocation.chunkOffset = str.Read32();
        chunkLocation.chunkSize = str.Read32();
        m_chunksLocation.push_back(chunkLocation);
    }
}

VCD_MP4_END