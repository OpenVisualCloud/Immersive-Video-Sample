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

#include "safe_mem.h"

#define GetOneParamValue(type, param)              \
    ((strncmp(type, "char", 4) == 0) ? param.charParam : ((strncmp(type, "uint32_t", 8) == 0) ? param.uint32Param : ((strncmp(type, "int32_t", 7) == 0) ? param.int32Param : ((strncmp(type, "int64_t", 7) == 0) ? param.int64Param : ((strncmp(type, "\0", 1) == 0) ? param.charParam : '!')))))       \

#define Print2ParamCharPlusChar(logLevel,sourceFile,line,fmt1,param0,fmt2,param1) \
    LOG(logLevel) << "" << sourceFile << ":" << line << "  " << fmt1 << " " << param0.charParam << " " << fmt2 << " " << param1.charParam << std::endl;\

#define Print2ParamCharPlusString(logLevel,sourceFile,line,fmt1,param0,fmt2,param1) \
    LOG(logLevel) << "" << sourceFile << ":" << line << "  " << fmt1 << " " << param0.charParam << " " << fmt2 << " " << param1.stringParam << std::endl;\

#define Print2ParamStringPlusString(logLevel,sourceFile,line,fmt1,param0,fmt2,param1) \
    LOG(logLevel) << "" << sourceFile << ":" << line << "  " << fmt1 << " " << param0.stringParam << " " << fmt2 << " " << param1.stringParam << std::endl;\

#define Print2ParamStringPlusChar(logLevel,sourceFile,line,fmt1,param0,fmt2,param1) \
    LOG(logLevel) << "" << sourceFile << ":" << line << "  " << fmt1 << " " << param0.stringParam << " " << fmt2 << " " << param1.charParam << std::endl;\

#define Print2ParamStringPlusInt(logLevel,sourceFile,line,fmt1,param0,fmt2,type,param1) \
    LOG(logLevel) << "" << sourceFile << ":" << line << "  " << fmt1 << " " << param0.stringParam << " " << fmt2 << " " << GetOneParamValue(type,param1) << std::endl;\

#define Print2ParamCharPlusInt(logLevel,sourceFile,line,fmt1,param0,fmt2,type,param1) \
    LOG(logLevel) << "" << sourceFile << ":" << line << "  " << fmt1 << " " << param0.charParam << " " << fmt2 << " " << GetOneParamValue(type,param1) << std::endl;\

#define Print2ParamIntPlusChar(logLevel,sourceFile,line,fmt1,type,param0,fmt2,param1) \
    LOG(logLevel) << "" << sourceFile << ":" << line << "  " << fmt1 << " " << GetOneParamValue(type,param0) << " " << fmt2 << " " << param1.charParam << std::endl;\

#define Print2ParamIntPlusString(logLevel,sourceFile,line,fmt1,type,param0,fmt2,param1) \
    LOG(logLevel) << "" << sourceFile << ":" << line << "  " << fmt1 << " " << GetOneParamValue(type,param0) << " " << fmt2 << " " << param1.stringParam << std::endl;\

#define Print2ParamIntPlusInt(logLevel,sourceFile,line,fmt1,type0,param0,fmt2,type1,param1) \
    LOG(logLevel) << "" << sourceFile << ":" << line << "  " << fmt1 << " " << GetOneParamValue(type0,param0) << " " << fmt2 << " " << GetOneParamValue(type1,param1) << std::endl;\

#define Print2ParamCharPlusDouble(logLevel,sourceFile,line,fmt1,param0,fmt2,param1) \
    LOG(logLevel) << "" << sourceFile << ":" << line << "  " << fmt1 << " " << param0.charParam << " " << fmt2 << " " << param1.doubleParam << std::endl;\

#define Print2ParamStringPlusDouble(logLevel,sourceFile,line,fmt1,param0,fmt2,param1) \
    LOG(logLevel) << "" << sourceFile << ":" << line << "  " << fmt1 << " " << param0.stringParam << " " << fmt2 << " " << param1.doubleParam << std::endl;\

#define Print2ParamIntPlusDouble(logLevel,sourceFile,line,fmt1,type,param0,fmt2,param1) \
    LOG(logLevel) << "" << sourceFile << ":" << line << "  " << fmt1 << " " << GetOneParamValue(type,param0) << " " << fmt2 << " " << param1.doubleParam << std::endl;\

#define Print2ParamDoublePlusDouble(logLevel,sourceFile,line,fmt1,param0,fmt2,param1) \
    LOG(logLevel) << "" << sourceFile << ":" << line << "  " << fmt1 << " " << param0.doubleParam << " " << fmt2 << " " << param1.doubleParam << std::endl;\

#define Print2ParamDoublePlusChar(logLevel,sourceFile,line,fmt1,param0,fmt2,param1) \
    LOG(logLevel) << "" << sourceFile << ":" << line << "  " << fmt1 << " " << param0.doubleParam << " " << fmt2 << " " << param1.charParam << std::endl;\

#define Print2ParamDoublePlusInt(logLevel,sourceFile,line,fmt1,param0,fmt2,type,param1) \
    LOG(logLevel) << "" << sourceFile << ":" << line << "  " << fmt1 << " " << param0.doubleParam << " " << fmt2 << " " << GetOneParamValue(type,param1) << std::endl;\

#define Print2ParamDoublePlusString(logLevel,sourceFile,line,fmt1,param0,fmt2,param1) \
    LOG(logLevel) << "" << sourceFile << ":" << line << "  " << fmt1 << " " << param0.doubleParam << " " << fmt2 << " " << param1.stringParam << std::endl;\

void GlogSupport2Params(LogLevel logLevel, const char* sourceFile, uint64_t line, std::list<std::pair<const char*, char*>>& tempStrs, va_list params)
{
    if (tempStrs.size() != 2)
        return;

    std::list<std::pair<const char*, char*>>::iterator it = tempStrs.begin();
    std::list<std::pair<const char*, char*>>::iterator it1 = tempStrs.begin();
    it1++;

    ParamValue param0;
    ParamValue param1;
    const char *type0 = it->first;
    const char *type1 = it1->first;

    if (strncmp(type0, "char", 4) == 0)
    {
        param0.charParam = va_arg(params, int32_t);
    }
    else if (strncmp(type0, "string", 6) == 0)
    {
        param0.stringParam = va_arg(params, char*);
    }
    else if (strncmp(type0, "uint32_t", 8) == 0)
    {
        param0.uint32Param = va_arg(params, uint32_t);
    }
    else if (strncmp(type0, "int32_t", 7) == 0)
    {
        param0.int32Param = va_arg(params, int32_t);
    }
    else if (strncmp(type0, "int64_t", 7) == 0)
    {
        param0.int64Param = va_arg(params, int64_t);
    }
    else if (strncmp(type0, "double", 6) == 0)
    {
        param0.doubleParam = va_arg(params, double);
    }
    else if (strncmp(type0, "\0", 2) == 0)
    {
        param0.charParam = '!';
    }

    if (strncmp(type1, "char", 4) == 0)
    {
        param1.charParam = va_arg(params, int32_t);
    }
    else if (strncmp(type1, "string", 6) == 0)
    {
        param1.stringParam = va_arg(params, char*);
    }
    else if (strncmp(type1, "uint32_t", 8) == 0)
    {
        param1.uint32Param = va_arg(params, uint32_t);
    }
    else if (strncmp(type1, "int32_t", 7) == 0)
    {
        param1.int32Param = va_arg(params, int32_t);
    }
    else if (strncmp(type1, "int64_t", 7) == 0)
    {
        param1.int64Param = va_arg(params, int64_t);
    }
    else if (strncmp(type1, "double", 6) == 0)
    {
        param1.doubleParam = va_arg(params, double);
    }
    else if (strncmp(type1, "\0", 2) == 0)
    {
        param1.charParam = '!';
    }

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
            if ((strncmp(type0, "string", 6) == 0) && (strncmp(type1, "string", 6) == 0))
            {
                Print2ParamStringPlusString(INFO,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "string", 6) == 0) && ((strncmp(type1, "char", 4) == 0) || (strncmp(type1, "\0", 2) == 0)))
            {
                Print2ParamStringPlusChar(INFO,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "char", 4) == 0) && ((strncmp(type1, "char", 4) == 0) || (strncmp(type1, "\0", 2) == 0)))
            {
                Print2ParamCharPlusChar(INFO,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "char", 4) == 0) && (strncmp(type1, "string", 6) == 0))
            {
                Print2ParamCharPlusString(INFO,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "char", 4) == 0) && ((strncmp(type1, "string", 6) != 0) && (strncmp(type1, "char", 4) != 0) && (strncmp(type1, "double", 6) != 0)))
            {
                Print2ParamCharPlusInt(INFO,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),type1,param1);
            }
            else if ((strncmp(type0, "string", 6) == 0) && ((strncmp(type1, "string", 6) != 0) && (strncmp(type1, "char", 4) != 0) && (strncmp(type1, "double", 6) != 0)))
            {
                Print2ParamStringPlusInt(INFO,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),type1,param1);
            }
            else if (((strncmp(type0, "string", 6) != 0) && (strncmp(type0, "char", 4) != 0) && (strncmp(type0, "double", 6) != 0)) && ((strncmp(type1, "char", 4) == 0) || (strncmp(type1, "\0", 2) == 0)))
            {
                Print2ParamIntPlusChar(INFO,sourceFile,line,it->second,type0,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if (((strncmp(type0, "string", 6) != 0) && (strncmp(type0, "char", 4) != 0) && (strncmp(type0, "double", 6) != 0)) && (strncmp(type1, "string", 6) == 0))
            {
                Print2ParamIntPlusString(INFO,sourceFile,line,it->second,type0,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if (((strncmp(type0, "string", 6) != 0) && (strncmp(type0, "char", 4) != 0) && (strncmp(type0, "double", 6) != 0)) &&
                    ((strncmp(type1, "string", 6) != 0) && (strncmp(type1, "char", 4) != 0) && (strncmp(type1, "double", 6) != 0)))
            {
                Print2ParamIntPlusInt(INFO,sourceFile,line,it->second,type0,param0,(newFmt3 ? newFmt3 : newFmt0),type1,param1);
            }
            else if ((strncmp(type0, "double", 6) == 0) && (strncmp(type1, "double", 6) == 0))
            {
                Print2ParamDoublePlusDouble(INFO,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "double", 6) == 0) && ((strncmp(type1, "char", 4) == 0) || (strncmp(type1, "\0", 2) == 0)))
            {
                Print2ParamDoublePlusChar(INFO,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "double", 6) == 0) && (strncmp(type1, "string", 6) == 0))
            {
                Print2ParamDoublePlusString(INFO,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "string", 6) == 0) && (strncmp(type1, "double", 6) == 0))
            {
                Print2ParamStringPlusDouble(INFO,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "char", 4) == 0) && (strncmp(type1, "double", 6) == 0))
            {
                Print2ParamCharPlusDouble(INFO,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if (((strncmp(type0, "string", 6) != 0) && (strncmp(type0, "char", 4) != 0) && (strncmp(type0, "double", 6) != 0)) && (strncmp(type1, "double", 6) == 0))
            {
                Print2ParamIntPlusDouble(INFO,sourceFile,line,it->second,type0,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "double", 6) == 0) && ((strncmp(type1, "string", 6) != 0) && (strncmp(type1, "char", 4) != 0) && (strncmp(type1, "double", 6) != 0)))
            {
                Print2ParamDoublePlusInt(INFO,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),type1,param1);
            }

            break;
        }
        case LOG_WARNING:
        {

            if ((strncmp(type0, "string", 6) == 0) && (strncmp(type1, "string", 6) == 0))
            {
                Print2ParamStringPlusString(WARNING,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "string", 6) == 0) && ((strncmp(type1, "char", 4) == 0) || (strncmp(type1, "\0", 2) == 0)))
            {
                Print2ParamStringPlusChar(WARNING,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "char", 4) == 0) && ((strncmp(type1, "char", 4) == 0) || (strncmp(type1, "\0", 2) == 0)))
            {
                Print2ParamCharPlusChar(WARNING,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "char", 4) == 0) && (strncmp(type1, "string", 6) == 0))
            {
                Print2ParamCharPlusString(WARNING,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "char", 4) == 0) && ((strncmp(type1, "string", 6) != 0) && (strncmp(type1, "char", 4) != 0) && (strncmp(type1, "double", 6) != 0)))
            {
                Print2ParamCharPlusInt(WARNING,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),type1,param1);
            }
            else if ((strncmp(type0, "string", 6) == 0) && ((strncmp(type1, "string", 6) != 0) && (strncmp(type1, "char", 4) != 0) && (strncmp(type1, "double", 6) != 0)))
            {
                Print2ParamStringPlusInt(WARNING,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),type1,param1);
            }
            else if (((strncmp(type0, "string", 6) != 0) && (strncmp(type0, "char", 4) != 0) && (strncmp(type0, "double", 6) != 0)) && ((strncmp(type1, "char", 4) == 0) || (strncmp(type1, "\0", 2) == 0)))
            {
                Print2ParamIntPlusChar(WARNING,sourceFile,line,it->second,type0,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if (((strncmp(type0, "string", 6) != 0) && (strncmp(type0, "char", 4) != 0) && (strncmp(type0, "double", 6) != 0)) && (strncmp(type1, "string", 6) == 0))
            {
                Print2ParamIntPlusString(WARNING,sourceFile,line,it->second,type0,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if (((strncmp(type0, "string", 6) != 0) && (strncmp(type0, "char", 4) != 0) && (strncmp(type0, "double", 6) != 0)) &&
                    ((strncmp(type1, "string", 6) != 0) && (strncmp(type1, "char", 4) != 0) && (strncmp(type1, "double", 6) != 0)))
            {
                Print2ParamIntPlusInt(WARNING,sourceFile,line,it->second,type0,param0,(newFmt3 ? newFmt3 : newFmt0),type1,param1);
            }
            else if ((strncmp(type0, "double", 6) == 0) && (strncmp(type1, "double", 6) == 0))
            {
                Print2ParamDoublePlusDouble(WARNING,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "double", 6) == 0) && ((strncmp(type1, "char", 4) == 0) || (strncmp(type1, "\0", 2) == 0)))
            {
                Print2ParamDoublePlusChar(WARNING,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "double", 6) == 0) && (strncmp(type1, "string", 6) == 0))
            {
                Print2ParamDoublePlusString(WARNING,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "string", 6) == 0) && (strncmp(type1, "double", 6) == 0))
            {
                Print2ParamStringPlusDouble(WARNING,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "char", 4) == 0) && (strncmp(type1, "double", 6) == 0))
            {
                Print2ParamCharPlusDouble(WARNING,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if (((strncmp(type0, "string", 6) != 0) && (strncmp(type0, "char", 4) != 0) && (strncmp(type0, "double", 6) != 0)) && (strncmp(type1, "double", 6) == 0))
            {
                Print2ParamIntPlusDouble(WARNING,sourceFile,line,it->second,type0,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "double", 6) == 0) && ((strncmp(type1, "string", 6) != 0) && (strncmp(type1, "char", 4) != 0) && (strncmp(type1, "double", 6) != 0)))
            {
                Print2ParamDoublePlusInt(WARNING,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),type1,param1);
            }

            break;
        }
        case LOG_ERROR:
        {

            if ((strncmp(type0, "string", 6) == 0) && (strncmp(type1, "string", 6) == 0))
            {
                Print2ParamStringPlusString(ERROR,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "string", 6) == 0) && ((strncmp(type1, "char", 4) == 0) || (strncmp(type1, "\0", 2) == 0)))
            {
                Print2ParamStringPlusChar(ERROR,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "char", 4) == 0) && ((strncmp(type1, "char", 4) == 0) || (strncmp(type1, "\0", 2) == 0)))
            {
                Print2ParamCharPlusChar(ERROR,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "char", 4) == 0) && (strncmp(type1, "string", 6) == 0))
            {
                Print2ParamCharPlusString(ERROR,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "char", 4) == 0) && ((strncmp(type1, "string", 6) != 0) && (strncmp(type1, "char", 4) != 0) && (strncmp(type1, "double", 6) != 0)))
            {
                Print2ParamCharPlusInt(ERROR,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),type1,param1);
            }
            else if ((strncmp(type0, "string", 6) == 0) && ((strncmp(type1, "string", 6) != 0) && (strncmp(type1, "char", 4) != 0) && (strncmp(type1, "double", 6) != 0)))
            {
                Print2ParamStringPlusInt(ERROR,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),type1,param1);
            }
            else if (((strncmp(type0, "string", 6) != 0) && (strncmp(type0, "char", 4) != 0) && (strncmp(type0, "double", 6) != 0)) && ((strncmp(type1, "char", 4) == 0) || (strncmp(type1, "\0", 2) == 0)))
            {
                Print2ParamIntPlusChar(ERROR,sourceFile,line,it->second,type0,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if (((strncmp(type0, "string", 6) != 0) && (strncmp(type0, "char", 4) != 0) && (strncmp(type0, "double", 6) != 0)) && (strncmp(type1, "string", 6) == 0))
            {
                Print2ParamIntPlusString(ERROR,sourceFile,line,it->second,type0,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if (((strncmp(type0, "string", 6) != 0) && (strncmp(type0, "char", 4) != 0) && (strncmp(type0, "double", 6) != 0)) &&
                    ((strncmp(type1, "string", 6) != 0) && (strncmp(type1, "char", 4) != 0) && (strncmp(type1, "double", 6) != 0)))
            {
                Print2ParamIntPlusInt(ERROR,sourceFile,line,it->second,type0,param0,(newFmt3 ? newFmt3 : newFmt0),type1,param1);
            }
            else if ((strncmp(type0, "double", 6) == 0) && (strncmp(type1, "double", 6) == 0))
            {
                Print2ParamDoublePlusDouble(ERROR,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "double", 6) == 0) && ((strncmp(type1, "char", 4) == 0) || (strncmp(type1, "\0", 2) == 0)))
            {
                Print2ParamDoublePlusChar(ERROR,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "double", 6) == 0) && (strncmp(type1, "string", 6) == 0))
            {
                Print2ParamDoublePlusString(ERROR,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "string", 6) == 0) && (strncmp(type1, "double", 6) == 0))
            {
                Print2ParamStringPlusDouble(ERROR,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "char", 4) == 0) && (strncmp(type1, "double", 6) == 0))
            {
                Print2ParamCharPlusDouble(ERROR,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if (((strncmp(type0, "string", 6) != 0) && (strncmp(type0, "char", 4) != 0) && (strncmp(type0, "double", 6) != 0)) && (strncmp(type1, "double", 6) == 0))
            {
                Print2ParamIntPlusDouble(ERROR,sourceFile,line,it->second,type0,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "double", 6) == 0) && ((strncmp(type1, "string", 6) != 0) && (strncmp(type1, "char", 4) != 0) && (strncmp(type1, "double", 6) != 0)))
            {
                Print2ParamDoublePlusInt(ERROR,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),type1,param1);
            }

            break;
        }
        case LOG_FATAL:
        {
            if ((strncmp(type0, "string", 6) == 0) && (strncmp(type1, "string", 6) == 0))
            {
                Print2ParamStringPlusString(FATAL,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "string", 6) == 0) && ((strncmp(type1, "char", 4) == 0) || (strncmp(type1, "\0", 2) == 0)))
            {
                Print2ParamStringPlusChar(FATAL,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "char", 4) == 0) && ((strncmp(type1, "char", 4) == 0) || (strncmp(type1, "\0", 2) == 0)))
            {
                Print2ParamCharPlusChar(FATAL,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "char", 4) == 0) && (strncmp(type1, "string", 6) == 0))
            {
                Print2ParamCharPlusString(FATAL,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "char", 4) == 0) && ((strncmp(type1, "string", 6) != 0) && (strncmp(type1, "char", 4) != 0) && (strncmp(type1, "double", 6) != 0)))
            {
                Print2ParamCharPlusInt(FATAL,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),type1,param1);
            }
            else if ((strncmp(type0, "string", 6) == 0) && ((strncmp(type1, "string", 6) != 0) && (strncmp(type1, "char", 4) != 0) && (strncmp(type1, "double", 6) != 0)))
            {
                Print2ParamStringPlusInt(FATAL,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),type1,param1);
            }
            else if (((strncmp(type0, "string", 6) != 0) && (strncmp(type0, "char", 4) != 0) && (strncmp(type0, "double", 6) != 0)) && ((strncmp(type1, "char", 4) == 0) || (strncmp(type1, "\0", 2) == 0)))
            {
                Print2ParamIntPlusChar(FATAL,sourceFile,line,it->second,type0,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if (((strncmp(type0, "string", 6) != 0) && (strncmp(type0, "char", 4) != 0) && (strncmp(type0, "double", 6) != 0)) && (strncmp(type1, "string", 6) == 0))
            {
                Print2ParamIntPlusString(FATAL,sourceFile,line,it->second,type0,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if (((strncmp(type0, "string", 6) != 0) && (strncmp(type0, "char", 4) != 0) && (strncmp(type0, "double", 6) != 0)) &&
                    ((strncmp(type1, "string", 6) != 0) && (strncmp(type1, "char", 4) != 0) && (strncmp(type1, "double", 6) != 0)))
            {
                Print2ParamIntPlusInt(FATAL,sourceFile,line,it->second,type0,param0,(newFmt3 ? newFmt3 : newFmt0),type1,param1);
            }
            else if ((strncmp(type0, "double", 6) == 0) && (strncmp(type1, "double", 6) == 0))
            {
                Print2ParamDoublePlusDouble(FATAL,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "double", 6) == 0) && ((strncmp(type1, "char", 4) == 0) || (strncmp(type1, "\0", 2) == 0)))
            {
                Print2ParamDoublePlusChar(FATAL,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "double", 6) == 0) && (strncmp(type1, "string", 6) == 0))
            {
                Print2ParamDoublePlusString(FATAL,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "string", 6) == 0) && (strncmp(type1, "double", 6) == 0))
            {
                Print2ParamStringPlusDouble(FATAL,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "char", 4) == 0) && (strncmp(type1, "double", 6) == 0))
            {
                Print2ParamCharPlusDouble(FATAL,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if (((strncmp(type0, "string", 6) != 0) && (strncmp(type0, "char", 4) != 0) && (strncmp(type0, "double", 6) != 0)) && (strncmp(type1, "double", 6) == 0))
            {
                Print2ParamIntPlusDouble(FATAL,sourceFile,line,it->second,type0,param0,(newFmt3 ? newFmt3 : newFmt0),param1);
            }
            else if ((strncmp(type0, "double", 6) == 0) && ((strncmp(type1, "string", 6) != 0) && (strncmp(type1, "char", 4) != 0) && (strncmp(type1, "double", 6) != 0)))
            {
                Print2ParamDoublePlusInt(FATAL,sourceFile,line,it->second,param0,(newFmt3 ? newFmt3 : newFmt0),type1,param1);
            }

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
    ParamValue param0;
    const char *type0 = it->first;
    if (strncmp(type0, "char", 4) == 0)
    {
        param0.charParam = va_arg(params, int32_t);
    }
    else if (strncmp(type0, "string", 6) == 0)
    {
        param0.stringParam = va_arg(params, char*);
    }
    else if (strncmp(type0, "uint32_t", 8) == 0)
    {
        param0.uint32Param = va_arg(params, uint32_t);
    }
    else if (strncmp(type0, "int32_t", 7) == 0)
    {
        param0.int32Param = va_arg(params, int32_t);
    }
    else if (strncmp(type0, "int64_t", 7) == 0)
    {
        param0.int64Param = va_arg(params, int64_t);
    }
    else if (strncmp(type0, "double", 6) == 0)
    {
        param0.doubleParam = va_arg(params, double);
    }
    else if (strncmp(type0, "\0", 2) == 0)
    {
        param0.charParam = '!';
    }

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
            if ((strncmp(type0, "char", 4) == 0) || (strncmp(type0, "\0", 2) == 0))
                LOG(INFO) << "" << sourceFile << ":" << line << "  " << (newFmt3 ? newFmt3 : newFmt0) << " " << param0.charParam << std::endl;
            else if (strncmp(type0, "string", 6) == 0)
                LOG(INFO) << "" << sourceFile << ":" << line << "  " << (newFmt3 ? newFmt3 : newFmt0) << " " << param0.stringParam << std::endl;
            else if (strncmp(type0, "double", 6) == 0)
                LOG(INFO) << "" << sourceFile << ":" << line << "  " << (newFmt3 ? newFmt3 : newFmt0) << " " << param0.doubleParam << std::endl;
            else
                LOG(INFO) << "" << sourceFile << ":" << line << "  " << (newFmt3 ? newFmt3 : newFmt0) << " " << GetOneParamValue(type0,param0) << std::endl;

            break;
        }
        case LOG_WARNING:
        {
            if ((strncmp(type0, "char", 4) == 0) || (strncmp(type0, "\0", 2) == 0))
                LOG(WARNING) << "" << sourceFile << ":" << line << "  " << (newFmt3 ? newFmt3 : newFmt0) << " " << param0.charParam << std::endl;
            else if (strncmp(type0, "string", 6) == 0)
                LOG(WARNING) << "" << sourceFile << ":" << line << "  " << (newFmt3 ? newFmt3 : newFmt0) << " " << param0.stringParam << std::endl;
            else if (strncmp(type0, "double", 6) == 0)
                LOG(WARNING) << "" << sourceFile << ":" << line << "  " << (newFmt3 ? newFmt3 : newFmt0) << " " << param0.doubleParam << std::endl;
            else
                LOG(WARNING) << "" << sourceFile << ":" << line << "  " << (newFmt3 ? newFmt3 : newFmt0) << " " << GetOneParamValue(type0,param0) << std::endl;

            break;
        }
        case LOG_ERROR:
        {
            if ((strncmp(type0, "char", 4) == 0) || (strncmp(type0, "\0", 2) == 0))
                LOG(ERROR) << "" << sourceFile << ":" << line << "  " << (newFmt3 ? newFmt3 : newFmt0) << " " << param0.charParam << std::endl;
            else if (strncmp(type0, "string", 6) == 0)
                LOG(ERROR) << "" << sourceFile << ":" << line << "  " << (newFmt3 ? newFmt3 : newFmt0) << " " << param0.stringParam << std::endl;
            else if (strncmp(type0, "double", 6) == 0)
                LOG(ERROR) << "" << sourceFile << ":" << line << "  " << (newFmt3 ? newFmt3 : newFmt0) << " " << param0.doubleParam << std::endl;
            else
                LOG(ERROR) << "" << sourceFile << ":" << line << "  " << (newFmt3 ? newFmt3 : newFmt0) << " " << GetOneParamValue(type0,param0) << std::endl;

            break;
        }
        case LOG_FATAL:
        {
            if ((strncmp(type0, "char", 4) == 0) || (strncmp(type0, "\0", 2) == 0))
                LOG(FATAL) << "" << sourceFile << ":" << line << "  " << (newFmt3 ? newFmt3 : newFmt0) << " " << param0.charParam << std::endl;
            else if (strncmp(type0, "string", 6) == 0)
                LOG(FATAL) << "" << sourceFile << ":" << line << "  " << (newFmt3 ? newFmt3 : newFmt0) << " " << param0.stringParam << std::endl;
            else if (strncmp(type0, "double", 6) == 0)
                LOG(FATAL) << "" << sourceFile << ":" << line << "  " << (newFmt3 ? newFmt3 : newFmt0) << " " << param0.doubleParam << std::endl;
            else
                LOG(FATAL) << "" << sourceFile << ":" << line << "  " << (newFmt3 ? newFmt3 : newFmt0) << " " << GetOneParamValue(type0,param0) << std::endl;

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
            else if (type1 == 'f')
            {
                tempStrs.push_back(std::make_pair("double", str));
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
    if (strCnt > 2)
    {
        LOG(ERROR) << "Now more than two parameters are not supported in glog output !" << std::endl;
    }

    switch (strCnt)
    {
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
            LOG(ERROR) << "Invalid log message format " << fmt << "!" << std::endl;
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
