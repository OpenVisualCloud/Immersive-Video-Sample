/*
 * Copyright (c) 2018, Intel Corporation
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

/* 
 * File:   ns_def.h
 * Author: Zhang, Andrew
 *
 * Created on December 28, 2018, 10:16 AM
 */

#ifndef NS_DEF_H
#define NS_DEF_H

#define VCD_MP4_BEGIN namespace VCD { namespace MP4 {
#define VCD_MP4_END }}

#define VCD_USE_MP4 using namespace VCD::MP4

#define VCD_NS_BEGIN namespace VCD { namespace VRVideo {
#define VCD_NS_END }}

#define VCD_USE_VRVIDEO using namespace VCD::VRVideo

#define VCD_VRVIDEO_NS VCD::VRVideo

#define VCD_OMAF_BEGIN namespace VCD { namespace OMAF {
#define VCD_OMAF_END }}

#define VCD_USE_VROMAF using namespace VCD::OMAF

#define VCD_VROMAF_NS VCD::OMAF

#endif /* NS_DEF_H */
