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
//! \file  RenderBackend.cpp
//! \brief Implement class for RenderBackend.
//!

#include "RenderBackend.h"
#include <string.h>
#include <GL/glu.h>
#include <GL/glu_mangle.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>
#include <GLFW/glfw3.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <GLES3/gl3platform.h>

VCD_NS_BEGIN

RenderBackend::RenderBackend()
{
    m_renderBackendType = 0;
    m_onScreenVAOHandle = 0;
    m_onScreenEBOHandle = 0;
    m_r2tVAOHandle      = 0;
    m_r2tEBOHandle      = 0;
    memset(m_onScreenAttribs, 0, ATTRIBTYPE*sizeof(uint32_t));
    memset(m_r2tAttribs, 0, ATTRIBTYPE*sizeof(uint32_t));
}

int32_t RenderBackend::GetRenderBackendType()
{
    return m_renderBackendType;
}

uint32_t RenderBackend::GetOnScreenVAOHandle()
{
    return m_onScreenVAOHandle;
}

uint32_t RenderBackend::GetOnScreenEBOHandle()
{
    return m_onScreenEBOHandle;
}

uint32_t *RenderBackend::GetOnScreenAttribs()
{
    return m_onScreenAttribs;
}

uint32_t RenderBackend::GetR2TVAOHandle()
{
    return m_r2tVAOHandle;
}

uint32_t RenderBackend::GetR2TEBOHandle()
{
    return m_r2tEBOHandle;
}

uint32_t *RenderBackend::GetR2TAttribs()
{
    return m_r2tAttribs;
}

RenderStatus RenderBackend::SetOnScreenVAOHandle(uint32_t vaoHandle)
{
    m_onScreenVAOHandle = vaoHandle;
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::SetOnScreenEBOHandle(uint32_t eboHandle)
{
    m_onScreenEBOHandle = eboHandle;
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::SetOnScreenAttribs(uint32_t *attribs)
{
    if (NULL == attribs)
    {
        return RENDER_ERROR;
    }
    mempcpy(m_onScreenAttribs, attribs, sizeof(uint32_t) * ATTRIBTYPE);
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::SetR2TVAOHandle(uint32_t vaoHandle)
{
    m_r2tVAOHandle = vaoHandle;
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::SetR2TEBOHandle(uint32_t eboHandle)
{
    m_r2tEBOHandle = eboHandle;
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::SetR2TAttribs(uint32_t *attribs)
{
    if (NULL == attribs)
    {
        return RENDER_ERROR;
    }
    mempcpy(m_r2tAttribs, attribs, sizeof(uint32_t) * ATTRIBTYPE);
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::SetRenderBackendType(int32_t type)
{
    m_renderBackendType = type;
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::Clear(int32_t mask)
{
    glClear(mask); // mask can be set as GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::BindTexture(int32_t target, int32_t texture)
{
    glBindTexture(target, texture); //target can be set as GL_TEXTURE_2D
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::UnBindTexture()
{
    glBindTexture(GL_TEXTURE_2D, 0);
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::BindVertexArray(uint32_t array)
{
    glBindVertexArray(array);
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::BindBuffer(uint32_t target, uint32_t buffer)
{
    glBindBuffer(target, buffer);
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::DrawArrays(int32_t mode, int32_t first, int32_t count)
{
    glDrawArrays(mode, first, count); //mode can be set as GL_TRIANGLES
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::BufferData(uint32_t target, int32_t size, const void *data, uint32_t usage)
{
    glBufferData(target, size, data, usage);
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::Enable(int32_t cap)
{
    glEnable(cap);
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::Disable(int32_t cap)
{
    glDisable(cap);
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::ActiveTexture(int32_t texture)
{
    glActiveTexture(texture);
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::GenTextures(int32_t number, uint32_t *textures)
{
    glGenTextures(number, textures);
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::GenVertexArrays(int32_t number, uint32_t *arrays)
{
    glGenVertexArrays(number, arrays);
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::GenBuffers(int32_t number, uint32_t *buffers)
{
    glGenBuffers(number, buffers);
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::PixelStorei(uint32_t pname, int32_t param)
{
    glPixelStorei(pname, param);
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::TexParameteri(int32_t target, int32_t pname, int32_t param)
{
    glTexParameteri(target, pname, param);
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::TexImage2D(int32_t target, int32_t level, int32_t internalformat, int32_t width, int32_t height, int32_t border, int32_t format, int32_t type, const void *data)
{
    glTexImage2D(target, level, internalformat, width, height, border, format, type, data);
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::VertexAttribPointer(uint32_t index, int32_t size, uint32_t type, bool normalized, int32_t stride, const void *ptr)
{
    glVertexAttribPointer(index, size, type, normalized, stride, ptr);
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::EnableVertexAttribArray(uint32_t index)
{
    glEnableVertexAttribArray(index);
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::DrawElements(uint32_t mode, int32_t size, uint32_t type, const void *indices)
{
    glDrawElements(mode, size, type, indices);
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::TexSubImage2D(int32_t target, int32_t level, int32_t xoffset, int32_t yoffset, int32_t width, int32_t height, int32_t format, int32_t type, const void *pixels)
{
    glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::DeleteTextures(int32_t n, const uint32_t *textures)
{
    glDeleteTextures(n, textures);
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::DeleteFramebuffers(int32_t n, const uint32_t *frameBuffers)
{
    glDeleteFramebuffers(n, frameBuffers);
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::GenFramebuffers(int32_t n, uint32_t *frameBuffers)
{
    glGenFramebuffers(n, frameBuffers);
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::BindFramebuffer(int32_t target, uint32_t frameBuffers)
{
    glBindFramebuffer(target, frameBuffers);
    return RENDER_STATUS_OK;
}

RenderStatus RenderBackend::FramebufferTexture2D(int32_t target, int32_t attachment, int32_t texTarget, uint32_t texture, int32_t level)
{
    glFramebufferTexture2D(target, attachment, texTarget, texture, level);
    return RENDER_STATUS_OK;
}

int32_t RenderBackend::CheckFramebufferStatus(int32_t target)
{
    return glCheckFramebufferStatus(target);
}

RenderStatus RenderBackend::Viewport(int32_t x, int32_t y, int32_t width, int32_t height)
{
    glViewport(x, y, width, height);
    return RENDER_STATUS_OK;
}
VCD_NS_END
