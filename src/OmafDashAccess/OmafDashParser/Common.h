/*
 * Copyright (c) 2019, Intel Corporation
 *  * All rights reserved.
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
//! \file:   Common.h
//! \brief:  Include all system and data type header files, macros that needed.
//!
//! Created on April 4, 2019, 5:26 AM
//!

#ifndef COMMON_H
#define COMMON_H

#include "../../utils/ns_def.h"
#include "data_type.h"
#include "../../utils/GlogWrapper.h"
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <cstring>
#include <cstdint>
#include <climits>
#include <sstream>
#include <regex>

#include <semaphore.h>
#include <thread>
#include <condition_variable>

#include <list>
#include <map>
#include <unordered_set>
#include <vector>
#include <memory.h>

#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>

#include "../general.h"
#include "../../utils/Threadable.h"

//! \brief Return status
typedef int32_t ODStatus;
#define OD_STATUS_SUCCESS          0X00000000
#define OD_STATUS_INVALID          0X00000001
#define OD_STATUS_OPERATION_FAILED 0X00000002
#define OD_STATUS_THREAD           0X00000003
#define OD_STATUS_AGAIN            0X00000004

using namespace std;

//!
//! \enum   DownloaderStatus
//! \brief  Downloader Status type
//!
enum DownloaderStatus
{
    NOT_START   = 0,
    DOWNLOADING = 1,
    STOPPING    = 2,
    STOPPED     = 3,
    DOWNLOADED  = 4
};

//!
//! \brief    check status, return status if it doesn't equal to success
//!
//! \param    [in] status
//!           status need to be checked
//!
//! \return   ODStatus
//!           nothing if success, else status
//!
#define CheckAndReturn(status)                      \
{                                                   \
    if(status != OD_STATUS_SUCCESS)                \
    {                                               \
        return status;                              \
    }                                               \
}                                                   \

//!
//! \brief    check status, print log and return status
//!
//! \param    [in] status
//!           status need to be checked
//! \param    [in] log
//!           log message
//! \param    [in] level
//!           log level in GLOG
//!
//! \return   ODStatus
//!           nothing if success, else status
//!
#define CheckPrintLogAndReturn(status, log, level)  \
{                                                   \
    if(status != OD_STATUS_SUCCESS)                \
    {                                               \
        LOG(level)<<log<<endl;                      \
        return status;                              \
    }                                               \
}                                                   \

//!
//! \brief    check null ptr, print log and return null ptr
//!
//! \param    [in] ptr
//!           the ptr need to be checked
//! \param    [in] log
//!           log message
//! \param    [in] level
//!           log level in GLOG
//!
//! \return   nullptr
//!           nothing if success, else null pointer
//!
#define CheckNullPtr_PrintLog_ReturnNullPtr(ptr, log, level)  \
{                                                   \
    if(!ptr)                \
    {                                               \
        LOG(level)<<log<<endl;                      \
        return nullptr;                              \
    }                                               \
}                                                   \

//!
//! \brief    check null ptr, print log and return status
//!
//! \param    [in] ptr
//!           the ptr need to be checked
//! \param    [in] log
//!           log message
//! \param    [in] level
//!           log level in GLOG
//! \param    [in] status
//!           the return status
//!
//! \return   ODStatus
//!           nothing if success, else status
//!
#define CheckNullPtr_PrintLog_ReturnStatus(ptr, log, level, status)  \
{                                                   \
    if(!ptr)                \
    {                                               \
        LOG(level)<<log<<endl;                      \
        return status;                              \
    }                                               \
}                                                   \

//!
//! \brief    set and get function for class member
//!
//! \param    [in] Type
//!           member type
//! \param    [in] Member
//!           member in class
//! \param    [in] MemberName
//!           member name in class
//!
//! \return   void or Type
//!           nothing if set, else member type
//!
#define MEMBER_SET_AND_GET_FUNC(Type, Member, MemberName)    \
public:                                             \
    void Set##MemberName(Type v)                \
    {                                               \
        Member = v;                                    \
    }                                               \
    Type Get##MemberName()                             \
    {                                               \
        return Member;                                 \
    }                                               \

#endif /* COMMON_H */
