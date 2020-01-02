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
//! \file     HWRenderSource.h
//! \brief    Defines class for HWRenderSource.
//!
#ifdef USE_DMA_BUFFER
#ifndef _HWRenderSource_H_
#define _HWRenderSource_H_

#include <vector>
#include "Common.h"
#include "RenderSource.h"
#include "RenderBackend.h"

VCD_NS_BEGIN

class HWRenderSource
    : public RenderSource
{
public:
    HWRenderSource(RenderBackend *renderBackend);
    virtual ~HWRenderSource();
    //! \brief Initialize RenderSource data according to mediaSource Info
    //!
    //! \param  [in] struct MediaSourceInfo *
    //!         Media Source Info
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus Initialize(struct MediaSourceInfo *mediaSourceInfo);
    //! \brief Create a render source
    //!
    //! \param  [in] RenderBackend*
    //!         RenderBackend interface
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus CreateRenderSource(RenderBackend *renderBackend);
    //! \brief Update the render source
    //!
    //! \param  [in] RenderBackend*
    //!         RenderBackend interface
    //!         [in] uint8_t** buffer
    //!         frame buffers.
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus UpdateR2T(RenderBackend *renderBackend, void **buffer);
    //! \brief Destroy the render source
    //!
    //! \param  [in] RenderBackend*
    //!         RenderBackend interface
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus DestroyRenderSource(RenderBackend *renderBackend);

private:
    //! \brief Create Source Texture
    //!
    //! \param  [in] RenderBackend*
    //!         RenderBackend interface
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus CreateSourceTex(RenderBackend *renderBackend);
    //! \brief Create R2T FBO
    //!
    //! \param  [in] RenderBackend*
    //!         RenderBackend interface
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus CreateR2TFBO(RenderBackend *renderBackend);
};

VCD_NS_END
#endif /* _RENDERSOURCE_H_ */
#endif /* USE_DMA_BUFFER */