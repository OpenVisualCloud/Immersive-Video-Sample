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

#include "MediaPlayer.h"
#include "Common.h"
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <chrono>

#include "Render/GLFWRenderContext.h"
#include "Render/EGLRenderContext.h"
#ifdef _USE_TRACE_
#include "../trace/MtHQ_tp.h"
#endif
#include "MediaSource/DashMediaSource.h"

// #include <time.h>

VCD_NS_BEGIN

MediaPlayer::MediaPlayer(struct RenderConfig config)
{
    m_renderConfig  = config;
    m_status        = STATUS_UNKNOWN;
    m_renderContext = NULL;
    m_mediaSource   = NULL;
    m_renderManager = NULL;
    m_rsFactory     = NULL;
}

MediaPlayer::~MediaPlayer()
{
    SAFE_DELETE(m_mediaSource);
    SAFE_DELETE(m_rsFactory);
    SAFE_DELETE(m_renderManager);
    SAFE_DELETE(m_renderContext);
}

RenderStatus MediaPlayer::Open()
{
    //initial renderContext
    m_renderContext = new GLFWRenderContext(m_renderConfig);
    if (m_renderContext == NULL)
    {
        LOG(ERROR) << " Init render context failed! " << endl;
        return RENDER_CREATE_ERROR;
    }
    //intital window
    void *window = m_renderContext->InitContext();
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

    if (m_renderConfig.url.empty())
    {
        LOG(ERROR)<<"Wrong url"<<std::endl;
        return RENDER_ERROR;
    }
    if (RENDER_STATUS_OK != m_renderManager->Initialize(m_mediaSource, m_rsFactory, m_renderContext))
    {
        return RENDER_ERROR;
    }
    m_status = STATUS_CREATED;

    return RENDER_STATUS_OK;
}

uint32_t MediaPlayer::GetStatus()
{
    return m_status;
}

RenderStatus MediaPlayer::Play()
{
    return RENDER_STATUS_OK;
}

RenderStatus MediaPlayer::Pause()
{
    return RENDER_STATUS_OK;
}

RenderStatus MediaPlayer::Resume()
{
    return RENDER_STATUS_OK;
}

RenderStatus MediaPlayer::Stop()
{
    return RENDER_STATUS_OK;
}

RenderStatus MediaPlayer::Seek()
{
    return RENDER_STATUS_OK;
}

RenderStatus MediaPlayer::Close()
{
    return RENDER_STATUS_OK;
}

RenderStatus UpdateUserInput()
{
    return RENDER_STATUS_OK;
}

RenderStatus MediaPlayer::PlayOneVideo(int64_t pts)
{
    if(!HasVideo()) return RENDER_STATUS_OK;
    float poseYaw, posePitch;
    m_renderManager->GetStatusAndPose(&poseYaw, &posePitch, (uint32_t*)&m_status);
    m_renderManager->SetViewport(poseYaw, posePitch);
    m_renderManager->ChangeViewport(poseYaw, posePitch, pts);

    m_renderManager->Render(0);

    return RENDER_STATUS_OK;
}

RenderStatus MediaPlayer::PlayOneAudio(int64_t pts)
{
    if(!HasAudio()) return RENDER_STATUS_OK;
    return RENDER_STATUS_OK;
}

bool MediaPlayer::HasAudio()
{
    if(m_mediaInfo.mAudioInfo.size() > 0) return true;
    return false;
}

bool MediaPlayer::HasVideo()
{
    if(m_mediaInfo.mVideoInfo.size() > 0) return true;
    return false;
}

void MediaPlayer::Run()
{
    while(1){
        switch(this->m_status){
            case STATUS_CREATED:
                usleep(1000);
                break;
            case STATUS_PAUSED:
                usleep(1000);
                break;
            case STATUS_PLAYING:
                PlayOneAudio(0);
                PlayOneVideo(0);
                break;
            case STATUS_STOPPED:
                break;
            case STATUS_CLOSED:
                return;
            default:
                break;
        }
    }

    /*
    float poseYaw, posePitch;
    std::chrono::high_resolution_clock clock;
    uint64_t lastTime = 0;
    uint64_t renderCount = 0; // record render times
    uint64_t start = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
    bool quitFlag = false;
    do
    {
        m_renderManager->GetStatusAndPose(&poseYaw, &posePitch, &m_status);
        m_renderManager->SetViewport(poseYaw, posePitch);
        m_renderManager->ChangeViewport(poseYaw, posePitch);
        if (0 == lastTime)
        {
            lastTime = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
        }
        if (PLAY == GetStatus())
        {
            uint64_t renderTime = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
            uint64_t interval = renderTime - lastTime;
            uint32_t renderInterval = m_renderManager->GetRenderConfig().renderInterval;
            if(interval < renderInterval)
            {
                usleep((renderInterval - interval) * 1000);
                LOG(INFO)<<"==========wait_time============== :"<<(renderInterval - interval)<<std::endl;
            }
            else
            {
                LOG(INFO)<<"=======interval>INTERVAL========"<<interval<<std::endl;
            }

            lastTime = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
            PlayOneVideo(0);
            LOG(INFO)<<"===========renderTime==============:"<<lastTime<<std::endl;
            //trace
            tracepoint(mthq_tp_provider, T9_render, renderCount + 1);
            renderCount++;
        }
        else if (READY == GetStatus() || PAUSE == GetStatus())
        {
            PlayOneVideo(0);
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
    return;
    */
}

VCD_NS_END
