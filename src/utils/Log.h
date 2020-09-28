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
//! \file:   Log.h
//! \brief:  Include the log function declaration
//!
//! Created on April 30, 2019, 6:04 AM
//!

#ifndef _LOG_H_
#define _LOG_H_

#include "common_data.h"
#include "GlogWrapper.h"

#include <utility>
#include <stdarg.h>
#include <list>

union ParamValue
{
    char charParam;
    char *stringParam;
    uint32_t uint32Param;
    int32_t  int32Param;
    int64_t  int64Param;
    double   doubleParam;
};

//!
//! \brief  Output log information by glog for two variables
//!
//! \param  [in] logLevel
//!         the level of the logging
//! \param  [in] sourceFile
//!         the source file name where log information comes from
//! \param  [in] line
//!         the line number the output log information in source file
//! \param  [in] tempStrs
//!         the separated log informaion format
//! \param  [in] params
//!         the variable parameters list
//!
//! \return void
//!
void GlogSupport2Params(LogLevel logLevel, const char* sourceFile, uint64_t line, std::list<std::pair<const char*, char*>>& tempStrs, va_list params);

//!
//! \brief  Output log information by glog for one variable
//!
//! \param  [in] logLevel
//!         the level of the logging
//! \param  [in] sourceFile
//!         the source file name where log information comes from
//! \param  [in] line
//!         the line number the output log information in source file
//! \param  [in] tempStrs
//!         the separated log informaion format
//! \param  [in] params
//!         the variable parameters list
//!
//! \return void
//!
void GlogSupport1Param(LogLevel logLevel, const char* sourceFile, uint64_t line, std::list<std::pair<const char*, char*>>& tempStrs, va_list params);

//!
//! \brief  Output log information by glog
//!
//! \param  [in] logLevel
//!         the level of the logging
//! \param  [in] sourceFile
//!         the source file name where log information comes from
//! \param  [in] line
//!         the line number the output log information in source file
//! \param  [in] fmt
//!         the log informaion format
//!
//! \return void
//!
void GlogFunction(LogLevel logLevel, const char* sourceFile, uint64_t line, const char* fmt, ...);

#endif /* _LOG_H_ */
