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
//! \file:   AudSampEntryAtom.cpp
//! \brief:  AudSampEntryAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "AudSampEntryAtom.h"
#include <stdexcept>
#include <string>


VCD_MP4_BEGIN

AudioSampleEntryAtom::AudioSampleEntryAtom(FourCCInt codingname)
    : SampleEntryAtom(codingname)
    , m_version(0)
    , m_channelNumber(0)
    , m_sampleSize(0)
    , m_sampleRate(0)
    , m_hasChannelLayoutAtom(false)
    , m_hasSamplingRateAtom(false)
    , m_channelLayoutAtom()
    , m_samplingRateAtom()
{
}

AudioSampleEntryAtom::AudioSampleEntryAtom(const AudioSampleEntryAtom& Atom)
    : SampleEntryAtom(Atom)
    , m_version(Atom.m_version)
    , m_channelNumber(Atom.m_channelNumber)
    , m_sampleSize(Atom.m_sampleSize)
    , m_sampleRate(Atom.m_sampleRate)
    , m_hasChannelLayoutAtom(Atom.m_hasChannelLayoutAtom)
    , m_hasSamplingRateAtom(Atom.m_hasSamplingRateAtom)
    , m_channelLayoutAtom(Atom.m_channelLayoutAtom)
    , m_samplingRateAtom(Atom.m_samplingRateAtom)
{
}


void AudioSampleEntryAtom::SetVersion(std::uint16_t version)
{
    if (version == 0 || version == 1)
    {
        m_version = version;
    }
    else
    {
        ISO_LOG(LOG_ERROR, "AudioSampleEntryAtom::SetVersion Error: trying to set value other than 0 or 1\n");
        throw Exception();
    }
}

std::uint16_t AudioSampleEntryAtom::GetVersion() const
{
    return m_version;
}

void AudioSampleEntryAtom::SetChannelCount(std::uint16_t channelCnt)
{
    m_channelNumber = channelCnt;
}

std::uint16_t AudioSampleEntryAtom::GetChannelCount() const
{
    return m_channelNumber;
}

void AudioSampleEntryAtom::SetSampleSize(std::uint16_t sampSize)
{
    m_sampleSize = sampSize;
}

std::uint16_t AudioSampleEntryAtom::GetSampleSize() const
{
    return m_sampleSize;
}

std::uint32_t AudioSampleEntryAtom::GetSampleRate() const
{
    if (m_version == 1 && m_hasSamplingRateAtom)
    {
        return m_samplingRateAtom.GetSamplingRate();
    }
    else
    {
        return m_sampleRate;
    }
}

void AudioSampleEntryAtom::SetSampleRate(std::uint32_t samplerate)
{
    m_sampleRate = samplerate;
}

bool AudioSampleEntryAtom::HasChannelLayoutAtom()
{
    return m_hasChannelLayoutAtom;
}

ChannelLayoutAtom& AudioSampleEntryAtom::GetChannelLayoutAtom()
{
    return m_channelLayoutAtom;
}

void AudioSampleEntryAtom::SetChannelLayoutAtom(ChannelLayoutAtom& channelLayoutAtom)
{
    m_channelLayoutAtom    = channelLayoutAtom;
    m_hasChannelLayoutAtom = true;
}

bool AudioSampleEntryAtom::HasSamplingRateAtom()
{
    if (m_version == 1)
    {
        return m_hasSamplingRateAtom;
    }
    else
    {
        return false;
    }
}

SamplingRateAtom& AudioSampleEntryAtom::GetSamplingRateAtom()
{
    if (m_version == 1)
    {
        return m_samplingRateAtom;
    }
    else
    {
        ISO_LOG(LOG_ERROR, "AudioSampleEntryAtom::GetSamplingRateAtom Error: trying to GetSamplingRateAtom from version other than 1\n");
        throw Exception();
    }
}

void AudioSampleEntryAtom::SetSamplingRateAtom(SamplingRateAtom& samplingRateAtom)
{
    this->SetVersion(1);
    m_samplingRateAtom    = samplingRateAtom;
    m_hasSamplingRateAtom = true;
}

void AudioSampleEntryAtom::ToStream(Stream& str)
{
    SampleEntryAtom::ToStream(str);

    if (m_version == 1)
    {
        str.Write16(m_version);
        str.Write16(0);  // reserved = 0
    }
    else
    {
        str.Write32(0);  // reserved = 0
    }
    str.Write32(0);                  // reserved = 0
    str.Write16(m_channelNumber);      // number of channels 1 (mono) or 2 (stereo)
    str.Write16(m_sampleSize);        // in bits and takes default value of 16
    str.Write16(0);                  // pre_defined = 0
    str.Write16(0);                  // reserved = 0
    str.Write32(m_sampleRate << 16);  // 32bit field expressed as 16.16 fixed-point number (hi.lo)

    if (m_version == 1)
    {
        if (m_hasSamplingRateAtom)
        {
            m_samplingRateAtom.ToStream(str);
        }
    }

    if (m_hasChannelLayoutAtom)
    {
        m_channelLayoutAtom.ToStream(str);
    }

    UpdateSize(str);
}

void AudioSampleEntryAtom::FromStream(Stream& str)
{
    SampleEntryAtom::FromStream(str);

    m_version = str.Read16();  // in case of v0 Atom this is first half of 32bit reserved = 0
    if (m_version != 1 && m_version != 0)
    {
        ISO_LOG(LOG_ERROR, "AudioSampleEntryV1Atom::FromStreamAtom Error: trying to read version other than 0 or 1\n");
        throw Exception();
    }

    str.Read16();
    str.Read32();
    m_channelNumber = str.Read16();
    m_sampleSize   = str.Read16();
    str.Read16();
    str.Read16();
    m_sampleRate = (str.Read32() >> 16);

    std::uint64_t revertOffset = ~0u;

    while (str.BytesRemain() > 0)
    {
        const std::uint64_t startOffset = str.GetPos();
        FourCCInt AtomType;
        Stream subBitstr = str.ReadSubAtomStream(AtomType);

        if (m_version == 1 && AtomType == "srat")
        {
            m_hasSamplingRateAtom = true;
            m_samplingRateAtom.FromStream(subBitstr);
        }
        else if (AtomType == "chnl")
        {
            m_hasChannelLayoutAtom = true;
            m_channelLayoutAtom.SetChannelNumber(m_channelNumber);
            m_channelLayoutAtom.FromStream(subBitstr);
        }
        else if (AtomType == "esds")
        {
            revertOffset = startOffset;
        }
    }

    if (revertOffset != ~0u)
    {
        str.SetPos(revertOffset);
    }
}

AudioSampleEntryAtom* AudioSampleEntryAtom::Clone() const
{
    return nullptr;
}

const Atom* AudioSampleEntryAtom::GetConfigurationAtom() const
{
    ISO_LOG(LOG_ERROR, "AudioSampleEntryAtom::GetConfigurationAtom() not impelmented \n");
    return nullptr;
}

const DecoderConfigurationRecord* AudioSampleEntryAtom::GetConfigurationRecord() const
{
    ISO_LOG(LOG_ERROR, "AudioSampleEntryAtom::GetConfigurationRecord() not impelmented \n");
    return nullptr;
}

VCD_MP4_END
