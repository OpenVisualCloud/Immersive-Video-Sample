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
//! \file:   FormAllocator.cpp
//! \brief:  FormAllocator class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!
#include "FormAllocator.h"

VCD_MP4_BEGIN

class DefaultAllocator : public FormAllocator
{
public:
    DefaultAllocator()
    {
    }
    ~DefaultAllocator()
    {
    }

    void* allocate(size_t n, size_t size) override
    {
        return malloc(n * size);
    }
    void deallocate(void* ptr) override
    {
        free(ptr);
    }
};

char defAllocData[sizeof(DefaultAllocator)];
FormAllocator* defAlloc;
static FormAllocator* formAlloc;

FormAllocator* GetDefaultAllocator()
{
    if (!defAlloc)
    {
        defAlloc = new (defAllocData) DefaultAllocator();
    }
    return defAlloc;
}

FormAllocator* GetFormAllocator()
{
    if (!formAlloc)
    {
        defAlloc = GetDefaultAllocator();
        formAlloc  = defAlloc;
    }
    return formAlloc;
}

VCD_MP4_END