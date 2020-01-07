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
//! \file     RenderManager.cpp
//! \brief    Implement class for RenderManager.
//!

#include "RenderManager.h"
#include "ERPRender.h"
#include "CubeMapRender.h"
#include "FFmpegMediaSource.h"
#include "SWRenderSource.h"
#include "HWRenderSource.h"
#include "DMABufferRenderSource.h"
#include "GLFWRenderContext.h"
#include "EGLRenderContext.h"
#ifdef _ENABLE_WEBRTC_SOURCE_
#include "WebRTCMediaSource.h"
#else
#include "DashMediaSource.h"
#endif
VCD_NS_BEGIN

RenderManager::RenderManager(struct RenderConfig config)
{
    pthread_mutex_init(&m_poseMutex, NULL);
    m_status = STATUS_UNKNOWN;
    //1.initial ViewPortManager
    m_viewPortManager = new ViewPortManager();
    //2.initial RenderBackend
    m_renderBackend = new RenderBackend();
    //3.initial renderConfig
    m_renderConfig = config;
    //4.initial renderContext
    switch(m_renderConfig.contextType)
    {
    case GLFW_CONTEXT:
        m_renderContext = new GLFWRenderContext(m_renderConfig.windowWidth, m_renderConfig.windowHeight);
        break;
    case EGL_CONTEXT:
        m_renderContext = new EGLRenderContext(m_renderConfig.windowWidth, m_renderConfig.windowHeight);
        break;
    default:
        m_renderContext = new EGLRenderContext();
        break;
    }
    //5.intital window
    m_renderContext->InitContext();
    //6.initial MediaSource
    switch (m_renderConfig.sourceType)
    {
#ifndef _ENABLE_WEBRTC_SOURCE_
    case DASH_SOURCE:
        m_mediaSource = new DashMediaSource();
        break;
#endif
#ifdef USE_DMA_BUFFER
    case FFMPEG_SOURCE:
        m_mediaSource = new FFmpegMediaSource();
        break;
#endif
#ifdef _ENABLE_WEBRTC_SOURCE_
    case WEBRTC_SOURCE:
        m_mediaSource = new WebRTCMediaSource();
        break;
#endif   
 default:
        m_mediaSource = NULL;
        LOG(ERROR)<<"initial media source error!"<<std::endl;
        break;
    }
    //7.initial RenderSource
    switch (m_renderConfig.decoderType)
    {
    case SW_DECODER: //sw decoder
        m_renderSource = new SWRenderSource(m_renderBackend);
        break;
#ifdef USE_DMA_BUFFER
    case VAAPI_DECODER: //hw decoder
        if(m_renderConfig.useDMABuffer == 1)
            m_renderSource = new DMABufferRenderSource(m_renderBackend);
        else
            m_renderSource = new HWRenderSource(m_renderBackend);
        break;
#endif
    default:
        m_renderSource = new SWRenderSource(m_renderBackend);
        break;
    }
    //8.initial RenderTarget
    m_renderTarget = new RenderTarget();
    //9.initial SurfaceRender
    m_surfaceRender = NULL;
}

RenderManager::~RenderManager()
{
    int32_t res = pthread_mutex_destroy(&m_poseMutex);
    if (res != 0) {return;}
    m_status = STATUS_STOPPED;
    this->Join();
    if (m_mediaSource != NULL)
    {
        delete m_mediaSource;
        m_mediaSource = NULL;
    }
    if (m_renderBackend != NULL)
    {
        delete m_renderBackend;
        m_renderBackend = NULL;
    }
    if (m_renderSource != NULL)
    {
        delete m_renderSource;
        m_renderSource = NULL;
    }
    if (m_renderTarget != NULL)
    {
        delete m_renderTarget;
        m_renderTarget = NULL;
    }
    if (m_surfaceRender != NULL)
    {
        delete m_surfaceRender;
        m_surfaceRender = NULL;
    }
    if (m_viewPortManager != NULL)
    {
        delete m_viewPortManager;
        m_viewPortManager = NULL;
    }
    if (m_renderContext != NULL)
    {
        delete m_renderContext;
        m_renderContext = NULL;
    }
}

RenderStatus RenderManager::PrepareRender()
{
    //1.get frame from m_mediaSource
    uint8_t *buffer[4];
    struct RegionInfo regionInfo;
    if (RENDER_STATUS_OK != m_mediaSource->GetFrame(&buffer[0], &regionInfo))
    {
        return RENDER_ERROR;
    }
    //2.update texture and render to texture in m_renderSource
    if (RENDER_STATUS_OK != m_renderSource->UpdateR2T(m_renderBackend, (void **)buffer))
    {
        return RENDER_ERROR;
    }
    //3.tile copy and render to FBO from m_renderTarget
    RenderStatus renderTargetStatus = m_renderTarget->Update(m_renderBackend, &regionInfo);
    if (RENDER_ERROR == renderTargetStatus)
    {
        return RENDER_ERROR;
    }
    //4. delete memory.
    m_mediaSource->DeleteBuffer(buffer);
    if (regionInfo.sourceInfo != NULL)
    {
        delete regionInfo.sourceInfo;
        regionInfo.sourceInfo = NULL;
    }
    m_mediaSource->ClearRWPK(regionInfo.regionWisePacking);
    return RENDER_STATUS_OK;
}

RenderStatus RenderManager::Render()
{
    uint32_t width = m_renderConfig.windowWidth;
    uint32_t height = m_renderConfig.windowHeight;
    if (RENDER_STATUS_OK != m_surfaceRender->Render(m_renderBackend, m_renderTarget, width, height, m_renderContext->GetProjectionMatrix(), m_renderContext->GetViewModelMatrix()))
    {
        return RENDER_ERROR;
    }
    m_renderContext->SwapBuffers(NULL, 0);
    return RENDER_STATUS_OK;
}

RenderStatus RenderManager::Initialize() //should input the decoderManager
{
    if (NULL == m_renderConfig.url)
    {
        return RENDER_ERROR;
    }
    //1.load media source and get type
    RenderStatus loadMediaStatus = m_mediaSource->Initialize(m_renderConfig);
    if (loadMediaStatus != RENDER_STATUS_OK)
    {
        return RENDER_ERROR;
    }
    //2. change viewport thread start
    StartThread();
    m_status= STATUS_CREATED;
    while (!m_mediaSource->getIsAllValid())
    {
        usleep(50*1000);
    }
    struct MediaSourceInfo mediaSourceInfo;
    mediaSourceInfo = m_mediaSource->GetMediaSourceInfo();
    m_renderConfig.projFormat = mediaSourceInfo.projFormat;
    m_renderConfig.renderInterval = 1000 / mediaSourceInfo.frameRate;
    //2.initial SurfaceRender and shaders
    if (CreateRender(m_renderConfig.projFormat) != RENDER_STATUS_OK)
    {
        return RENDER_ERROR;
    }
    //3.set uniform frameTex
    m_renderSource->m_videoShaderOfR2T.Bind();
    m_renderSource->m_videoShaderOfR2T.SetUniform1i("frameTex", 0);
    m_renderSource->m_videoShaderOfR2T.SetUniform1i("frameU", 1);
    m_renderSource->m_videoShaderOfR2T.SetUniform1i("frameV", 2);
    if (m_renderConfig.decoderType == 1) //hardware decoding with libva
    {
        m_renderSource->m_videoShaderOfR2T.SetUniform1i("isNV12", 1);
    }
    else //sw decoding
    {
        m_renderSource->m_videoShaderOfR2T.SetUniform1i("isNV12", 0);
    }
    m_surfaceRender->m_videoShaderOfOnScreen.Bind();
    m_surfaceRender->m_videoShaderOfOnScreen.SetUniform1i("frameTex_screen", 0);
    //4.initial renderSource
    if (m_renderSource->Initialize(&mediaSourceInfo) != RENDER_STATUS_OK)
    {
        return RENDER_ERROR;
    }
    if (m_renderSource->CreateRenderSource(m_renderBackend) != RENDER_STATUS_OK)
    {
        return RENDER_ERROR;
    }
    //5.initial renderTarget
    if (m_renderTarget->Initialize(&mediaSourceInfo, m_renderSource->GetFboR2THandle()) != RENDER_STATUS_OK)
    {
        return RENDER_ERROR;
    }
    if (m_renderTarget->CreateRenderTarget(m_renderBackend) != RENDER_STATUS_OK)
    {
        return RENDER_ERROR;
    }
    return RENDER_STATUS_OK;
}

RenderStatus RenderManager::CreateRender(int32_t projFormat)
{
    switch (projFormat)
    {
#ifndef LOW_LATENCY_USAGE
    case VCD::OMAF::PF_ERP:
#else
    case PT_ERP:
#endif
    {
        m_surfaceRender = new ERPRender(m_renderBackend);
        if (NULL == m_surfaceRender)
        {
            LOG(ERROR)<< "ERPRender creation failed" << std::endl;
            return RENDER_ERROR;
        }
        break;
    }
#ifndef LOW_LATENCY_USAGE
    case VCD::OMAF::PF_CUBEMAP:
#else
    case PT_CUBEMAP:
#endif
    {
        m_surfaceRender = new CubeMapRender(m_renderBackend);
        if (NULL == m_surfaceRender)
        {
            LOG(ERROR)<< "CubeMapRender creation failed" << std::endl;
            return RENDER_ERROR;
        }
        break;
    }

    default:
        return RENDER_ERROR;
    }

    return RENDER_STATUS_OK;
}

bool RenderManager::IsEOS()
{
    return m_mediaSource->IsEOS() || !(m_renderContext->isRunning());
}

RenderStatus RenderManager::ChangeViewport(float yaw, float pitch)
{
    m_mediaSource->ChangeViewport(yaw, pitch);
    return RENDER_STATUS_OK;
}

void RenderManager::Run()
{
    m_status = STATUS_RUNNING;
    while (m_status != STATUS_STOPPED)
    {
        int32_t res = pthread_mutex_lock(&m_poseMutex);
        if (res != 0) {return;}
        struct Pose pose = m_viewPortManager->GetViewPort();
        res = pthread_mutex_unlock(&m_poseMutex);
        if (res != 0) {return;}
        ChangeViewport(pose.yaw, pose.pitch);
        usleep(5*1000);
    }
}

RenderStatus RenderManager::SetViewport(float yaw, float pitch)
{
    struct Pose pose;
    pose.yaw = yaw;
    pose.pitch = pitch;
    int32_t res = pthread_mutex_lock(&m_poseMutex);
    if (res != 0) {return RENDER_ERROR;}
    m_viewPortManager->SetViewPort(pose);
    res = pthread_mutex_unlock(&m_poseMutex);
    if (res != 0) {return RENDER_ERROR;}
    return RENDER_STATUS_OK;
}

struct RenderConfig RenderManager::GetRenderConfig()
{
    return m_renderConfig;
}

void RenderManager::GetStatusAndPose(float *yaw, float *pitch, uint32_t *status)
{
    m_renderContext->GetStatusAndPose(yaw, pitch, status);
}

VCD_NS_END
