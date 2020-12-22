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
//! \file     Render2TextureMesh_android.cpp
//! \brief    Implement class for Render2TextureMesh_android.
//!

#ifdef _ANDROID_OS_
#include <stdlib.h>
#include <math.h>
#include <GLES2/gl2.h>
#include "Render2TextureMesh_android.h"
#include "360SCVPAPI.h"

VCD_NS_BEGIN

RenderStatus Render2TextureMesh_android::Create()
{
    m_vertexNum = 4; //6 vertex * 3
    m_vertices = new float[m_vertexNum * 2];
    if (NULL == m_vertices)
    {
        return RENDER_ERROR;
    }
    float squareVertices[] = {
            -1.0f, -1.0f,
            -1.0f,  1.0f,
             1.0f, -1.0f,
             1.0f,  1.0f
    };
    for (uint32_t i = 0; i < m_vertexNum * 2; i++)
    {
        m_vertices[i] = squareVertices[i];
    }
    m_texCoords = new float[m_vertexNum * 2]; // 6 vertex * 2
    if (NULL == m_texCoords)
    {
        return RENDER_ERROR;
    }
    float squareTexCoords[] = {
            0.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 0.0f,
            1.0f, 1.0f
    };
    for (uint32_t i = 0; i < m_vertexNum * 2; i++)
    {
        m_texCoords[i] = squareTexCoords[i];
    }
    return RENDER_STATUS_OK;
}

RenderStatus Render2TextureMesh_android::Bind(uint32_t vertexAttrib, uint32_t texCoordAttrib)
{
    // ANDROID_LOGD("Render2TextureMesh_android::84 vertexAttrib IS %d texCoordAttrib is %d", vertexAttrib, texCoordAttrib);
    // glGenVertexArrays(1, &m_VAOHandle);
    // glBindVertexArray(m_VAOHandle);
    // ANDROID_LOGD("Render2TextureMesh_android::86 m_VAOHandle IS %d", m_VAOHandle);
    glVertexAttribPointer(vertexAttrib, 2, GL_FLOAT, GL_FALSE, 0, m_vertices);
    glEnableVertexAttribArray(vertexAttrib);
    glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_FALSE, 0, m_texCoords);
    glEnableVertexAttribArray(texCoordAttrib);
    glBindVertexArray(0);
    return RENDER_STATUS_OK;
}

RenderStatus Render2TextureMesh_android::Destroy()
{
    return RENDER_STATUS_OK;
}

RenderStatus Render2TextureMesh_android::BufferUpdate(void *tile_info)
{
    TileInformation *tileInfo = static_cast<TileInformation*>(tile_info);
    // proj vertices
    m_vertices[0] = (float(tileInfo->projRegLeft) / tileInfo->picWidth) * 2.0 - 1.0;
    m_vertices[1] = (float(tileInfo->projRegTop + tileInfo->projRegHeight) / tileInfo->picHeight) * 2.0 - 1.0;
    m_vertices[2] = (float(tileInfo->projRegLeft) / tileInfo->picWidth) * 2.0 - 1.0;
    m_vertices[3] = (float(tileInfo->projRegTop) / tileInfo->picHeight) * 2.0 - 1.0;
    m_vertices[4] = (float(tileInfo->projRegLeft + tileInfo->projRegWidth) / tileInfo->picWidth) * 2.0 - 1.0;
    m_vertices[5] = (float(tileInfo->projRegTop + tileInfo->projRegHeight) / tileInfo->picHeight) * 2.0 - 1.0;
    m_vertices[6] = (float(tileInfo->projRegLeft + tileInfo->projRegWidth) / tileInfo->picWidth) * 2.0 - 1.0;
    m_vertices[7] = (float(tileInfo->projRegTop) / tileInfo->picHeight) * 2.0 - 1.0;
    // ANDROID_LOGD("proj l:%d, t:%d, w:%d, h:%d picW:%d, picH:%d",
    // tileInfo->projRegLeft, tileInfo->projRegTop, tileInfo->projRegWidth, tileInfo->projRegHeight, tileInfo->picWidth, tileInfo->picHeight);

    // packed texture
    m_texCoords[0] = (float(tileInfo->packedRegLeft) / tileInfo->packedPicWidth);
    m_texCoords[1] = (float(tileInfo->packedRegTop + tileInfo->packedRegHeight) / tileInfo->packedPicHeight);
    m_texCoords[2] = (float(tileInfo->packedRegLeft) / tileInfo->packedPicWidth);
    m_texCoords[3] = (float(tileInfo->packedRegTop) / tileInfo->packedPicHeight);
    m_texCoords[4] = (float(tileInfo->packedRegLeft + tileInfo->packedRegWidth) / tileInfo->packedPicWidth);
    m_texCoords[5] = (float(tileInfo->packedRegTop + tileInfo->packedRegHeight) / tileInfo->packedPicHeight);
    m_texCoords[6] = (float(tileInfo->packedRegLeft + tileInfo->packedRegWidth) / tileInfo->packedPicWidth);
    m_texCoords[7] = (float(tileInfo->packedRegTop) / tileInfo->packedPicHeight);
    // ANDROID_LOGD("pack l:%d, t:%d, w:%d, h:%d, picW:%d, picH:%d", tileInfo->packedRegLeft, tileInfo->packedRegTop, tileInfo->packedRegWidth, tileInfo->packedRegHeight, tileInfo->packedPicWidth, tileInfo->packedPicHeight);
    return RENDER_STATUS_OK;
}

VCD_NS_END
#endif