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
//! \file     SWRenderSource.h
//! \brief    Defines class for SWRenderSource.
//!

#ifdef _LINUX_OS_

#ifndef _SWRENDERSOURCE_H_
#define _SWRENDERSOURCE_H_

#include <vector>
#include "../Common/Common.h"
#include "RenderSource.h"
#include "../Render/RenderBackend.h"


VCD_NS_BEGIN

class SWRenderSource
    : public RenderSource
{
public:
    SWRenderSource();
    virtual ~SWRenderSource();
    //! \brief Initialize RenderSource data according to mediaSource Info
    //!
    //! \param  [in] struct MediaSourceInfo *
    //!         Media Source Info
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus Initialize(struct MediaSourceInfo *mediaSourceInfo);
    virtual RenderStatus Initialize( int32_t pix_fmt, int32_t width, int32_t height );

    //! \brief Update the render source
    //!
    //! \param  [in] BufferInfo*
    //!         frame buffer info
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus UpdateR2T(BufferInfo* bufInfo);
    //! \brief Destroy the render source
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus DestroyRenderSource();

    //! \brief Create a render source
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus CreateRenderSource(bool hasInited);

    virtual RenderStatus process(BufferInfo* bufInfo);

private:

    //! \brief Create Source Texture
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus CreateSourceTex();
    //! \brief Create R2T FBO
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus CreateR2TFBO(bool hasInited);
private:
    bool            bInited;
};

VCD_NS_END
#endif /* _RENDERSOURCE_H_ */
#endif