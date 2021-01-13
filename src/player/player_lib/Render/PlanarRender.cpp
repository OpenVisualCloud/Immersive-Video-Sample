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
//! \file     PlanarRender.cpp
//! \brief    Implement class for PlanarRender.
//!

#ifdef _LINUX_OS_

#include "PlanarRender.h"
#include "../Mesh/PlanarMesh.h"
#include <GL/gl.h>

VCD_NS_BEGIN

PlanarRender::PlanarRender()
{
    m_videoShaderOfOnScreen = new VideoShader(shader_screen_vs, shader_screen_fs);
    //2.render to screen : vertex and texCoords assign
    m_videoShaderOfOnScreen->Bind();
    m_meshOfOnScreen = new PlanarMesh();
    m_meshOfOnScreen->Create();
    uint32_t vertexAttribOfOnScreen = m_videoShaderOfOnScreen->SetAttrib("vertex");
    uint32_t texCoordsAttribOfOnScreen = m_videoShaderOfOnScreen->SetAttrib("texCoord0");
    m_meshOfOnScreen->Bind(vertexAttribOfOnScreen, texCoordsAttribOfOnScreen);
}

PlanarRender::~PlanarRender()
{
    if (m_meshOfOnScreen != NULL)
    {
        delete m_meshOfOnScreen;
        m_meshOfOnScreen = NULL;
    }
}

RenderStatus PlanarRender::Render(uint32_t onScreenTexHandle, uint32_t width, uint32_t height, glm::mat4 ProjectionMatrix, glm::mat4 ViewModelMatrix)
{
    RenderBackend* renderBackend = RENDERBACKEND::GetInstance();
    renderBackend->BindFramebuffer(GL_FRAMEBUFFER, 0);
    renderBackend->Clear(GL_COLOR_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    renderBackend->Disable(GL_DEPTH_TEST);
    m_videoShaderOfOnScreen->Bind();
    m_videoShaderOfOnScreen->SetUniformMatrix4f("uProjMatrix", ProjectionMatrix);
    m_videoShaderOfOnScreen->SetUniformMatrix4f("uViewMatrix", ViewModelMatrix);
    renderBackend->Viewport(0, 0, width, height);
    uint32_t onScreenVAO = this->m_meshOfOnScreen->GetVAOHandle();//renderBackend->GetOnScreenVAOHandle();
    renderBackend->BindVertexArray(onScreenVAO);
    renderBackend->ActiveTexture(GL_TEXTURE0);
    renderBackend->BindTexture(GL_TEXTURE_2D, onScreenTexHandle);
    uint32_t meshVertexNum = m_meshOfOnScreen->GetVertexNum();
    renderBackend->DrawArrays(GL_TRIANGLES, 0, meshVertexNum);
    renderBackend->BindVertexArray(0);
    return RENDER_STATUS_OK;
}

void PlanarRender::SetUniformFrameTex()
{
    m_videoShaderOfOnScreen->Bind();
    m_videoShaderOfOnScreen->SetUniform1i("frameTex_screen", 0);
}

VCD_NS_END
#endif