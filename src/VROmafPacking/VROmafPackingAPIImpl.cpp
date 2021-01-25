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
//! \file:  VROmafPackingAPIImpl.cpp
//! \brief: VR OMAF Packing library interfaces implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include "VROmafPackingAPI.h"
#include "OmafPackage.h"
#include "Log.h"
#ifdef _USE_TRACE_
#include "../trace/E2E_latency_tp.h"
#endif

VCD_USE_VRVIDEO;

Handler VROmafPackingInit(InitialInfo *initInfo)
{
    if (!initInfo)
        return NULL;

    OmafPackage *omafPackage = new OmafPackage();
    if (!omafPackage)
        return NULL;

    int32_t ret = omafPackage->InitOmafPackage(initInfo);
    if (ret)
    {
        DELETE_MEMORY(omafPackage);
        return NULL;
    }

    return (Handler)((long)omafPackage);
}

int32_t VROmafPackingSetLogCallBack(Handler hdl, void* externalLog)
{
    OmafPackage *omafPackage = (OmafPackage*)hdl;
    if (!omafPackage)
        return OMAF_ERROR_NULL_PTR;

    LogFunction logFunction = (LogFunction)externalLog;
    if (!logFunction)
        return OMAF_ERROR_NULL_PTR;

    int32_t ret = omafPackage->SetLogCallBack(logFunction);
    if (ret)
        return ret;

    return ERROR_NONE;
}

int32_t VROmafPackingWriteSegment(Handler hdl, uint8_t streamIdx, FrameBSInfo *frameInfo)
{
    OmafPackage *omafPackage = (OmafPackage*)hdl;
    if (!omafPackage)
        return OMAF_ERROR_NULL_PTR;

#ifdef _USE_TRACE_
    string tag = "StremIdx:" + to_string(streamIdx);
    tracepoint(E2E_latency_tp_provider,
               pre_op_info,
               frameInfo->pts,
               tag.c_str());
#endif

    int32_t ret = omafPackage->OmafPacketStream(streamIdx, frameInfo);
    if (ret)
        return ret;

    return ERROR_NONE;
}

int32_t VROmafPackingEndStreams(Handler hdl)
{
    OmafPackage *omafPackage = (OmafPackage*)hdl;
    if (!omafPackage)
        return OMAF_ERROR_NULL_PTR;

    int32_t ret = omafPackage->OmafEndStreams();
    if (ret)
        return ret;

    return ERROR_NONE;
}

int32_t VROmafPackingClose(Handler hdl)
{
    OmafPackage *omafPackage = (OmafPackage*)hdl;
    if (omafPackage)
    {
        delete omafPackage;
        omafPackage = NULL;
    }
    return ERROR_NONE;
}
