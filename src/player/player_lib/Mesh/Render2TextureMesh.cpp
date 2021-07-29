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
//! \file     Render2TextureMesh.cpp
//! \brief    Implement class for Render2TextureMesh.
//!
#include <stdlib.h>
#include <math.h>
#ifdef _LINUX_OS_
#include <GL/gl.h>
#endif
#ifdef _ANDROID_OS_
#include <GLES3/gl3.h>
#endif
#include "Render2TextureMesh.h"
#include "../Render/RenderBackend.h"

VCD_NS_BEGIN

RenderStatus Render2TextureMesh::Create()
{
    m_vertexNum = 6; //6 vertex * 5
    m_vertices = new float[m_vertexNum * 5];
    if (NULL == m_vertices)
    {
        return RENDER_ERROR;
    }
    float squareVertices[] = {
        // positions         // texture coords
         1.0f,  1.0f, 0.0f,    1.0f, 1.0f, // top right
         1.0f, -1.0f, 0.0f,    1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f,    0.0f, 0.0f, // bottom left
         1.0f,  1.0f, 0.0f,    1.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,    0.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,    0.0f, 1.0f  // top left
    };
    for (uint32_t i = 0; i < m_vertexNum * 5; i++)
    {
        m_vertices[i] = squareVertices[i];
    }
    return RENDER_STATUS_OK;
}

RenderStatus Render2TextureMesh::Bind(uint32_t vertexAttrib, uint32_t texCoordAttrib)
{
    RenderBackend *renderBackend = RENDERBACKEND::GetInstance();
    renderBackend->GenVertexArrays(1, &m_VAOHandle);
    renderBackend->GenBuffers(1, &m_VBOHandle);
    renderBackend->BindVertexArray(m_VAOHandle);

    renderBackend->BindBuffer(GL_ARRAY_BUFFER, m_VBOHandle);
    renderBackend->BufferData(GL_ARRAY_BUFFER, m_vertexNum * 5 * sizeof(float), m_vertices, GL_STATIC_DRAW);

    renderBackend->VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    renderBackend->EnableVertexAttribArray(0);
    renderBackend->VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    renderBackend->EnableVertexAttribArray(1);
    return RENDER_STATUS_OK;
}

RenderStatus Render2TextureMesh::Destroy()
{
    return RENDER_STATUS_OK;
}

VCD_NS_END