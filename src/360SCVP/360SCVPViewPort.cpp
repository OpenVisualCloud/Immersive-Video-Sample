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

/** \file     360SCVPViewPort.cpp
    \brief    ViewPort class
*/

#include <assert.h>
#include <math.h>
#include "360SCVPViewPort.h"


ViewPort::ViewPort(SVideoInfo& sVideoInfo) : Geometry()
{
    geoInit(sVideoInfo);
}

ViewPort::~ViewPort()
{
}


void ViewPort::map2DTo3D(SPos& IPosIn, SPos *pSPosOut)
{
  POSType u= IPosIn.x+(POSType)(0.5);
  POSType v= IPosIn.y+(POSType)(0.5);
  POSType x2 = m_matInvK[0][0]*u + m_matInvK[0][1]*v + m_matInvK[0][2];
  POSType y2 = m_matInvK[1][0]*u + m_matInvK[1][1]*v + m_matInvK[1][2];

  // undo perspective division
  POSType z1 = 1/ssqrt(x2*x2+y2*y2+1);
  POSType x1 = z1*x2;
  POSType y1 = z1*y2;

  POSType p1[3] = {x1, y1, z1};

  // rotate: p = R * p1
  pSPosOut->x = m_matRotMatx[0][0]*p1[0] + m_matRotMatx[0][1]*p1[1] + m_matRotMatx[0][2]*p1[2];
  pSPosOut->y = m_matRotMatx[1][0]*p1[0] + m_matRotMatx[1][1]*p1[1] + m_matRotMatx[1][2]*p1[2];
  pSPosOut->z = m_matRotMatx[2][0]*p1[0] + m_matRotMatx[2][1]*p1[1] + m_matRotMatx[2][2]*p1[2];

}

void ViewPort::setViewPort(float fovx,float fovy,float yaw,float pitch)
{
   m_sVideoInfo.viewPort.hFOV= fovx;
   m_sVideoInfo.viewPort.vFOV= fovy;
   m_sVideoInfo.viewPort.fYaw= yaw;
   m_sVideoInfo.viewPort.fPitch= pitch;
   m_bGeometryMapping=false;
}
void ViewPort::setRotMat()
{
  //phi==lattitude; th=longitude
   POSType tht = (m_sVideoInfo.viewPort.fYaw+90);
   POSType phi = -m_sVideoInfo.viewPort.fPitch;
   tht = (POSType)(tht*S_PI/180);
   phi = (POSType)(phi*S_PI/180);
   m_matRotMatx[0][0] =  scos(tht);    m_matRotMatx[0][1] = ssin(tht)*ssin(phi);    m_matRotMatx[0][2] = ssin(tht)*scos(phi);
   m_matRotMatx[1][0] =       0.0f;    m_matRotMatx[1][1] =           scos(phi);    m_matRotMatx[1][2] =          -ssin(phi);
   m_matRotMatx[2][0] = -ssin(tht);    m_matRotMatx[2][1] = scos(tht)*ssin(phi);    m_matRotMatx[2][2] = scos(tht)*scos(phi);
}
void ViewPort::setInvK()
{
  POSType fovx = (POSType)(S_PI * (m_sVideoInfo.viewPort.hFOV)/180.0);
  POSType fovy = (POSType)(S_PI * (m_sVideoInfo.viewPort.vFOV)/180.0);

  POSType fx=(m_sVideoInfo.iFaceWidth/2)*(1/stan(fovx/2));
  POSType fy=(m_sVideoInfo.iFaceHeight/2)*(1/stan(fovy/2));

  POSType K[3][3]={{fx,0,m_sVideoInfo.iFaceWidth/2.0f},{0,-fy,m_sVideoInfo.iFaceHeight/2.0f},{0,0,1}};
  matInv(K);

}

void ViewPort::matInv(POSType K[3][3])
{
  // Optimised for current K matrix structure;
  POSType det=K[0][0]*K[1][1];
  m_matInvK[0][0] = (K[1][1] * K[2][2] - K[2][1] * K[1][2]) /det;
  m_matInvK[0][1] = (K[0][2] * K[2][1] - K[0][1] * K[2][2]) /det;
  m_matInvK[0][2] = (K[0][1] * K[1][2] - K[0][2] * K[1][1]) /det;
  m_matInvK[1][0] = (K[1][2] * K[2][0] - K[1][0] * K[2][2]) /det;
  m_matInvK[1][1] = (K[0][0] * K[2][2] - K[0][2] * K[2][0]) /det;
  m_matInvK[1][2] = (K[1][0] * K[0][2] - K[0][0] * K[1][2]) /det;
  m_matInvK[2][0] = (K[1][0] * K[2][1] - K[2][0] * K[1][1]) /det;
  m_matInvK[2][1] = (K[2][0] * K[0][1] - K[0][0] * K[2][1]) /det;
  m_matInvK[2][2] = (K[0][0] * K[1][1] - K[1][0] * K[0][1]) /det;
}
void ViewPort::map3DTo2D(SPos *,SPos *)
{
    assert(0 && "Viewport 3D to 2D is not supported ");
}

