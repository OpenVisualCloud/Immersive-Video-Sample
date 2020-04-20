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
//! \file:   HandlerAtom.cpp
//! \brief:  HandlerAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "HandlerAtom.h"


VCD_MP4_BEGIN

HandlerAtom::HandlerAtom()
    : FullAtom("hdlr", 0, 0)
    , m_handlerType()
    , m_name("")
{
}

void HandlerAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);

    str.Write32(0);  // pre_defined = 0
    str.Write32(m_handlerType.GetUInt32());
    str.Write32(0);  // reserved = 0
    str.Write32(0);  // reserved = 0
    str.Write32(0);  // reserved = 0
    str.WriteZeroEndString(m_name);

    UpdateSize(str);
}

void HandlerAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);

    str.Read32();  // pre_defined = 0
    m_handlerType = str.Read32();
    str.Read32();  // reserved = 0
    str.Read32();  // reserved = 0
    str.Read32();  // reserved = 0
    str.ReadZeroEndString(m_name);
}

VCD_MP4_END