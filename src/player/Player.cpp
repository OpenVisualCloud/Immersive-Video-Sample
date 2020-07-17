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

#include "Player.h"
#include "Common.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
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

Player::Player(struct RenderConfig config)
{
    m_renderConfig  = config;
    m_status        = STATUS_UNKNOWN;
    m_renderContext = NULL;
    m_mediaSource   = NULL;
    m_renderManager = NULL;
    m_rsFactory     = NULL;
}

Player::~Player()
{
    SAFE_DELETE(m_mediaSource);
    SAFE_DELETE(m_rsFactory);
    SAFE_DELETE(m_renderManager);
    // SAFE_DELETE(m_renderContext); // was deleted in renderManager.
}

RenderStatus Player::Open()
{
    //initial renderContext
    switch(m_renderConfig.contextType)
    {
    case GLFW_CONTEXT:
        m_renderContext = new GLFWRenderContext(m_renderConfig);
        break;
    case EGL_CONTEXT:
        m_renderContext = new EGLRenderContext(m_renderConfig);
        break;
    default:
        m_renderContext = new GLFWRenderContext(m_renderConfig);
        break;
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

    if (NULL == m_renderConfig.url)
    {
        LOG(ERROR)<<"Wrong url"<<std::endl;
        return RENDER_ERROR;
    }
    if (RENDER_STATUS_OK != m_renderManager->Initialize(m_mediaSource, m_rsFactory, m_renderContext))
    {
        return RENDER_ERROR;
    }
    m_status = PLAY;

    return RENDER_STATUS_OK;
}

RenderStatus Player::Play()
{
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
            RenderStatus renderStatus = m_renderManager->Render(renderCount);
            if (renderStatus != RENDER_NO_FRAME){
                renderCount++;
                if (renderStatus != RENDER_ERROR)
                {
                    m_renderContext->SwapBuffers(NULL, 0);
                    LOG(INFO)<<"===========renderTime==============:"<<lastTime<<std::endl;
                }
            }
            LOG(INFO)<<"render count is"<<renderCount<<endl;
#ifdef _USE_TRACE_
            //trace
            tracepoint(mthq_tp_provider, T9_render, renderCount + 1);
#endif
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

uint32_t Player::GetStatus()
{
    return m_status;
}

VCD_NS_END
