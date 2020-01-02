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
//! \file     RenderSource.h
//! \brief    Defines class for RenderSource.
//!
#ifndef _RENDERSOURCE_H_
#define _RENDERSOURCE_H_

#include "Common.h"
#include "RenderBackend.h"
#include "Mesh.h"
#include "VideoShader.h"

VCD_NS_BEGIN

class RenderSource
{
public:
    RenderSource();
    virtual ~RenderSource();
    //! \brief Initialize RenderSource data according to mediaSource Info
    //!
    //! \param  [in] struct MediaSourceInfo *
    //!         Media Source Info
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus Initialize(struct MediaSourceInfo *mediaSourceInfo) = 0;
    //! \brief Create a render source
    //!
    //! \param  [in] RenderBackend*
    //!         RenderBackend interface
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus CreateRenderSource(RenderBackend *renderBackend) = 0;
    //! \brief Update the render source
    //!
    //! \param  [in] RenderBackend*
    //!         RenderBackend interface
    //!         [in] void* buffer[]
    //!         frame buffers.
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus UpdateR2T(RenderBackend *renderBackend, void **buffer) = 0;
    //! \brief Destroy the render source
    //!
    //! \param  [in] RenderBackend*
    //!         RenderBackend interface
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus DestroyRenderSource(RenderBackend *renderBackend) = 0;
    //! \brief Get Source Texture Handle
    //! \return uint32_t*
    //!         return Source Texture Handle
    //!
    uint32_t *GetSourceTextureHandle();
    //! \brief Get Source Texture Number
    //! \return uint32_t
    //!         return Source Texture Number
    //!
    uint32_t GetSourceTextureNumber();
    //! \brief Get Fbo R2T Handle
    //! \return uint32_t
    //!         return Fbo R2T Handle
    //!
    uint32_t GetFboR2THandle();
    //! \brief Get Texture Of R2T
    //! \return uint32_t
    //!         return Texture Of R2T
    //!
    uint32_t GetTextureOfR2T();
    //! \brief Get Source width and height
    //! \return struct SourceWH
    //!         return Source width and height
    //!
    struct SourceWH GetSourceWH();
    //! \brief Set Source Texture Handle
    //!
    //! \param  [in] uint32_t*
    //!         handles
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus SetSourceTextureHandle(uint32_t *handle);
    //! \brief Set Source Texture Number
    //!
    //! \param  [in] uint32_t
    //!         number
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus SetSourceTextureNumber(uint32_t number);
    //! \brief Set Fbo R2T Handle
    //!
    //! \param  [in] uint32_t
    //!         handle
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus SetFboR2THandle(uint32_t handle);
    //! \brief Set Texture Of R2T
    //!
    //! \param  [in] uint32_t
    //!         texture
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus SetTextureOfR2T(uint32_t texture);
    //! \brief Set Source Width and Height
    //!
    //! \param  [in] struct SourceWH
    //!         sourceWH
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus SetSourceWH(struct SourceWH sourceWH);
    VideoShader m_videoShaderOfR2T;
    Mesh *m_meshOfR2T;

private:
    uint32_t *m_sourceTextureHandle;
    uint32_t m_sourceTextureNumber; // yuv : 3 or rgb : 1
    uint32_t m_fboR2THandle;
    uint32_t m_textureOfR2T;    //output render to texture
    struct SourceWH m_sourceWH; //sourceTexture size
};

VCD_NS_END
#endif /* _RENDERSOURCE_H_ */
