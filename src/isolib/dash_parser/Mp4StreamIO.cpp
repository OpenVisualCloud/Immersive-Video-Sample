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
//! \file:   Mp4StreamIO.cpp
//! \brief:  StreamIO class implementation
//!

#include "Mp4StreamIO.h"
#include <iostream>
#include "../atoms/FormAllocator.h"

using namespace std;

VCD_MP4_BEGIN

StreamIOInternal::StreamIOInternal(StreamIO* stream)
    : m_stream(stream)
    , m_error(false)
    , m_eof(false)
{
    m_error = !stream || !stream->SeekAbsoluteOffset(0);
}

/*
StreamIOInternal& StreamIOInternal::operator=(StreamIOInternal&& other)
{
    m_error = other.m_error;
    m_eof   = other.m_eof;
    m_stream = std::move(other.m_stream);
    return *this;
}
*/
StreamIOInternal::~StreamIOInternal()
{
    if(m_stream)
    {
        delete m_stream;
        m_stream = NULL;
    }
    // nothing
}

void StreamIOInternal::ReadStream(char* buffer, StreamIO::offset_t size_)
{
    StreamIO::offset_t got = m_stream->ReadStream(buffer, size_);
    if (got < size_)
    {
        m_eof   = true;
        m_error = true;
    }
}

int StreamIOInternal::GetOneByte()
{
    char ch;
    StreamIO::offset_t got = m_stream->ReadStream(&ch, sizeof(ch));
    if (got)
    {
        return static_cast<unsigned char>(ch);
    }
    else
    {
        m_eof = true;
        return 0;
    }
}

bool StreamIOInternal::PeekEOS()
{
    char buffer;
    auto was = m_stream->TellOffset();
    if (m_stream->ReadStream(&buffer, sizeof(buffer)) == 0)
    {
        return true;
    }
    else
    {
        m_stream->SeekAbsoluteOffset(was);
        return false;
    }
}

void StreamIOInternal::SeekOffset(StreamIO::offset_t offset)
{
    if (!m_stream->SeekAbsoluteOffset(offset))
    {
        m_eof   = true;
        m_error = true;
    }
}

StreamIO::offset_t StreamIOInternal::TellOffset()
{
    return m_stream->TellOffset();
}

StreamIO::offset_t StreamIOInternal::GetStreamSize()
{
    return m_stream->GetStreamSize();
}

void StreamIOInternal::ClearStatus()
{
    m_eof   = false;
    m_error = false;
}

bool StreamIOInternal::IsStreamGood() const
{
    return !m_error;
}

bool StreamIOInternal::IsReachEOS() const
{
    return m_eof;
}

VCD_MP4_END
