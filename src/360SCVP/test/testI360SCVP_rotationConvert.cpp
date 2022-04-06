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
 */

#include "gtest/gtest.h"
#include <string>
#include <fstream>
#include "../360SCVPAPI.h"
#include <math.h>

#include "../../utils/safe_mem.h"

//#define XYZ_ORDER

namespace{
class I360SCVPTest_rotationConvert : public testing::Test {
public:

    virtual void SetUp()
    {
      memset(&param, 0, sizeof(param_360SCVP));
    }
    virtual void TearDown()
    {
    }

    param_360SCVP           param;
};

TEST_F(I360SCVPTest_rotationConvert, Euler)
{
    float matrixR[3][3]={ 0.962107,-0.005824,0.272486,0.004023,0.999964,0.007166,-0.272519,-0.005795,0.962095 };
    float matrixTest[3][3]={ 0 };

    EulerAngle* angle;
    angle = (EulerAngle*)malloc(sizeof(EulerAngle));
    EXPECT_TRUE( angle != NULL);
    if (!(angle))
    {
        return;
    }
    memset(angle, 0, sizeof(EulerAngle));

    EulerAngle* angleTest;
    angleTest = (EulerAngle*)malloc(sizeof(EulerAngle));
    EXPECT_TRUE( angleTest != NULL);
    if (!(angleTest))
    {
        free (angle);
        angle = NULL;
        return;
    }
    memset(angleTest, 0, sizeof(EulerAngle));

    angle->roll = -0.0060249;
    angle->pitch = 0.2760151;
    angle->yaw = 0.0041818;
#ifdef XYZ_ORDER
    angle->roll = -0.0074463;
    angle->pitch = 0.2759813;
    angle->yaw = 0.0060527;
#endif

    int ret = 0;

    param.usedType = E_PARSER_ONENAL;

    void* pI360SCVP = I360SCVP_Init(&param);
    EXPECT_TRUE(pI360SCVP != NULL);
    if (!pI360SCVP)
    {
        free (angle);
        angle = NULL;
        free (angleTest);
        angleTest = NULL;
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    I360SCVP_Matrix2Euler(pI360SCVP, matrixR, angleTest);

    if((abs(angleTest->roll-angle->roll)>1e-4)||
       (abs(angleTest->pitch-angle->pitch)>1e-4)||
       (abs(angleTest->yaw-angle->yaw)>1e-4))
       {
         printf("Matrix2Euler Error\n");
         ret = 1;
       }

    I360SCVP_Euler2Matrix(pI360SCVP, angle, matrixTest);

    for(int i = 0; i < 3; i++) {
      for(int j = 0; j < 3; j++) {
        if(abs(matrixTest[i][j]-matrixR[i][j])>1e-4){
          printf("Euler2Matrix[%d][%d] Error\n", i, j);
         ret = 1;
        }
      }
    }

    free (angle);
    angle = NULL;
    free (angleTest);
    angleTest = NULL;
    I360SCVP_unInit(pI360SCVP);
    EXPECT_TRUE(ret == 0);
}

TEST_F(I360SCVPTest_rotationConvert, Quaternion)
{
    float matrixR[3][3]={ 0.962107,-0.005824,0.272486,0.004023,0.999964,0.007166,-0.272519,-0.005795,0.962095 };
    float matrixTest[3][3]={ 0 };

    Quaternion* quaternion;
    quaternion = (Quaternion*)malloc(sizeof(Quaternion));
    EXPECT_TRUE( quaternion != NULL);
    if (!(quaternion))
    {
        return;
    }
    memset(quaternion, 0, sizeof(Quaternion));

    Quaternion* quaternionTest;
    quaternionTest = (Quaternion*)malloc(sizeof(Quaternion));
    EXPECT_TRUE( quaternionTest != NULL);
    if (!(quaternionTest))
    {
        free (quaternion);
        quaternion = NULL;
        return;
    }
    memset(quaternionTest, 0, sizeof(Quaternion));

    quaternion->x = -0.0032714;
    quaternion->y = 0.1375627;
    quaternion->z = 0.0024854;
    quaternion->w = 0.9904845;

    int ret = 0;

    param.usedType = E_PARSER_ONENAL;

    void* pI360SCVP = I360SCVP_Init(&param);
    EXPECT_TRUE(pI360SCVP != NULL);
    if (!pI360SCVP)
    {
        free (quaternion);
        quaternion = NULL;
        free (quaternionTest);
        quaternionTest = NULL;
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    I360SCVP_Matrix2Quaternion(pI360SCVP, matrixR, quaternionTest);

    if((abs(quaternionTest->x-quaternion->x)>1e-4)||
       (abs(quaternionTest->y-quaternion->y)>1e-4)||
       (abs(quaternionTest->z-quaternion->z)>1e-4)||
       (abs(quaternionTest->w-quaternion->w)>1e-4))
       {
         printf("Matrix2Quaternion Error\n");
         ret = 1;
       }

    I360SCVP_Quaternion2Matrix(pI360SCVP, quaternion, matrixTest);

    for(int i = 0; i < 3; i++) {
      for(int j = 0; j < 3; j++) {
        if(abs(matrixTest[i][j]-matrixR[i][j])>1e-4){
          printf("Quaternion2Matrix[%d][%d] Error\n", i, j);
         ret = 1;
        }
      }
    }

    free (quaternion);
    quaternion = NULL;
    free (quaternionTest);
    quaternionTest = NULL;
    I360SCVP_unInit(pI360SCVP);
    EXPECT_TRUE(ret == 0);
}
}
