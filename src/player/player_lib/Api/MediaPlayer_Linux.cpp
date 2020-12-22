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
//! \file     Player.cpp
//! \brief    Implement class for Player.
//!

#ifdef _LINUX_OS_
#include "MediaPlayer_Linux.h"
#include "../Common/Common.h"
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include "../../app/linux/GLFWRenderContext.h"
#ifdef _USE_TRACE_
#include "../../../trace/MtHQ_tp.h"
#endif
#include "../MediaSource/DashMediaSource.h"

// #include <time.h>

VCD_NS_BEGIN


MediaPlayer_Linux::MediaPlayer_Linux()
{
    m_status        = STATUS_UNKNOWN;
    m_renderContext = NULL;
    m_mediaSource   = NULL;
    m_renderManager = NULL;
    m_rsFactory     = NULL;
}

MediaPlayer_Linux::~MediaPlayer_Linux()
{
    SAFE_DELETE(m_mediaSource);
    SAFE_DELETE(m_rsFactory);
    SAFE_DELETE(m_renderManager);
    // SAFE_DELETE(m_renderContext);
}

RenderStatus MediaPlayer_Linux::Create(struct RenderConfig config)
{
    m_renderConfig = config;
    return RENDER_STATUS_OK;
}

RenderStatus MediaPlayer_Linux::Start(void *render_context)
{
    //initial renderContext
    m_renderContext = static_cast<GLFWRenderContext*>(render_context);
    if (m_renderContext == NULL)
    {
        LOG(ERROR) << " Init render context failed! " << endl;
        return RENDER_CREATE_ERROR;
    }
    //intital window
    void *window = m_renderContext->GetWindow();
    if( NULL == window ){
        LOG(ERROR) << "failed to initial render context!" << std::endl;
        return RENDER_CREATE_ERROR;
    }
    m_renderManager = new RenderManager(m_renderConfig);

    //initial RenderSource
    m_rsFactory = new RenderSourceFactory(window);

    //initial MediaSource
    m_mediaSource = new DashMediaSource();

    //load media source and get type
    RenderStatus loadMediaStatus = m_mediaSource->Initialize(m_renderConfig, m_rsFactory);
    if (loadMediaStatus != RENDER_STATUS_OK)
    {
        return RENDER_ERROR;
    }

    this->m_mediaInfo = m_mediaSource->GetMediaInfo();
    m_mediaSource->SetActiveStream(0, 0);
    RenderStatus startStatus = m_mediaSource->Start();
    if (startStatus != RENDER_STATUS_OK)
    {
        return RENDER_ERROR;
    }
    if (!m_renderConfig.url) {
        LOG(ERROR) << "Wrong url" << std::endl;
        return RENDER_ERROR;
    }
    if (RENDER_STATUS_OK != m_renderManager->Initialize(m_mediaSource, m_rsFactory, m_renderContext)) {
        return RENDER_ERROR;
    }
    m_status = PLAY;

    return RENDER_STATUS_OK;
}

RenderStatus MediaPlayer_Linux::Play()
{
    float poseYaw, posePitch;
    std::chrono::high_resolution_clock clock;
    uint64_t lastTime = 0;
    uint64_t prevLastTime = 0;
    uint64_t deltaTime = 0;
    uint64_t renderCount = 0; // record render times
    uint64_t start = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
    bool quitFlag = false;
    uint64_t needDropFrames = 0;
    uint64_t accumTimeDelay = 0;
    do
    {
        m_renderManager->GetStatusAndPose(&poseYaw, &posePitch, &m_status);
        m_renderManager->SetViewport(poseYaw, posePitch);
        m_renderManager->ChangeViewport(poseYaw, posePitch, renderCount);
        if (0 == lastTime)
        {
            lastTime = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
        }
        if (PLAY == GetStatus())
        {
            uint64_t renderTime = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
            uint64_t interval = renderTime - lastTime + deltaTime;
            uint32_t renderInterval = m_renderManager->GetRenderConfig().renderInterval;
            if(interval < renderInterval)
            {
                usleep((renderInterval - interval) * 1000);
                needDropFrames = 0;
                LOG(INFO)<<"==========wait_time============== :"<<(renderInterval - interval)<<std::endl;
            }
            else
            {
                accumTimeDelay += interval - renderInterval;
                if (accumTimeDelay > renderInterval)
                {
                    needDropFrames = round(float(accumTimeDelay) / renderInterval);
                    accumTimeDelay = 0;
                }
                LOG(INFO)<<"=======interval>INTERVAL========"<<interval - renderInterval<<" and needDropFrames is:" << needDropFrames<< std::endl;
            }
            prevLastTime = lastTime;
            lastTime = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
            deltaTime = lastTime - prevLastTime < renderInterval ? 0 : lastTime - prevLastTime - renderInterval;
            RenderStatus renderStatus = m_renderManager->Render(renderCount);
            LOG(INFO)<<"render count is"<<renderCount<<endl;
#ifdef _USE_TRACE_
            //trace
            tracepoint(mthq_tp_provider, T13_render_time, renderCount);
#endif
            if (renderStatus != RENDER_NO_FRAME){
                renderCount += needDropFrames + 1;
                needDropFrames = 0;
                if (renderStatus != RENDER_ERROR)
                {
                    m_renderContext->SwapBuffers(NULL, 0);
                    LOG(INFO)<<"===========renderTime==============:"<<lastTime<<std::endl;
                }
            }
        }
        LOG(INFO)<<"status:"<<GetStatus()<<std::endl;
        if (m_renderManager->IsEOS())
        {
            cout<<"Soon to quit player!"<<endl;
            quitFlag = true;
        }
    } while (!quitFlag);
    uint64_t end = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
    LOG(INFO)<<"-----------------------------"<<std::endl;
    LOG(INFO)<<"----[render duration]:------ "<<float(end - start)/1000<<"s"<<std::endl;
    LOG(INFO)<<"----[render frame count]:--- "<<renderCount<<std::endl;
    LOG(INFO)<<"----[actual render fps]:---- "<<renderCount / (float(end - start)/1000)<<std::endl;
    LOG(INFO)<<"-----------------------------"<<std::endl;
    return RENDER_STATUS_OK;
}

RenderStatus MediaPlayer_Linux::Pause()
{
    return RENDER_STATUS_OK;
}

RenderStatus MediaPlayer_Linux::Resume()
{
    return RENDER_STATUS_OK;
}

RenderStatus MediaPlayer_Linux::Stop()
{
    return RENDER_STATUS_OK;
}

RenderStatus MediaPlayer_Linux::Seek()
{
    return RENDER_STATUS_OK;
}

RenderStatus MediaPlayer_Linux::Close()
{
    return RENDER_STATUS_OK;
}

VCD_NS_END
#endif