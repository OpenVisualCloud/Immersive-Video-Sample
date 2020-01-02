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

#include "SWRenderSource.h"
#include <GL/glu.h>
#include <GL/glu_mangle.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <GLES3/gl3platform.h>
#include "Render2TextureMesh.h"

VCD_NS_BEGIN

SWRenderSource::SWRenderSource(RenderBackend *renderBackend)
{
    //1.render to texture : vertex and texCoords assign
    m_videoShaderOfR2T.Bind();
    m_meshOfR2T = new Render2TextureMesh();
    m_meshOfR2T->Create();
    uint32_t vertexAttribOfR2T = m_videoShaderOfR2T.SetAttrib("vPosition");
    uint32_t texCoordsAttribOfR2T = m_videoShaderOfR2T.SetAttrib("aTexCoord");
    m_meshOfR2T->Bind(renderBackend, vertexAttribOfR2T, texCoordsAttribOfR2T);
}

SWRenderSource::~SWRenderSource()
{
    if (m_meshOfR2T)
    {
        delete m_meshOfR2T;
        m_meshOfR2T = NULL;
    }
}

RenderStatus SWRenderSource::Initialize(struct MediaSourceInfo *mediaSourceInfo)
{
    if (NULL == mediaSourceInfo)
    {
        return RENDER_ERROR;
    }
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
    SetSourceWH(packedWH);
    SetSourceTextureNumber(number);
    return RENDER_STATUS_OK;
}

RenderStatus SWRenderSource::CreateRenderSource(RenderBackend *renderBackend)
{
    if (NULL == renderBackend)
    {
        return RENDER_ERROR;
    }
    if (CreateSourceTex(renderBackend) != RENDER_STATUS_OK || CreateR2TFBO(renderBackend) != RENDER_STATUS_OK)
    {
        return RENDER_ERROR;
    }
    return RENDER_STATUS_OK;
}

RenderStatus SWRenderSource::CreateSourceTex(RenderBackend *renderBackend)
{
    if (NULL == renderBackend)
    {
        return RENDER_ERROR;
    }
    //1. initial r2t three textures.
    uint32_t sourceTextureNumber = GetSourceTextureNumber();
    uint32_t *sourceTextureHandle = new uint32_t[sourceTextureNumber];
    renderBackend->GenTextures(sourceTextureNumber, sourceTextureHandle);
    SetSourceTextureHandle(sourceTextureHandle);
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
        struct SourceWH sourceWH = GetSourceWH();
        renderBackend->PixelStorei(GL_UNPACK_ROW_LENGTH, sourceWH.width[i]);
        renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        renderBackend->TexImage2D(GL_TEXTURE_2D, 0, GL_R8, sourceWH.width[i], sourceWH.height[i], 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    }
    return RENDER_STATUS_OK;
}

RenderStatus SWRenderSource::CreateR2TFBO(RenderBackend *renderBackend)
{
    if (NULL == renderBackend)
    {
        return RENDER_ERROR;
    }
    //2.initial FBOs
    uint32_t textureOfR2T;
    renderBackend->GenTextures(1, &textureOfR2T);
    renderBackend->BindTexture(GL_TEXTURE_2D, textureOfR2T);
    SetTextureOfR2T(textureOfR2T);
    struct SourceWH sourceWH = GetSourceWH();
    renderBackend->PixelStorei(GL_UNPACK_ROW_LENGTH, sourceWH.width[0]);
    renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    renderBackend->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    renderBackend->TexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sourceWH.width[0], sourceWH.height[0], 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    uint32_t fboR2THandle;
    renderBackend->GenFramebuffers(1, &fboR2THandle);
    renderBackend->BindFramebuffer(GL_FRAMEBUFFER, fboR2THandle);
    SetFboR2THandle(fboR2THandle);
    renderBackend->FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureOfR2T, 0);

    if (renderBackend->CheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("glCheckFramebufferStatus not complete\n");
        return RENDER_ERROR;
    }
    else
    {
        printf("glCheckFramebufferStatus complete\n");
    }
    return RENDER_STATUS_OK;
}

RenderStatus SWRenderSource::UpdateR2T(RenderBackend *renderBackend, void **buffer)
{
    if (NULL == renderBackend || NULL == buffer)
    {
        return RENDER_ERROR;
    }
    //1. update source texture
    uint32_t sourceTextureNumber = GetSourceTextureNumber();
    uint32_t *sourceTextureHandle = GetSourceTextureHandle();
    struct SourceWH sourceWH = GetSourceWH();
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
        renderBackend->PixelStorei(GL_UNPACK_ROW_LENGTH, sourceWH.width[i]);
        if (GetSourceTextureNumber() == 1)
            renderBackend->TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, sourceWH.width[i], sourceWH.height[i], GL_RGB, GL_UNSIGNED_BYTE, buffer[i]); //use rgb data
        else
            renderBackend->TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, sourceWH.width[i], sourceWH.height[i], GL_RED, GL_UNSIGNED_BYTE, buffer[i]); //use yuv data
    }
    //2. bind source texture and r2tFBO
    uint32_t fboR2THandle = GetFboR2THandle();
    renderBackend->BindFramebuffer(GL_FRAMEBUFFER, fboR2THandle);

    m_videoShaderOfR2T.Bind();
    renderBackend->BindVertexArray(renderBackend->GetR2TVAOHandle()); // check
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
    renderBackend->Viewport(0, 0, sourceWH.width[0], sourceWH.height[0]);
    renderBackend->DrawArrays(GL_TRIANGLES, 0, 6);
    return RENDER_STATUS_OK;
}

RenderStatus SWRenderSource::DestroyRenderSource(RenderBackend *renderBackend)
{
    if (NULL == renderBackend)
    {
        return RENDER_ERROR;
    }
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

VCD_NS_END
