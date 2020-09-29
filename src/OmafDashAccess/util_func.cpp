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


/*!
 *	\brief NTP seconds from 1900 to 1970
 *	\hideinitializer
 *
 *	Macro giving the number of seconds from from 1900 to 1970
*/

#include "general.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>

VCD_OMAF_BEGIN

#define NTP_SEC_1900_TO_1970 2208988800ul

static uint32_t ntp_shift         = NTP_SEC_1900_TO_1970;
static uint32_t sys_start_time    = 0;
static uint64_t sys_start_time_hr = 0;

uint32_t sys_clock()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return (uint32_t) ( ( (now.tv_sec)*1000 + (now.tv_usec) / 1000) - sys_start_time );
}

uint64_t sys_clock_high_res()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_sec)*1000000 + (now.tv_usec) - sys_start_time_hr;
}

void net_set_ntp_shift(int32_t shift)
{
    ntp_shift = NTP_SEC_1900_TO_1970 + shift;
}

uint64_t net_parse_date(const char *val)
{
    uint64_t current_time;
    char szDay[50], szMonth[50];
    int32_t curr_year, curr_month, curr_day, curr_hour, curr_min, curr_sec, ms;
    int32_t oh, om;
    float seconds;
    bool neg_time_zone = false;

    struct tm t;
    memset(&t, 0, sizeof(struct tm));

    szDay[0] = szMonth[0] = 0;
    curr_year = curr_month = curr_day = curr_hour = curr_min = curr_sec = 0;
    oh = om = 0;
    seconds = 0;

    if (sscanf(val, "%d-%d-%dT%d:%d:%gZ", &curr_year, &curr_month, &curr_day, &curr_hour, &curr_min, &seconds) == 6) {
    }
    else if (sscanf(val, "%d-%d-%dT%d:%d:%g-%d:%d", &curr_year, &curr_month, &curr_day, &curr_hour, &curr_min, &seconds, &oh, &om) == 8) {
        neg_time_zone = true;
    }
    else if (sscanf(val, "%d-%d-%dT%d:%d:%g+%d:%d", &curr_year, &curr_month, &curr_day, &curr_hour, &curr_min, &seconds, &oh, &om) == 8) {
    }
    else if (sscanf(val, "%3s, %d %3s %d %d:%d:%d", szDay, &curr_day, szMonth, &curr_year, &curr_hour, &curr_min, &curr_sec)==7) {
        seconds  = (float) curr_sec;
    }
    else if (sscanf(val, "%9s, %d-%3s-%d %02d:%02d:%02d GMT", szDay, &curr_day, szMonth, &curr_year, &curr_hour, &curr_min, &curr_sec)==7) {
        seconds  = (float) curr_sec;
    }
    else if (sscanf(val, "%3s %3s %d %02d:%02d:%02d %d", szDay, szMonth, &curr_day, &curr_year, &curr_hour, &curr_min, &curr_sec)==7) {
        seconds  = (float) curr_sec;
    }
    else {
        OMAF_LOG(LOG_ERROR, "[Core] Cannot parse date string %s\n", val);
        return 0;
    }
    if (curr_month <= 12 && curr_month >= 0) {
    if (curr_month) {
        curr_month -= 1;
    } else {
        if (!strcmp(szMonth, "Jan")) curr_month = 0;
        else if (!strcmp(szMonth, "Feb")) curr_month = 1;
        else if (!strcmp(szMonth, "Mar")) curr_month = 2;
        else if (!strcmp(szMonth, "Apr")) curr_month = 3;
        else if (!strcmp(szMonth, "May")) curr_month = 4;
        else if (!strcmp(szMonth, "Jun")) curr_month = 5;
        else if (!strcmp(szMonth, "Jul")) curr_month = 6;
        else if (!strcmp(szMonth, "Aug")) curr_month = 7;
        else if (!strcmp(szMonth, "Sep")) curr_month = 8;
        else if (!strcmp(szMonth, "Oct")) curr_month = 9;
        else if (!strcmp(szMonth, "Nov")) curr_month = 10;
        else if (!strcmp(szMonth, "Dec")) curr_month = 11;
    }
    }
    if (curr_year > INT32_MAX - 1 || curr_year < 0) return 0;
    t.tm_year = curr_year>1000 ? curr_year-1900 : curr_year;
    t.tm_mday = curr_day;
    t.tm_hour = curr_hour;
    t.tm_min = curr_min;
    t.tm_sec = (uint32_t) seconds;
    t.tm_mon = curr_month;

    if (strlen(szDay) ) {
        if (!strcmp(szDay, "Mon") || !strcmp(szDay, "Monday")) t.tm_wday = 0;
        else if (!strcmp(szDay, "Tue") || !strcmp(szDay, "Tuesday")) t.tm_wday = 1;
        else if (!strcmp(szDay, "Wed") || !strcmp(szDay, "Wednesday")) t.tm_wday = 2;
        else if (!strcmp(szDay, "Thu") || !strcmp(szDay, "Thursday")) t.tm_wday = 3;
        else if (!strcmp(szDay, "Fri") || !strcmp(szDay, "Friday")) t.tm_wday = 4;
        else if (!strcmp(szDay, "Sat") || !strcmp(szDay, "Saturday")) t.tm_wday = 5;
        else if (!strcmp(szDay, "Sun") || !strcmp(szDay, "Sunday")) t.tm_wday = 6;
    }

    current_time = mktime_utc(&t);

    if ((int64_t) current_time == -1) {
        //use 1 ms
        return 1;
    }
    if (current_time == 0) {
        //use 1 ms
        return 1;
    }

    if (om > 0 && om <= 60 && oh > 0 && oh <= 12) {
        int32_t diff = (60*oh + om)*60;
        if (neg_time_zone) diff = -diff;
        current_time = current_time + diff;
    }
    current_time *= 1000;
    if (current_time > UINT64_MAX - 1) return 0;
    if (seconds > 10000000000) return 0;
    uint32_t currs = seconds - (uint32_t) seconds;
    if (currs >= UINT32_MAX - 1) return 0;
    uint32_t currms = currs * 1000;
    if (currms < UINT32_MAX - 1)
    {
        ms = currms;
    }
    else
    {
        OMAF_LOG(LOG_ERROR, "invalid ms input!\n");
        return 0;
    }
    uint64_t ret_time = current_time + ms;
    if (ret_time < UINT64_MAX - 1)
        return ret_time;
    else
        return 0;

}

uint64_t net_get_utc()
{
    uint64_t current_time;
    double msec;
    uint32_t sec, frac;

    net_get_ntp(&sec, &frac);
    current_time = sec - NTP_SEC_1900_TO_1970;
    current_time *= 1000;
    msec = frac*1000.0;
    msec /= 0xFFFFFFFF;
    current_time += (uint64_t) msec;
    return current_time;
}

void net_get_ntp(uint32_t *sec, uint32_t *frac)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    *sec = (uint32_t) (now.tv_sec) + NTP_SEC_1900_TO_1970;
    *frac = (uint32_t) ( (now.tv_usec << 12) + (now.tv_usec << 8) - ((now.tv_usec * 3650) >> 6) );
}

uint64_t net_get_ntp_ts()
{
    uint64_t res;
    uint32_t sec, frac;
    net_get_ntp(&sec, &frac);
    res = sec;
    res<<= 32;
    res |= (uint64_t)frac;
    return res;
}

int32_t net_get_ntp_diff_ms(uint64_t ntp)
{
    uint32_t remote_s, remote_f, local_s, local_f;
    int64_t local, remote;

    remote_s = (ntp >> 32);
    remote_f = (uint32_t) (ntp & 0xFFFFFFFFULL);
    net_get_ntp(&local_s, &local_f);

    local = local_s;
    local *= 1000;
    local += ((uint64_t) local_f)*1000 / 0xFFFFFFFFULL;

    remote = remote_s;
    remote *= 1000;
    remote += ((uint64_t) remote_f)*1000 / 0xFFFFFFFFULL;

    return (int32_t) (local - remote);
}

int32_t net_get_timezone()
{
    //this has been commented due to some reports of broken implementation on some systems ...
    //		s32 val = timezone;
    //		return val;
    /*FIXME - avoid errors at midnight when estimating timezone this does not work !!*/
    int32_t t_timezone;
    tm *t_gmt = nullptr, *t_local = nullptr;
    time_t t_time;
    t_time = time(NULL);
    t_gmt = gmtime(&t_time);
    if(!t_gmt)
        return 0;
    t_local = localtime(&t_time);
    if(!t_local)
        return 0;

    t_timezone = (t_gmt->tm_hour - t_local->tm_hour) * 3600 + (t_gmt->tm_min - t_local->tm_min) * 60;
    return t_timezone;

}

time_t mktime_utc(struct tm *tm)
{
    return timegm(tm);
}

bool parse_bool(const char * const attr)
{
    if (!strcmp(attr, "true")) return 1;
    if (!strcmp(attr, "1")) return 1;
    return ERROR_NONE;
}

uint32_t parse_int(const char * const attr)
{
    return atoi(attr);
}

uint64_t parse_long_int(const char * const attr)
{
    uint64_t longint;
    sscanf(attr, "%lu", &longint);
    return longint;
}

double parse_double(const char * const attr)
{
    return atof(attr);
}

uint64_t parse_date(const char * const attr)
{
    return net_parse_date(attr);
}

uint64_t parse_duration(const char * const duration)
{
    uint32_t i;
    char *sep1, *sep2;
    uint32_t h, m;
    double s;
    const char *startT;
    if (!duration) {
        OMAF_LOG(LOG_ERROR, "[MPD] Error parsing duration: no value indicated\n");
        return ERROR_PARSE;
    }
    i = 0;
    while (1) {
        if (duration[i] == ' ') i++;
        else if (duration[i] == 0) return 0;
        else {
            break;
        }
    }
    if (duration[i] != 'P') {
        OMAF_LOG(LOG_ERROR, "[MPD] Error parsing duration: no value indicated\n");
        return ERROR_PARSE;
    }
    startT = strchr(duration+1, 'T');

    if (duration[i+1] == 0) {
        OMAF_LOG(LOG_ERROR, "[MPD] Error parsing duration: no value indicated\n");
        return ERROR_PARSE;
    }

    if (! startT) {
        OMAF_LOG(LOG_ERROR, "[MPD] Error parsing duration: no Time section found\n");
        return ERROR_PARSE;
    }

    h = m = 0;
    s = 0;
    if (NULL != (sep1 = strchr(const_cast<char*>(startT)+1, 'H'))) {
        *sep1 = 0;
        h = atoi(duration+i+2);
        *sep1 = 'H';
        sep1++;
    } else {
        sep1 = (char *) startT+1;
    }
    if (NULL != (sep2 = strchr(sep1, 'M'))) {
        *sep2 = 0;
        m = atoi(sep1);
        *sep2 = 'M';
        sep2++;
    } else {
        sep2 = sep1;
    }

    if (NULL != (sep1 = strchr(sep2, 'S'))) {
        *sep1 = 0;
        s = atof(sep2);
        *sep1 = 'S';
    }
    if (h < 1000 && m < 1000)
    {
        uint64_t tmp_h = h * 3600 * 1000;
        uint64_t tmp_m = m * 60 * 1000;
        if (tmp_h > UINT64_MAX - 1 || tmp_m > UINT64_MAX - 1) return 0;
        uint64_t tmp_time = tmp_h + tmp_m + s * 1000;
        if (tmp_time < UINT64_MAX - 1) {
            return tmp_time;
        }
    }
    else
    {
        OMAF_LOG(LOG_ERROR, "[MPD] Error parsing duration: time overflow\n");
        return ERROR_PARSE;
    }
    return 0;
}

uint32_t mpd_parse_duration_u32(const char* const duration)
{
    uint64_t dur = parse_duration(duration);
    if (dur <= UINT_MAX) {
        return (uint32_t)dur;
    } else {
        OMAF_LOG(LOG_ERROR, "[MPD] Parsed duration %ld doesn't fit on 32 bits! Setting to the 32 bits max.\n", dur);
        return UINT_MAX;
    }
}


void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c)
{
    std::string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(std::string::npos != pos2){
        v.push_back(s.substr(pos1, pos2 - pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }

    if(pos1 != s.length())
        v.push_back(s.substr(pos1));
}

std::string GetSubstr(std::string str, char sep, bool bBefore)
{
    std::string ret = "";
    std::size_t found = str.rfind(sep);
    if(found != std::string::npos){
        if(bBefore){
            ret = str.substr(0, found);
        }else{
            ret = str.substr( found+1, str.length()-1-found );
        }
    }
    return ret;
}

char *strlwr(char *s)
{
    char *str;
    str = s;
    while(*str != '\0')
    {
        if(*str >= 'A' && *str <= 'Z') {
            *str += 'a'-'A';
        }
        str++;
    }
    return s;
 }

// function to splice basePath + appendedPath
std::string PathSplice( std::string basePath, std::string appendedPath)
{
    uint32_t baseLen = basePath.length();
    uint32_t appendedLen = appendedPath.length();
    if(!baseLen) return appendedPath;
    if(!appendedLen) return basePath;

    string splicedPath;
    if(basePath.back() == '/' && appendedPath[0] == '/')
    {
        splicedPath = basePath.append(appendedPath.substr(1, appendedLen - 1));
    }
    else if(basePath.back() == '/' || appendedPath[0] == '/')
    {
        splicedPath = basePath + appendedPath;
    }
    else
    {
        splicedPath = basePath + "/" + appendedPath;
    }

    return splicedPath;
}

int32_t StringToInt(string str)
{
    if(!str.length())
        return -11;

    return stoi(str);
}

VCD_OMAF_END
