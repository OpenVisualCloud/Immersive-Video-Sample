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
//! \file:   MovieFragDataTypes.h
//! \brief:  MovieFragDataTypes class
//! \detail: Movie fragments data types definitions.
//!
//! Created on October 14, 2019, 13:39 PM
//!
#ifndef MOVIEFRAGMENTSDATATYPES_H
#define MOVIEFRAGMENTSDATATYPES_H

#include <stdint.h>
#include "Stream.h"
#include "FormAllocator.h"

VCD_MP4_BEGIN

struct SampleFlagsType  //!< sample flags type
{
    uint32_t reserved : 4, is_leading : 2, sample_depends_on : 2, sample_is_depended_on : 2,
        sample_has_redundancy : 2, sample_padding_value : 3, sample_is_non_sync_sample : 1,
        sample_degradation_priority : 16;
};

union SampleFlags { //!< sample flags
    uint32_t flagsAsUInt;
    SampleFlagsType flags;

    static SampleFlags Read(Stream& str)
    {
        SampleFlags r;
        r.flags.reserved                    = str.Read1(4);
        r.flags.is_leading                  = str.Read1(2);
        r.flags.sample_depends_on           = str.Read1(2);
        r.flags.sample_is_depended_on       = str.Read1(2);
        r.flags.sample_has_redundancy       = str.Read1(2);
        r.flags.sample_padding_value        = str.Read1(3);
        r.flags.sample_is_non_sync_sample   = str.Read1(1);
        r.flags.sample_degradation_priority = str.Read1(16);
        return r;
    }

    static void Write(Stream& str, const SampleFlags& r)
    {
        str.Write1(r.flags.reserved, 4);
        str.Write1(r.flags.is_leading, 2);
        str.Write1(r.flags.sample_depends_on, 2);
        str.Write1(r.flags.sample_is_depended_on, 2);
        str.Write1(r.flags.sample_has_redundancy, 2);
        str.Write1(r.flags.sample_padding_value, 3);
        str.Write1(r.flags.sample_is_non_sync_sample, 1);
        str.Write1(r.flags.sample_degradation_priority, 16);
    }
};

struct SampleDefaults   //!< sample defaults
{
    std::uint32_t trackId;
    std::uint32_t defaultSampleDescriptionIndex;
    std::uint32_t defaultSampleDuration;
    std::uint32_t defaultSampleSize;
    SampleFlags defaultSampleFlags;
};

VCD_MP4_END;
#endif /* MOVIEFRAGMENTSDATATYPES_H */
