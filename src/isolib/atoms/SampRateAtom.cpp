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
//! \file:   SampRateAtom.cpp
//! \brief:  SampRateAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "SampRateAtom.h"

VCD_MP4_BEGIN

SamplingRateAtom::SamplingRateAtom()
    : FullAtom("srat", 0, 0)
    , m_samplingRate(0)
{
}

SamplingRateAtom::SamplingRateAtom(const SamplingRateAtom& atom)
    : FullAtom(atom.GetType(), 0, 0)
    , m_samplingRate(atom.m_samplingRate)
{
}

std::uint32_t SamplingRateAtom::GetSamplingRate() const
{
    return m_samplingRate;
}

void SamplingRateAtom::SetSamplingRate(std::uint32_t samplingRate)
{
    m_samplingRate = samplingRate;
}

void SamplingRateAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);

    str.Write32(m_samplingRate);

    UpdateSize(str);
}

void SamplingRateAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);
    m_samplingRate = str.Read32();
}

VCD_MP4_END