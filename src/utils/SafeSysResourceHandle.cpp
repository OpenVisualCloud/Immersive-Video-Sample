/*
 * Copyright (c) 2022, Intel Corporation
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
//! \file:   SafeSysResourceHandle.cpp
//! \brief:  Include the safe system resource handling functions implementation
//!
//! Created on Jun 14, 2022, 13:58 AM
//!

#include "SafeSysResourceHandle.h"
#include "error.h"

int32_t SafeFileOpen(char *fileName, const char *openMode, FILE **pFileHandler)
{
    char buf[PATH_MAX] = { 0 };
    char *res = realpath(fileName, buf);
    FILE *fp = NULL;
    int32_t ret = 0;
    if (res)
    {
        fp = fopen(buf, openMode);
        if (!fp)
            return OMAF_FILE_OPEN_ERROR ;

        *pFileHandler = fp;
        fp = NULL;
        ret = ERROR_NONE;
    }
    else
    {
        perror("realpath");
        ret = OMAF_ERROR_REALPATH_FAILED;
    }

    return ret;
}
