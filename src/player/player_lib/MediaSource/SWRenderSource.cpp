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
//! \file     SWRenderSource.cpp
//! \brief    Implement class for SWRenderSource.
//!

#ifdef _LINUX_OS_

#include "SWRenderSource.h"
#include <chrono>
#include "../Mesh/Render2TextureMesh.h"

VCD_NS_BEGIN

SWRenderSource::SWRenderSource()
{
    bInited = false;
    //1.render to texture : vertex and texCoords assign
    m_videoShaderOfR2T.Bind();
    m_meshOfR2T = new Render2TextureMesh();
    m_meshOfR2T->Create();
    uint32_t vertexAttribOfR2T = m_videoShaderOfR2T.SetAttrib("vPosition");
    uint32_t texCoordsAttribOfR2T = m_videoShaderOfR2T.SetAttrib("aTexCoord");
    m_meshOfR2T->Bind( vertexAttribOfR2T, texCoordsAttribOfR2T );
}

SWRenderSource::~SWRenderSource()
{
    DestroyRenderSource();

    if (m_meshOfR2T)
    {
        delete m_meshOfR2T;
        m_meshOfR2T = NULL;
    }
}

RenderStatus SWRenderSource::Initialize( int32_t pix_fmt, int32_t width, int32_t height )
{
    m_videoShaderOfR2T.Bind();
    m_videoShaderOfR2T.SetUniform1i("frameTex", 0);
    m_videoShaderOfR2T.SetUniform1i("frameU", 1);
    m_videoShaderOfR2T.SetUniform1i("frameV", 2);
    m_videoShaderOfR2T.SetUniform1i("isNV12", 0);

    uint32_t number = 0;
    switch (pix_fmt)
    {
    case PixelFormat::PIX_FMT_RGB24:
        number = 1;
        if (m_sourceWH == NULL || m_sourceWH->width == NULL || m_sourceWH->height == NULL)
        {
            if (m_sourceWH != NULL)
            {
                SAFE_DELETE_ARRAY(m_sourceWH->width);
                SAFE_DELETE_ARRAY(m_sourceWH->height);
            }
            SAFE_DELETE(m_sourceWH);
            m_sourceWH = new struct SourceWH;
            m_sourceWH->width = new uint32_t[number];
            m_sourceWH->height = new uint32_t[number];
        }
        m_sourceWH->width[0] = width;
        m_sourceWH->height[0] = height;
        break;
    case PixelFormat::PIX_FMT_YUV420P:
        number = 3;
        if (m_sourceWH == NULL || m_sourceWH->width == NULL || m_sourceWH->height == NULL)
        {
            if (m_sourceWH != NULL)
            {
                SAFE_DELETE_ARRAY(m_sourceWH->width);
                SAFE_DELETE_ARRAY(m_sourceWH->height);
            }
            SAFE_DELETE(m_sourceWH);
            m_sourceWH = new struct SourceWH;
            m_sourceWH->width = new uint32_t[number];
            m_sourceWH->height = new uint32_t[number];
        }
        m_sourceWH->width[0] = width;
        m_sourceWH->width[1] = m_sourceWH->width[0] / 2;
        m_sourceWH->width[2] = m_sourceWH->width[1];
        m_sourceWH->height[0] = height;
        m_sourceWH->height[1] = m_sourceWH->height[0] / 2;
        m_sourceWH->height[2] = m_sourceWH->height[1];
        break;
    default:
        break;
    }
    SetSourceTextureNumber(number);
    CreateRenderSource(bInited);
    bInited = true;
    return RENDER_STATUS_OK;
}

RenderStatus SWRenderSource::Initialize(struct MediaSourceInfo *mediaSourceInfo)
{
    if (NULL == mediaSourceInfo)
    {
        return RENDER_ERROR;
    }
    m_videoShaderOfR2T.Bind();
    m_videoShaderOfR2T.SetUniform1i("frameTex", 0);
    m_videoShaderOfR2T.SetUniform1i("frameU", 1);
    m_videoShaderOfR2T.SetUniform1i("frameV", 2);
    m_videoShaderOfR2T.SetUniform1i("isNV12", 0);

    uint32_t number = 0;
    struct SourceWH packedWH;
    switch (mediaSourceInfo->pixFormat)
    {
    case PixelFormat::PIX_FMT_RGB24:
        number = 1;
        packedWH.width = new uint32_t[number];
        packedWH.height = new uint32_t[number];
        packedWH.width[0] = mediaSourceInfo->width;
        packedWH.height[0] = mediaSourceInfo->height;
        break;
    case PixelFormat::PIX_FMT_YUV420P:
        number = 3;
        packedWH.width = new uint32_t[number];
        packedWH.height = new uint32_t[number];
        packedWH.width[0] = mediaSourceInfo->width;
        packedWH.width[1] = packedWH.width[0] / 2;
        packedWH.width[2] = packedWH.width[1];
        packedWH.height[0] = mediaSourceInfo->height;
        packedWH.height[1] = packedWH.height[0] / 2;
        packedWH.height[2] = packedWH.height[1];
        break;
    default:
        break;
    }
    SetSourceWH(&packedWH);
    SetSourceTextureNumber(number);
    CreateRenderSource(bInited);
    bInited = true;
    return RENDER_STATUS_OK;
}

RenderStatus SWRenderSource::CreateRenderSource(bool hasInited)
{
    if (CreateSourceTex() != RENDER_STATUS_OK || CreateR2TFBO(hasInited) != RENDER_STATUS_OK)
    {
        return RENDER_ERROR;
    }
    return RENDER_STATUS_OK;
}

RenderStatus SWRenderSource::CreateSourceTex()
{
    RenderBackend *renderBackend = RENDERBACKEND::GetInstance();
    //1. initial r2t three textures.
    uint32_t sourceTextureNumber = GetSourceTextureNumber();
    if (m_sourceTextureHandle == NULL)
    {
        m_sourceTextureHandle = new uint32_t[sourceTextureNumber];
        renderBackend->GenTextures(sourceTextureNumber, m_sourceTextureHandle);
    }
    for (uint32_t i = 0; i < sourceTextureNumber; i++)
    {
        if (i == 0)
            renderBackend->ActiveTexture(GL_TEXTURE0);
        else if (i == 1)
            renderBackend->ActiveTexture(GL_TEXTURE1);
        else if (i == 2)
            renderBackend->ActiveTexture(GL_TEXTURE2);
        else if (i == 3)
            renderBackend->ActiveTexture(GL_TEXTURE3);

        renderBackend->BindTexture(GL_TEXTURE_2D, m_sourceTextureHandle[i]);
        struct SourceWH *sourceWH = GetSourceWH();

        renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        renderBackend->TexImage2D(GL_TEXTURE_2D, 0, GL_R8, sourceWH->width[i], sourceWH->height[i], 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    }
    return RENDER_STATUS_OK;
}

RenderStatus SWRenderSource::CreateR2TFBO(bool hasInited)
{
    RenderBackend *renderBackend = RENDERBACKEND::GetInstance();
    //2.initial FBOs
    if (!hasInited)
    {
        renderBackend->GenTextures(1, &m_textureOfR2T);
    }
    renderBackend->BindTexture(GL_TEXTURE_2D, m_textureOfR2T);
    struct SourceWH *sourceWH = GetSourceWH();

    renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    renderBackend->TexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sourceWH->width[0], sourceWH->height[0], 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    if (!hasInited)
    {
        renderBackend->GenFramebuffers(1, &m_fboR2THandle);
    }
    renderBackend->BindFramebuffer(GL_FRAMEBUFFER, m_fboR2THandle);
    renderBackend->FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureOfR2T, 0);

    if (renderBackend->CheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        LOG(ERROR)<<"Video "<< GetVideoID() <<": glCheckFramebufferStatus not complete when CreateR2TFBO"<<std::endl;
        return RENDER_ERROR;
    }
    else
    {
        LOG(INFO)<<"Video "<< GetVideoID() <<": glCheckFramebufferStatus complete when CreateR2TFBO"<<std::endl;
    }
    return RENDER_STATUS_OK;
}

RenderStatus SWRenderSource::UpdateR2T(BufferInfo* bufInfo)
{
    std::chrono::high_resolution_clock clock;
    uint64_t start1 = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
    RenderBackend *renderBackend = RENDERBACKEND::GetInstance();
    //1. update source texture
    uint32_t sourceTextureNumber = GetSourceTextureNumber();
    uint32_t *sourceTextureHandle = GetSourceTextureHandle();
    struct SourceWH *sourceWH = GetSourceWH();
    for (uint32_t i = 0; i < sourceTextureNumber; i++)
    {
        if (i == 0)
            renderBackend->ActiveTexture(GL_TEXTURE0);
        else if (i == 1)
            renderBackend->ActiveTexture(GL_TEXTURE1);
        else if (i == 2)
            renderBackend->ActiveTexture(GL_TEXTURE2);
        else if (i == 3)
            renderBackend->ActiveTexture(GL_TEXTURE3);
        renderBackend->BindTexture(GL_TEXTURE_2D, sourceTextureHandle[i]);
        if (bufInfo->stride[i] == 0)
        {
            LOG(ERROR) << "i " << i << "buf stride is zero! PTS " << bufInfo->pts << " video id " << m_VideoID << endl;
            return RENDER_ERROR;
        }
        renderBackend->PixelStorei(GL_UNPACK_ROW_LENGTH, bufInfo->stride[i]);
        LOG(INFO) <<" i = " << i << " TexSubImage2D width " << sourceWH->width[i] << " height " << sourceWH->height[i] << " video id " << m_VideoID << " PTS " << bufInfo->pts << endl;
        if (GetSourceTextureNumber() == 1)
            renderBackend->TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, sourceWH->width[i], sourceWH->height[i], GL_RGB, GL_UNSIGNED_BYTE, bufInfo->buffer[i]); //use rgb data
        else
            renderBackend->TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, sourceWH->width[i], sourceWH->height[i], GL_RED, GL_UNSIGNED_BYTE, bufInfo->buffer[i]); //use yuv data
    }
    uint64_t end1 = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
    LOG(INFO)<<"update process is:"<<(end1 - start1)<<endl;
    //2. bind source texture and r2tFBO
    uint64_t start2 = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
    uint32_t fboR2THandle = GetFboR2THandle();
    renderBackend->BindFramebuffer(GL_FRAMEBUFFER, fboR2THandle);

    m_videoShaderOfR2T.Bind();
    renderBackend->BindVertexArray(this->m_meshOfR2T->GetVAOHandle()); // check
    for (uint32_t i = 0; i < sourceTextureNumber; i++)
    {
        if (i == 0)
            renderBackend->ActiveTexture(GL_TEXTURE0);
        else if (i == 1)
            renderBackend->ActiveTexture(GL_TEXTURE1);
        else if (i == 2)
            renderBackend->ActiveTexture(GL_TEXTURE2);
        else if (i == 3)
            renderBackend->ActiveTexture(GL_TEXTURE3);
        renderBackend->BindTexture(GL_TEXTURE_2D, sourceTextureHandle[i]);
    }
    renderBackend->Viewport(0, 0, sourceWH->width[0], sourceWH->height[0]);
    renderBackend->DrawArrays(GL_TRIANGLES, 0, 6);
    uint64_t end2 = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
    LOG(INFO)<<"bind process is:"<<(end2 - start2)<<endl;
    return RENDER_STATUS_OK;
}

RenderStatus SWRenderSource::DestroyRenderSource()
{
    RenderBackend *renderBackend = RENDERBACKEND::GetInstance();
    uint32_t textureOfR2T = GetTextureOfR2T();
    if (textureOfR2T)
    {
        renderBackend->DeleteTextures(1, &textureOfR2T);
    }
    uint32_t sourceTextureNumber = GetSourceTextureNumber();
    uint32_t *sourceTextureHandle = GetSourceTextureHandle();
    if (sourceTextureHandle)
    {
        renderBackend->DeleteTextures(sourceTextureNumber, sourceTextureHandle);
    }
    uint32_t fboR2THandle = GetFboR2THandle();
    if (fboR2THandle)
    {
        renderBackend->DeleteFramebuffers(1, &fboR2THandle);
    }
    return RENDER_STATUS_OK;
}

RenderStatus SWRenderSource::process(BufferInfo* bufInfo)
{
    RenderStatus ret = RENDER_STATUS_OK;
    std::chrono::high_resolution_clock clock;
    uint64_t start3 = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
    if(bufInfo->bFormatChange || !bInited ){
        ret = Initialize(bufInfo->pixelFormat, bufInfo->width, bufInfo->height);
        LOG(INFO)<< "PTS " << bufInfo->pts << "texture need to resize to "<<bufInfo->width<<" x "<<bufInfo->height<<endl;
        if(RENDER_STATUS_OK!=ret){
            LOG(ERROR)<<"Video "<< GetVideoID() <<": Initialize Render source failed"<<std::endl;
            return ret;
        }
    }
    uint64_t end3 = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
    LOG(INFO)<<"init process is:"<<(end3 - start3)<<endl;
    // mCurRegionInfo = bufInfo->regionInfo;
    uint64_t start1 = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
    RegionData* curData = new RegionData(bufInfo->regionInfo->GetRegionWisePacking(), bufInfo->regionInfo->GetSourceInRegion(), bufInfo->regionInfo->GetSourceInfo());
    mCurRegionInfo.push_back(curData);
    // LOG(INFO)<<"regionInfo ptr:"<<mCurRegionInfo->GetSourceInRegion()<<" rwpk:"<<mCurRegionInfo->GetRegionWisePacking()->rectRegionPacking<<" source:"<<mCurRegionInfo->GetSourceInfo()->width<<endl;
    uint64_t end1 = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
    LOG(INFO)<<"regioninfo process is:"<<(end1 - start1)<<endl;
    uint64_t start2 = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
    ret = this->UpdateR2T(bufInfo);
    uint64_t end2 = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now().time_since_epoch()).count();
    LOG(INFO)<<"UpdateR2T process is:"<<(end2 - start2)<<endl;
    if(RENDER_STATUS_OK!=ret){
        LOG(ERROR)<<"Video "<< GetVideoID() <<": UpdateR2T failed"<<std::endl;
        return ret;
    }

    return RENDER_STATUS_OK;
}

VCD_NS_END
#endif