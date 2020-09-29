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
//! \file:   SampToChunkAtom.cpp
//! \brief:  SampToChunkAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "SampToChunkAtom.h"


#include <limits>
#include <stdexcept>

VCD_MP4_BEGIN

SampleToChunkAtom::SampleToChunkAtom()
    : FullAtom("stsc", 0, 0)
    , m_runOfChunks()
    , m_maxSampleNum(-1)
{
}

bool SampleToChunkAtom::GetSampleDescrIndex(std::uint32_t sampleIndex, std::uint32_t& sampleDescriptionIdx) const
{
    if (sampleIndex >= m_decodedEntries.size())
    {
        return false;
    }

    sampleDescriptionIdx = m_decodedEntries.at(sampleIndex).sampleDescrIndex;
    return true;
}

bool SampleToChunkAtom::GetSampleChunkIndex(std::uint32_t sampleIndex, std::uint32_t& chunkIdx) const
{
    if (sampleIndex >= m_decodedEntries.size())
    {
        return false;
    }

    chunkIdx = m_decodedEntries.at(sampleIndex).chunkIndex;
    return true;
}

void SampleToChunkAtom::SetSampleNumMaxSafety(int64_t maxSampleNum)
{
    m_maxSampleNum = maxSampleNum;
}

void SampleToChunkAtom::AddChunkEntry(const ChunkEntry& chunkEntry)
{
    m_runOfChunks.push_back(chunkEntry);
}

void SampleToChunkAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);

    str.Write32(static_cast<std::uint32_t>(m_runOfChunks.size()));
    for (const auto& run : m_runOfChunks)
    {
        str.Write32(run.firstChunk);
        str.Write32(run.oneChunkSamples);
        str.Write32(run.sampleDescrIndex);
    }

    // Update the size of the movie Atom
    UpdateSize(str);
}

void SampleToChunkAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);

    const uint32_t entryCount = str.Read32();
    for (uint32_t i = 0; i < entryCount; ++i)
    {
        ChunkEntry chunkEntry;
        chunkEntry.firstChunk      = str.Read32();
        chunkEntry.oneChunkSamples = str.Read32();

        if (m_maxSampleNum != -1 && (chunkEntry.oneChunkSamples > m_maxSampleNum))
        {
            ISO_LOG(LOG_ERROR, "SampleToChunkAtom::FromStreamAtom sampChunk is larger than total number of samples\n");
            throw Exception();
        }

        chunkEntry.sampleDescrIndex = str.Read32();
        m_runOfChunks.push_back(chunkEntry);
    }
}

uint32_t SampleToChunkAtom::GetSampleNumLowerBound(uint32_t pCount) const
{
    if (pCount == 0)
    {
        // nothing to do.
        return 0;
    }

    uint64_t sampleNum = 0;

    if (m_runOfChunks.at(0).firstChunk != 1)
    {
        ISO_LOG(LOG_ERROR, "SampleToChunkAtom first entry first_chunk != 1\n");
        throw Exception();
    }

    for (unsigned int pIndex = 0; pIndex < m_runOfChunks.size(); ++pIndex)
    {
        const uint32_t firstChunk      = m_runOfChunks.at(pIndex).firstChunk;
        const uint32_t sampChunk = m_runOfChunks.at(pIndex).oneChunkSamples;

        uint32_t pChunkRep = 1;
        if ((pIndex + 1) < m_runOfChunks.size())
        {
            if (m_runOfChunks.at(pIndex + 1).firstChunk <= firstChunk)
            {
                ISO_LOG(LOG_ERROR, "Invalid first_chunk value in SampleToChunkAtom entry. Must be greater than previous\n");
                throw Exception();
            }

            pChunkRep = m_runOfChunks.at(pIndex + 1).firstChunk - firstChunk;
        }
        else if (pIndex == m_runOfChunks.size() - 1)
        {
            // handle last entry.
            pChunkRep = pCount - m_runOfChunks.at(pIndex).firstChunk + 1;
        }

        sampleNum += uint64_t(pChunkRep) * sampChunk;
    }

    if (sampleNum <= std::numeric_limits<uint32_t>::max())
    {
        return static_cast<uint32_t>(sampleNum);
    }
    else
    {
        ISO_LOG(LOG_ERROR, "SampleToChunkAtom has >= 2^32 samples\n");
        throw Exception();
    }
}

void SampleToChunkAtom::DecodeEntries(std::uint32_t pCount)
{
    m_decodedEntries.clear();

    if (m_runOfChunks.size() == 0 || pCount == 0)
    {
        // nothing to do.
        return;
    }

    if (m_runOfChunks.at(0).firstChunk != 1)
    {
        ISO_LOG(LOG_ERROR, "SampleToChunkAtom first entry first_chunk != 1\n");
        throw Exception();
    }

    for (unsigned int pIndex = 0; pIndex < m_runOfChunks.size(); ++pIndex)
    {
        const uint32_t firstChunk             = m_runOfChunks.at(pIndex).firstChunk;
        const uint32_t sampChunk        = m_runOfChunks.at(pIndex).oneChunkSamples;
        const uint32_t sampDescrIndex = m_runOfChunks.at(pIndex).sampleDescrIndex;

        uint32_t pChunkRep = 1;
        if ((pIndex + 1) < m_runOfChunks.size())
        {
            if (m_runOfChunks.at(pIndex + 1).firstChunk <= firstChunk)
            {
                ISO_LOG(LOG_ERROR, "Invalid first_chunk value in SampleToChunkAtom entry. Must be greater than previous\n");
                throw Exception();
            }

            pChunkRep = m_runOfChunks.at(pIndex + 1).firstChunk - firstChunk;
        }
        else if (pIndex == m_runOfChunks.size() - 1)
        {
            // handle last entry.
            pChunkRep = pCount - m_runOfChunks.at(pIndex).firstChunk + 1;
        }

        if (m_maxSampleNum != -1 && std::uint64_t(sampChunk) * pChunkRep > std::uint64_t(m_maxSampleNum))
        {
            ISO_LOG(LOG_ERROR, "SampleToChunkAtom::FromStreamAtom sampChunk is larger than total number of samples\n");
            throw Exception();
        }

        DecEntry entry;
        entry.oneChunkSamples        = sampChunk;
        entry.sampleDescrIndex = sampDescrIndex;
        for (unsigned int i = 0; i < pChunkRep; ++i)
        {
            entry.chunkIndex = firstChunk + i;
            for (unsigned int k = 0; k < sampChunk; ++k)
            {
                m_decodedEntries.push_back(entry);
            }
        }
    }
}

VCD_MP4_END
