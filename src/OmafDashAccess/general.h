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

/*
 * File:   general.h
 * Author: media
 *
 * Created on May 22, 2019, 12:34 PM
 */

#ifndef GENERAL_H
#define GENERAL_H

#include <string>
#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "../utils/ns_def.h"
#include "../utils/error.h"
#include "../utils/data_type.h"
#include "../utils/Threadable.h"
#include "../utils/Singleton.h"
#include "OmafDashAccessLog.h"
#include "glog/logging.h"
#include "OmafStructure.h"

#ifdef _ANDROID_NDK_OPTION_
#include <android/log.h>
#define LOG_TAG "OmafDashAccess"
#define ANDROID_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#endif

#define SAFE_DELETE(x) \
  if (NULL != (x)) {   \
    delete (x);        \
    (x) = NULL;        \
  };
#define SAFE_DELARRAY(x) \
  if (NULL != (x)) {     \
    delete [] (x);       \
    (x) = NULL;          \
  };
#define SAFE_FREE(x) \
  if (NULL != (x)) { \
    free((x));       \
    (x) = NULL;      \
  };

VCD_OMAF_BEGIN

typedef struct SourceInfo {
  QualityRank qualityRanking;
  int32_t width;
  int32_t height;
} SourceInfo;

typedef struct FRACTIONAL {
  uint32_t num;
  uint32_t den;
} Fractional;

typedef struct VIDEOINFO {
  uint32_t width;
  uint32_t height;
  Fractional frame_Rate;
  uint32_t bit_rate;
  Fractional sar;
} VideoInfo;

typedef struct AUDIOINFO {
  uint32_t channels;       /// for audio
  uint32_t channel_bytes;  /// for audio
  uint32_t sample_rate;    /// for audio
} AudioInfo;

//!
//! \brief function to parse string to data type
//!
bool parse_bool(const char* const attr);
uint32_t parse_int(const char* const attr);
uint64_t parse_long_int(const char* const attr);
double parse_double(const char* const attr);
uint64_t parse_date(const char* const attr);
uint64_t parse_duration(const char* const duration);
uint32_t parse_duration_u32(const char* const duration);
uint32_t sys_clock();
uint64_t sys_clock_high_res();
//!
//! \brief function to deal with time
//!
time_t mktime_utc(struct tm* tm);
int32_t net_get_timezone();
int32_t net_get_ntp_diff_ms(uint64_t ntp);
uint64_t net_get_ntp_ts();
void net_get_ntp(uint32_t* sec, uint32_t* frac);
uint64_t net_get_utc();
void net_set_ntp_shift(int32_t shift);
uint64_t net_parse_date(const char* val);

//!
//! \brief function to deal with string
//!
void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c);
std::string GetSubstr(std::string str, char sep, bool bBefore);
char* strlwr(char* s);

std::string PathSplice(std::string basePath, std::string appendedPath);

int32_t StringToInt(string str);

VCD_OMAF_END;

#endif /* GENERAL_H */
