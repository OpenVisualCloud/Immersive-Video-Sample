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
//! \file:   safe_mem.h
//! \brief:  Define safe mem library functions
//!
//! Created on April 30, 2019, 6:04 AM
//!

#ifndef _SAFE_MEM_H_
#define _SAFE_MEM_H_

#include <algorithm>
using namespace std;

//memcpy
#ifndef _SAFE_MEM_
#define memcpy_s(dest, dest_sz, src, src_sz) \
        memcpy((void *)(dest), (const void *)(src), (size_t)min((size_t)dest_sz, (size_t)src_sz))
#else
extern "C" {
#include "safestringlib/safe_mem_lib.h"
}
#define memcpy_s(dest, dest_sz, src, src_sz) \
        memcpy_s((void *)(dest), dest_sz, (const void *)(src), (size_t)min((size_t)dest_sz, (size_t)src_sz))
#endif

//memset
#ifndef _SAFE_MEM_
#define memset_s(dest, dmax, value) \
        memset((void *)(dest), (int)(value), (size_t)dmax)
#else
extern "C" {
#include "safestringlib/safe_mem_lib.h"
}
#define memset_s(dest, dmax, value) \
        memset_s((void *)(dest), dmax, value)
#endif

//memmove
#ifndef _SAFE_MEM_
#define memmove_s(dest, dest_sz, src, src_sz) \
        memmove((void *)(dest), (const void *)(src), (size_t)min((size_t)dest_sz, (size_t)src_sz))
#else
extern "C" {
#include "safestringlib/safe_mem_lib.h"
}
#define memmove_s(dest, dest_sz, src, src_sz) \
        memmove_s((void *)(dest), dest_sz, (const void *)(src), (size_t)min((size_t)dest_sz, (size_t)src_sz))
#endif


//memcmp
#ifndef _SAFE_MEM_
#define memcmp_s(dest, dmax, src, slen, diff) \
        (*diff = {memcmp((const void *)(dest), (const void *)(src), (size_t)min((size_t)dmax, (size_t)slen))})
#else
extern "C" {
#include "safestringlib/safe_mem_lib.h"
}
#define memcmp_s(dest, dmax, src, slen, diff) \
        memcmp_s((void *)(dest), dmax, (const void *)(src), (size_t)min((size_t)dmax, (size_t)slen), (int *)diff)
#endif

#endif /* _SAFE_MEM_H_ */
