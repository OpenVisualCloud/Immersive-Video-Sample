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
//! \file     CubeMapMesh.cpp
//! \brief    Implement class for CubeMapMesh.
//!

#include "CubeMapMesh.h"
#include <GL/gl.h>
#include "../Render/RenderBackend.h"

VCD_NS_BEGIN

#define VERTEX_NUM 36

CubeMapMesh::CubeMapMesh()
{
}

CubeMapMesh::~CubeMapMesh()
{
    SAFE_DELETE_ARRAY(m_vertices);
    SAFE_DELETE_ARRAY(m_indices);
    SAFE_DELETE_ARRAY(m_texCoords);
}

RenderStatus CubeMapMesh::Create()
{
    m_vertexNum = VERTEX_NUM;
    m_vertices = new float[m_vertexNum * 6];
    if (NULL == m_vertices)
    {
        return RENDER_ERROR;
    }
    float skyboxVertices[] = {
        // vertex postion         transform position
        // right- z (for flip)
         1.0f, -1.0f, -1.0f,      1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,      1.0f, -1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,      1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,      1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,      1.0f,  1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,      1.0f, -1.0f,  1.0f,
        // left- z (for flip)
        -1.0f, -1.0f,  1.0f,     -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,     -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,     -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,     -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,     -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,     -1.0f, -1.0f, -1.0f,
        // top- z (for flip)
        -1.0f,  1.0f, -1.0f,     -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,      1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,      1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,      1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,     -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,     -1.0f,  1.0f,  1.0f,
        // bottom- z (for flip)
        -1.0f, -1.0f, -1.0f,     -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,     -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,      1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,      1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,     -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,      1.0f, -1.0f, -1.0f,
        // back- x (for flip)
        -1.0f, -1.0f,  1.0f,      1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,      1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,     -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,     -1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,     -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,      1.0f, -1.0f,  1.0f,
        // front- x (for flip)
        -1.0f,  1.0f, -1.0f,      1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,      1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,     -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,     -1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,     -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,      1.0f,  1.0f, -1.0f,
    };
    for (uint32_t i=0; i<m_vertexNum * 6; i++)
    {
        m_vertices[i] = skyboxVertices[i];
    }
    m_texCoords = NULL; // in skybox, need not texture coords.
    return RENDER_STATUS_OK;
}

RenderStatus CubeMapMesh::Destroy()
{
    return RENDER_STATUS_OK;
}

RenderStatus CubeMapMesh::Bind(uint32_t vertexAttrib, uint32_t transVertexAttrib)
{
    if (m_typeChanged)
    {
        for (auto it = m_transformType.begin(); it != m_transformType.end(); it++)
        {
            uint32_t face_id = it->first;
            uint8_t transformType = it->second;
            if (transformType != 0) // NEED TO FIX! rotate 90/180/270
            {
                LOG(INFO)<< "transform type changed!" << endl;
                for (uint32_t i = face_id * VERTEX_NUM; i < (face_id + 1) * VERTEX_NUM; i++)
                {
                    if (i % 6 == 3) // transformed position x
                    {
                        if (transformType == 1) // hor-mirror
                            m_vertices[i] = -m_vertices[i];
                        else
                        {
                            if (transformType == 3) // first hor-mirror and then anti-clockwise
                            {
                                m_vertices[i] = -m_vertices[i];
                            }
                            float transDegree = 0;
                            if (transformType == 2 || transformType == 3)
                                transDegree = M_PI;
                            else if (transformType == 5 || transformType == 4)
                                transDegree = M_PI / 2 * 3;
                            else if (transformType == 7 || transformType == 6)
                                transDegree = M_PI / 2;
                            float x = m_vertices[i];
                            float y = m_vertices[i + 1];
                            m_vertices[i] = x * cos(transDegree) - y * sin(transDegree);
                            m_vertices[i + 1] = x * sin(transDegree) + y * cos(transDegree);
                            if (transformType == 4 || transformType == 6) // first anti-clockwise and then hor-mirror
                            {
                                m_vertices[i] = -m_vertices[i];
                            }
                        }
                    }
                }
            }
        }
    }
    RenderBackend *renderBackend = RENDERBACKEND::GetInstance();
    renderBackend->GenVertexArrays(1, &m_VAOHandle);
    renderBackend->GenBuffers(1, &m_VBOHandle);
    renderBackend->BindBuffer(GL_ARRAY_BUFFER, m_VBOHandle);
    renderBackend->BufferData(GL_ARRAY_BUFFER, m_vertexNum * 6 * sizeof(float), m_vertices, GL_STATIC_DRAW);
    renderBackend->BindVertexArray(m_VAOHandle);
    renderBackend->VertexAttribPointer(vertexAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    renderBackend->EnableVertexAttribArray(vertexAttrib);
    renderBackend->VertexAttribPointer(transVertexAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    renderBackend->EnableVertexAttribArray(transVertexAttrib);
    renderBackend->BindBuffer(GL_ARRAY_BUFFER, 0);
    renderBackend->BindVertexArray(0);
    return RENDER_STATUS_OK;
}

VCD_NS_END