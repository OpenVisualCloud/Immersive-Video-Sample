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
//! \file:   FourCCInt.h
//! \brief:  FourCCInt class
//! \detail: FourCCInt operator
//!
//! Created on October 14, 2019, 13:39 PM
//!
#ifndef FOURCCINT_H
#define FOURCCINT_H

#include <cstdint>
#include <iostream>
#include "FormAllocator.h"
#include "../include/Common.h"

VCD_MP4_BEGIN

class FourCCInt
{
public:

    //!
    //! \brief Constructor
    //!
    FourCCInt()
        : m_value(0)
    {
        // nothing
    }

    FourCCInt(std::uint32_t value)
        : m_value(value)
    {
        // nothing
    }

    FourCCInt(const char (&str)[5])
        : m_value(0 | (std::uint32_t(str[0]) << 24) | (std::uint32_t(str[1]) << 16) | (std::uint32_t(str[2]) << 8) |
                 (std::uint32_t(str[3]) << 0))
    {
        // nothing
    }

    explicit FourCCInt(const std::string& str);

    std::uint32_t GetUInt32() const
    {
        return m_value;
    }

    std::string GetString() const;

    bool operator==(FourCCInt other) const
    {
        return m_value == other.m_value;
    }
    bool operator!=(FourCCInt other) const
    {
        return m_value != other.m_value;
    }
    bool operator>=(FourCCInt other) const
    {
        return m_value >= other.m_value;
    }
    bool operator<=(FourCCInt other) const
    {
        return m_value <= other.m_value;
    }
    bool operator>(FourCCInt other) const
    {
        return m_value > other.m_value;
    }
    bool operator<(FourCCInt other) const
    {
        return m_value < other.m_value;
    }

private:
    std::uint32_t m_value;  //!< value
};

std::ostream& operator<<(std::ostream& stream, FourCCInt fourcc);

VCD_MP4_END;
#endif  /* FOURCCINT_H */
