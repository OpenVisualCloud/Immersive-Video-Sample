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
//! \file:   Common.h
//! \brief:  Include the common system and data type header files that needed
//!

#ifndef _DASHCOMMON_H_
#define _DASHCOMMON_H_

#include "../../utils/ns_def.h"
#include "../../utils/error.h"
#include "../../utils/GlogWrapper.h"
#include "../common/ISOLog.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

VCD_MP4_BEGIN

#define MP4_BUILD_VERSION "v1.0.0"

#define DELETE_MEMORY(x) \
    if (x)               \
    {                    \
        delete x;        \
        x = NULL;        \
    }

#define DELETE_ARRAY(x)  \
    if (x)               \
    {                    \
        delete[] x;      \
        x = NULL;        \
    }

#define MEMBER_SETANDGET_FUNC_WITH_OPTION(Type, Member, MemberName, Option)    \
public:                                             \
    void Set##MemberName(Type v)                \
    {                                               \
        Member = v;                                    \
    }                                               \
    Type Get##MemberName() Option                     \
    {                                               \
        return Member;                                 \
    }                                               \

struct FourCC
{
    char item[5];
    inline FourCC()
        : item{}
    {
    }
    inline FourCC(uint32_t value)
    {
        item[0] = char((value >> 24) & 0xff);
        item[1] = char((value >> 16) & 0xff);
        item[2] = char((value >> 8) & 0xff);
        item[3] = char((value >> 0) & 0xff);
        item[4] = '\0';
    }
    inline FourCC(const char* str)
    {
        item[0] = str[0];
        item[1] = str[1];
        item[2] = str[2];
        item[3] = str[3];
        item[4] = '\0';
    }
    inline FourCC(const FourCC& fourcc)
    {
        item[0] = fourcc.item[0];
        item[1] = fourcc.item[1];
        item[2] = fourcc.item[2];
        item[3] = fourcc.item[3];
        item[4] = '\0';
    }
    inline FourCC& operator=(const FourCC& other)
    {
        item[0] = other.item[0];
        item[1] = other.item[1];
        item[2] = other.item[2];
        item[3] = other.item[3];
        item[4] = '\0';
        return *this;
    }
    inline bool operator==(const FourCC& other) const
    {
        return (item[0] == other.item[0]) && (item[1] == other.item[1]) && (item[2] == other.item[2]) &&
               (item[3] == other.item[3]);
    }
    inline bool operator!=(const FourCC& other) const
    {
        return (item[0] != other.item[0]) || (item[1] != other.item[1]) || (item[2] != other.item[2]) ||
               (item[3] != other.item[3]);
    }
    inline bool operator<(const FourCC& other) const
    {
        return (item[0] < other.item[0])
                   ? true
                   : (item[0] > other.item[0])
                         ? false
                         : (item[1] < other.item[1])
                               ? true
                               : (item[1] > other.item[1])
                                     ? false
                                     : (item[2] < other.item[2])
                                           ? true
                                           : (item[2] > other.item[2])
                                                 ? false
                                                 : (item[3] < other.item[3])
                                                       ? true
                                                       : (item[3] > other.item[3]) ? false : false;
    }
    inline bool operator<=(const FourCC& other) const
    {
        return *this == other || *this < other;
    }
    inline bool operator>=(const FourCC& other) const
    {
        return !(*this < other);
    }
    inline bool operator>(const FourCC& other) const
    {
        return !(*this <= other);
    }
};

enum class OmniViewIdc : uint8_t
{
    OMNI_MONOSCOPIC     = 0,
    OMNI_LEFT           = 1,
    OMNI_RIGHT          = 2,
    OMNI_LEFT_AND_RIGHT = 3,
    OMNI_INVALID        = 0xff
};

enum class COVIShapeType : uint8_t
{
    FOUR_GREAT_CIRCLES = 0,
    TWO_AZIMUTH_AND_TWO_ELEVATION_CIRCLES
};

struct COVIRegion
{
    OmniViewIdc viewIdc;
    int32_t centAzimuth;
    int32_t centElevation;
    int32_t centTilt;
    uint32_t azimuthRange;
    uint32_t elevationRange;
    bool interpolate;
};

enum class OmniProjFormat
{
    OMNI_ERP = 0,
    OMNI_Cubemap,
    OMNI_Planar
};

enum class VideoFramePackingType : uint8_t
{
    OMNI_TOPBOTTOM     = 3,
    OMNI_SIDEBYSIDE    = 4,
    OMNI_TEMPINTERLEAVING = 5,
    OMNI_MONOSCOPIC       = 0x8f
};

struct Rotation
{
    int32_t yaw;
    int32_t pitch;
    int32_t roll;
};

struct FlagTypesForSample
{
    uint32_t reserved : 4, is_leading : 2, sample_depends_on : 2, sample_is_depended_on : 2,
        sample_has_redundancy : 2, sample_padding_value : 3, sample_is_non_sync_sample : 1,
        sample_degradation_priority : 16;
};

union FlagsOfSample {
    uint32_t flagsAsUInt;
    FlagTypesForSample flags;
};

VCD_MP4_END;
#endif /* _DASHCOMMON_H_ */
