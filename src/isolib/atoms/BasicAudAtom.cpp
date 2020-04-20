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
//! \file:   BasicAudAtom.cpp
//! \brief:  BasicAudAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "BasicAudAtom.h"

VCD_MP4_BEGIN


SpatialAudioAtom::SpatialAudioAtom()
    : Atom("SA3D")
    , m_version(0)
    , m_type(0)
    , m_order(1)
    , m_channelOrder(0)
    , m_norm(0)
    , m_channelMap()
{
}

void SpatialAudioAtom::SetChannelMap(const std::vector<uint32_t>& channelMap)
{
    m_channelMap.clear();
    m_channelMap = channelMap;
}

std::vector<uint32_t> SpatialAudioAtom::GetChannelMap() const
{
    return m_channelMap;
}

void SpatialAudioAtom::ToStream(Stream& str)
{
    WriteAtomHeader(str);

    str.Write8(m_version);
    str.Write8(m_type);
    str.Write32(m_order);
    str.Write8(m_channelOrder);
    str.Write8(m_norm);
    str.Write32(static_cast<uint32_t>(m_channelMap.size()));
    for (uint32_t i = 0; i < m_channelMap.size(); i++)
    {
        str.Write32(m_channelMap.at(i));
    }

    UpdateSize(str);
}

void SpatialAudioAtom::FromStream(Stream& str)
{
    ParseAtomHeader(str);

    m_version                  = str.Read8();
    m_type            = str.Read8();
    m_order           = str.Read32();
    m_channelOrder = str.Read8();
    m_norm   = str.Read8();
    uint32_t numberOfChannels = str.Read32();
    m_channelMap.clear();
    for (uint32_t i = 0; i < numberOfChannels; i++)
    {
        uint32_t value = str.Read32();
        m_channelMap.push_back(value);
    }
}

NonDiegeticAudioAtom::NonDiegeticAudioAtom()
    : Atom("SAND")
    , m_version(0)
{
}

void NonDiegeticAudioAtom::ToStream(Stream& str)
{
    WriteAtomHeader(str);

    str.Write8(m_version);

    UpdateSize(str);
}

void NonDiegeticAudioAtom::FromStream(Stream& str)
{
    ParseAtomHeader(str);

    m_version = str.Read8();
}

VCD_MP4_END