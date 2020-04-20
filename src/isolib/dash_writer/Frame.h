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
//! \file:   Frame.h
//! \brief:  Frame related information definition
//! \detail: Define related information about frame, like
//!          frame rate, frame CTS and so on.
//!

#ifndef _FRAME_H_
#define _FRAME_H_

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <list>
#include <vector>

#include "../include/Common.h"
#include "DataItem.h"
#include "Fraction.h"

using namespace std;

VCD_MP4_BEGIN

typedef vector<uint8_t> FrameBuf;

typedef FractS64 FrameTime;
typedef FractU64 FrameDuration;
typedef FractU64 FrameRate;
typedef list<FrameTime> FrameCts;
typedef FrameTime FrameDts;

struct FrameInfo
{
    FrameCts cts;
    FrameDuration duration;
    bool isIDR;
    FlagsOfSample sampleFlags;

    DataItem<FrameDts> dts;

    FrameInfo() = default;
};

struct Frame
{
    FrameInfo frameInfo;
    FrameBuf  frameBuf;
};

VCD_MP4_END;
#endif  // _FRAME_H_
