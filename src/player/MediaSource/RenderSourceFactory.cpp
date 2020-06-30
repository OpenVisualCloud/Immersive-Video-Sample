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

//!
//! \file     RenderSourceFactory.cpp
//! \brief    Implement class for RenderSourceFactory.
//!

#include "RenderSourceFactory.h"
#include "SWRenderSource.h"
#include <GLFW/glfw3.h>

VCD_NS_BEGIN


RenderSourceFactory::RenderSourceFactory(void *window)
{
    mMapRenderSource.clear();
    share_window = window;
    mWidth       = 0;
    mHeight      = 0;
    m_highTileCol= 0;
    m_highTileRow= 0;
}

RenderSourceFactory::~RenderSourceFactory()
{
    RemoveAll();
}

FrameHandler* RenderSourceFactory::CreateHandler(uint32_t video_id)
{
    glfwMakeContextCurrent((GLFWwindow*)share_window); // share context in multiple thread
    SWRenderSource* rs = new SWRenderSource();
    rs->SetVideoID(video_id);
    this->mMapRenderSource[video_id] = rs;

    return rs;
}

RenderStatus RenderSourceFactory::RemoveHandler(uint32_t video_id)
{
    if(mMapRenderSource.find(video_id)==mMapRenderSource.end()) return RENDER_NOT_FOUND;
    for(auto it=mMapRenderSource.begin(); it!=mMapRenderSource.end(); ++it){
        if(video_id == it->first){
            it->second->DestroyRenderSource();
            SAFE_DELETE(it->second);
            it=mMapRenderSource.erase(it);
            break;
        }
    }
    return RENDER_STATUS_OK;
}

RenderStatus RenderSourceFactory::RemoveAll()
{
    for(auto it=mMapRenderSource.begin(); it!=mMapRenderSource.end(); ++it){
        it->second->DestroyRenderSource();
        SAFE_DELETE(it->second);
        // it=mMapRenderSource.erase(it);
    }
    return RENDER_STATUS_OK;
}

VCD_NS_END