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
//! \file:   UserDataAtom.cpp
//! \brief:  UserDataAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "UserDataAtom.h"
#include "Stream.h"

VCD_MP4_BEGIN

UserDataAtom::UserDataAtom()
    : Atom("udta")
{
}

/** @brief Retrieve the Atoms inside the Atom **/
bool UserDataAtom::GetAtom(Atom& atom) const
{
    FourCCInt AtomType = atom.GetType();
    auto bsIt         = m_bitStreams.find(AtomType);
    if (bsIt != m_bitStreams.end())
    {
        // Copy as Atom::FromStreamAtom takes a non-const argument
        Stream bs = bsIt->second;
        atom.FromStream(bs);
        return true;
    }
    else
    {
        return false;
    }
}

void UserDataAtom::AddAtom(Atom& atom)
{
    Stream bitstream;
    atom.ToStream(bitstream);
    bitstream.Reset();
    m_bitStreams[atom.GetType()] = std::move(bitstream);
}

void UserDataAtom::ToStream(Stream& str)
{
    WriteAtomHeader(str);

    for (auto& typeAndBitstream : m_bitStreams)
    {
        str.WriteStream(typeAndBitstream.second);
    }

    UpdateSize(str);
}

void UserDataAtom::FromStream(Stream& str)
{
    ParseAtomHeader(str);

    while (str.BytesRemain() > 0)
    {
        FourCCInt AtomType;
        Stream subBitstr = str.ReadSubAtomStream(AtomType);

        m_bitStreams[AtomType] = std::move(subBitstr);
    }
}

VCD_MP4_END