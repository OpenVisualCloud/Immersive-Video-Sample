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
//! \file:   DataInfoAtom.cpp
//! \brief:  DataInfoAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "DataInfoAtom.h"

#include <stdexcept>

VCD_MP4_BEGIN

DataInformationAtom::DataInformationAtom()
    : Atom("dinf")
    , m_dataReferenceAtom()
{
}

std::uint16_t DataInformationAtom::AddDataEntryAtom(std::shared_ptr<DataEntryAtom> dataEntryAtom)
{
    return static_cast<std::uint16_t>(m_dataReferenceAtom.AddEntry(dataEntryAtom));
}

void DataInformationAtom::ToStream(Stream& str)
{
    WriteAtomHeader(str);
    m_dataReferenceAtom.ToStream(str);
    UpdateSize(str);
}

void DataInformationAtom::FromStream(Stream& str)
{
    ParseAtomHeader(str);

    if (str.BytesRemain() > 0)
    {
        FourCCInt AtomType;
        Stream subBitstr = str.ReadSubAtomStream(AtomType);
        m_dataReferenceAtom.FromStream(subBitstr);
    }
    else
    {
        ISO_LOG(LOG_ERROR, "Read an empty dinf Atom.\n");
    }
}

VCD_MP4_END