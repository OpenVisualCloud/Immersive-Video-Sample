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

/** \file     Geometry.h
    \brief    Geometry class (header)
*/

#ifndef __360SCVP_GEOMETRY__
#define __360SCVP_GEOMETRY__
#include <math.h>
#include <stdint.h>
#include "360SCVPCommonDef.h"
// ====================================================================================================================
// Class definition
// ====================================================================================================================

#define scos(x)         cos((double)(x))
#define ssin(x)         sin((double)(x))
#define satan(x)        atan((double)(x))
#define satan2(y, x)    atan2((double)(y), (double)(x))
#define sacos(x)        acos((double)(x))
#define sasin(x)        asin((double)(x))
#define ssqrt(x)        sqrt((double)(x))
#define sfloor(x)       floor((double)(x))
#define sfabs(x)        fabs((double)(x))
#define stan(x)         tan((double)(x))


typedef double          POSType;
static const double S_PI = 3.14159265358979323846;
static const double S_PI_2 = 1.57079632679489661923;
static const double S_EPS = 1.0e-6;
static const int32_t    S_PAD_MAX = 16;
static const POSType S_ICOSA_GOLDEN = ((ssqrt(5.0)+1.0)/2.0);

struct SPos
{
    int32_t   faceIdx;
    POSType x;
    POSType y;
    POSType z;
    SPos() : faceIdx(0), x(0), y(0), z(0) {}
    SPos(int32_t f, POSType xIn, POSType yIn, POSType zIn ) : faceIdx(f), x(xIn), y(yIn), z(zIn) {}
};

struct GeometryRotation
{
    int32_t degree[3];  //[x/y/z];
};
struct ViewPortSettings
{
    float hFOV;
    float vFOV;
    float fYaw;              //
    float fPitch;
    ViewPortSettings() : hFOV(0), vFOV(0), fYaw(0), fPitch(0) {}
};

struct SVideoInfo
{
    int32_t geoType;
    GeometryRotation sVideoRotation;

    int32_t iFaceWidth;          //native size
    int32_t iFaceHeight;         //native size
    int32_t iNumFaces;          //geometry faces
    ViewPortSettings viewPort;
    int32_t fullWidth;
    int32_t fullHeight;
};

class Geometry
{
protected:
    SVideoInfo m_sVideoInfo;
    int32_t  m_iMarginX;
    int32_t  m_iMarginY;
    bool m_bPadded;
    bool m_bGeometryMapping;
    bool m_bConvOutputPaddingNeeded;
    inline int32_t round(POSType t) { return (int32_t)(t+ (t>=0? 0.5 :-0.5)); }
    void rotate3D(SPos& sPos, int32_t rx, int32_t ry, int32_t rz);
public:
    int32_t m_numFaces;
    SPos* m_upLeft;
    SPos* m_downRight;
    Geometry();
    virtual ~Geometry();
    void geoInit(SVideoInfo& sVideoInfo);
    void geoUnInit(); // just use in the viewport
    GeometryType getType() { return (GeometryType)m_sVideoInfo.geoType; }
    void setPaddingFlag(bool bFlag) { m_bPadded = bFlag; }
    virtual void map2DTo3D(SPos& IPosIn, SPos *pSPosOut) = 0;
    virtual void map3DTo2D(SPos *pSPosIn, SPos *pSPosOut) = 0;
    virtual void geoConvert(Geometry *pGeoDst);
    virtual bool insideFace(int32_t x, int32_t y) { return ( x>=0 && x<(m_sVideoInfo.iFaceWidth) && y>=0 && y<(m_sVideoInfo.iFaceHeight) ); }
    virtual void geometryMapping(Geometry *pGeoSrc);
    static Geometry* create(SVideoInfo& sVideoInfo);
 };

#endif // __360SCVP_TGEOMETRY__

