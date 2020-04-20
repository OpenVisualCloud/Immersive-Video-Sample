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
//! \file:   FrameWrapper.h
//! \brief:  Frame proxy class definition
//! \detail: Define the class to wrap frame
//!

#ifndef _FRAMEWRAPPER_H_
#define _FRAMEWRAPPER_H_

#include <memory>
#include "Frame.h"

using namespace std;

VCD_MP4_BEGIN

class GetDataOfFrame
{
public:
    GetDataOfFrame();
    virtual ~GetDataOfFrame();

    GetDataOfFrame(const GetDataOfFrame& other) = delete;
    GetDataOfFrame& operator=(const GetDataOfFrame&) = delete;

    virtual size_t GetDataSize() const = 0;
    virtual FrameBuf Get() const  = 0;

    virtual GetDataOfFrame* Clone() const = 0;
};

class FrameWrapper
{
public:
    FrameWrapper(unique_ptr<GetDataOfFrame>&& aAcquire, FrameInfo aFrameInfo);
    FrameWrapper(const FrameWrapper& aOther);
    FrameWrapper(FrameWrapper&& aOther);
    FrameWrapper& operator=(const FrameWrapper& aOther);
    FrameWrapper& operator=(FrameWrapper&& aOther);
    ~FrameWrapper();
    Frame operator*() const;
    unique_ptr<Frame> operator->() const;
    FrameInfo GetFrameInfo() const;
    void SetFrameInfo(const FrameInfo& aFrameInfo);
    size_t GetSize() const;

private:
    unique_ptr<GetDataOfFrame> m_acquire;
    FrameInfo m_frameInfo;

    bool mValid = true;
};

VCD_MP4_END;
#endif  // _FRAMEWRAPPER_H_
