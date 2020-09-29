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
//! \file:   SampTableAtom.cpp
//! \brief:  SampTableAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "SampTableAtom.h"

using namespace std;

VCD_MP4_BEGIN

SampleTableAtom::SampleTableAtom()
    : Atom("stbl")
    , m_sampleDescrAtom()
    , m_timeToSampAtom()
    , m_sampToChunkAtom()
    , m_chunkOffsetAtom()
    , m_sampSizeAtom()
    , m_compToDecodeAtom(nullptr)
    , m_sampGroupDescrAtom(nullptr)
    , m_hasSyncSampleAtom(false)
{
}

const SampleDescriptionAtom& SampleTableAtom::GetSampleDescriptionAtom() const
{
    return m_sampleDescrAtom;
}

SampleDescriptionAtom& SampleTableAtom::GetSampleDescriptionAtom()
{
    return m_sampleDescrAtom;
}

const TimeToSampleAtom& SampleTableAtom::GetTimeToSampleAtom() const
{
    return m_timeToSampAtom;
}

TimeToSampleAtom& SampleTableAtom::GetTimeToSampleAtom()
{
    return m_timeToSampAtom;
}

void SampleTableAtom::SetCompositionOffsetAtom(const CompositionOffsetAtom& compositionOffsetAtom)
{
    if (m_compOffsetAtom == nullptr)
    {
        m_compOffsetAtom = MakeShared<CompositionOffsetAtom>(compositionOffsetAtom);
    }
    else
    {
        *m_compOffsetAtom = compositionOffsetAtom;
    }
}

std::shared_ptr<const CompositionOffsetAtom> SampleTableAtom::GetCompositionOffsetAtom() const
{
    return m_compOffsetAtom;
}

void SampleTableAtom::SetCompositionToDecodeAtom(const CompositionToDecodeAtom& compositionToDecodeAtom)
{
    if (m_compToDecodeAtom == nullptr)
    {
        m_compToDecodeAtom = MakeShared<CompositionToDecodeAtom>(compositionToDecodeAtom);
    }
    else
    {
        *m_compToDecodeAtom = compositionToDecodeAtom;
    }
}

std::shared_ptr<const CompositionToDecodeAtom> SampleTableAtom::GetCompositionToDecodeAtom() const
{
    return m_compToDecodeAtom;
}

const SampleToChunkAtom& SampleTableAtom::GetSampleToChunkAtom() const
{
    return m_sampToChunkAtom;
}

SampleToChunkAtom& SampleTableAtom::GetSampleToChunkAtom()
{
    return m_sampToChunkAtom;
}

const ChunkOffsetAtom& SampleTableAtom::GetChunkOffsetAtom() const
{
    return m_chunkOffsetAtom;
}

ChunkOffsetAtom& SampleTableAtom::GetChunkOffsetAtom()
{
    return m_chunkOffsetAtom;
}

const SampleSizeAtom& SampleTableAtom::GetSampleSizeAtom() const
{
    return m_sampSizeAtom;
}

SampleSizeAtom& SampleTableAtom::GetSampleSizeAtom()
{
    return m_sampSizeAtom;
}

void SampleTableAtom::SetSampleGroupDescriptionAtom(UniquePtr<SampleGroupDescriptionAtom> sgpd)
{
    m_sampGroupDescrAtom = std::move(sgpd);
}

SampleToGroupAtom& SampleTableAtom::GetSampleToGroupAtom()
{
    m_sampToGroupAtom.resize(m_sampToGroupAtom.size() + 1);
    return m_sampToGroupAtom.back();
}

const std::vector<SampleToGroupAtom>& SampleTableAtom::GetSampleToGroupAtoms() const
{
    return m_sampToGroupAtom;
}

const SampleGroupDescriptionAtom* SampleTableAtom::GetSampleGroupDescriptionAtom(FourCCInt groupingType) const
{
    if (m_sampGroupDescrAtom)
    {
        if (m_sampGroupDescrAtom->GetGroupingType() == groupingType)
        {
            return m_sampGroupDescrAtom.get();
        }
    }
    ISO_LOG(LOG_ERROR, "SampleGroupDescriptionAtom NOT found!\n");
    throw Exception();
}

void SampleTableAtom::SetSyncSampleAtom(const SyncSampleAtom& syncSampleAtom)
{
    if (m_syncSampAtom == nullptr)
    {
        m_syncSampAtom = MakeShared<SyncSampleAtom>(syncSampleAtom);
    }
    else
    {
        *m_syncSampAtom = syncSampleAtom;
    }
    m_hasSyncSampleAtom = true;
}

bool SampleTableAtom::HasSyncSampleAtom()
{
    return m_hasSyncSampleAtom;
}

std::shared_ptr<const SyncSampleAtom> SampleTableAtom::GetSyncSampleAtom() const
{
    return m_syncSampAtom;
}

void SampleTableAtom::ToStream(Stream& str)
{
    // Write Atom headers
    WriteAtomHeader(str);

    // Write other Atoms contained in the movie Atom
    m_sampleDescrAtom.ToStream(str);
    m_timeToSampAtom.ToStream(str);
    m_sampToChunkAtom.ToStream(str);
    m_chunkOffsetAtom.ToStream(str);
    m_sampSizeAtom.ToStream(str);

    if (m_syncSampAtom != nullptr)
    {
        m_syncSampAtom->ToStream(str);
    }

    if (m_compOffsetAtom != nullptr)
    {
        m_compOffsetAtom->ToStream(str);
    }

    if (m_compToDecodeAtom != nullptr)
    {
        m_compToDecodeAtom->ToStream(str);
    }

    if (m_sampGroupDescrAtom)
    {
        m_sampGroupDescrAtom->ToStream(str);
    }

    if (m_sampToGroupAtom.empty() == false)
    {
        for (auto sbgp : m_sampToGroupAtom)
        {
            sbgp.ToStream(str);
        }
    }

    // Update the size
    UpdateSize(str);
}

void SampleTableAtom::FromStream(Stream& str)
{
    //  First parse the Atom header
    ParseAtomHeader(str);

    int64_t pMaxNum = -1;
    int64_t pMaxAbNum =
        ABMAX_SAMP_CNT;  // 4 194 304  (more than day worth of 48hz samples)

    // if there a data available in the file
    while (str.BytesRemain() > 0)
    {
        // Extract contained Atom bitstream and type
        FourCCInt pAtomType;
        Stream subStr = str.ReadSubAtomStream(pAtomType);

        // Handle this Atom based on the type
        if (pAtomType == "stsd")
        {
            m_sampleDescrAtom.FromStream(subStr);
        }
        else if (pAtomType == "stco" || pAtomType == "co64")  // 'co64' is the 64-bit version
        {
            m_chunkOffsetAtom.FromStream(subStr);
        }
        else if (pAtomType == "stsz")
        {
            m_sampSizeAtom.FromStream(subStr);
            uint32_t pSampNum = m_sampSizeAtom.GetSampleNum();
            if (pMaxNum == -1)
            {
                if (pSampNum > pMaxAbNum)
                {
                    ISO_LOG(LOG_ERROR, "Over max sample counts from stsz to rest of sample table\n");
                    throw Exception();
                }
                pMaxNum = static_cast<int64_t>(pSampNum);
            }
            else if (pSampNum != pMaxNum)
            {
                ISO_LOG(LOG_ERROR, "Non-matching sample counts from stsz to rest of sample table\n");
                throw Exception();
            }
        }
        else if (pAtomType == "stts")
        {
            m_timeToSampAtom.FromStream(subStr);
            uint32_t pSampNum = static_cast<uint32_t>(m_timeToSampAtom.GetSampleNum());
            if (pMaxNum == -1)
            {
                if (pSampNum > pMaxAbNum)
                {
                    ISO_LOG(LOG_ERROR, "Over max sample counts from stts to rest of sample table\n");
                    throw Exception();
                }
                pMaxNum = static_cast<int64_t>(pSampNum);
            }
            else if (pSampNum != pMaxNum)
            {
                ISO_LOG(LOG_ERROR, "Non-matching sample counts from stts to rest of sample table\n");
                throw Exception();
            }
        }
        else if (pAtomType == "stsc")
        {
            if (pMaxNum != -1)
            {
                m_sampToChunkAtom.SetSampleNumMaxSafety(pMaxNum);
            }
            m_sampToChunkAtom.FromStream(subStr);
        }
        else if (pAtomType == "stss")
        {
            m_syncSampAtom = MakeShared<SyncSampleAtom>();
            if (pMaxNum != -1)
            {
                m_syncSampAtom->SetSampleNumMaxSafety(pMaxNum);
            }
            m_syncSampAtom->FromStream(subStr);
            m_hasSyncSampleAtom = true;
        }
        else if (pAtomType == "sgpd")
        {
            auto sgdb = new SampleGroupDescriptionAtom();
            sgdb->FromStream(subStr);
            m_sampGroupDescrAtom.reset(sgdb);
        }
        else if (pAtomType == "sbgp")
        {
            SampleToGroupAtom sampleToGroupAtom;
            sampleToGroupAtom.FromStream(subStr);
            uint32_t pSampNum = static_cast<uint32_t>(sampleToGroupAtom.GetNumberOfSamples());
            if (pMaxNum == -1)
            {
                if (pSampNum > pMaxAbNum)
                {
                    ISO_LOG(LOG_ERROR, "Over max sample counts from sbgp to rest of sample table\n");
                    throw Exception();
                }
                // we can't update pMaxNum here as sbgp can have less samples than total.
            }
            else if (pSampNum > pMaxNum)
            {
                ISO_LOG(LOG_ERROR, "Non-matching sample counts from sbgp to rest of sample table\n");
                throw Exception();
            }
            m_sampToGroupAtom.push_back(move(sampleToGroupAtom));
        }
        else if (pAtomType == "cslg")
        {
            m_compToDecodeAtom = MakeShared<CompositionToDecodeAtom>();
            m_compToDecodeAtom->FromStream(subStr);
        }
        else if (pAtomType == "ctts")
        {
            m_compOffsetAtom = MakeShared<CompositionOffsetAtom>();
            m_compOffsetAtom->FromStream(subStr);
            uint32_t pSampNum = static_cast<uint32_t>(m_compOffsetAtom->GetSampleNum());
            if (pMaxNum == -1)
            {
                if (pSampNum > pMaxAbNum)
                {
                    ISO_LOG(LOG_ERROR, "Over max sample counts from ctts to rest of sample table\n");
                    throw Exception();
                }
                pMaxNum = static_cast<int64_t>(pSampNum);
            }
            else if (pSampNum != pMaxNum)
            {
                ISO_LOG(LOG_ERROR, "Non-matching sample counts from ctts to rest of sample table\n");
                throw Exception();
            }
        }
        else
        {
			char type[4];
            pAtomType.GetString().copy(type, 4, 0);
            ISO_LOG(LOG_WARNING, "Skipping an unsupported Atom '%s' inside SampleTableAtom.\n", type);
        }
    }

    if (pMaxNum == -1)
    {
        ISO_LOG(LOG_ERROR, "SampleToTableAtom does not determine number of samples\n");
        throw Exception();
    }
    else
    {
        std::vector<uint32_t> sizes;
        sizes.push_back(m_timeToSampAtom.GetSampleNum());
        sizes.push_back(m_sampSizeAtom.GetSampleNum());
        auto lowerBound = m_sampToChunkAtom.GetSampleNumLowerBound(
            static_cast<unsigned int>(m_chunkOffsetAtom.GetChunkOffsets().size()));
        auto referenceSize = sizes[0];
        for (size_t c = 0; c < sizes.size(); ++c)
        {
            if (sizes[c] != referenceSize || sizes[c] < lowerBound)
            {
                ISO_LOG(LOG_ERROR, "SampleToTableAtom contains Atoms with mismatching sample counts\n");
                throw Exception();
            }
        }

        // reset it here in case the order of Atoms didn't allow it to
        // be set on time for m_sampToGroupAtom parsing
        m_sampToChunkAtom.SetSampleNumMaxSafety(pMaxNum);

        // we need to update stsc decoded presentation of chunk entries.
        m_sampToChunkAtom.DecodeEntries(static_cast<unsigned int>(m_chunkOffsetAtom.GetChunkOffsets().size()));
    }
}

void SampleTableAtom::ResetSamples()
{
    m_timeToSampAtom  = TimeToSampleAtom();
    m_sampToChunkAtom = SampleToChunkAtom();
    m_chunkOffsetAtom   = ChunkOffsetAtom();
    m_sampSizeAtom    = SampleSizeAtom();
    m_syncSampAtom.reset();
    m_compOffsetAtom.reset();
    m_compToDecodeAtom.reset();
    m_sampToGroupAtom.clear();
}

VCD_MP4_END
