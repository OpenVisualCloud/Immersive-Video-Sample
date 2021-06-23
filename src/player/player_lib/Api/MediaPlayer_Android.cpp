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
//! \file     MediaPlayer_Android.cpp
//! \brief    Implement class for MediaPlayer_Android.
//!

#ifdef _ANDROID_OS_
#include "MediaPlayer_Android.h"
#include "../Common/Common.h"
#include "../MediaSource/DashMediaSource.h"
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <iostream>
#include <chrono>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#ifdef _USE_TRACE_
#include "../../../trace/MtHQ_tp.h"
#endif

VCD_NS_BEGIN

MediaPlayer_Android::MediaPlayer_Android()
{
    m_status        = STATUS_UNKNOWN;
    m_renderManager = NULL;
    //initial RenderSource need gl context
    m_rsFactory = new RenderSourceFactory(nullptr);
    //initial MediaSource
    m_mediaSource = new DashMediaSource();
}

MediaPlayer_Android::~MediaPlayer_Android()
{
    SAFE_DELETE(m_mediaSource);
    SAFE_DELETE(m_rsFactory);
    SAFE_DELETE(m_renderManager);
    Close();
}

RenderStatus MediaPlayer_Android::Create(struct RenderConfig config)
{
    m_renderConfig = config;
    m_renderManager = new RenderManager(m_renderConfig);
    //load media source and get type
    RenderStatus loadMediaStatus = m_mediaSource->Initialize(m_renderConfig, m_rsFactory);
    if (loadMediaStatus != RENDER_STATUS_OK)
    {
        ANDROID_LOGD("failed to load media status");
        return RENDER_ERROR;
    }
    this->m_mediaInfo = m_mediaSource->GetMediaInfo();
    m_mediaSource->SetActiveStream(0, 0);
    return RENDER_STATUS_OK;
}

RenderStatus MediaPlayer_Android::Start()
{
    RenderStatus startStatus = m_mediaSource->Start();
    if (startStatus != RENDER_STATUS_OK)
    {
        return RENDER_ERROR;
    }
    if (!m_renderConfig.url) {
        LOG(ERROR) << "Wrong url" << std::endl;
        return RENDER_ERROR;
    }
    if (RENDER_STATUS_OK != m_renderManager->Initialize(m_mediaSource, m_rsFactory, nullptr)) {
        return RENDER_ERROR;
    }
    m_status = PLAY;

    return RENDER_STATUS_OK;
}

RenderStatus MediaPlayer_Android::Play()
{
    m_status = PLAY;
    return RENDER_STATUS_OK;
}

RenderStatus MediaPlayer_Android::Pause()
{
    m_status = PAUSE;
    return RENDER_STATUS_OK;
}

RenderStatus MediaPlayer_Android::Resume()
{
    m_status = PLAY;
    return RENDER_STATUS_OK;
}

RenderStatus MediaPlayer_Android::Stop()
{
    m_status = STOPPED;
    return RENDER_STATUS_OK;
}

RenderStatus MediaPlayer_Android::Seek()
{
    return RENDER_STATUS_OK;
}

RenderStatus MediaPlayer_Android::Close()
{
    m_status = STOPPED;
    return RENDER_STATUS_OK;
}

int MediaPlayer_Android::UpdateDisplayTex(int render_count)
{
    m_renderManager->UpdateFrames(render_count);
    int ret = m_renderManager->UpdateDisplayTex();
    return ret;
}

VCD_NS_END
#endif