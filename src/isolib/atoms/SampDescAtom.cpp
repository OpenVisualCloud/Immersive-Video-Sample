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
//! \file:   SampDescAtom.cpp
//! \brief:  SampDescAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "SampDescAtom.h"

#include "AvcSampEntry.h"
#include "HevcSampEntry.h"
#include "InitViewOrientationSampEntry.h"

#include "Mp4AudSampEntryAtom.h"
#include "Mp4VisualSampEntryAtom.h"
#include "RestSchemeInfoAtom.h"
#include "UriMetaSampEntryAtom.h"

VCD_MP4_BEGIN

SampleDescriptionAtom::SampleDescriptionAtom()
    : FullAtom("stsd", 0, 0)
{
}

void SampleDescriptionAtom::AddSampleEntry(UniquePtr<SampleEntryAtom> sampleEntry)
{
    m_index.push_back(std::move(sampleEntry));
}

void SampleDescriptionAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);
    str.Write32(static_cast<unsigned int>(m_index.size()));

    for (auto& entry : m_index)
    {
        if (!entry)
        {
            ISO_LOG(LOG_ERROR, "ToStreamAtom can not write file\n");
            throw Exception();
        }
        Stream entryBitStr;
        entry->ToStream(entryBitStr);
        str.WriteStream(entryBitStr);
    }
    UpdateSize(str);
}

void SampleDescriptionAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);

    const unsigned int entryCount = str.Read32();

    UniquePtr<RestrictedSchemeInfoAtom> entrysResvAtom;

    for (unsigned int i = 0; i < entryCount; ++i)
    {
        // Extract contained Atom bitstream and type
        FourCCInt AtomType;
        uint64_t entryStart      = str.GetPos();
        Stream entryStream = str.ReadSubAtomStream(AtomType);

        // Add new sample entry types based on handler when necessary
        if (AtomType == "hvc1" || AtomType == "hev1" || AtomType == "hvc2")
        {
            UniquePtr<HevcSampleEntry, SampleEntryAtom> hevcSampleEntry(new HevcSampleEntry());
            hevcSampleEntry->FromStream(entryStream);

            m_index.push_back(std::move(hevcSampleEntry));
        }
        else if (AtomType == "avc1" || AtomType == "avc3")
        {
            UniquePtr<AvcSampleEntry, SampleEntryAtom> avcSampleEntry(new AvcSampleEntry());
            avcSampleEntry->FromStream(entryStream);
            m_index.push_back(std::move(avcSampleEntry));
        }
        else if (AtomType == "mp4a")
        {
            UniquePtr<MP4AudioSampleEntryAtom, SampleEntryAtom> mp4AudioSampleEntry(new MP4AudioSampleEntryAtom());
            mp4AudioSampleEntry->FromStream(entryStream);
            m_index.push_back(std::move(mp4AudioSampleEntry));
        }
        else if (AtomType == "urim")
        {
            UniquePtr<UriMetaSampleEntryAtom, SampleEntryAtom> uriMetaSampleEntry(new UriMetaSampleEntryAtom());
            uriMetaSampleEntry->FromStream(entryStream);
            m_index.push_back(std::move(uriMetaSampleEntry));
        }
        else if (AtomType == "invo")
        {
            UniquePtr<InitViewOrient, SampleEntryAtom> invoSampleEntry(new InitViewOrient());
            invoSampleEntry->FromStream(entryStream);
            m_index.push_back(std::move(invoSampleEntry));
        }
        else if (AtomType == "mp4v")
        {
            UniquePtr<MP4VisualSampleEntryAtom, SampleEntryAtom> mp4VisualSampleEntry(new MP4VisualSampleEntryAtom());
            mp4VisualSampleEntry->FromStream(entryStream);
            m_index.push_back(std::move(mp4VisualSampleEntry));
        }
        else if (AtomType == "resv")
        {
            // resv is a special case. It is normal video sample entry, with an additional rinf Atom, which describes how
            // sample entry is encoded etc. First we read info from rinf Atom, rewrite correct sample entry fromat to
            // to the stream replacing "resv" with original format e.g. "avc1". After rewrite stream is rewinded to
            // entry start position and additional rinf Atom is stored along next read sample entry.

            // seek restricted video until rinf Atom to find out original format etc.
            MP4VisualSampleEntryAtom visualSampleEntryHeaderParser;
            visualSampleEntryHeaderParser.VisualSampleEntryAtom::FromStream(entryStream);
            Stream rinfAtomSubBitstream;
            while (entryStream.BytesRemain() > 0)
            {
                FourCCInt resvAtomType;
                rinfAtomSubBitstream = entryStream.ReadSubAtomStream(resvAtomType);
                if (resvAtomType == "rinf")
                {
                    break;
                }
            }

            if (rinfAtomSubBitstream.GetSize() == 0)
            {
                ISO_LOG(LOG_ERROR, "There must be rinf Atom inside resv\n");
                throw Exception();
            }

            entrysResvAtom = MakeUnique<RestrictedSchemeInfoAtom, RestrictedSchemeInfoAtom>();
            entrysResvAtom->FromStream(rinfAtomSubBitstream);

            // rewind & rewrite the sample entry Atom type
            if (entrysResvAtom->GetOriginalFormat() == "resv")
            {
                ISO_LOG(LOG_ERROR, "OriginalFormat cannot be resv\n");
                throw Exception();
            }

            std::uint32_t originalFormat = entrysResvAtom->GetOriginalFormat().GetUInt32();
            str.SetPos(entryStart);
            str.SetByte(entryStart + 4, (uint8_t)(originalFormat >> 24));
            str.SetByte(entryStart + 5, (uint8_t)(originalFormat >> 16));
            str.SetByte(entryStart + 6, (uint8_t)(originalFormat >> 8));
            str.SetByte(entryStart + 7, (uint8_t) originalFormat);

            i--;
            continue;
        }
        else
        {
            char type[4];
            AtomType.GetString().copy(type, 4, 0);
			ISO_LOG(LOG_WARNING, "Skipping unsupported SampleDescriptionAtom entry of type '%s'\n", type);
            // Push nullptr to keep indexing correct, in case it will still be possible to operate with the file.
            m_index.push_back(nullptr);
        }

        // added entry was transformed from revs Atom, add resv info to the entry
        if (entrysResvAtom)
        {
            if (m_index[i])
            {
                m_index[i]->AddRestrictedSchemeInfoAtom(std::move(entrysResvAtom));
            }
            else
            {
                entrysResvAtom.release();
            }
        }
    }
}

VCD_MP4_END
