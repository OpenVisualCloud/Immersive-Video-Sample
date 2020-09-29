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
//! \file:   ElemStreamDescAtom.cpp
//! \brief:  ElemStreamDescAtom class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "ElemStreamDescAtom.h"
#include <algorithm>
#include <stdexcept>
#include "Stream.h"

VCD_MP4_BEGIN

ElementaryStreamDescriptorAtom::ElementaryStreamDescriptorAtom()
    : FullAtom("esds", 0, 0)
    , m_ES_Params()
{
}

ElementaryStreamDescriptorAtom::ElementaryStreamDescriptorAtom(const ElementaryStreamDescriptorAtom& atom)
    : FullAtom(atom.GetType(), 0, 0)
    , m_ES_Params(atom.m_ES_Params)
    , m_otherinfo(atom.m_otherinfo)
{
}

bool ElementaryStreamDescriptorAtom::GetOneParameterSet(std::vector<uint8_t>& byteStream) const
{
    if (m_ES_Params.decConfig.info.flag == 5)
    {
        byteStream = m_ES_Params.decConfig.info.info;
        return true;
    }
    else
    {
        return false;
    }
}

void ElementaryStreamDescriptorAtom::SetESDescriptor(ElementaryStreamDescriptorAtom::ES_Params esDescriptor)
{
    m_ES_Params = esDescriptor;
}

ElementaryStreamDescriptorAtom::ES_Params ElementaryStreamDescriptorAtom::GetESDescriptor() const
{
    return m_ES_Params;
}

int HighestBit(std::uint32_t value)
{
    int n = 0;
    while (value)
    {
        value >>= 1;
        ++n;
    }
    return n;
}

uint64_t WriteSize(Stream& str, std::uint32_t size)
{
    uint64_t sizeSize = 0;
    int currentBit    = (std::max(0, HighestBit(size) - 1)) / 7 * 7;
    bool hasMore;
    do
    {
        hasMore = (size >> (currentBit - 7)) != 0;
        str.Write8(((size >> currentBit) & 0x7F) | (hasMore ? 1u << 7 : 0));
        currentBit -= 7;
        ++sizeSize;
    } while (hasMore);
    return sizeSize;
}

void WriteDecodeSpec(Stream& str, const ElementaryStreamDescriptorAtom::DecodeSpec& info)
{
    str.Write8(info.flag);
    WriteSize(str, info.size);
    str.WriteArray(info.info, info.size);
}

void ElementaryStreamDescriptorAtom::ToStream(Stream& str)
{
    WriteFullAtomHeader(str);

    str.Write8(m_ES_Params.descrFlag);

    bool sizeConverge = false;
    std::uint64_t esSizeSize;
    std::uint32_t esDescriptorSize = m_ES_Params.size;

    Stream esStr;

    while (!sizeConverge)
    {
        esStr.Clear();
        esSizeSize = WriteSize(esStr, esDescriptorSize);
        esStr.Write16(m_ES_Params.id);
        esStr.Write8(m_ES_Params.flags);
        if (m_ES_Params.flags & 0x80)
        {
            esStr.Write16(m_ES_Params.depend);
        }

        if (m_ES_Params.flags & 0x40)
        {
            esStr.Write8(m_ES_Params.URLlen);
            if (m_ES_Params.URLlen)
            {
                esStr.WriteString(m_ES_Params.URL);
            }
        }

        if (m_ES_Params.flags & 0x20)
        {
            esStr.Write16(m_ES_Params.OCR_Id);
        }

        esStr.Write8(m_ES_Params.decConfig.flag);

        Stream decStr;
        std::uint64_t decConfigSize = m_ES_Params.decConfig.size;
        std::uint64_t decConfigSizeSize;
        bool decSize = false;
        while (!decSize)
        {
            decStr.Clear();
            decConfigSizeSize = WriteSize(decStr, static_cast<uint32_t>(decConfigSize));
            decStr.Write8(m_ES_Params.decConfig.idc);
            decStr.Write8((m_ES_Params.decConfig.strType << 2) | 0x01);
            decStr.Write24(m_ES_Params.decConfig.bufferSizeDB);
            decStr.Write32(m_ES_Params.decConfig.maxBitrate);
            decStr.Write32(m_ES_Params.decConfig.avgBitrate);

            if (m_ES_Params.decConfig.info.flag == 5)
            {
                WriteDecodeSpec(decStr, m_ES_Params.decConfig.info);
            }

            for (const auto& info : m_otherinfo)
            {
                WriteDecodeSpec(decStr, info);
            }

            decSize = decStr.GetSize() == std::uint64_t(decConfigSize) + decConfigSizeSize;

            if (!decSize)
            {
                decConfigSize = decStr.GetSize() - decConfigSizeSize;
            }
        }
        esStr.WriteStream(decStr);

        sizeConverge = esStr.GetSize() == std::uint64_t(esDescriptorSize) + esSizeSize;

        if (!sizeConverge)
        {
            esDescriptorSize = std::uint32_t(esStr.GetSize() - esSizeSize);
        }
    }
    str.WriteStream(esStr);
    UpdateSize(str);
}

void ElementaryStreamDescriptorAtom::FromStream(Stream& str)
{
    ParseFullAtomHeader(str);

    m_ES_Params.descrFlag = str.Read8();
    if (m_ES_Params.descrFlag != 3)  // descrFlag
    {
        ISO_LOG(LOG_ERROR, "ElementaryStreamDescritorAtom ES_Params.descrFlag not valid\n");
        throw Exception();
    }
    std::uint8_t readByte = 0;
    std::uint32_t size    = 0;
    do
    {
        readByte              = str.Read8();
        std::uint8_t sizeByte = (readByte & 0x7F);
        size                  = (size << 7) | sizeByte;
    } while (readByte & 0x80);

    m_ES_Params.size  = size;
    m_ES_Params.id = str.Read16();
    m_ES_Params.flags = str.Read8();

    if (m_ES_Params.flags & 0x80)
    {
        m_ES_Params.depend = str.Read16();
    }

    if (m_ES_Params.flags & 0x40)
    {
        m_ES_Params.URLlen = str.Read8();
        if (m_ES_Params.URLlen)
        {
            str.ReadStringWithLen(m_ES_Params.URL, m_ES_Params.URLlen);
        }
    }

    if (m_ES_Params.flags & 0x20)
    {
        m_ES_Params.OCR_Id = str.Read16();
    }

    m_ES_Params.decConfig.flag = str.Read8();
    if (m_ES_Params.decConfig.flag != 4)  // flag
    {
        ISO_LOG(LOG_ERROR, "ElementaryStreamDescritorAtom DecoderConfig.flag not valid\n");
        throw Exception();
    }

    readByte = 0;
    size     = 0;
    do
    {
        readByte              = str.Read8();
        std::uint8_t sizeByte = (readByte & 0x7f);
        size                  = (size << 7) | sizeByte;
    } while (readByte & 0x80);

    m_ES_Params.decConfig.size                 = size;
    m_ES_Params.decConfig.idc = str.Read8();
    m_ES_Params.decConfig.strType           = (str.Read8() >> 2);
    m_ES_Params.decConfig.bufferSizeDB         = str.Read24();
    m_ES_Params.decConfig.maxBitrate           = str.Read32();
    m_ES_Params.decConfig.avgBitrate           = str.Read32();

    while (str.BytesRemain())
    {
        std::uint8_t tag = str.Read8();

        readByte = 0;
        size     = 0;
        do
        {
            readByte              = str.Read8();
            std::uint8_t sizeByte = (readByte & 0x7f);
            size                  = (size << 7) | sizeByte;
        } while (readByte & 0x80);

        DecodeSpec info;

        info.flag = tag;
        info.size               = size;
        str.ReadArray(info.info, info.size);

        if (tag == 5)
        {
            m_ES_Params.decConfig.info = std::move(info);
        }
        else
        {
            m_otherinfo.push_back(std::move(info));
        }
    }
}

VCD_MP4_END