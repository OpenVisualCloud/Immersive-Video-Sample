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
#ifndef __360SCVP_VIEWPORTIMPL__
#define __360SCVP_VIEWPORTIMPL__

#include "360SCVPAPI.h"
#include "360SCVPGeometry.h"

#include <sstream>
#include <vector>

///< for cubemap, given the facesize (960x960), the maxsimum viewport size is defined in the below table
typedef struct SIZE_DEF
{
    int32_t x;
    int32_t y;
}SIZE;

#define MAX_FOV_ANGLE 100;
#define NORMAL_FACE_SIZE 960;
#define NORMAL_PITCH_MIN -75
#define NORMAL_PITCH_MAX -15

enum FOVAngle
{
    FOV_100 = 0,
    FOV_90,
    FOV_80,
    FOV_70,
    FOV_Angle_NUM
};

SIZE Max_Viewport_Size[FOV_Angle_NUM][4] =
{
    {{478, 959}, {423,  355}, {473,  356}, {564,  959}},
    {{350, 959}, {349,  201}, {349,  202}, {607,  959}},
    {{479, 959}, {134,  161}, {134,  160}, {394,  959}},
    {{351, 946}, {437,  944}, {0,    0  }, {0,    0  }}
};
// ====================================================================================================================
// Class definition,
// ====================================================================================================================
struct ITileInfo
{
    short x;
    short y;
    short tilewidth;
    short tileheight;
    int32_t   faceId;
    uint32_t  isOccupy;
    float vertPos;
    float horzPos;
};
/// generate viewport class
class TgenViewport
{
public:
  //tile info
    uint32_t m_tileNumRow;
    uint32_t m_tileNumCol;
    ITileInfo *m_srd;
    int32_t m_numFaces;
    SPos *m_pUpLeft;
    SPos *m_pDownRight;
    // source specification
    int32_t       m_iFrameRate;                                     ///< source frame-rates (Hz)
    int32_t       m_iSourceWidth;                                   ///< source width in pixel
    int32_t       m_iSourceHeight;                                  ///< source height in pixel (when interlaced = field height)
    int32_t       m_iInputWidth;
    int32_t       m_iInputHeight;
    SVideoInfo m_sourceSVideoInfo;
    SVideoInfo m_codingSVideoInfo;
    int32_t       m_iCodingFaceWidth;
    int32_t       m_iCodingFaceHeight;
    int32_t       m_aiPad[2];                                       ///< number of padded pixels for width and height
    int32_t   m_faceSizeAlignment;
    int32_t       m_maxTileNum;
    UsageType     m_usageType;
    inline int32_t round(POSType t) { return (int32_t)(t+ (t>=0? 0.5 :-0.5)); }
public:
    TgenViewport();
    virtual ~TgenViewport();
    TgenViewport& operator=(const TgenViewport& src);

public:
    int32_t  create(uint32_t tileNumRow, uint32_t tileNumCol);  ///< create option handling class
    void     destroy();    ///< destroy option handling class
    int32_t  parseCfg(  );  ///< parse configuration file to fill member variables
    int32_t  convert();
    int32_t  selectregion(short inputWidth, short inputHeight, short dstWidth, short dstHeight);
    //analysis;
    bool     isInside(int32_t x, int32_t y, int32_t width, int32_t height, int32_t faceId);
    int32_t  calcTilesInViewport(ITileInfo* pTileInfo, int32_t tileCol, int32_t tileRow);

};// END CLASS DEFINITION

//! \}
#endif // __360SCVP_VIEWPORTIMPL__
