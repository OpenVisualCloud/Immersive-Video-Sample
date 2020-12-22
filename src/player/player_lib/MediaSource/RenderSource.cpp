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
//! \file     RenderSource.cpp
//! \brief    Implement class for RenderSource.
//!

#include "RenderSource.h"
#include "../Render/ShaderString.h"

VCD_NS_BEGIN

RenderSource::RenderSource() : m_videoShaderOfR2T(shader_r2t_vs, shader_r2t_fs)
{
    m_sourceTextureHandle = NULL;
    m_sourceTextureNumber = 0;
    m_fboR2THandle        = 0;
    m_textureOfR2T        = 0;
    m_sourceWH            = NULL;
    m_sourceWH            = NULL;
    m_meshOfR2T           = NULL;
    m_VideoID             = -1;
    mCurRegionInfo.clear();
}

RenderSource::~RenderSource()
{
    if (m_sourceTextureHandle != NULL)
    {
        delete m_sourceTextureHandle;
        m_sourceTextureHandle = NULL;
    }
}

uint32_t *RenderSource::GetSourceTextureHandle()
{
    return m_sourceTextureHandle;
}

uint32_t RenderSource::GetSourceTextureNumber()
{
    return m_sourceTextureNumber;
}

uint32_t RenderSource::GetFboR2THandle()
{
    return m_fboR2THandle;
}

uint32_t RenderSource::GetTextureOfR2T()
{
    return m_textureOfR2T;
}

struct SourceWH* RenderSource::GetSourceWH()
{
    return m_sourceWH;
}

RenderStatus RenderSource::SetSourceTextureHandle(uint32_t *handle)
{
    if (NULL == handle)
    {
        return RENDER_ERROR;
    }
    m_sourceTextureHandle = handle;
    return RENDER_STATUS_OK;
}

RenderStatus RenderSource::SetSourceTextureNumber(uint32_t number)
{
    m_sourceTextureNumber = number;
    return RENDER_STATUS_OK;
}

RenderStatus RenderSource::SetFboR2THandle(uint32_t handle)
{
    m_fboR2THandle = handle;
    return RENDER_STATUS_OK;
}

RenderStatus RenderSource::SetTextureOfR2T(uint32_t texture)
{
    m_textureOfR2T = texture;
    return RENDER_STATUS_OK;
}

RenderStatus RenderSource::SetSourceWH(struct SourceWH *sourceWH)
{
    m_sourceWH = sourceWH;
    return RENDER_STATUS_OK;
}

void RenderSource::SetCurrentRegionInfo(RegionData* regionInfo)
{
}

VCD_NS_END
