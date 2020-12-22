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
//! \file     SurfaceRender.h
//! \brief    Defines class for SurfaceRender.
//!

#ifdef _LINUX_OS_

#ifndef _SURFACERENDER_H_
#define _SURFACERENDER_H_

#include "ViewPortManager.h"
#include "../Common/Common.h"
#include "VideoShader.h"
#include "../Mesh/Mesh.h"
#include "../MediaSource/RenderSource.h"
#include "RenderTarget.h"
#include "RenderBackend.h"
#include "ShaderString.h"

VCD_NS_BEGIN

class SurfaceRender
{
public:
    SurfaceRender()
    {
        m_meshOfOnScreen = NULL;
        m_renderType = 0;
        m_videoShaderOfOnScreen = NULL;
    };
    virtual ~SurfaceRender()=default;

    //! \brief The render function
    //!
    //! \param  [in] uint32_t width
    //!         render width
    //!         [in] uint32_t height
    //!         render height
    //!         glm::mat4
    //!         ProjectionMatrix
    //!         glm::mat4
    //!         ViewModelMatrix
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus Render(uint32_t onScreenTexHandle, uint32_t width, uint32_t height, glm::mat4 ProjectionMatrix, glm::mat4 ViewModelMatrix) = 0;

    virtual void SetUniformFrameTex() = 0;

    void SetTransformTypeToMesh(std::map<uint32_t, uint8_t> type) { m_meshOfOnScreen->SetTransformType(type); };
    std::map<uint32_t, uint8_t> GetTransformTypeToMesh() { return m_meshOfOnScreen->GetTransformType(); };

protected:
    Mesh        *m_meshOfOnScreen;
    int32_t      m_renderType;
    VideoShader *m_videoShaderOfOnScreen;
};

VCD_NS_END
#endif /* _SURFACERENDER_H_ */
#endif