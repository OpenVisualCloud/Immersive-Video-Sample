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

#include <assert.h>
#include <math.h>
#include "360SCVPEquiRect.h"

/********************************************
Equirectangular geometry related functions;
*********************************************/
EquiRect::EquiRect(SVideoInfo& sVideoInfo) : Geometry()
{
    geoInit(sVideoInfo);
}

EquiRect::~EquiRect()
{
}

/**************************************
    -180                         180
90                                   0
                                     |
                                     v
                                     |
-90                                  1
    0 ----------u-----------------1
***************************************/
void EquiRect::map2DTo3D(SPos& IPosIn, SPos *pSPosOut)
{
    POSType u, v;
    //u = IPosIn.x;
    u = IPosIn.x + (POSType)(0.5);
    v = IPosIn.y + (POSType)(0.5);
    if ((u < 0 || u >= m_sVideoInfo.iFaceWidth) && ( v >= 0 && v < m_sVideoInfo.iFaceHeight)) 
    {
        u = u < 0 ? m_sVideoInfo.iFaceWidth+u : (u - m_sVideoInfo.iFaceWidth);
    }
    else if (v < 0)
    {
        v = -v;
        u = u + (m_sVideoInfo.iFaceWidth>>1);
        u = u >= m_sVideoInfo.iFaceWidth ? u - m_sVideoInfo.iFaceWidth : u;
    }
    else if(v >= m_sVideoInfo.iFaceHeight)
    {
        v = (m_sVideoInfo.iFaceHeight<<1)-v;
        u = u + (m_sVideoInfo.iFaceWidth>>1);
        u = u >= m_sVideoInfo.iFaceWidth ? u - m_sVideoInfo.iFaceWidth : u;
    }

    POSType yaw, pitch;
    pSPosOut->faceIdx =IPosIn.faceIdx;

    yaw = (POSType)(u*S_PI*2/m_sVideoInfo.iFaceWidth - S_PI);
    pitch = (POSType)(S_PI_2 - v*S_PI/m_sVideoInfo.iFaceHeight);
    pSPosOut->x = (POSType)(scos(pitch)*scos(yaw));
    pSPosOut->y = (POSType)(ssin(pitch));
    pSPosOut->z = -(POSType)(scos(pitch)*ssin(yaw));
}

//The output is within [0.0, width)*[0.0, height) in sampling grid;
void EquiRect::map3DTo2D(SPos *pSPosIn, SPos *pSPosOut)
{
    POSType x = pSPosIn->x;
    POSType y = pSPosIn->y;
    POSType z = pSPosIn->z;

    pSPosOut->faceIdx = 0;
    pSPosOut->z = 0;
    //yaw;
    pSPosOut->x = (POSType)((S_PI-satan2(z, x))*m_sVideoInfo.iFaceWidth/(2*S_PI));
    pSPosOut->x -= 0.5;
    POSType len = ssqrt(x*x + y*y + z*z);

    //pitch;
    pSPosOut->y = (POSType)((len < S_EPS? 0.5 : sacos(y/len)/S_PI)*m_sVideoInfo.iFaceHeight);
    pSPosOut->y -= 0.5;
}
