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
//! \file:   VROmafPackingLog.cpp
//! \brief:  Include the log function implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include "Log.h"

extern "C"
{
#include "safestringlib/safe_mem_lib.h"
}

//LogFunction logCallBack = GlogFunction;

ParamVariant GetParameter(const char *type, va_list params)
{
    if (!type)
        return -1;

    if (strncmp(type, "char", 4) == 0)
        return va_arg(params, int32_t);
    else if (strncmp(type, "string", 6) == 0)
        return va_arg(params, char*);
    else if (strncmp(type, "uint32_t", 8) == 0)
        return va_arg(params, uint32_t);
    else if (strncmp(type, "int32_t", 7) == 0)
        return va_arg(params, int32_t);
    else if (strncmp(type, "int64_t", 7) == 0)
        return va_arg(params, int64_t);
    else if (strncmp(type, "\0", 1) == 0)
        return '!';

    return -1;
}

void GlogSupport3Params(LogLevel logLevel, const char* sourceFile, uint64_t line, std::list<std::pair<const char*, char*>>& tempStrs, va_list params)
{
    if (tempStrs.size() != 3)
        return;

    std::list<std::pair<const char*, char*>>::iterator it  = tempStrs.begin();
    std::list<std::pair<const char*, char*>>::iterator it1 = tempStrs.begin();
    it1++;
    std::list<std::pair<const char*, char*>>::iterator it2 = tempStrs.begin();
    it2++;
    it2++;

    ParamVariant param0 = GetParameter(it->first, params);
    ParamVariant param1 = GetParameter(it1->first, params);
    ParamVariant param2 = GetParameter(it2->first, params);

    char *newFmt0 = it2->second;
    char *newFmt1 = newFmt0;
    char *newFmt2 = strchr(newFmt1, '\n');
    char *newFmt3 = NULL;
    if (newFmt2)
    {
        newFmt3 = new char[1024];
        memset_s(newFmt3, 1024, 0);
        uint32_t moveCnt = newFmt2 - newFmt0;
        memcpy_s(newFmt3, moveCnt, newFmt0, moveCnt);
        newFmt3[moveCnt] = '\0';
    }

    switch (logLevel)
    {
        case LOG_INFO:
        {
            LOG(INFO) << "" << sourceFile << ":" << line << "  " << (it->second) << " " << param0 << " " << (it1->second) << " " << param1 << " " << (newFmt3 ? newFmt3 : newFmt0) << " " << param2 << std::endl;
            break;
        }
        case LOG_WARNING:
        {
            LOG(WARNING) << "" << sourceFile << ":" << line << "  " << (it->second) << " " << param0 << " " << (it1->second) << " " << param1 << " " << (newFmt3 ? newFmt3 : newFmt0) << " " << param2 << std::endl;
            break;
        }
        case LOG_ERROR:
        {
            LOG(ERROR) << "" << sourceFile << ":" << line << "  " << (it->second) << " " << param0 << " " << (it1->second) << " " << param1 << " " << (newFmt3 ? newFmt3 : newFmt0) << " " << param2 << std::endl;
            break;
        }
        case LOG_FATAL:
        {
            LOG(FATAL) << "" << sourceFile << ":" << line << "  " << (it->second) << " " << param0 << " " << (it1->second) << " " << param1 << " " << (newFmt3 ? newFmt3 : newFmt0) << " " << param2 << std::endl;
            break;
        }
        default:
        {
            LOG(ERROR) << "Invalid log level !" << std::endl;
            break;
        }
    }

    if (newFmt3)
    {
        delete [] newFmt3;
        newFmt3 = NULL;
    }

    return;
}

void GlogSupport2Params(LogLevel logLevel, const char* sourceFile, uint64_t line, std::list<std::pair<const char*, char*>>& tempStrs, va_list params)
{
    if (tempStrs.size() != 2)
        return;

    std::list<std::pair<const char*, char*>>::iterator it = tempStrs.begin();
    std::list<std::pair<const char*, char*>>::iterator it1 = tempStrs.begin();
    it1++;

    ParamVariant param0 = GetParameter(it->first, params);
    ParamVariant param1 = GetParameter(it1->first, params);

    char *newFmt0 = it1->second;
    char *newFmt1 = newFmt0;
    char *newFmt2 = strchr(newFmt1, '\n');
    char *newFmt3 = NULL;
    if (newFmt2)
    {
        newFmt3 = new char[1024];
        memset_s(newFmt3, 1024, 0);
        uint32_t moveCnt = newFmt2 - newFmt0;
        memcpy_s(newFmt3, moveCnt, newFmt0, moveCnt);
        newFmt3[moveCnt] = '\0';
    }

    switch (logLevel)
    {
        case LOG_INFO:
        {
            LOG(INFO) << "" << sourceFile << ":" << line << "  " << (it->second) << " " << param0 << " " << (newFmt3 ? newFmt3 : newFmt0) << " " << param1 << std::endl;
            break;
        }
        case LOG_WARNING:
        {
            LOG(WARNING) << "" << sourceFile << ":" << line << "  " << (it->second) << " " << param0 << " " << (newFmt3 ? newFmt3 : newFmt0) << " " << param1 << std::endl;
            break;
        }
        case LOG_ERROR:
        {
            LOG(ERROR) << "" << sourceFile << ":" << line << "  " << (it->second) << " " << param0 << " " << (newFmt3 ? newFmt3 : newFmt0) << " " << param1 << std::endl;
            break;
        }
        case LOG_FATAL:
        {
            LOG(FATAL) << "" << sourceFile << ":" << line << "  " << (it->second) << " " << param0 << " " << (newFmt3 ? newFmt3 : newFmt0) << " " << param1 << std::endl;
            break;
        }
        default:
        {
            LOG(ERROR) << "Invalid log level !" << std::endl;
            break;
        }
    }

    if (newFmt3)
    {
        delete [] newFmt3;
        newFmt3 = NULL;
    }

    return;
}

void GlogSupport1Param(LogLevel logLevel, const char* sourceFile, uint64_t line, std::list<std::pair<const char*, char*>>& tempStrs, va_list params)
{
    if (tempStrs.size() != 1)
        return;

    std::list<std::pair<const char*, char*>>::iterator it = tempStrs.begin();
    char *newFmt0 = it->second;
    char *newFmt1 = newFmt0;
    char *newFmt2 = strchr(newFmt1, '\n');
    char *newFmt3 = NULL;
    if (newFmt2)
    {
        newFmt3 = new char[1024];
        memset_s(newFmt3, 1024, 0);
        uint32_t moveCnt = newFmt2 - newFmt0;
        memcpy_s(newFmt3, moveCnt, newFmt0, moveCnt);
        newFmt3[moveCnt] = '\0';
    }

    switch (logLevel)
    {
        case LOG_INFO:
        {
            LOG(INFO) << "" << sourceFile << ":" << line << "  " << (newFmt3 ? newFmt3 : newFmt0) << " " << GetParameter(it->first, params) << std::endl;
            break;
        }
        case LOG_WARNING:
        {
            LOG(WARNING) << "" << sourceFile << ":" << line << "  " << (newFmt3 ? newFmt3 : newFmt0) << " " << GetParameter(it->first, params) << std::endl;
            break;
        }
        case LOG_ERROR:
        {
            LOG(ERROR) << "" << sourceFile << ":" << line << "  " << (newFmt3 ? newFmt3 : newFmt0) << " " << GetParameter(it->first, params) << std::endl;
            break;
        }
        case LOG_FATAL:
        {
            LOG(FATAL) << "" << sourceFile << ":" << line << "  " << (newFmt3 ? newFmt3 : newFmt0) << " " << GetParameter(it->first, params) << std::endl;
            break;
        }
        default:
        {
            LOG(ERROR) << "Invalid log level !" << std::endl;
            break;
        }
    }

    if (newFmt3)
    {
        delete [] newFmt3;
        newFmt3 = NULL;
    }

    return;
}

void GlogFunction(LogLevel logLevel, const char* sourceFile, uint64_t line, const char* fmt, ...)
{
    if (!fmt)
        return;

    char *tempFmt1 = (char*)(fmt);
    char *tempFmt2 = NULL;
    char *tempFmt3 = NULL;
    va_list params;
    va_start(params, fmt);
    std::list<std::pair<const char*, char*>> tempStrs;

    while(tempFmt1 && (*tempFmt1 != '\0') && (*tempFmt1 != '\n'))
    {
        tempFmt2 = tempFmt1;
        tempFmt3 = strchr(tempFmt1, '%');
        if (tempFmt3 == tempFmt2)
        {
            while(tempFmt3 && (*tempFmt3 != '\0'))
            {
                if (*tempFmt3 == ' ')
                    break;

                tempFmt3++;
            }
            tempFmt1 = tempFmt3;
            continue;
        }

        if (tempFmt3 && (*tempFmt3 != '\0'))
        {
            uint64_t charCnt = tempFmt3 - tempFmt2;
            char *str = new char[1024];
            if (!str)
            {
                std::list<std::pair<const char*, char*>>::iterator it;
                for (it = tempStrs.begin(); it != tempStrs.end(); )
                {
                    if (it->second)
                    {
                        delete [] (it->second);
                        it->second = NULL;
                    }
                    tempStrs.erase(it++);
                }
                tempStrs.clear();
            }
            memset_s(str, 1024, 0);
            memcpy_s(str, charCnt, tempFmt2, charCnt);
            str[charCnt] = '\0';
            char type1 = *(tempFmt3+1);
            char type2 = *(tempFmt3+2);
            char type3 = *(tempFmt3+3);
            if (type1 == 'c')
            {
                tempStrs.push_back(std::make_pair("char", str));
            }
            else if (type1 == 's')
            {
                tempStrs.push_back(std::make_pair("string", str));
            }
            else if (type1 == 'u')
            {
                tempStrs.push_back(std::make_pair("uint32_t", str));
            }
            else if (type1 == 'd')
            {
                tempStrs.push_back(std::make_pair("int32_t", str));
            }
            else if ((type1 == 'l') && (type2 == 'd'))
            {
                tempStrs.push_back(std::make_pair("int32_t", str));
            }
            else if ((type1 == 'l') && (type2 == 'l') && (type3 == 'd'))
            {
                tempStrs.push_back(std::make_pair("int64_t", str));
            }
        }
        else
        {
            char *str = new char[1024];
            if (!str)
            {
                std::list<std::pair<const char*, char*>>::iterator it;
                for (it = tempStrs.begin(); it != tempStrs.end(); )
                {
                    if (it->second)
                    {
                        delete [] (it->second);
                        it->second = NULL;
                    }
                    tempStrs.erase(it++);
                }
                tempStrs.clear();
            }
            memset_s(str, 1024, 0);
            memcpy_s(str, strlen(tempFmt2), tempFmt2, strlen(tempFmt2));
            str[strlen(tempFmt2)] = '\0';
            tempStrs.push_back(std::make_pair("\0", str));
        }
        tempFmt1 = tempFmt3;
    }

    uint64_t strCnt = tempStrs.size();
    if (strCnt > 3)
    {
        LOG(ERROR) << "Now more than three parameters are not supported in glog output !" << std::endl;
    }

    switch (strCnt)
    {
        case 3:
        {
            GlogSupport3Params(logLevel, sourceFile, line, tempStrs, params);
            break;
        }
        case 2:
        {
            GlogSupport2Params(logLevel, sourceFile, line, tempStrs, params);
            break;
        }
        case 1:
        {
            GlogSupport1Param(logLevel, sourceFile, line, tempStrs, params);
            break;
        }
        default:
        {
            LOG(ERROR) << "Invalid log message format !" << std::endl;
        }
    }

    va_end(params);

    std::list<std::pair<const char*, char*>>::iterator it;
    for (it = tempStrs.begin(); it != tempStrs.end(); )
    {
        if (it->second)
        {
            delete [] (it->second);
            it->second = NULL;
        }
        tempStrs.erase(it++);
    }
    tempStrs.clear();

    return;
}
