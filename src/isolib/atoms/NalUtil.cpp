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
//! \file:   NalUtil.cpp
//! \brief:  NalUtil class implementation
//!
//! Created on October 15, 2019, 13:39 PM
//!

#include "NalUtil.h"

VCD_MP4_BEGIN

unsigned int FindStartCodeLen(const std::vector<uint8_t>& data)
{
    unsigned int pos = 0;
    const auto pSize = data.size();

    while ((pos + 1) < pSize && data[pos] == 0)
    {
        ++pos;
    }

    if (pos > 1 && data[pos] == 1)
    {
        return pos + 1;
    }
    else
    {
        return 0;
    }
}

std::vector<uint8_t> TransferStreamToRBSP(const std::vector<uint8_t>& pStr)
{
    std::vector<uint8_t> dest;
    const unsigned int nalBytes = pStr.size();

    dest.reserve(nalBytes);

    unsigned int i = FindStartCodeLen(pStr);

    static const size_t NAL_HEAD_LEN = 2;
    dest.insert(dest.end(), pStr.cbegin() + i, pStr.cbegin() + i + NAL_HEAD_LEN);
    i += NAL_HEAD_LEN;

    // copy rest of the data while removing start code emulation prevention bytes
    enum class Status
    {
        DATA_COPY,
        SINGLE_ZERO,
        TWO_ZEROS
    };
    Status status     = Status::DATA_COPY;
    int pBeginOffset = static_cast<int>(i);
    for (; i < nalBytes; ++i)
    {
        const unsigned int byte = pStr[i];
        switch (status)
        {
        case Status::DATA_COPY:
            if (byte != 0)
                status = Status::DATA_COPY;
            else
                status = Status::SINGLE_ZERO;
            break;

        case Status::SINGLE_ZERO:
            if (byte != 0)
                status = Status::DATA_COPY;
            else
                status = Status::TWO_ZEROS;
            break;

        case Status::TWO_ZEROS:
            if (byte == 0x03)
            {
                dest.insert(dest.end(), pStr.cbegin() + pBeginOffset, pStr.cbegin() + i);
                pBeginOffset = static_cast<int>(i) + 1;
                status = Status::DATA_COPY;
            }
            else if (byte == 0)
                status = Status::TWO_ZEROS;
            else
                status = Status::DATA_COPY;
            break;
        }
    }
    dest.insert(dest.end(), pStr.cbegin() + pBeginOffset, pStr.cend());
    return dest;
}

VCD_MP4_END
