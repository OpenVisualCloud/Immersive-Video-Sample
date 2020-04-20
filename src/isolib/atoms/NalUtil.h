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
//! \file:   NalUtil.h
//! \brief:  NalUtil class
//! \detail: defines the number of bytes in start code
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _NALUTIL_H_
#define _NALUTIL_H_

#include "FormAllocator.h"

VCD_MP4_BEGIN

//!
//! \brief    Find Start Code Len
//!
//! \param    [in] const std::vector<uint8_t>
//!           source data
//!
//! \return   unsigned int
//!           length
//!
unsigned int FindStartCodeLen(const std::vector<uint8_t> &data);

//!
//! \brief    Transfer Stream To RBSP
//!
//! \param    [in] const std::vector<uint8_t> &
//!           byte Stream
//!
//! \return   std::vector<uint8_t>
//!           byte stream output
//!
std::vector<uint8_t> TransferStreamToRBSP(const std::vector<uint8_t> &byteStr);

VCD_MP4_END;
#endif  /* _NALUTIL_H_ */
