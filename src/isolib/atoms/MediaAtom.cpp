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
//! \file:   MediaAtom.cpp
//! \brief:  MediaAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "MediaAtom.h"


VCD_MP4_BEGIN

MediaAtom::MediaAtom()
    : Atom("mdia")
    , m_mediaHeaderAtom()
    , m_handlerAtom()
    , m_mediaInformationAtom()
{
}

const MediaHeaderAtom& MediaAtom::GetMediaHeaderAtom() const
{
    return m_mediaHeaderAtom;
}

MediaHeaderAtom& MediaAtom::GetMediaHeaderAtom()
{
    return m_mediaHeaderAtom;
}

const HandlerAtom& MediaAtom::GetHandlerAtom() const
{
    return m_handlerAtom;
}

HandlerAtom& MediaAtom::GetHandlerAtom()
{
    return m_handlerAtom;
}

const MediaInformationAtom& MediaAtom::GetMediaInformationAtom() const
{
    return m_mediaInformationAtom;
}

MediaInformationAtom& MediaAtom::GetMediaInformationAtom()
{
    return m_mediaInformationAtom;
}

void MediaAtom::ToStream(Stream& str)
{
    // Write Atom headers
    WriteAtomHeader(str);

    // Write other Atoms contained in the movie Atom
    m_mediaHeaderAtom.ToStream(str);
    m_handlerAtom.ToStream(str);
    m_mediaInformationAtom.ToStream(str);

    // Update the size of the movie Atom
    UpdateSize(str);
}

void MediaAtom::FromStream(Stream& str)
{
    //  First parse the Atom header
    ParseAtomHeader(str);

    // if there a data available in the file
    while (str.BytesRemain() > 0)
    {
        // Extract contained Atom bitstream and type
        FourCCInt AtomType;
        Stream subBitstr = str.ReadSubAtomStream(AtomType);

        if (AtomType == "mdhd")
        {
            m_mediaHeaderAtom.FromStream(subBitstr);
        }
        else if (AtomType == "hdlr")
        {
            m_handlerAtom.FromStream(subBitstr);
        }
        else if (AtomType == "minf")
        {
            m_mediaInformationAtom.FromStream(subBitstr);
        }
        else
        {
			char type[4];
            AtomType.GetString().copy(type, 4, 0);
            ISO_LOG(LOG_WARNING, "Skipping an unsupported Atom '%s' inside MediaAtom.\n", type);
        }
    }
}

VCD_MP4_END
