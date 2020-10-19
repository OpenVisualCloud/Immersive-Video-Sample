/*
 * Copyright (c) 2018, Intel Corporation
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
 */

#ifndef _GENVIEWPORT_API_H_
#define _GENVIEWPORT_API_H_
#include <stdbool.h>
#include <stdint.h>
#include "360SCVPAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct POINTDEF
{
    int32_t faceId;
    int32_t x;
    int32_t y;
} point;


//!
//! \brief  This structure is for one input bistream, which may contain one tile or multi-tiles
//!
//! \param    m_iViewportWidth,      input,    the width for the viewport
//! \param    m_iViewportHeight,     input,    the height for the viewport
//! \param    m_viewPort_fYaw,       input,    the angle rotated aroud z(-180 ~ 180)
//! \param    m_viewPort_fPitch,     input,    the angle rotated aroud x(-90 ~ 90)
//! \param    m_viewPort_hFOV,       input,    the horizontal FOV angle
//! \param    m_viewPort_vFOV,       input,    the vertical FOV angle
//! \param    m_output_geoType,      input,    the type for the output projection(viewport)
//! \param    m_input_geoType,       input,    the type for the output projection(ERP)
//! \param    m_iInputWidth,         input,    the width of the input
//! \param    m_iInputHeight,        input,    the height of the input
//! \param    m_tileNumRow,          input,    the tile number in row
//! \param    m_tileNumCol,          input,    the tile number in col
//! \param    m_numFaces,            output,   the face number taken by the viewport
//! \param    m_pUpLeft,             output,   the list of the up left point for each face in the input according to the view port information
//! \param    m_pDownRight,          output,   the list of the down right point for each face in the input according to the view port information
//! \param    viewportDestWidth,     output,    the destination width of the viewport
//! \param    viewportDestHeight,    output,    the destination height of the viewport
typedef struct GENERATE_VIEWPORT_PARAM
{
    int32_t m_iViewportWidth;
    int32_t m_iViewportHeight;
    float   m_viewPort_fPitch;
    float   m_viewPort_fYaw;
    float   m_viewPort_hFOV;
    float   m_viewPort_vFOV;
    int32_t m_output_geoType;
    int32_t m_input_geoType;
    int32_t m_iInputWidth;
    int32_t m_iInputHeight;
    uint32_t m_tileNumRow;
    uint32_t m_tileNumCol;
    int32_t m_numFaces;
    point*  m_pUpLeft;
    point*  m_pDownRight;
    int32_t m_viewportDestWidth;
    int32_t m_viewportDestHeight;
    UsageType m_usageType;
    Param_VideoFPStruct m_paramVideoFP;
} generateViewPortParam;

//!
//! \brief    This function mainly do the initialization, pass the input paramters to the genViewport library, malloc the needed memory
//!           and return the handle of the  generateViewPortParam library
//! \param    generateViewPortParam* pParamGenViewport, input, refer to the structure generateViewPortParam
//!
//! \return   void*, this is the handle of the genTiledstream library.
//!           not null, if the initialization is ok
//!           null, if the initialization fails
//!
void* genViewport_Init(generateViewPortParam* pParamGenViewport);

//!
//! \brief    This function completes the viewport range for the input , according to the FOV information.
//!
//! \param    generateViewPortParam* pParamGenViewport, output, refer to the structure generateViewPortParam
//! \param    void*                 pGenHandle,            input, which is created by the genTiledStream_Init function
//!
//! \return   s32, the status of the function.
//!           0,     if succeed
//!           not 0, if fail
//!
int32_t   genViewport_process(generateViewPortParam* pParamGenViewport, void* pGenHandle);

//!
//! \brief    This function completes the viewport selection by look up table , according to the FOV information.
//!
//! \param    generateViewPortParam* pParamGenViewport, output, refer to the structure generateViewPortParam
//! \param    void*                 pGenHandle,            input, which is created by the genTiledStream_Init function
//!
//! \return   s32, the status of the function.
//!           0,     if succeed
//!           not 0, if fail
//!
int32_t   genViewport_postprocess(generateViewPortParam* pParamGenViewport, void* pGenHandle);

//!
//! \brief    This function sets the parameter of the viewPort.
//!
//! \param    void*  pGenHandle,        input, which is created by the genTiledStream_Init function
//! \param    float   yaw,               input, the angle rotated aroud x
//! \param    float   pitch,             input, the angle rotated aroud z
//!
//! \return   s32, the status of the function.
//!           0,     if succeed
//!           not 0, if fail
//!
int32_t genViewport_setViewPort(void* pGenHandle, float yaw, float pitch);

//!
//! \brief    This function sets the maxmimum selected tile number for the viewPort.
//!
//! \param    void*     pGenHandle,        input, which is created by the genTiledStream_Init function
//! \param    int32_t   maxSelTiles,       input, the maxmimum selected tile number
//!
//! \return   s32, the status of the function.
//!           0,     if succeed
//!           not 0, if fail
//!
int32_t genViewport_setMaxSelTiles(void* pGenHandle, int32_t maxSelTiles);

//!
//! \brief    This function judges whether one area(topleft(x,y), width,height, faceid) is inside the viewPort.
//!
//! \param    void*  pGenHandle,     input, which is created by the genTiledStream_Init function
//! \param    int32_t   x,                input, the top left position in the x direction
//! \param    int32_t   y,                input, the top left position in the y direction
//! \param    int32_t   width,            input, the width of the area
//! \param    int32_t   height,           input, the height of the area
//! \param    int32_t   faceId,           input, the faceId, for ERP, only one face; and for cubmap, there are 6 faces.
//!
//! \return   bool, the status of the function.
//!           1,     the input is in the viewport
//!           0,     the input is not in the viewport
//!
bool genViewport_isInside(void* pGenHandle, int32_t x, int32_t y, int32_t width, int32_t height, int32_t faceId);

//!
//! \brief    This function output the fixed number tiles according to the viewPort information in the initialization phase,
//!           especially these tiles are put in the original picture order.
//!           for cube map source, currently support FOV range 100 ~70 degree
//!
//! \param    void*  pGenHandle,     input,  which is created by the genTiledStream_Init function
//! \param    TileDef*   pOutTile,      output, the list for the tiles inside the viewport
//!
//! \return   int32_t, the number of the tiles inside the viewport.
//!
int32_t genViewport_getFixedNumTiles(void* pGenHandle, TileDef* pOutTile);

//!
//! \brief    This function output the selected tiles according to the timely changed viewPort information,
//!           especially these tiles are put in the original picture order.
//!           for cube map source, currently support FOV range 100 ~70 degree
//!
//! \param    void*  pGenHandle,     input,  which is created by the genTiledStream_Init function
//! \param    TileDef*   pOutTile,      output, the list for the tiles inside the viewport
//!
//! \return   int32_t, the number of the tiles inside the viewport.
//!
int32_t genViewport_getTilesInViewport(void* pGenHandle, TileDef* pOutTile);

//!
//! \brief    This function output the fixed number tiles according to the viewPort information in the initialization phase,
//!           especially these tiles are put in the original picture order.
//!           for cube map source, currently support FOV range 100 ~70 degree
//!
//! \param    void*  pGenHandle,     input,  which is created by the genTiledStream_Init function
//! \param    TileDef*   pOutTile,      output, the list for the tiles inside the viewport
//!
//! \return   int32_t, the number of the tiles inside the viewport.
//!
int32_t genViewport_getViewportTiles(void* pGenHandle, TileDef* pOutTile);

int32_t genViewport_getContentCoverage(void* pGenHandle, CCDef* pOutCC);

//!
//! \brief    This function output the selected tiles according to the timely changed viewPort information,
//!           By the legacy tile selection algorithm.
//!           for cube map source, currently support FOV range 100 ~70 degree
//!
//! \param    void*      pGenHandle,     input,  which is created by the genTiledStream_Init function
//! \param    TileDef*   pOutTile,      output, the list for the tiles inside the viewport
//!
//! \return   int32_t, the number of the tiles inside the viewport.
//!
int32_t genViewport_getTilesInViewportByLegacyWay(void* pGenHandle, TileDef* pOutTile);

//!
//! \brief      This function completes the un-initialization, free the memory
//!
//! \param      void*   pGenHandle, input, which is created by the genViewport_Init function
//!
//! \return     int32_t, the status of the function.
//!     0,      if succeed
//!     not 0,  if fail
//!
int32_t   genViewport_unInit(void* pGenHandle);

#ifdef __cplusplus
}
#endif

#endif //_GENVIEWPORT_API_H_
