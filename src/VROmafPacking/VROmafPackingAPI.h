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
//! \file:  VROmafPackingAPI.h
//! \brief: VR OMAF Packing library interfaces
//!
//! Created on April 30, 2019, 6:04 AM
//!

#ifndef _VROMAFPACKINGAPI_H_
#define _VROMAFPACKINGAPI_H_

#include "VROmafPacking_data.h"
#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* Handler;

//!
//! \brief  Initialize VR OMAF Packing library resources and
//!         get its handle
//!
//! \param  [in] initInfo
//!         initial information input for library initialization,
//!         including media streams information, viewport
//!         information and so on
//!
//! \return Handler
//!         VR OMAF Packing library handle
//!
Handler VROmafPackingInit(InitialInfo *initInfo);

//!
//! \brief  VR OMAF Packing library set customized logging callback.
//!         The default logging callback is glog.
//!         If customized logging callback is needed, call this API
//!         after initialization API
//!
//! \param  [in] hdl
//!         VR OMAF Packing library handle
//! \param  [in] externalLog
//!         the customized logging callback function pointer
//!
//! \return int32_t
//!         ERROR_NONE if success, else failed reason
//!
int32_t VROmafPackingSetLogCallBack(Handler hdl, void* externalLog);

//!
//! \brief  VR OMAF Packing library writes segment for specified
//!         media stream, called when one new frame is needed to
//!         be written into the segment
//!
//! \param  [in] hdl
//!         VR OMAF Packing library handle
//! \param  [in] streamIdx
//!         the index of the specified media stream
//! \param  [in] frameInfo
//!         pointer to the frame bitstream information of new frame
//!         needed to be written into the segment for the
//!         specified media stream
//!
//! \return int32_t
//!         ERROR_NONE if success, else failed reason
//!
int32_t VROmafPackingWriteSegment(Handler hdl, uint8_t streamIdx, FrameBSInfo *frameInfo);

//!
//! \brief  VR OMAF Packing library ends the processing
//!         for all media streams, called when there is
//!         no new frames coming
//!
//! \param  [in] hdl
//!         VR OMAF Packing library handle
//!
//! \return int32_t
//!         ERROR_NONE if success, else failed reason
//!
int32_t VROmafPackingEndStreams(Handler hdl);

//!
//! \brief  Free VR OMAF Packing library resources
//!
//! \param  [in] hdl
//!         VR OMAF Packing library handle
//!
//! \return int32_t
//!         ERROR_NONE if success, else failed reason
//!
int32_t VROmafPackingClose(Handler hdl);

#ifdef __cplusplus
}
#endif

#endif /* _VROMAFPACKINGAPI_H_ */
