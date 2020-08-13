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
//! \file:   FrameWrapper.cpp
//! \brief:  FrameWrapper class implementation
//!

#include "FrameWrapper.h"
#include "AcquireTrackData.h"

using namespace std;

VCD_MP4_BEGIN

GetDataOfFrame::GetDataOfFrame()
{
}

GetDataOfFrame::~GetDataOfFrame()
{
}

FrameWrapper::FrameWrapper(unique_ptr<GetDataOfFrame>&& aAcquire, FrameInfo aFrameInfo)
    : m_acquire(move(aAcquire))
    , m_frameInfo(aFrameInfo)
{
}

FrameWrapper::FrameWrapper(const FrameWrapper& aOther)
    : m_acquire(aOther.m_acquire->Clone())
    , m_frameInfo(aOther.m_frameInfo)
{
}

FrameWrapper::FrameWrapper(FrameWrapper&& aOther)
    : m_acquire(move(aOther.m_acquire))
    , m_frameInfo(move(aOther.m_frameInfo))
{
    assert(aOther.mValid);
    aOther.mValid = false;
}

FrameWrapper& FrameWrapper::operator=(const FrameWrapper& aOther)
{
    m_acquire.reset(aOther.m_acquire->Clone());
    m_frameInfo = aOther.m_frameInfo;
    return *this;
}

FrameWrapper& FrameWrapper::operator=(FrameWrapper&& aOther)
{
    assert(aOther.mValid);
    m_acquire      = move(aOther.m_acquire);
    m_frameInfo    = move(aOther.m_frameInfo);
    aOther.mValid = false;
    return *this;
}

FrameWrapper::~FrameWrapper()
{
}

Frame FrameWrapper::operator*() const
{
    assert(mValid);
    return {m_frameInfo, m_acquire->Get()};
}

unique_ptr<Frame> FrameWrapper::operator->() const
{
    assert(mValid);
    return unique_ptr<Frame>{new Frame{m_frameInfo, m_acquire->Get()}};
}

size_t FrameWrapper::GetSize() const
{
    return m_acquire->GetDataSize();
}

FrameInfo FrameWrapper::GetFrameInfo() const
{
    return m_frameInfo;
}

void FrameWrapper::SetFrameInfo(const FrameInfo& aFrameInfo)
{
    m_frameInfo = aFrameInfo;
}

VCD_MP4_END
