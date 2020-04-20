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
//! \file:   VideoMediaHeaderAtom.cpp
//! \brief:  VideoMediaHeaderAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "VideoMediaHeaderAtom.h"
#include "Stream.h"

VCD_MP4_BEGIN

VideoMediaHeaderAtom::VideoMediaHeaderAtom()
    : FullAtom("vmhd", 0, 1)
{
}

void VideoMediaHeaderAtom::ToStream(Stream& str)
{
    // Write Atom headers
    WriteFullAtomHeader(str);

    str.Write16(0);  // graphicsmode = 0
    str.Write16(0);  // opcolor = {0, 0, 0}
    str.Write16(0);
    str.Write16(0);

    // Update the size of the movie Atom
    UpdateSize(str);
}

void VideoMediaHeaderAtom::FromStream(Stream& str)
{
    Stream subBitstr;

    //  First parse the Atom header
    ParseFullAtomHeader(str);

    str.Read16();  // graphicsmode = 0
    str.Read16();  // opcolor = {0, 0, 0}
    str.Read16();
    str.Read16();
}

VCD_MP4_END