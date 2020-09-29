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
//! \file:   MediaDataAtom.cpp
//! \brief:  MediaDataAtom class implementation
//!
//! Created on October 16, 2019, 13:39 PM
//!

#include "MediaDataAtom.h"

#include <fstream>
#include <limits>
#include <stdexcept>

using namespace std;

VCD_MP4_BEGIN

MediaDataAtom::MediaDataAtom()
    : Atom("mdat")
    , m_headerData()
    , m_mediaData()
    , m_totalSize(0)
    , m_dataOffsetList()
    , m_dataLenList()
{
    SetLargeSize();
    WriteAtomHeader(m_headerData);  // write Atom header
}

void MediaDataAtom::Write(std::ofstream& output) const
{
    output.write(reinterpret_cast<const char*>(m_headerData.GetStorage().data()),
                 static_cast<streamsize>(m_headerData.GetStorage().size()));
    for (const auto& pData : m_mediaData)
    {
        output.write(reinterpret_cast<const char*>(pData.data()), static_cast<streamsize>(pData.size()));
    }
}

void MediaDataAtom::ToStream(Stream& str)
{
    const std::vector<uint8_t>& data = m_headerData.GetStorage();
    str.WriteArray(data, data.size());

    for (const auto& pData : m_mediaData)
    {
        str.WriteArray(pData, pData.size());
    }
}

void MediaDataAtom::FromStream(Stream& /*str*/)
{
    return;
}

void MediaDataAtom::UpdateSize(Stream& str)
{
    uint64_t AtomSize = m_totalSize + str.GetSize();
    SetSize(AtomSize);

    if ((AtomSize > std::numeric_limits<std::uint32_t>::max()) && (GetLargeSize() == false))
    {
        ISO_LOG(LOG_ERROR, "Atom::UpdateSize(): Atom size exceeds 4GB but large size for 64-bit size field was not set.\n");
        throw Exception();
    }

    // Write updated size to the bitstream.
    if (GetLargeSize())
    {
        str.SetByte(8, (AtomSize >> 56) & 0xff);
        str.SetByte(9, (AtomSize >> 48) & 0xff);
        str.SetByte(10, (AtomSize >> 40) & 0xff);
        str.SetByte(11, (AtomSize >> 32) & 0xff);
        str.SetByte(12, (AtomSize >> 24) & 0xff);
        str.SetByte(13, (AtomSize >> 16) & 0xff);
        str.SetByte(14, (AtomSize >> 8) & 0xff);
        str.SetByte(15, AtomSize & 0xff);
    }
    else
    {
        str.SetByte(0, (AtomSize >> 24) & 0xff);
        str.SetByte(1, (AtomSize >> 16) & 0xff);
        str.SetByte(2, (AtomSize >> 8) & 0xff);
        str.SetByte(3, AtomSize & 0xff);
    }
}

std::uint64_t MediaDataAtom::AddData(const std::vector<uint8_t>& srcData)
{
    std::uint64_t offset =
        m_headerData.GetSize() + m_totalSize;  // offset from the beginning of the Atom (including header)

    m_dataOffsetList.push_back(offset);          // current offset
    m_dataLenList.push_back(srcData.size());  // length of the data to be added

    m_mediaData.push_back(srcData);
    m_totalSize += srcData.size();

    UpdateSize(m_headerData);
    return offset;
}

std::uint64_t MediaDataAtom::AddData(const uint8_t* buffer, const uint64_t bufferSize)
{
    std::uint64_t offset =
        m_headerData.GetSize() + m_totalSize;  // offset from the beginning of the Atom (including header)

    m_dataOffsetList.push_back(offset);      // current offset
    m_dataLenList.push_back(bufferSize);  // length of the data to be added

    std::vector<uint8_t> tmp(buffer, buffer + bufferSize);
    m_mediaData.insert(m_mediaData.end(), std::move(tmp));

    m_totalSize += bufferSize;

    UpdateSize(m_headerData);
    return offset;
}

void MediaDataAtom::AddNalData(const std::vector<std::vector<uint8_t>>& srcData)
{
    std::uint64_t totalLen = 0;
    uint64_t headSize = m_headerData.GetSize();
    size_t dataOffset = headSize + m_totalSize;  // data offset for the image within the media data Atom

    for (const auto& i : srcData)
    {
        AddNalData(i);
        totalLen += m_dataLenList.back();
        m_dataOffsetList.pop_back();
        m_dataLenList.pop_back();
    }

    m_dataOffsetList.push_back(static_cast<std::uint64_t>(dataOffset));
    m_dataLenList.push_back(totalLen);  // total length of the data added
}

void MediaDataAtom::AddNalData(const std::vector<uint8_t>& srcData)
{
    std::uint64_t pStartLen;
    std::uint64_t pStartPos;
    std::uint64_t pCurr  = 0;
    std::uint64_t totalLen = 0;

    m_dataOffsetList.push_back(static_cast<std::uint64_t>(
        m_headerData.GetSize() + m_totalSize));  // record offset for the picture to be added

    std::vector<uint8_t> pMediaData;
    pMediaData.reserve(srcData.size());

    // replace start codes with nal length fields
    pStartLen = FindStartCode(srcData, 0, pStartPos);
    pCurr += pStartLen;
    while (pCurr < srcData.size())
    {
        pStartLen =
            FindStartCode(srcData, pCurr, pStartPos);  // find next start code to determine NAL unit size
        const std::uint64_t nalSize = pStartPos - pCurr;

        // write length field
        pMediaData.push_back(static_cast<uint8_t>((uint32_t(nalSize) >> 24) & 0xff));
        pMediaData.push_back(static_cast<uint8_t>((uint32_t(nalSize) >> 16) & 0xff));
        pMediaData.push_back(static_cast<uint8_t>((uint32_t(nalSize) >> 8) & 0xff));
        pMediaData.push_back(static_cast<uint8_t>(uint32_t(nalSize) & 0xff));

        std::vector<uint8_t>::const_iterator sourceIt =
            srcData.begin() + static_cast<std::vector<uint8_t>::difference_type>(pCurr);
        pMediaData.insert(pMediaData.end(), sourceIt,
                              sourceIt + static_cast<std::vector<uint8_t>::difference_type>(nalSize));

        pCurr = pStartPos + pStartLen;
        totalLen += (nalSize + 4);
    }

    m_mediaData.push_back(std::move(pMediaData));
    m_totalSize += m_mediaData.back().size();

    m_dataLenList.push_back(totalLen);  // total length of the data added

    UpdateSize(m_headerData);
}

std::uint64_t MediaDataAtom::FindStartCode(const std::vector<uint8_t>& srcData,
                                          const std::uint64_t pPos,
                                          std::uint64_t& pStartPos)
{
    std::uint64_t i          = pPos;
    std::uint64_t len        = 0;
    bool pFound              = false;
    const size_t srcDataSize = srcData.size();

    while (i < srcDataSize && !pFound)
    {
        const uint8_t byte = srcData[i];
        if (byte == 0)
        {
            ++len;
        }
        else if (len > 1 && byte == 1)
        {
            ++len;
            pFound = true;
        }
        else
        {
            len = 0;
        }
        ++i;
    }

    if (pFound)
    {
        pStartPos = i - len;
    }
    else
    {
        pStartPos = i;
        len          = 0;
    }

    return len;
}

VCD_MP4_END
