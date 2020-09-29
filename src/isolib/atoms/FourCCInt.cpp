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
//! \file:   FourCCInt.cpp
//! \brief:  FourCCInt class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!
#include <stdexcept>
#include "FourCCInt.h"

VCD_MP4_BEGIN

FourCCInt::FourCCInt(const std::string& str)
{
    if (str.size() != 4)
    {
        ISO_LOG(LOG_ERROR, "FourCCInt given an std::string argument not exactly 4 characters long\n");
        throw Exception();
    }
    m_value = 0 | (std::uint32_t(str[0]) << 24) | (std::uint32_t(str[1]) << 16) | (std::uint32_t(str[2]) << 8) |
             (std::uint32_t(str[3]) << 0);
}

std::string FourCCInt::GetString() const
{
    std::string str(4, ' ');
    str[0] = char((m_value >> 24) & 0xff);
    str[1] = char((m_value >> 16) & 0xff);
    str[2] = char((m_value >> 8) & 0xff);
    str[3] = char((m_value >> 0) & 0xff);
    return str;
}

std::ostream& operator<<(std::ostream& stream, FourCCInt fourcc)
{
    return stream << fourcc.GetString();
}

VCD_MP4_END