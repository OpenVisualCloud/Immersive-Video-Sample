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
#include "../Render/RenderBackend.h"

VCD_NS_BEGIN

CubeMapMesh::CubeMapMesh()
{

}

CubeMapMesh::~CubeMapMesh()
{
    if (m_vertices != NULL)
    {
        delete m_vertices;
        m_vertices = NULL;
    }
    if (m_texCoords != NULL)
    {
        delete m_texCoords;
        m_texCoords = NULL;
    }
    if (m_indices != NULL)
    {
        delete m_indices;
        m_indices = NULL;
    }
}

RenderStatus CubeMapMesh::Create()
{
    return RENDER_STATUS_OK;
}

RenderStatus CubeMapMesh::Destroy()
{
    return RENDER_STATUS_OK;
}

RenderStatus CubeMapMesh::Bind(uint32_t vertexAttrib, uint32_t texCoordAttrib)
{
    return RENDER_STATUS_OK;
}

VCD_NS_END