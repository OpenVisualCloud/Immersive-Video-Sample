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
//! \file     ERPMesh.cpp
//! \brief    Implement class for ERPMesh.
//!
#include <stdlib.h>
#include <math.h>
#ifdef _LINUX_OS_
#include <GL/gl.h>
#endif
#ifdef _ANDROID_OS_
#include <GLES3/gl3.h>
#endif
#include "ERPMesh.h"
#include "../Render/RenderBackend.h"

VCD_NS_BEGIN

ERPMesh::ERPMesh()
{
    m_rows    = 0;
    m_columns = 0;
}

ERPMesh::~ERPMesh()
{
    if (m_vertices != NULL)
    {
        free(m_vertices);
        m_vertices = NULL;
    }
    if (m_texCoords != NULL)
    {
        free(m_texCoords);
        m_texCoords = NULL;
    }
    if (m_indices != NULL)
    {
        free(m_indices);
        m_indices = NULL;
    }
}

ERPMesh::ERPMesh(uint32_t columns, uint32_t rows)
{
    m_columns = columns;
    m_rows = rows;
}

RenderStatus ERPMesh::Create()
{
    if (m_columns <= 0 || m_rows <= 0)
    {
        return RENDER_CREATE_ERROR;
    }

    uint32_t meshColumns = m_columns;
    uint32_t meshRows = m_rows;
    uint32_t vertexColumns = m_columns + 1;
    uint32_t vertexRows = m_rows + 1;

    float width = 1.0f;
    float height = 1.0f;

    float bottom = 0.0f;
    float top = 1.0f;
    float left = 0.0f;
    float right = 1.0f;

    uint32_t numVertices = vertexColumns * vertexRows;
    m_vertices = (float *)malloc(numVertices * sizeof(float) * 3);
    if (NULL == m_vertices)
    {
        return RENDER_ERROR;
    }
    m_texCoords = (float *)malloc(numVertices * sizeof(float) * 2);
    if (NULL == m_texCoords)
    {
        return RENDER_ERROR;
    }
    m_vertexNum = numVertices;
    uint32_t vertexPtr = 0;
    uint32_t texPtr = 0;

    float longitudeStep = width / m_columns;
    float latitudeStep = height / m_rows;
    float u = 0.0f;
    float v = top;

    for (uint32_t j = 0; j < vertexRows; j++, v -= latitudeStep)
    {
        u = left;
        for (uint32_t i = 0; i < vertexColumns; ++i, u += longitudeStep)
        {
            float r = (float)sin(M_PI * j / m_rows);
            float x = -r * (float)sin(2.0f * M_PI * i / m_columns);
            float y = (float)cos(M_PI * j / m_rows);
            float z = r * (float)cos(2.0f * M_PI * i / m_columns);

            m_vertices[vertexPtr++] = x;
            m_vertices[vertexPtr++] = y;
            m_vertices[vertexPtr++] = z;

            m_texCoords[texPtr++] = i < m_columns ? u : right;
            m_texCoords[texPtr++] = j < m_rows ? 1 - v : 1 - bottom;
        }
    }

    uint32_t numIndices = 2 * meshColumns * meshRows - 2 + 4 * meshRows;
    m_indices = (uint32_t *)malloc(sizeof(uint32_t) * numIndices);
    if (NULL == m_indices)
    {
        return RENDER_ERROR;
    }
    m_indexNum = numIndices;
    uint32_t index = 0;
    uint32_t n = 0;

    for (uint32_t j = 0; j < meshRows; ++j)
    {
        if (j != 0)
        {
            m_indices[index++] = n;
        }
        for (uint32_t i = 0; i < vertexColumns; ++i, ++n)
        {
            m_indices[index++] = n;
            m_indices[index++] = n + vertexColumns;
        }
        if (j < meshRows - 1)
        {
            m_indices[index++] = n + vertexColumns - 1;
        }
    }
    return RENDER_STATUS_OK;
}

RenderStatus ERPMesh::Bind(uint32_t vertexAttrib, uint32_t texCoordAttrib)
{
    RenderBackend *renderBackend = RENDERBACKEND::GetInstance();
    renderBackend->GenVertexArrays(1, &m_VAOHandle);
    renderBackend->GenBuffers(1, &m_EBOHandle);
    renderBackend->BindVertexArray(m_VAOHandle);
    renderBackend->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBOHandle);
    renderBackend->BufferData(GL_ELEMENT_ARRAY_BUFFER, GetIndexNum() * sizeof(uint32_t), &GetIndices()[0], GL_STATIC_DRAW);
    renderBackend->VertexAttribPointer(vertexAttrib, 3, GL_FLOAT, GL_FALSE, 0, GetVertices());
    renderBackend->EnableVertexAttribArray(vertexAttrib);
    renderBackend->VertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_FALSE, 0, GetTexCoords());
    renderBackend->EnableVertexAttribArray(texCoordAttrib);
    renderBackend->BindVertexArray(0);
    return RENDER_STATUS_OK;
}

RenderStatus ERPMesh::Destroy()
{
    return RENDER_STATUS_OK;
}

VCD_NS_END