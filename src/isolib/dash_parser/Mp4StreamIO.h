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
//! \file:   Mp4StreamIO.h
//! \brief:  StreamIO class definition
//! \detail: Define upper layer file operation above StreamIO
//!

#ifndef _MP4STREAMIO_H_
#define _MP4STREAMIO_H_

#include <stdint.h>
#include "../include/Common.h"
#include "../atoms/FormAllocator.h"

VCD_MP4_BEGIN

class StreamIO
{
public:
    typedef int64_t offset_t;

    /** Size of an indeterminately sized source, ie. a network stream */
    static const offset_t IndeterminateSize = 0x7fffffffffffffffll;

    StreamIO() {};

    virtual ~StreamIO() {};

    virtual offset_t ReadStream(char* buffer, offset_t size) = 0;

    virtual bool SeekAbsoluteOffset(offset_t offset) = 0;

    virtual offset_t TellOffset() = 0;

    virtual offset_t GetStreamSize() = 0;
};

class StreamIOInternal
{
public:
    StreamIOInternal(StreamIO* stream = nullptr);
    StreamIOInternal(const StreamIOInternal&) = default;
    StreamIOInternal& operator=(const StreamIOInternal&) = default;
    //StreamIOInternal& operator=(StreamIOInternal&&);
    ~StreamIOInternal();

    void ReadStream(char* buffer, StreamIO::offset_t size);

    int GetOneByte();


    void SeekOffset(StreamIO::offset_t offset);

    StreamIO::offset_t TellOffset();

    StreamIO::offset_t GetStreamSize();

    bool PeekEOS();

    bool IsStreamGood() const;

    bool IsReachEOS() const;

    void ClearStatus();

private:
    StreamIO* m_stream;
    bool m_error;
    bool m_eof;
};

VCD_MP4_END;
#endif  // _MP4STREAMIO_H_
