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
//! \file:   MediaInfoAtom.cpp
//! \brief:  MediaInfoAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "MediaInfoAtom.h"


VCD_MP4_BEGIN

MediaInformationAtom::MediaInformationAtom()
    : Atom("minf")
    , m_mediaType(MediaType::Null)
    , m_videoMediaHeaderAtom()
    , m_soundMediaHeaderAtom()
    , m_nullMediaHeaderAtom()
    , m_dataInfoAtom()
    , m_sampleTableAtom()
{
}

void MediaInformationAtom::SetMediaType(MediaType type)
{
    m_mediaType = type;
}

MediaInformationAtom::MediaType MediaInformationAtom::GetMediaType() const
{
    return m_mediaType;
}

const VideoMediaHeaderAtom& MediaInformationAtom::GetVideoMediaHeaderAtom() const
{
    return m_videoMediaHeaderAtom;
}

VideoMediaHeaderAtom& MediaInformationAtom::GetVideoMediaHeaderAtom()
{
    return m_videoMediaHeaderAtom;
}

const DataInformationAtom& MediaInformationAtom::GetDataInformationAtom() const
{
    return m_dataInfoAtom;
}

DataInformationAtom& MediaInformationAtom::GetDataInformationAtom()
{
    return m_dataInfoAtom;
}

const SampleTableAtom& MediaInformationAtom::GetSampleTableAtom() const
{
    return m_sampleTableAtom;
}

SampleTableAtom& MediaInformationAtom::GetSampleTableAtom()
{
    return m_sampleTableAtom;
}

const NullMediaHeaderAtom& MediaInformationAtom::GetNullMediaHeaderAtom() const
{
    return m_nullMediaHeaderAtom;
}

NullMediaHeaderAtom& MediaInformationAtom::GetNullMediaHeaderAtom()
{
    return m_nullMediaHeaderAtom;
}

const SoundMediaHeaderAtom& MediaInformationAtom::GetSoundMediaHeaderAtom() const
{
    return m_soundMediaHeaderAtom;
}

SoundMediaHeaderAtom& MediaInformationAtom::GetSoundMediaHeaderAtom()
{
    return m_soundMediaHeaderAtom;
}

void MediaInformationAtom::ToStream(Stream& str)
{
    // Write Atom headers
    WriteAtomHeader(str);

    // Write other Atoms contained in the movie Atom
    switch (m_mediaType)
    {
    case MediaType::Null:
    {
        m_nullMediaHeaderAtom.ToStream(str);
        break;
    }
    case MediaType::Video:
    {
        m_videoMediaHeaderAtom.ToStream(str);
        break;
    }
    case MediaType::Sound:
    {
        m_soundMediaHeaderAtom.ToStream(str);
        break;
    }
        // @todo should also support hmhd, sthd
    }
    m_dataInfoAtom.ToStream(str);
    m_sampleTableAtom.ToStream(str);

    // Update the size of the movie Atom
    UpdateSize(str);
}

void MediaInformationAtom::FromStream(Stream& str)
{
    //  First parse the Atom header
    ParseAtomHeader(str);

    // if there a data available in the file
    while (str.BytesRemain() > 0)
    {
        // Extract contained Atom bitstream and type
        FourCCInt AtomType;
        Stream subBitstr = str.ReadSubAtomStream(AtomType);

        // Handle this Atom based on the type
        if (AtomType == "vmhd")
        {
            m_videoMediaHeaderAtom.FromStream(subBitstr);
            SetMediaType(MediaType::Video);
        }
        else if (AtomType == "smhd")
        {
            m_soundMediaHeaderAtom.FromStream(subBitstr);
            SetMediaType(MediaType::Sound);
        }
        else if (AtomType == "nmhd")
        {
            m_nullMediaHeaderAtom.FromStream(subBitstr);
            SetMediaType(MediaType::Null);
        }
        else if (AtomType == "dinf")
        {
            m_dataInfoAtom.FromStream(subBitstr);
        }
        else if (AtomType == "stbl")
        {
            m_sampleTableAtom.FromStream(subBitstr);
        }
        else
        {
			char type[4];
            AtomType.GetString().copy(type, 4, 0);
            ISO_LOG(LOG_WARNING, "Skipping an unsupported Atom '%s' inside MediaInformationAtom.\n", type);
        }
    }
}

VCD_MP4_END
