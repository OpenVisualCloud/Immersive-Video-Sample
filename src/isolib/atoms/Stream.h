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
//! \file:   Stream.h
//! \brief:  Stream class
//! \detail: Stream read and write operation
//!
//! Created on October 14, 2019, 13:39 PM
//!
#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <cstdint>
#include "FormAllocator.h"
#include "../include/Common.h"
#include "FourCCInt.h"

VCD_MP4_BEGIN

class Stream
{
public:

    //!
    //! \brief Constructor
    //!
    Stream();
    Stream(const std::vector<std::uint8_t>& strData);
    Stream(const Stream&) = default;
    Stream& operator=(const Stream&) = default;
    Stream(Stream&&);
    Stream& operator=(Stream&&);

    //!
    //! \brief Destructor
    //!
    ~Stream();

    //!
    //! \brief    Set and Get function for m_byteOffset member
    //!
    //! \param    [in] std::uint64_t
    //!           value to set
    //! \param    [in] m_byteOffset
    //!           m_byteOffset member in class
    //! \param    [in] Pos
    //!           m_byteOffset name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(uint64_t, m_byteOffset, Pos, const);

    //!
    //! \brief    Get Size
    //!
    //! \return   std::uint64_t
    //!           Size
    //!
    std::uint64_t GetSize() const;

    //!
    //! \brief    Set Size
    //!
    //! \param    [in] std::uint64_t
    //!           Size value
    //!
    //! \return   void
    //!
    void SetSize(std::uint64_t newSize);

    //!
    //! \brief    Get Storage
    //!
    //! \return   const std::vector<std::uint8_t>&
    //!           Storage
    //!
    const std::vector<std::uint8_t>& GetStorage() const;

    //!
    //! \brief Reset function
    //!
    void Reset();

    //!
    //! \brief Clear function
    //!
    void Clear();

    //!
    //! \brief    Skip Bytes
    //!
    //! \param    [in] std::uint64_t
    //!           offset
    //!
    //! \return   void
    //!
    void SkipBytes(std::uint64_t x);

    //!
    //! \brief    Set Byte
    //!
    //! \param    [in] std::uint64_t
    //!           offset
    //! \param    [in] std::uint8_t
    //!           byte
    //!
    //! \return   void
    //!
    void SetByte(std::uint64_t offset, std::uint8_t byte);

    //!
    //! \brief    Get Storage
    //!
    //! \param    [in] std::uint64_t
    //!           offset
    //!
    //! \return   std::uint8_t
    //!           byte data
    //!
    std::uint8_t GetByte(std::uint64_t offset) const;

    //!
    //! \brief    Write 1 bit
    //!
    //! \param    [in] std::uint64_t
    //!           bits
    //! \param   std::uint32_t
    //!           length
    //!
    //! \return   void
    //!
    void Write1(std::uint64_t bits, std::uint32_t len);

    //!
    //! \brief    Write 8 bit
    //!
    //! \param    [in] std::uint64_t
    //!           bits
    //!
    //! \return   std::uint32_t
    //!           length
    //!
    void Write8(std::uint8_t bits);

    //!
    //! \brief    Write Stream
    //!
    //! \param    [in,out] const Stream& str
    //!           bitstream
    //!
    //! \return   void
    //!
    void WriteStream(const Stream& str);

    //!
    //! \brief    Write 16 bit
    //!
    //! \param    [in] std::uint16_t
    //!           bits
    //!
    //! \return   void
    //!
    void Write16(std::uint16_t bits);

    //!
    //! \brief    Write 24 bit
    //!
    //! \param    [in] std::uint32_t
    //!           bits
    //!
    //! \return   void
    //!
    void Write24(std::uint32_t bits);

    //!
    //! \brief    Write 32 bit
    //!
    //! \param    [in] std::uint32_t
    //!           bits
    //!
    //! \return   void
    //!
    void Write32(std::uint32_t bits);

    //!
    //! \brief    Write 64 bit
    //!
    //! \param    [in] std::uint64_t
    //!           bits
    //!
    //! \return   void
    //!
    void Write64(std::uint64_t bits);

    //!
    //! \brief    Write array 8bit
    //!
    //! \param    [in,out] const std::vector<std::uint8_t>&
    //!           bits
    //! \param    [in] std::uint64_t
    //!           len
    //! \param    [in] std::uint64_t
    //!           source Offset
    //!
    //! \return   void
    //!
    void WriteArray(const std::vector<std::uint8_t>& bits,
                            std::uint64_t len       = UINT64_MAX,
                            std::uint64_t srcOffset = 0);

    //!
    //! \brief    Write String
    //!
    //! \param    [in,out] const std::string& srcString
    //!           source string
    //!
    //! \return   void
    //!
    void WriteString(const std::string& srcString);

    //!
    //! \brief    Write Zero End String
    //!
    //! \param    [in,out] const std::string& srcString
    //!           source string
    //!
    //! \return   void
    //!
    void WriteZeroEndString(const std::string& srcString);

    //!
    //! \brief    Write 32bits Float
    //!
    //! \param    [in] float
    //!           value
    //!
    //! \return   void
    //!
    void WriteFloat32(float value);

    //!
    //! \brief    Write headers
    //!
    //! \param    [in] FourCCInt
    //!           type
    //! \param    [in] std::uint64_t
    //!           Atom Pay load Size
    //!
    //! \return   void
    //!
    void WriteHeaders(FourCCInt type, std::uint64_t AtomPayloadSize);

    //!
    //! \brief    Read 1 bit
    //!
    //! \param    [in] const std::uint32_t
    //!           len
    //!
    //! \return   std::uint32_t
    //!           bits size offset
    //!
    std::uint32_t Read1(const std::uint32_t len);

    //!
    //! \brief    Read 8 bit
    //!
    //! \return   std::uint8_t
    //!           bits size offset
    //!
    std::uint8_t Read8();

    //!
    //! \brief    Read 16 bit
    //!
    //! \return   std::uint16_t
    //!           bits size offset
    //!
    std::uint16_t Read16();

    //!
    //! \brief    Read 24 bit
    //!
    //! \return   std::uint24_t
    //!           bits size offset
    //!
    std::uint32_t Read24();

    //!
    //! \brief    Read 32 bit
    //!
    //! \return   std::uint32_t
    //!           bits size offset
    //!
    std::uint32_t Read32();

    //!
    //! \brief    Read 64 bit
    //!
    //! \return   std::uint64_t
    //!           bits size offset
    //!
    std::uint64_t Read64();

    //!
    //! \brief    Read array 8bit
    //!
    //! \param    [in] std::vector<std::uint8_t>& bits
    //!           bits
    //! \param    [in] std::uint64_t
    //!           len
    //!
    //! \return   void
    //!
    void ReadArray(std::vector<std::uint8_t>& bits, std::uint64_t len);

    //!
    //! \brief    Read Byte Array To Buffer
    //!
    //! \param    [in] char*
    //!           buffer
    //! \param    [in] std::uint64_t
    //!           len
    //!
    //! \return   void
    //!
    void ReadByteArrayToBuffer(char* buffer, std::uint64_t len);

    //!
    //! \brief    Read String With Len
    //!
    //! \param    [in] std::string&
    //!           dst String
    //! \param    [in] std::uint32_t
    //!           len
    //!
    //! \return   void
    //!
    void ReadStringWithLen(std::string& dstString, std::uint32_t len);

    //!
    //! \brief    Read String With Pos And Len
    //!
    //! \param    [in] std::string&
    //!           dst String
    //! \param    [in] std::uint64_t
    //!           pos
    //! \param    [in] std::uint32_t
    //!           len
    //!
    //! \return   void
    //!
    void ReadStringWithPosAndLen(std::string& dstString, std::uint64_t pos, std::uint32_t len);

    //!
    //! \brief    Read Zero End String
    //!
    //! \param    [in] std::string&
    //!           dst String
    //!
    //! \return   void
    //!
    void ReadZeroEndString(std::string& dstString);

    //!
    //! \brief    Read 32bits Float
    //!
    //! \return   float
    //!           float data
    //!
    float ReadFloat32();

    //!
    //! \brief    Read Exp Golomb Code
    //!
    //! \return   uint32_t
    //!           code number
    //!
    uint32_t ReadExpGolombCode();

    //!
    //! \brief    Read Signed Exp Golomb Code
    //!
    //! \return   int32_t
    //!           code number
    //!
    int32_t ReadSignedExpGolombCode();

    //!
    //! \brief    Read Sub Atom Stream
    //!
    //! \param    [in] FourCCInt&
    //!           Atom Type
    //!
    //! \return   Stream
    //!           sub Bitstream
    //!
    Stream ReadSubAtomStream(FourCCInt& AtomType);

    //!
    //! \brief    Read UUID
    //!
    //! \param    [in] std::vector<uint8_t>&
    //!           uuid
    //!
    //! \return   void
    //!
    void ReadUUID(std::vector<uint8_t>& uuid);

    //!
    //! \brief    Read Atom Headers
    //!
    //! \param    [in] FourCCInt&
    //!           Atom Type
    //!
    //! \return   std::uint64_t
    //!           size
    //!
    std::uint64_t ReadAtomHeaders(FourCCInt& type);

    //!
    //! \brief    Bytes Remain to process
    //!
    //! \return   std::uint64_t
    //!           storage size
    //!
    std::uint64_t BytesRemain() const;

    //!
    //! \brief    Read String With Pos And Len
    //!
    //! \param    [in] std::uint64_t
    //!           begin pos
    //! \param    [in] std::uint64_t
    //!           end pos
    //! \param    [in] Stream&
    //!           dest stream
    //!
    //! \return   void
    //!
    void Extract(std::uint64_t begin, std::uint64_t end, Stream& dest) const;

    //!
    //! \brief    Is Byte Aligned or not
    //!
    //! \return   bool
    //!           Is Byte Aligned or not
    //!
    bool IsByteAligned() const;

private:
    std::vector<std::uint8_t> m_storage;    //!< storage
    unsigned int m_currByte;                //!< current byte postion
    std::uint64_t m_byteOffset;             //!< byte offset
    unsigned int m_bitOffset;               //!< bit offset
    bool m_storageAllocated;                //!< is storage Allocated successfully
};

VCD_MP4_END;
#endif /* BITSTREAM_H */
