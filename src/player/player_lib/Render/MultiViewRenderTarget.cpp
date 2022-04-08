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
//! \file     ERPRenderTarget.cpp
//! \brief    Implement class for ERP RenderTarget.
//!
#ifdef _LINUX_OS_
#include "MultiViewRenderTarget.h"
#include "RenderContext.h"

#include <GL/glu.h>
#include <GL/glu_mangle.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <GLES3/gl3platform.h>
#include <algorithm>
#include <iostream>
#include <chrono>
#ifdef _USE_TRACE_
#include "../../../trace/MtHQ_tp.h"
#endif
#include "../Common/DataLog.h"

VCD_NS_BEGIN

MultiViewRenderTarget::MultiViewRenderTarget()
{
    m_activeTextureId = 0;
}

MultiViewRenderTarget::~MultiViewRenderTarget()
{
    RenderBackend *renderBackend = RENDERBACKEND::GetInstance();
    renderBackend->DeleteFramebuffers(1, &m_fboOnScreenHandle);
    renderBackend->DeleteTextures(1, m_textureOfR2S);
    std::cout<<"AVG CHANGED TIME COST : "<<m_avgChangedTime<<"ms"<<std::endl;
}

RenderStatus MultiViewRenderTarget::Initialize(RenderSourceFactory* rsFactory)
{
    if (NULL == rsFactory)
    {
        return RENDER_ERROR;
    }
    this->m_rsFactory = rsFactory;

    return RENDER_STATUS_OK;
}

RenderStatus MultiViewRenderTarget::CreateRenderTarget()
{
    if(NULL==this->m_rsFactory){
        return RENDER_NULL_HANDLE;
    }

    uint32_t source_number = m_rsFactory->GetSourceNumber();
    SourceResolution *source_resolution = m_rsFactory->GetSourceResolution();

    RenderBackend *renderBackend = RENDERBACKEND::GetInstance();

    renderBackend->GenTextures(source_number, m_textureOfR2S);
    for (uint32_t i = 0; i < source_number; i++)
    {
        uint32_t width = source_resolution[i].width;
        uint32_t height = source_resolution[i].height;
        // bind texture according to quality ranking (1,2,3...)
        renderBackend->BindTexture(GL_TEXTURE_2D, m_textureOfR2S[source_resolution[i].qualityRanking - 1]);
        renderBackend->PixelStorei(GL_UNPACK_ROW_LENGTH, width);
        renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        renderBackend->TexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    }

    renderBackend->GenFramebuffers(1, &m_fboOnScreenHandle);
    renderBackend->BindFramebuffer(GL_FRAMEBUFFER, m_fboOnScreenHandle);

    return RENDER_STATUS_OK;
}

RenderStatus MultiViewRenderTarget::Update( HeadPose* pose, float hFOV, float vFOV, uint64_t pts )
{
    if(NULL==this->m_rsFactory){
        return  RENDER_NULL_HANDLE;
    }

    RenderBackend *renderBackend = RENDERBACKEND::GetInstance();
    RenderStatus ret = RENDER_STATUS_OK;

    // 1. find the target render source according to input pose
    std::map<uint32_t, RenderSource*> mapRenderSources = m_rsFactory->GetRenderSources();

    RenderSource* targetRS = nullptr;

    for (auto rs = mapRenderSources.begin(); rs != mapRenderSources.end(); rs++) {
        RenderSource* cur_rs = rs->second;
        if (cur_rs->GetViewID() == make_pair(pose->hViewId, pose->vViewId)) {
            targetRS = cur_rs;
            break;
        }
    }
    // 2. screen texture assignment
    if (targetRS == nullptr) {
        LOG(ERROR) << "Target render source is not available" << endl;
        return RENDER_ERROR;
    }
    LOG(INFO) << "Target render view id " << targetRS->GetViewID().first << endl;
    m_textureOfR2S[m_activeTextureId] = targetRS->GetTextureOfR2T();

    return RENDER_STATUS_OK;
}

RenderStatus MultiViewRenderTarget::UpdateDisplayTex()
{
    return RENDER_STATUS_OK;
}


VCD_NS_END
#endif