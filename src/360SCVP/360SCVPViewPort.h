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

#ifndef __360SCVP_VIEWPORT__
#define __360SCVP_VIEWPORT__
#include "360SCVPGeometry.h"

#define FACE_NUMBER 6
#define ERP_HORZ_ANGLE 360
#define ERP_VERT_ANGLE 180
#define ERP_HORZ_START -180
#define ERP_VERT_START 90
#define PI_IN_DEGREE 180
#define HALF_PI_IN_DEGREE 90
#define DEG2RAD_FACTOR (S_PI/PI_IN_DEGREE)
#define RAD2DEG_FACTOR (PI_IN_DEGREE/S_PI)
#define HORZ_BOUNDING_STEP 5
#define VERT_BOUNDING_STEP 5

// ====================================================================================================================
// Class definition
// ====================================================================================================================

class ViewPort : public Geometry
{
private:
    POSType m_matRotMatx[3][3];
    POSType m_matInvK[3][3];

public:
    ViewPort(SVideoInfo& sVideoInfo);
    virtual ~ViewPort();
    virtual void map2DTo3D(SPos& IPosIn, SPos *pSPosOut);
    //own methods;
    virtual void map3DTo2D(SPos *pSPosIn, SPos *pSPosOut);
    void setViewPort(float, float, float, float);
    void setRotMat();
    void setInvK();
    void matInv(POSType[3][3]);
};

#endif // __T360SCVP_GEOMETRY__

