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

 *
 */


//!
//! \file:   common.h
//! \brief:  Include the common system and data type header files that needed
//!
//! Created on April 30, 2019, 6:04 AM
//!

#ifndef _COMMON_H_
#define _COMMON_H_

#include "../../../utils/ns_def.h"
#include "RenderType.h"
#include "data_type.h"
#include "../../../utils/safe_mem.h"
#include "../../../utils/OmafStructure.h"
#include "../../../utils/GlogWrapper.h"

#ifdef _ANDROID_OS_
#include <android/log.h>
#define LOG_TAG "Player"
#define ANDROID_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#endif

#define SAFE_DELETE(x) if(NULL != (x)) { delete (x); (x)=NULL; };
#define SAFE_FREE(x)   if(NULL != (x)) { free((x));    (x)=NULL; };
#define SAFE_DELETE_ARRAY(x) if(NULL != (x)) { delete [] (x); (x)=NULL; };

#endif /* _COMMON_H_ */
