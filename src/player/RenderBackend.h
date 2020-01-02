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
//! \file     RenderBackend.h
//! \brief    Define RenderBackend.
//!
#ifndef _RENDERBACKEND_H_
#define _RENDERBACKEND_H_
#define ATTRIBTYPE 2

#include "Common.h"
#include "../utils/Singleton.h"

VCD_NS_BEGIN

class RenderBackend
{
public:
    RenderBackend();
    virtual ~RenderBackend(){};

    //! \brief Get the render backend type
    //!
    //! \param  [in] N/A
    //!
    //! \return int32_t
    //!         RenderBackendType
    //!
    int32_t GetRenderBackendType();
    //! \brief Set the render backend type
    //!
    //! \param  [in] type
    //!         the render backend type
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus SetRenderBackendType(int32_t type);
    uint32_t GetOnScreenVAOHandle();
    uint32_t GetOnScreenEBOHandle();
    uint32_t *GetOnScreenAttribs();
    uint32_t GetR2TVAOHandle();
    uint32_t GetR2TEBOHandle();
    uint32_t *GetR2TAttribs();
    RenderStatus SetOnScreenVAOHandle(uint32_t vaoHandle);
    RenderStatus SetOnScreenEBOHandle(uint32_t eboHandle);
    RenderStatus SetOnScreenAttribs(uint32_t *attribs);
    RenderStatus SetR2TVAOHandle(uint32_t vaoHandle);
    RenderStatus SetR2TEBOHandle(uint32_t eboHandle);
    RenderStatus SetR2TAttribs(uint32_t *attribs);
    RenderStatus Clear(int32_t mask);
    RenderStatus BindTexture(int32_t target, int32_t texture);
    RenderStatus UnBindTexture();
    RenderStatus BindVertexArray(uint32_t array);
    RenderStatus BindBuffer(uint32_t target, uint32_t buffer);
    RenderStatus DrawArrays(int32_t mode, int32_t first, int32_t count);
    RenderStatus BufferData(uint32_t target, int32_t size, const void *data, uint32_t usage);
    RenderStatus Enable(int32_t cap);
    RenderStatus Disable(int32_t cap);
    RenderStatus ActiveTexture(int32_t texture);
    RenderStatus GenTextures(int32_t number, uint32_t *textures);
    RenderStatus GenVertexArrays(int32_t number, uint32_t *arrays);
    RenderStatus GenBuffers(int32_t number, uint32_t *buffers);
    RenderStatus PixelStorei(uint32_t pname, int32_t param);
    RenderStatus TexParameteri(int32_t target, int32_t pname, int32_t param);
    RenderStatus TexImage2D(int32_t target, int32_t level, int32_t internalformat, int32_t width, int32_t height, int32_t border, int32_t format, int32_t type, const void *data);
    RenderStatus VertexAttribPointer(uint32_t index, int32_t size, uint32_t type, bool normalized, int32_t stride, const void *ptr);
    RenderStatus EnableVertexAttribArray(uint32_t index);
    RenderStatus DrawElements(uint32_t mode, int32_t size, uint32_t type, const void *indices);
    RenderStatus TexSubImage2D(int32_t target, int32_t level, int32_t xoffset, int32_t yoffset, int32_t width, int32_t height, int32_t format, int32_t type, const void *pixels);
    RenderStatus DeleteTextures(int32_t n, const uint32_t *textures);
    RenderStatus DeleteFramebuffers(int32_t n, const uint32_t *frameBuffers);
    RenderStatus GenFramebuffers(int32_t n, uint32_t *frameBuffers);
    RenderStatus BindFramebuffer(int32_t target, uint32_t frameBuffers);
    RenderStatus FramebufferTexture2D(int32_t target, int32_t attachment, int32_t texTarget, uint32_t texture, int32_t level);
    int32_t CheckFramebufferStatus(int32_t target);
    RenderStatus Viewport(int32_t x, int32_t y, int32_t width, int32_t height);

private:
    int32_t m_renderBackendType;
    uint32_t m_onScreenVAOHandle;
    uint32_t m_onScreenEBOHandle;
    uint32_t m_onScreenAttribs[ATTRIBTYPE];
    uint32_t m_r2tVAOHandle;
    uint32_t m_r2tEBOHandle;
    uint32_t m_r2tAttribs[ATTRIBTYPE];
};

typedef Singleton<RenderBackend> RENDERBACKEND;

VCD_NS_END
#endif /* _RENDERBACKEND_H_ */
