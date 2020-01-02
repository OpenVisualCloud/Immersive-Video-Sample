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

#include "GLFWRenderContext.h"
#include "EGLRenderContext.h"
// #include <time.h>

glm::mat4 ProjectionMatrix;
glm::mat4 ViewModelMatrix;

VCD_NS_BEGIN

Player::Player(struct RenderConfig config)
{
    m_renderManager = new RenderManager(config);
    m_status = READY;
}

Player::~Player()
{
    if (m_renderManager != NULL)
    {
        delete m_renderManager;
        m_renderManager = NULL;
    }
}

RenderStatus Player::Open()
{
    if (NULL == m_renderManager->GetRenderConfig().url)
    {
        LOG(ERROR)<<"Wrong url"<<std::endl;
        return RENDER_ERROR;
    }
    if (RENDER_STATUS_OK != m_renderManager->Initialize())
    {
        return RENDER_ERROR;
    }
    m_status = READY;
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
        if (0 == lastTime)
        {
            lastTime = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
        }
        if (PLAY == GetStatus())
        {
            m_renderManager->PrepareRender();

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
            LOG(INFO)<<"===========renderTime==============:"<<lastTime<<std::endl;
            m_renderManager->Render();
            renderCount++;
        }
        else if (READY == GetStatus() || PAUSE == GetStatus())
        {
            m_renderManager->Render();
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
