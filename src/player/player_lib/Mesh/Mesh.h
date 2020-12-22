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
//! \file     Mesh.h
//! \brief    Defines class for Mesh.
//!
#ifndef _MESH_H_
#define _MESH_H_

#include "../Common/Common.h"
#include <map>

VCD_NS_BEGIN

class Mesh
{
public:
    Mesh(){
         m_vertices = NULL;
         m_texCoords = NULL;
         m_indices = NULL;
         m_vertexNum = 0;
         m_indexNum = 0;
         m_VAOHandle = 0;
         m_EBOHandle = 0;
         m_VBOHandle = 0;
         m_typeChanged = false;
         for (uint32_t i = 0; i < FACE_SIZE; i++)
         {
             m_transformType[i] = 0;
         }
    };

    virtual ~Mesh()=default;

    ///abstract interface
    virtual RenderStatus Create() = 0;
    virtual RenderStatus Destroy() = 0;
    virtual RenderStatus Bind(uint32_t vertexAttrib, uint32_t texCoordAttrib) = 0;

    ///get Methods
    uint32_t GetVertexNum(){ return m_vertexNum; };
    uint32_t GetIndexNum(){ return m_indexNum; };
    float *GetVertices(){ return m_vertices; };
    float *GetTexCoords(){ return m_texCoords; };
    uint32_t *GetIndices(){ return m_indices; };
    uint32_t GetVAOHandle(){return m_VAOHandle; };
    uint32_t GetEBOHandle(){return m_EBOHandle; };
    uint32_t GetVBOHandle(){return m_VBOHandle; };
    void SetTransformType(std::map<uint32_t, uint8_t> type)
    {
        if (type.empty())
        {
            // LOG(ERROR) << "Current transform type is invalid!" << std::endl;
            return;
        }
        std::map<uint32_t, uint8_t>::iterator it;
        for (it = type.begin(); it != type.end(); it++)
        {
            uint32_t face_id = it->first;
            uint8_t type = it->second;
            if (m_transformType.find(face_id) != m_transformType.end() && m_transformType[face_id] != type) // diff
            {
                m_typeChanged = true;
                break;
            }
        }
        if (it == type.end())
        {
            m_typeChanged = false;
        }
        m_transformType = type;
        return;
    };
    std::map<uint32_t, uint8_t> GetTransformType() { return m_transformType; };

    virtual RenderStatus BufferUpdate(void *tile_info) { return RENDER_STATUS_OK; };

protected:
    float    *m_vertices;
    float    *m_texCoords;
    uint32_t *m_indices;
    uint32_t  m_vertexNum;
    uint32_t  m_indexNum;
    uint32_t  m_VAOHandle;
    uint32_t  m_EBOHandle;
    uint32_t  m_VBOHandle;
    bool      m_typeChanged;
    std::map<uint32_t, uint8_t> m_transformType;
};

VCD_NS_END
#endif /* _MESH_H_ */
