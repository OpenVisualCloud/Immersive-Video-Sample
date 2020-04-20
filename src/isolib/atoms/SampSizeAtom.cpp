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
//! \file:   SampSizeAtom.cpp
//! \brief:  SampSizeAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "SampSizeAtom.h"


VCD_MP4_BEGIN

SampleSizeAtom::SampleSizeAtom()
    : FullAtom("stsz", 0, 0)
    , m_sampleSize(0)
    , m_sampleNum(0)
    , m_entrySize()
{
}

void SampleSizeAtom::SetEntrySize(std::vector<uint32_t> sample_sizes)
{
    m_entrySize = sample_sizes;
}

std::vector<uint32_t> SampleSizeAtom::GetEntrySize() const
{
    // Fill the entry size in a lazy fashion to avoid doing too much work
    // if the Atom ends up being discarded due to invalid data determined
    // from othere sources
    if (m_entrySize.size() == 0 && m_sampleSize != 0)
    {
        for (uint32_t i = 0; i < m_sampleNum; i++)
        {
            m_entrySize.push_back(m_sampleSize);
        }
    }
    return m_entrySize;
}

void SampleSizeAtom::ToStream(Stream& str)
{
    // Write Atom headers
    WriteFullAtomHeader(str);
    str.Write32(m_sampleSize);
    str.Write32(m_sampleNum);  // number of samples in the track
    for (uint32_t i = 0; i < m_sampleNum; i++)
    {
        str.Write32(m_entrySize.at(i));
    }

    // Update the size of the movie Atom
    UpdateSize(str);
}

void SampleSizeAtom::FromStream(Stream& str)
{
    //  First parse the Atom header
    ParseFullAtomHeader(str);

    m_sampleSize  = str.Read32();
    m_sampleNum = str.Read32();

    if (m_sampleSize == 0)
    {
        for (uint32_t i = 0; i < m_sampleNum; i++)
        {
            if (m_sampleSize == 0)
            {
                m_entrySize.push_back(str.Read32());
            }
        }
    }
}

VCD_MP4_END