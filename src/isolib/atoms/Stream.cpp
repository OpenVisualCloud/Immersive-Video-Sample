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
//! \file:   Stream.cpp
//! \brief:  Stream implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!
#include "Stream.h"
#include <cstring>
#include <limits>
#include <stdexcept>


VCD_MP4_BEGIN

Stream::Stream()
    : m_storage()
    , m_currByte(0)
    , m_byteOffset(0)
    , m_bitOffset(0)
    , m_storageAllocated(true)
{
}

Stream::Stream(const std::vector<std::uint8_t>& strData)
    : m_storage(strData)
    , m_currByte(0)
    , m_byteOffset(0)
    , m_bitOffset(0)
    , m_storageAllocated(false)
{
}

Stream::Stream(Stream&& other)
    : m_storage(std::move(other.m_storage))
    , m_currByte(other.m_currByte)
    , m_byteOffset(other.m_byteOffset)
    , m_bitOffset(other.m_bitOffset)
    , m_storageAllocated(other.m_storageAllocated)
{
    other.m_currByte         = {};
    other.m_byteOffset       = {};
    other.m_bitOffset        = {};
    other.m_storageAllocated = {};
    other.m_storage.clear();
}

Stream& Stream::operator=(Stream&& other)
{
    m_currByte         = other.m_currByte;
    m_byteOffset       = other.m_byteOffset;
    m_bitOffset        = other.m_bitOffset;
    m_storageAllocated = other.m_storageAllocated;
    m_storage          = std::move(other.m_storage);
    return *this;
}

Stream::~Stream()
{
    if (m_storageAllocated == true)
    {
        m_storage.clear();
    }
}

bool Stream::IsByteAligned() const
{
    return m_bitOffset ? false : true;
}

std::uint64_t Stream::GetSize() const
{
    std::uint64_t size = m_storage.size();
    return size;
}

void Stream::SetSize(const std::uint64_t newSize)
{
    m_storage.resize(newSize);
}

const std::vector<std::uint8_t>& Stream::GetStorage() const
{
    return m_storage;
}

void Stream::Reset()
{
    m_currByte   = 0;
    m_bitOffset  = 0;
    m_byteOffset = 0;
}

void Stream::Clear()
{
    m_storage.clear();
}

void Stream::SkipBytes(const std::uint64_t x)
{
    m_byteOffset += x;
}

void Stream::SetByte(const std::uint64_t offset, const std::uint8_t byte)
{
    m_storage.at(offset) = byte;
}

std::uint8_t Stream::GetByte(const std::uint64_t offset) const
{
    std::uint8_t ret = m_storage.at(offset);
    return ret;
}

std::uint64_t Stream::ReadAtomHeaders(FourCCInt& type)
{
    auto size = Read32();
    type      = Read32();
    return size == 1 ? Read64() : size;
}

std::uint64_t Stream::BytesRemain() const
{
    return m_storage.size() - m_byteOffset;
}
void Stream::Extract(const std::uint64_t begin, const std::uint64_t end, Stream& dest) const
{
    dest.Clear();
    dest.Reset();
    if (begin <= m_storage.size() && end <= m_storage.size() && begin <= end)
    {
        dest.m_storage.insert(dest.m_storage.begin(), m_storage.begin() + static_cast<std::int64_t>(begin),
                                m_storage.begin() + static_cast<std::int64_t>(end));
    }
    else
    {
        ISO_LOG(LOG_ERROR, "ReadSubAtomStream trying to Read 0 size Atom\n");
        throw Exception();
    }
}

void Stream::WriteStream(const Stream& str)
{
    m_storage.insert(m_storage.end(), str.m_storage.begin(), str.m_storage.end());
}


void Stream::Write8(const std::uint8_t bits)
{
    m_storage.push_back(bits);
}

void Stream::Write16(const std::uint16_t bits)
{
    for (int i=8;i>=0;)
    {
        m_storage.push_back(static_cast<uint8_t>((bits >> i) & 0xff));
        i -= 8;
    }
}

void Stream::Write24(const std::uint32_t bits)
{
    for (int i=16;i>=0;)
    {
        m_storage.push_back(static_cast<uint8_t>((bits >> i) & 0xff));
        i -= 8;
    }
}

void Stream::Write32(const std::uint32_t bits)
{
    for (int i=24;i>=0;)
    {
        m_storage.push_back(static_cast<uint8_t>((bits >> i) & 0xff));
        i -= 8;
    }
}

void Stream::Write64(const std::uint64_t bits)
{
    for (int i=56;i>=0;)
    {
        m_storage.push_back(static_cast<uint8_t>((bits >> i) & 0xff));
        i -= 8;
    }
}

void Stream::WriteArray(const std::vector<std::uint8_t>& bits,
                                const std::uint64_t len,
                                const std::uint64_t srcOffset)
{
    // if len was not given, add everything until end of the vector
    auto copyLen = len == UINT64_MAX ? (bits.size() - srcOffset) : len;

    m_storage.insert(m_storage.end(), bits.begin() + static_cast<std::int64_t>(srcOffset),
                    bits.begin() + static_cast<std::int64_t>(srcOffset + copyLen));
}

void Stream::Write1(std::uint64_t bits, std::uint32_t len)
{
    if (len == 0)
    {
        ISO_LOG(LOG_WARNING, "Stream::Write1 called for zero-length bit sequence.\n");
    }
    else
    {
        do
        {
            const unsigned int pLeftByte = 8 - m_bitOffset;
            if (pLeftByte > len)
            {
                m_currByte =
                    m_currByte |
                    (static_cast<unsigned int>((bits & (std::numeric_limits<std::uint64_t>::max() >> (64 - len)))
                                                << (pLeftByte - len)));
                m_bitOffset += len;
                len = 0;
            }
            else
            {
                m_currByte = m_currByte | (static_cast<unsigned int>((bits >> (len - pLeftByte)) &
                                                                    ~((std::numeric_limits<std::uint64_t>::max()
                                                                        << (64 - pLeftByte)))));
                m_storage.push_back((uint8_t) m_currByte);
                m_bitOffset = 0;
                m_currByte  = 0;
                len -= pLeftByte;
            }
        } while (len > 0);
    }
}

void Stream::WriteString(const std::string& srcString)
{
    if (srcString.length() == 0)
    {
        ISO_LOG(LOG_WARNING, "Stream::WriteString called for zero-length string.\n");
    }

    for (const auto character : srcString)
    {
        m_storage.push_back(static_cast<unsigned char>(character));
    }
}

void Stream::WriteZeroEndString(const std::string& srcString)
{
    for (const auto character : srcString)
    {
        m_storage.push_back(static_cast<unsigned char>(character));
    }
    m_storage.push_back('\0');
}

void Stream::WriteFloat32(float value)
{
    std::uint32_t convertedValue = (std::uint32_t)value;
    Write32(convertedValue);
    //Write32(*((std::uint32_t*)(&value)));
}

void Stream::WriteHeaders(FourCCInt type, std::uint64_t AtomPayloadSize)
{
    bool over32BitSize = AtomPayloadSize > (UINT32_MAX - 8);
    Write32(over32BitSize ? 1 : (uint32_t) AtomPayloadSize + 8);
    Write32(type.GetUInt32());
    if (over32BitSize)
    {
        Write64(AtomPayloadSize + 16);
    }
}

float Stream::ReadFloat32()
{
    std::uint32_t value = Read32();
    float convertedValue = (float)value;
    return convertedValue;
    //return *(float*) &value;
}

std::uint8_t Stream::Read8()
{
    const std::uint8_t ret = m_storage.at(m_byteOffset);
    ++m_byteOffset;
    return ret;
}

std::uint16_t Stream::Read16()
{
    std::uint16_t ret = m_storage.at(m_byteOffset);
    m_byteOffset++;
    ret = (ret << 8) | m_storage.at(m_byteOffset);
    m_byteOffset++;
    return ret;
}

std::uint32_t Stream::Read24()
{
    unsigned int ret = m_storage.at(m_byteOffset);
    m_byteOffset++;
    ret = (ret << 8) | m_storage.at(m_byteOffset);
    m_byteOffset++;
    ret = (ret << 8) | m_storage.at(m_byteOffset);
    m_byteOffset++;
    return ret;
}

std::uint32_t Stream::Read32()
{
    unsigned int ret = m_storage.at(m_byteOffset);
    m_byteOffset++;
    ret = (ret << 8) | m_storage.at(m_byteOffset);
    m_byteOffset++;
    ret = (ret << 8) | m_storage.at(m_byteOffset);
    m_byteOffset++;
    ret = (ret << 8) | m_storage.at(m_byteOffset);
    m_byteOffset++;
    return ret;
}

std::uint64_t Stream::Read64()
{
    unsigned long long int ret = m_storage.at(m_byteOffset);
    m_byteOffset++;
    ret = (ret << 8) | m_storage.at(m_byteOffset);
    m_byteOffset++;
    ret = (ret << 8) | m_storage.at(m_byteOffset);
    m_byteOffset++;
    ret = (ret << 8) | m_storage.at(m_byteOffset);
    m_byteOffset++;
    ret = (ret << 8) | m_storage.at(m_byteOffset);
    m_byteOffset++;
    ret = (ret << 8) | m_storage.at(m_byteOffset);
    m_byteOffset++;
    ret = (ret << 8) | m_storage.at(m_byteOffset);
    m_byteOffset++;
    ret = (ret << 8) | m_storage.at(m_byteOffset);
    m_byteOffset++;

    return ret;
}

void Stream::ReadArray(std::vector<std::uint8_t>& bits, const std::uint64_t len)
{
    if (static_cast<std::size_t>(m_byteOffset + len) <= m_storage.size())
    {
        bits.insert(bits.end(), m_storage.begin() + static_cast<std::int64_t>(m_byteOffset),
                    m_storage.begin() + static_cast<std::int64_t>(m_byteOffset + len));
        m_byteOffset += len;
    }
    else
    {
        ISO_LOG(LOG_ERROR, "ReadArray trying to Read outside of m_storage\n");
        throw Exception();
    }
}

void Stream::ReadByteArrayToBuffer(char* buffer, const std::uint64_t len)
{
    if (static_cast<std::size_t>(m_byteOffset + len) <= m_storage.size())
    {
        std::memcpy(buffer, m_storage.data() + m_byteOffset, len);
        m_byteOffset += len;
    }
    else
    {
        ISO_LOG(LOG_ERROR, "ReadArray trying to Read outside of m_storage\n");
        throw Exception();
    }
}

std::uint32_t Stream::Read1(const std::uint32_t len)
{
    std::uint32_t retBits        = 0;
    std::uint32_t pLeftByte = 8 - m_bitOffset;

    if (len == 0)
    {
        return 0;
    }

    if (pLeftByte >= len)
    {
        retBits = (unsigned int) ((m_storage).at(m_byteOffset) >> (pLeftByte - len)) &
                        (unsigned int) ((1 << len) - 1);
        m_bitOffset += (unsigned int) len;
    }
    else
    {
        std::uint32_t pBitsGo = len - pLeftByte;
        retBits                = (m_storage).at(m_byteOffset) & (((unsigned int) 1 << pLeftByte) - 1);
        m_byteOffset++;
        m_bitOffset = 0;
        while (pBitsGo > 0)
        {
            if (pBitsGo >= 8)
            {
                retBits = (retBits << 8) | (m_storage).at(m_byteOffset);
                m_byteOffset++;
                pBitsGo -= 8;
            }
            else
            {
                retBits = (retBits << pBitsGo) |
                                ((unsigned int) ((m_storage).at(m_byteOffset) >> (8 - pBitsGo)) &
                                (((unsigned int) 1 << pBitsGo) - 1));
                m_bitOffset += (unsigned int) (pBitsGo);
                pBitsGo = 0;
            }
        }
    }

    if (m_bitOffset == 8)
    {
        m_byteOffset++;
        m_bitOffset = 0;
    }

    return retBits;
}

void Stream::ReadStringWithLen(std::string& pDst, const std::uint32_t len)
{
    pDst.clear();
    for (std::uint32_t i = 0; i < len; i++)
    {
        std::uint8_t pCurr = Read8();
        pDst += static_cast<char>(pCurr);
    }
}

void Stream::ReadStringWithPosAndLen(std::string& pDst, const std::uint64_t pos, const std::uint32_t len)
{
    pDst.clear();
    for (std::uint32_t i = 0; i < len; i++)
    {
        std::uint8_t pCurr = GetByte(pos + i);
        pDst += static_cast<char>(pCurr);
    }
}

void Stream::ReadZeroEndString(std::string& pDst)
{
    std::uint8_t pCurr = 0xff;
    pDst.clear();

    while (m_byteOffset < m_storage.size())
    {
        pCurr = Read8();
        if ((char) pCurr != '\0')
        {
            pDst += static_cast<char>(pCurr);
        }
        else
        {
            break;
        }
    }
}

uint32_t Stream::ReadExpGolombCode()
{
    std::int32_t pLeadZeros = -1;
    std::uint32_t codeNum;
    std::uint32_t tmpBit = 0;

    while (tmpBit == 0)
    {
        tmpBit = Read1(1);
        pLeadZeros++;
    }

    std::uint32_t shiftAmount = static_cast<std::uint32_t>(pLeadZeros);
    codeNum                   = ((std::uint32_t) 1 << shiftAmount) - 1 + Read1(shiftAmount);
    return codeNum;
}

int32_t Stream::ReadSignedExpGolombCode()
{
    unsigned int codeNum = ReadExpGolombCode();
    int signedVal        = int((codeNum + 1) >> 1);

    if ((codeNum & 1) == 0)
    {
        signedVal = -signedVal;
    }

    return signedVal;
}

Stream Stream::ReadSubAtomStream(FourCCInt& pType)
{
    std::uint64_t pSize = Read32();

    pType = Read32();

    std::uint64_t minAtomSize = 8;

    if (pSize == 1)  // Check if 'largesize' field is used
    {
        pSize = Read64();
        minAtomSize += 4;
        m_byteOffset -= 8;
    }

    m_byteOffset -= 8;

    if (pSize < minAtomSize)
    {
        ISO_LOG(LOG_ERROR, "Stream::ReadSubAtomStream trying to Read too small Atom\n");
        throw Exception();
    }

    Stream subBitstr;
    Extract(GetPos(), GetPos() + pSize, subBitstr);
    m_byteOffset += pSize;

    return subBitstr;
}

void Stream::ReadUUID(std::vector<uint8_t>& uuid)
{
    std::uint64_t initialOffset = GetPos();
    std::uint64_t pSize       = Read32();
    FourCCInt pType           = Read32();
    if (pSize == 1)  // Check if 'largesize' field is used
    {
        pSize = Read64();
    }
    if (pType != "uuid")
    {
        return;
    }

    ReadArray(uuid, 16);
    SetPos(initialOffset);
}

VCD_MP4_END
