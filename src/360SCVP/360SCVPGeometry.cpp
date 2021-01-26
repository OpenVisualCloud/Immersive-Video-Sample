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

/** \file     360SCVPGeometry.cpp
    \brief    Geometry class
*/

#include <assert.h>
#include <math.h>
#include <string.h>
#include "360SCVPGeometry.h"
#include "360SCVPEquiRect.h"
#include "360SCVPCubeMap.h"
#include "360SCVPViewPort.h"

Geometry::Geometry()
{
    memset_s(&m_sVideoInfo, sizeof(struct SVideoInfo), 0);
    m_iMarginX = m_iMarginY =0;

    m_bPadded = false;
    m_bGeometryMapping = false;
    m_bConvOutputPaddingNeeded = false;
    m_numFaces = 0;
    m_upLeft = nullptr;
    m_downRight = nullptr;
}

void Geometry::geoInit(SVideoInfo& sVideoInfo)
{
    if(sVideoInfo.geoType==SVIDEO_VIEWPORT)
    {
        m_numFaces = 0;
        m_upLeft = new SPos[FACE_NUMBER];//
        m_downRight = new SPos[FACE_NUMBER];//
        SPos* pUpleftTmp = m_upLeft;
        SPos* pDownRightTmp = m_downRight;
        for (int32_t i = 0; i < FACE_NUMBER; i++)
        {
            pUpleftTmp->faceIdx = -1;
            pUpleftTmp->x = sVideoInfo.fullWidth;
            pUpleftTmp->y = sVideoInfo.fullHeight;
            pDownRightTmp->faceIdx = -1;
            pDownRightTmp->x = 0;
            pDownRightTmp->y = 0;
            pUpleftTmp++;
            pDownRightTmp++;
        }
    }
    m_sVideoInfo = sVideoInfo;
    m_bPadded = false;
    m_iMarginX = m_iMarginY = S_PAD_MAX;
}

void Geometry::geoUnInit()
{
    SAFE_DELETE_ARRAY(m_upLeft);
    SAFE_DELETE_ARRAY(m_downRight);

}

Geometry::~Geometry()
{
}

Geometry* Geometry::create(SVideoInfo& sVideoInfo)
{
    Geometry *pRet = NULL;
    if (sVideoInfo.geoType == SVIDEO_EQUIRECT)
        pRet = new EquiRect(sVideoInfo);
    if (sVideoInfo.geoType == SVIDEO_CUBEMAP)
        pRet = new CubeMap(sVideoInfo);
    else if (sVideoInfo.geoType == SVIDEO_VIEWPORT)
        pRet = new ViewPort(sVideoInfo);
    return pRet;
}

void Geometry::geometryMapping(Geometry *pGeoSrc)
{
    assert(!m_bGeometryMapping);
    int32_t *pRot = m_sVideoInfo.sVideoRotation.degree;

    //For ViewPort, Set Rotation Matrix and K matrix
    if (m_sVideoInfo.geoType==SVIDEO_VIEWPORT)
    {
      ((ViewPort*)this)->setRotMat();
      ((ViewPort*)this)->setInvK();
    }
    //generate the map;
    for(int32_t fIdx=0; fIdx<m_sVideoInfo.iNumFaces; fIdx++)
    {
      for(int32_t ch=0; ch<1; ch++)//iNumMaps
      {
          int32_t iWidth = m_sVideoInfo.iFaceWidth;
          int32_t iHeight = m_sVideoInfo.iFaceHeight;
          int32_t nMarginX = m_iMarginX;
          int32_t nMarginY = m_iMarginY;
          int32_t nNextAreaX = iWidth + nMarginX;
          for (int32_t j = -nMarginY; j < iHeight + nMarginY; j++)
          {
              for (int32_t i = -nMarginX; i < iWidth + nMarginX; i++)
              {
                  if (!m_bConvOutputPaddingNeeded && !insideFace((i), (j)))
                      continue;

                  int32_t xOrg = (i + nMarginX);
                  int32_t ic = i;
                  int32_t jc = j;
                  {
                      POSType x = (ic);
                      POSType y = (jc);
                      SPos in(fIdx, x, y, 0), pos3D;
                      map2DTo3D(in, &pos3D);

                      rotate3D(pos3D, pRot[0], pRot[1], pRot[2]);
                      pGeoSrc->map3DTo2D(&pos3D, &pos3D);
                      if (m_sVideoInfo.geoType == SVIDEO_VIEWPORT)
                      {
                          SPos *pUpLeftTmp = m_upLeft + pos3D.faceIdx;
                          SPos *pDownRightTmp = m_downRight + pos3D.faceIdx;
                          int32_t yTmp = (int32_t)pos3D.y;//(int32_t)(pos / stride);
                          int32_t xTmp = (int32_t)pos3D.x;//(int32_t)(pos % stride);
                                                          //if the input is the erp, should consider the boundary case
                          if (xTmp == 0 && xOrg != 16 && pGeoSrc->getType() == SVIDEO_EQUIRECT)
                          {
                              nNextAreaX = i;
                              break;
                          }
                          if (pUpLeftTmp->x > xTmp)
                              pUpLeftTmp->x = xTmp;
                          if (pUpLeftTmp->y > yTmp)
                              pUpLeftTmp->y = yTmp;
                          if (pDownRightTmp->x < xTmp)
                              pDownRightTmp->x = xTmp;
                          if (pDownRightTmp->y < yTmp)
                              pDownRightTmp->y = yTmp;
                          pUpLeftTmp->faceIdx = pos3D.faceIdx;
                          pDownRightTmp->faceIdx = pos3D.faceIdx;
                      }
                  }
              }
          }

          // judge if exiting boundary when the source is erp format
          if (nNextAreaX != (iWidth + nMarginX) && (pGeoSrc->getType() == SVIDEO_EQUIRECT))
          {
              for (int32_t j = -nMarginY; j < iHeight + nMarginY; j++)
              {
                  for (int32_t i = nNextAreaX; i < iWidth + nMarginX; i++)
                  {
                      if (!m_bConvOutputPaddingNeeded && !insideFace((i), (j)))
                          continue;
                      int32_t ic = i;
                      int32_t jc = j;
                      {
                          POSType x = (ic);
                          POSType y = (jc);
                          SPos in(fIdx, x, y, 0), pos3D;

                          map2DTo3D(in, &pos3D);
                          rotate3D(pos3D, pRot[0], pRot[1], pRot[2]);
                          pGeoSrc->map3DTo2D(&pos3D, &pos3D);
                          if (m_sVideoInfo.geoType == SVIDEO_VIEWPORT)
                          {
                              SPos *pUpLeftTmp = m_upLeft + 1;
                              SPos *pDownRightTmp = m_downRight + 1;
                              int32_t yTmp = (int32_t)pos3D.y;//(int32_t)(pos / stride);
                              int32_t xTmp = (int32_t)pos3D.x;//(int32_t)(pos % stride);
                              if (pUpLeftTmp->x > xTmp)
                                  pUpLeftTmp->x = xTmp;
                              if (pUpLeftTmp->y > yTmp)
                                  pUpLeftTmp->y = yTmp;
                              if (pDownRightTmp->x < xTmp)
                                  pDownRightTmp->x = xTmp;
                              if (pDownRightTmp->y < yTmp)
                                  pDownRightTmp->y = yTmp;
                              pUpLeftTmp->faceIdx = pos3D.faceIdx;
                              pDownRightTmp->faceIdx = pos3D.faceIdx;
                          }
                      }
                  }
              }
          }
      }
    }

    if (m_sVideoInfo.geoType == SVIDEO_VIEWPORT)
    {
        SPos *pUpLeftTmp = m_upLeft;
        for (int32_t i = 0; i < FACE_NUMBER; i++)
        {
            if (pUpLeftTmp->faceIdx >= 0)
                m_numFaces++;
            pUpLeftTmp++;
        }
    }
    m_bGeometryMapping = true;
}

/***************************************************
//convert source geometry to destination geometry;
****************************************************/
void Geometry::geoConvert(Geometry *pGeoDst)
{
    if(!pGeoDst->m_bGeometryMapping)
    pGeoDst->geometryMapping(this);
    return;
}

void Geometry::rotate3D(SPos& sPos, int32_t rx, int32_t ry, int32_t rz)
{
    POSType x = sPos.x;
    POSType y = sPos.y;
    POSType z = sPos.z;
    if(rx)
    {
        POSType rcos = scos((POSType)(rx*S_PI/180.0));
        POSType rsin = ssin((POSType)(rx*S_PI/180.0));
        POSType t1 = rcos*y - rsin*z;
        POSType t2 = rsin*y + rcos*z;
        y = t1;
        z = t2;
    }
    if(ry)
    {
        POSType rcos = scos((POSType)(ry*S_PI/180.0));
        POSType rsin = ssin((POSType)(ry*S_PI/180.0));
        POSType t1 = rcos*x + rsin*z;
        POSType t2 = -rsin*x + rcos*z;
        x = t1;
        z = t2;
    }
    if(rz)
    {
        POSType rcos = scos((POSType)(rz*S_PI/180.0));
        POSType rsin = ssin((POSType)(rz*S_PI/180.0));
        POSType t1 = rcos*x - rsin*y;
        POSType t2 = rsin*x + rcos*y;
        x = t1;
        y = t2;
    }
    sPos.x = x;
    sPos.y = y;
    sPos.z = z;
}
