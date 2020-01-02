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
//! \file     DMABufferRenderSource.cpp
//! \brief    Implement class for DMABufferRenderSource.
//!
#ifdef USE_DMA_BUFFER
#include "DMABufferRenderSource.h"
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

#include <va/va.h>
#include <va/va_drmcommon.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <libdrm/drm_fourcc.h>

void (EGLAPIENTRY *EGLImageTargetTexture2DOES)(GLenum, GLeglImageOES);
EGLImageKHR (EGLAPIENTRY *CreateImageKHR)(EGLDisplay, EGLContext,EGLenum, EGLClientBuffer, const EGLint *);

EGLImageKHR images[2] = {EGL_NO_IMAGE_KHR, EGL_NO_IMAGE_KHR};
extern VABufferInfo buffer_info;
extern VAImage va_image;
extern EGLDisplay  pEglDisplay; //to do: move it into the class

#define MP_ARRAY_SIZE(s) (sizeof(s) / sizeof((s)[0]))
#define ADD_ATTRIB(name, value)                         \
    do {                                                \
    assert(num_attribs + 3 < MP_ARRAY_SIZE(attribs));   \
    attribs[num_attribs++] = (name);                    \
    attribs[num_attribs++] = (value);                   \
    attribs[num_attribs] = EGL_NONE;                    \
    } while(0)

VCD_NS_BEGIN

DMABufferRenderSource::DMABufferRenderSource(RenderBackend *renderBackend)
{
    //1.render to texture : vertex and texCoords assign
    m_videoShaderOfR2T.Bind();
    m_meshOfR2T = new Render2TextureMesh();
    m_meshOfR2T->Create();
    uint32_t vertexAttribOfR2T = m_videoShaderOfR2T.SetAttrib("vPosition");
    uint32_t texCoordsAttribOfR2T = m_videoShaderOfR2T.SetAttrib("aTexCoord");
    m_meshOfR2T->Bind(renderBackend, vertexAttribOfR2T, texCoordsAttribOfR2T);
}

DMABufferRenderSource::~DMABufferRenderSource()
{
    if (m_meshOfR2T)
    {
        delete m_meshOfR2T;
        m_meshOfR2T = NULL;
    }
}

RenderStatus DMABufferRenderSource::Initialize(struct MediaSourceInfo *mediaSourceInfo)
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
        break;
    case PixelFormat::PIX_FMT_YUV420P:
        break;
     case PixelFormat::AV_PIX_FMT_NV12_DMA_BUFFER:
        number = 2;
        packedWH.width = new uint32_t[number];
        packedWH.height = new uint32_t[number];
        packedWH.width[0] = mediaSourceInfo->width;
        packedWH.width[1] = packedWH.width[0] / 2;
        packedWH.height[0] = mediaSourceInfo->height;
        packedWH.height[1] = packedWH.height[0] / 2;
        break;
   default:
        break;
    }
    SetSourceWH(packedWH);
    SetSourceTextureNumber(number);
    EGLImageTargetTexture2DOES = (void (*)(GLenum, GLeglImageOES))eglGetProcAddress("glEGLImageTargetTexture2DOES");
    CreateImageKHR = (void* (*)(EGLDisplay, EGLContext, EGLenum, EGLClientBuffer, const EGLint*))eglGetProcAddress("eglCreateImageKHR");
    return RENDER_STATUS_OK;
}

RenderStatus DMABufferRenderSource::CreateRenderSource(RenderBackend *renderBackend)
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

RenderStatus DMABufferRenderSource::CreateSourceTex(RenderBackend *renderBackend)
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
        if (i == 1 && sourceTextureNumber == 2) //hardware decoding
            renderBackend->TexImage2D(GL_TEXTURE_2D, 0, GL_RG, sourceWH.width[i], sourceWH.height[i], 0, GL_RG, GL_UNSIGNED_BYTE, NULL);
        else
            renderBackend->TexImage2D(GL_TEXTURE_2D, 0, GL_RED, sourceWH.width[i], sourceWH.height[i], 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    }

    //renderBackend->BindTexture(GL_TEXTURE_2D, 0);
    return RENDER_STATUS_OK;
}

RenderStatus DMABufferRenderSource::CreateR2TFBO(RenderBackend *renderBackend)
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

RenderStatus DMABufferRenderSource::UpdateR2T(RenderBackend *renderBackend, void **buffer)
{
    static int frame_number =0;
    frame_number++;
    if (frame_number <2) return RENDER_ERROR;
    if (NULL == renderBackend || NULL == buffer)
    {
        return RENDER_ERROR;
    }
    //1. update source texture
    uint32_t sourceTextureNumber = GetSourceTextureNumber();
    uint32_t *sourceTextureHandle = GetSourceTextureHandle();
    struct SourceWH sourceWH = GetSourceWH();

    if (buffer_info.handle == 0)
        return RENDER_ERROR;
    for (int n = 0; n < 2; n++) {
        int attribs[20] = {EGL_NONE};
        uint32_t num_attribs = 0;
        int drm_fmt;

        if (n == 0) drm_fmt = DRM_FORMAT_R8;
        else if (n ==1) drm_fmt = DRM_FORMAT_GR88;

        ADD_ATTRIB(EGL_LINUX_DRM_FOURCC_EXT, drm_fmt);
        if (n == 0) ADD_ATTRIB(EGL_WIDTH, 5760); //hard coded
        else ADD_ATTRIB(EGL_WIDTH, (5760/2));
        if (n == 0)ADD_ATTRIB(EGL_HEIGHT, 3840); //hard coded
        else ADD_ATTRIB(EGL_HEIGHT, (3840/2)); //hard coded
        ADD_ATTRIB(EGL_DMA_BUF_PLANE0_FD_EXT, buffer_info.handle);
        ADD_ATTRIB(EGL_DMA_BUF_PLANE0_OFFSET_EXT, va_image.offsets[n]);
        ADD_ATTRIB(EGL_DMA_BUF_PLANE0_PITCH_EXT, va_image.pitches[n]);
        //if(images[n] == NULL)
            images[n] = CreateImageKHR(pEglDisplay,EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, attribs);

        if (!images[n])
            return RENDER_ERROR;

    }

    for (uint32_t i = 0; i < sourceTextureNumber; i++)
    {
        //if (buffer == NULL) break;
        //if (buffer[i] == NULL) break;
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
        if ( sourceTextureNumber == 1)
        {
            renderBackend->TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, sourceWH.width[i], sourceWH.height[i], GL_RGB, GL_UNSIGNED_BYTE, buffer[i]); //use rgb data
        }
        else if (sourceTextureNumber == 2 && i == 1 )
        {
            EGLImageTargetTexture2DOES(GL_TEXTURE_2D, images[i]);
        }
        else
        {
            EGLImageTargetTexture2DOES(GL_TEXTURE_2D, images[i]);
        }
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

RenderStatus DMABufferRenderSource::DestroyRenderSource(RenderBackend *renderBackend)
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
#endif /* USE_DMA_BUFFER */