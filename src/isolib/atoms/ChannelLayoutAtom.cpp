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
//! \file:   ChannelLayoutAtom.cpp
//! \brief:  ChannelLayoutAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "ChannelLayoutAtom.h"
#include <stdexcept>

VCD_MP4_BEGIN

#define CHANNEL_STRUCTURED 1u
#define OBJECT_STRUCTURED 2u

ChannelLayoutAtom::ChannelLayoutAtom()
    : FullAtom("chnl", 0, 0)
    , m_strStructured(0)
    , m_definedLayout(0)
    , m_omittedChannelsMap(0)
    , m_objectCount(0)
    , m_channelNumber(0)
    , m_channelLayouts()
{
}

ChannelLayoutAtom::ChannelLayoutAtom(const ChannelLayoutAtom& atom)
    : FullAtom(atom.GetType(), atom.GetVersion(), atom.GetFlags())
    , m_strStructured(atom.m_strStructured)
    , m_definedLayout(atom.m_definedLayout)
    , m_omittedChannelsMap(atom.m_omittedChannelsMap)
    , m_objectCount(atom.m_objectCount)
    , m_channelNumber(atom.m_channelNumber)
    , m_channelLayouts(atom.m_channelLayouts)
{
}

std::vector<ChannelLayoutAtom::ChannelLayout> ChannelLayoutAtom::GetChannelLayouts() const
{
    return m_channelLayouts;
}

void ChannelLayoutAtom::AddChannelLayout(ChannelLayout& channelLayout)
{
    m_channelLayouts.push_back(channelLayout);
    // set other member variables to mirror fact that m_channelLayouts is present
    m_strStructured |= CHANNEL_STRUCTURED;  // if channellayouts are present then stream is structured.
    m_definedLayout      = 0;                 // if hannellayouts are present then layout is defined.
    m_omittedChannelsMap = 0;
}

std::uint8_t ChannelLayoutAtom::GetStreamStructure() const
{
    return m_strStructured;
}

std::uint8_t ChannelLayoutAtom::GetDefinedLayout() const
{
    return m_definedLayout;
}

void ChannelLayoutAtom::SetDefinedLayout(std::uint8_t definedLayout)
{
    m_definedLayout = definedLayout;
    // set other member variables to mirror fact that m_definedLayout is present
    if (m_definedLayout)
    {
        m_channelLayouts.clear();
    }
    m_strStructured |= CHANNEL_STRUCTURED;
}

std::uint64_t ChannelLayoutAtom::GetOmittedChannelsMap() const
{
    return m_omittedChannelsMap;
}

void ChannelLayoutAtom::SetOmittedChannelsMap(std::uint64_t omittedChannelsMap)
{
    m_omittedChannelsMap = omittedChannelsMap;
    // set other member variables to mirror fact that m_omittedChannelsMap is present
    m_channelLayouts.clear();
    m_strStructured |= CHANNEL_STRUCTURED;
}

std::uint8_t ChannelLayoutAtom::GetObjectCount() const
{
    return m_objectCount;
}

void ChannelLayoutAtom::SetObjectCount(std::uint8_t objectCount)
{
    m_objectCount = objectCount;
    m_strStructured |= OBJECT_STRUCTURED;
}

void ChannelLayoutAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);

    str.Write8(m_strStructured);
    if (m_strStructured & CHANNEL_STRUCTURED)  // channels
    {
        str.Write8(m_definedLayout);
        if (m_definedLayout == 0)
        {
            if (m_channelLayouts.size() != m_channelNumber)
            {
                ISO_LOG(LOG_ERROR, "Size doesn't match in ChannelLayoutAtom\n");
                throw Exception();
            }

            for (std::uint16_t i = 0; i < m_channelLayouts.size(); i++)
            {
                str.Write8(m_channelLayouts.at(i).speakerPosition);
                if (m_channelLayouts[i].speakerPosition == 126)
                {
                    str.Write16(static_cast<std::uint16_t>(m_channelLayouts.at(i).azimuthAngle));
                    str.Write8(static_cast<std::uint8_t>(m_channelLayouts.at(i).elevationAngle));
                }
            }
        }
        else
        {
            str.Write64(m_omittedChannelsMap);
        }
    }
    else if (m_strStructured & OBJECT_STRUCTURED)  // objects
    {
        str.Write8(m_objectCount);
    }

    UpdateSize(str);
}

void ChannelLayoutAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);
    m_strStructured = str.Read8();

    if (m_strStructured & CHANNEL_STRUCTURED)  // Stream carries channels.
    {
        m_definedLayout = str.Read8();
        if (m_definedLayout == 0)
        {
            for (std::uint16_t i = 1; i <= m_channelNumber; i++)
            {
                ChannelLayout channelPosition;
                channelPosition.speakerPosition = str.Read8();
                channelPosition.azimuthAngle         = 0;
                channelPosition.elevationAngle       = 0;
                if (channelPosition.speakerPosition == 126)  // explicit position
                {
                    channelPosition.azimuthAngle   = static_cast<std::int16_t>(str.Read16());
                    channelPosition.elevationAngle = static_cast<std::int8_t>(str.Read8());
                }
                m_channelLayouts.push_back(channelPosition);
            }
        }
        else
        {
            m_omittedChannelsMap = str.Read64();
        }
    }

    if (m_strStructured & OBJECT_STRUCTURED)  // Stream carries objects
    {
        m_objectCount = str.Read8();
    }
}

VCD_MP4_END