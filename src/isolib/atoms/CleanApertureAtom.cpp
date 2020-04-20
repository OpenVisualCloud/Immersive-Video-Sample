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
//! \file:   CleanApertureAtom.cpp
//! \brief:  CleanApertureAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "CleanApertureAtom.h"
#include "Stream.h"

VCD_MP4_BEGIN

CleanApertureAtom::CleanApertureAtom()
    : Atom("clap")
    , m_width()
    , m_height()
    , m_horizOffset()
    , m_vertOffset()
{
}

void CleanApertureAtom::ToStream(Stream& output)
{
    WriteAtomHeader(output);
    output.Write32(m_width.numerator);
    output.Write32(m_width.denominator);
    output.Write32(m_height.numerator);
    output.Write32(m_height.denominator);
    output.Write32(m_horizOffset.numerator);
    output.Write32(m_horizOffset.denominator);
    output.Write32(m_vertOffset.numerator);
    output.Write32(m_vertOffset.denominator);
    UpdateSize(output);
}

void CleanApertureAtom::FromStream(Stream& input)
{
    ParseAtomHeader(input);
    m_width.numerator         = input.Read32();
    m_width.denominator       = input.Read32();
    m_height.numerator        = input.Read32();
    m_height.denominator      = input.Read32();
    m_horizOffset.numerator   = input.Read32();
    m_horizOffset.denominator = input.Read32();
    m_vertOffset.numerator    = input.Read32();
    m_vertOffset.denominator  = input.Read32();
}

VCD_MP4_END