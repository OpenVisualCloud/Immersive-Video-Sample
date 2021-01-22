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
#include <math.h>
#ifdef _LINUX_OS_
#include <GL/gl.h>
#endif
#ifdef _ANDROID_OS_
#include <GLES3/gl3.h>
#endif
#include "../Render/RenderBackend.h"

VCD_NS_BEGIN

#define VERTEX_NUM 36

#define MAXVTX  1.0f
#define MINVTX -1.0f

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
        MAXVTX, MINVTX, MINVTX,     MAXVTX, MINVTX, MAXVTX,
        MAXVTX, MINVTX, MAXVTX,     MAXVTX, MINVTX, MINVTX,
        MAXVTX, MAXVTX, MAXVTX,     MAXVTX, MAXVTX, MINVTX,
        MAXVTX, MAXVTX, MAXVTX,     MAXVTX, MAXVTX, MINVTX,
        MAXVTX, MAXVTX, MINVTX,     MAXVTX, MAXVTX, MAXVTX,
        MAXVTX, MINVTX, MINVTX,     MAXVTX, MINVTX, MAXVTX,
        // left- z (for flip)
        MINVTX, MINVTX, MAXVTX,     MINVTX, MINVTX, MINVTX,
        MINVTX, MINVTX, MINVTX,     MINVTX, MINVTX, MAXVTX,
        MINVTX, MAXVTX, MINVTX,     MINVTX, MAXVTX, MAXVTX,
        MINVTX, MAXVTX, MINVTX,     MINVTX, MAXVTX, MAXVTX,
        MINVTX, MAXVTX, MAXVTX,     MINVTX, MAXVTX, MINVTX,
        MINVTX, MINVTX, MAXVTX,     MINVTX, MINVTX, MINVTX,
        // top- z (for flip)
        MINVTX, MAXVTX, MINVTX,     MINVTX, MAXVTX, MAXVTX,
        MAXVTX, MAXVTX, MINVTX,     MAXVTX, MAXVTX, MAXVTX,
        MAXVTX, MAXVTX, MAXVTX,     MAXVTX, MAXVTX, MINVTX,
        MAXVTX, MAXVTX, MAXVTX,     MAXVTX, MAXVTX, MINVTX,
        MINVTX, MAXVTX, MAXVTX,     MINVTX, MAXVTX, MINVTX,
        MINVTX, MAXVTX, MINVTX,     MINVTX, MAXVTX, MAXVTX,
        // bottom- z (for flip)
        MINVTX, MINVTX, MINVTX,     MINVTX, MINVTX, MAXVTX,
        MINVTX, MINVTX, MAXVTX,     MINVTX, MINVTX, MINVTX,
        MAXVTX, MINVTX, MINVTX,     MAXVTX, MINVTX, MAXVTX,
        MAXVTX, MINVTX, MINVTX,     MAXVTX, MINVTX, MAXVTX,
        MINVTX, MINVTX, MAXVTX,     MINVTX, MINVTX, MINVTX,
        MAXVTX, MINVTX, MAXVTX,     MAXVTX, MINVTX, MINVTX,
        // back- x (for flip)
        MINVTX, MINVTX, MAXVTX,     MAXVTX, MINVTX, MAXVTX,
        MINVTX, MAXVTX, MAXVTX,     MAXVTX, MAXVTX, MAXVTX,
        MAXVTX, MAXVTX, MAXVTX,     MINVTX, MAXVTX, MAXVTX,
        MAXVTX, MAXVTX, MAXVTX,     MINVTX, MAXVTX, MAXVTX,
        MAXVTX, MINVTX, MAXVTX,     MINVTX, MINVTX, MAXVTX,
        MINVTX, MINVTX, MAXVTX,     MAXVTX, MINVTX, MAXVTX,
        // front- x (for flip)
        MINVTX, MAXVTX, MINVTX,     MAXVTX, MAXVTX, MINVTX,
        MINVTX, MINVTX, MINVTX,     MAXVTX, MINVTX, MINVTX,
        MAXVTX, MINVTX, MINVTX,     MINVTX, MINVTX, MINVTX,
        MAXVTX, MINVTX, MINVTX,     MINVTX, MINVTX, MINVTX,
        MAXVTX, MAXVTX, MINVTX,     MINVTX, MAXVTX, MINVTX,
        MINVTX, MAXVTX, MINVTX,     MAXVTX, MAXVTX, MINVTX,
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
            if (transformType != NO_TRANSFORM) // NEED TO FIX! rotate 90/180/270
            {
                LOG(INFO)<< "transform type changed!" << endl;
                for (uint32_t i = face_id * VERTEX_NUM; i < (face_id + 1) * VERTEX_NUM; i++)
                {
                    if (i % 6 == 3) // transformed position x
                    {
                         // hor-mirror
                        if (transformType == MIRRORING_HORIZONTALLY)
                        {
                            if (face_id == CUBE_MAP_RIGHT || face_id == CUBE_MAP_LEFT)
                                m_vertices[i + 2] = -m_vertices[i + 2];
                            else if (face_id == CUBE_MAP_TOP || face_id == CUBE_MAP_BOTTOM)
                                m_vertices[i] = -m_vertices[i];
                            else if (face_id == CUBE_MAP_BACK || face_id == CUBE_MAP_FRONT)
                                m_vertices[i] = -m_vertices[i];
                        }
                        else
                        {
                            // first hor-mirror and then anti-clockwise
                            if (transformType == ROTATION_180_ANTICLOCKWISE_AFTER_MIRRORING_HOR)
                            {
                                if (face_id == CUBE_MAP_RIGHT || face_id == CUBE_MAP_LEFT)
                                    m_vertices[i + 2] = -m_vertices[i + 2];
                                else if (face_id == CUBE_MAP_TOP || face_id == CUBE_MAP_BOTTOM)
                                    m_vertices[i] = -m_vertices[i];
                                else if (face_id == CUBE_MAP_BACK || face_id == CUBE_MAP_FRONT)
                                    m_vertices[i] = -m_vertices[i];
                            }
                            // anti-clockwise
                            float transDegree = 0; // anti-clockwise degree
                            if (face_id == CUBE_MAP_RIGHT || face_id == CUBE_MAP_BOTTOM || face_id == CUBE_MAP_BACK)
                            {
                                if (transformType == ROTATION_180_ANTICLOCKWISE || transformType == ROTATION_180_ANTICLOCKWISE_AFTER_MIRRORING_HOR)
                                    transDegree = M_PI;
                                else if (transformType == ROTATION_90_ANTICLOCKWISE || transformType == ROTATION_90_ANTICLOCKWISE_BEFORE_MIRRORING_HOR)
                                    transDegree = M_PI / 2;
                                else if (transformType == ROTATION_270_ANTICLOCKWISE || transformType == ROTATION_270_ANTICLOCKWISE_BEFORE_MIRRORING_HOR)
                                    transDegree = M_PI / 2 * 3;
                            }
                            else
                            {
                                if (transformType == ROTATION_180_ANTICLOCKWISE || transformType == ROTATION_180_ANTICLOCKWISE_AFTER_MIRRORING_HOR)
                                    transDegree = M_PI;
                                else if (transformType == ROTATION_90_ANTICLOCKWISE || transformType == ROTATION_90_ANTICLOCKWISE_BEFORE_MIRRORING_HOR)
                                    transDegree = M_PI / 2 * 3;
                                else if (transformType == ROTATION_270_ANTICLOCKWISE || transformType == ROTATION_270_ANTICLOCKWISE_BEFORE_MIRRORING_HOR)
                                    transDegree = M_PI / 2;
                            }

                            // different face id
                            if (face_id == CUBE_MAP_RIGHT) // NY
                            {
                                float y = m_vertices[i + 1];
                                float z = m_vertices[i + 2];
                                m_vertices[i + 1] = y * cos(transDegree) - z * sin(transDegree);
                                m_vertices[i + 2] = y * sin(transDegree) + z * cos(transDegree);
                            }
                            else if (face_id == CUBE_MAP_LEFT) // PY
                            {
                                float y = m_vertices[i + 1];
                                float z = m_vertices[i + 2];
                                m_vertices[i + 1] = y * cos(transDegree) - z * sin(transDegree);
                                m_vertices[i + 2] = y * sin(transDegree) + z * cos(transDegree);
                            }
                            else if (face_id == CUBE_MAP_TOP) // PZ
                            {
                                float x = m_vertices[i];
                                float z = m_vertices[i + 2];
                                m_vertices[i] = x * cos(transDegree) - z * sin(transDegree);
                                m_vertices[i + 2] = x * sin(transDegree) + z * cos(transDegree);
                            }
                            else if (face_id == CUBE_MAP_BOTTOM) // NZ
                            {
                                float x = m_vertices[i];
                                float z = m_vertices[i + 2];
                                m_vertices[i] = x * cos(transDegree) - z * sin(transDegree);
                                m_vertices[i + 2] = x * sin(transDegree) + z * cos(transDegree);
                            }
                            else if (face_id == CUBE_MAP_BACK) // NX
                            {
                                float x = m_vertices[i];
                                float y = m_vertices[i + 1];
                                m_vertices[i] = x * cos(transDegree) - y * sin(transDegree);
                                m_vertices[i + 1] = x * sin(transDegree) + y * cos(transDegree);
                            }
                            else if (face_id == CUBE_MAP_FRONT) // PX
                            {
                                float x = m_vertices[i];
                                float y = m_vertices[i + 1];
                                m_vertices[i] = x * cos(transDegree) - y * sin(transDegree);
                                m_vertices[i + 1] = x * sin(transDegree) + y * cos(transDegree);
                            }
                            // first anti-clockwise and then hor-mirror
                            if (transformType == ROTATION_90_ANTICLOCKWISE_BEFORE_MIRRORING_HOR || transformType == ROTATION_270_ANTICLOCKWISE_BEFORE_MIRRORING_HOR)
                            {
                                if (face_id == CUBE_MAP_RIGHT || face_id == CUBE_MAP_LEFT)
                                    m_vertices[i + 2] = -m_vertices[i + 2];
                                else if (face_id == CUBE_MAP_TOP || face_id == CUBE_MAP_BOTTOM)
                                    m_vertices[i] = -m_vertices[i];
                                else if (face_id == CUBE_MAP_BACK || face_id == CUBE_MAP_FRONT)
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