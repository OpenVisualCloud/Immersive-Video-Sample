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

namespace{
class I360SCVPTest_novelview : public testing::Test {
public:
    virtual void SetUp()
    {
      memset(&param, 0, sizeof(param_360SCVP));

      pOutputSEI = new unsigned char[1000];
      if(!pOutputSEI) return;
      param.pOutputSEI = pOutputSEI;
      param.outputSEILen = 0;
    }
    virtual void TearDown()
    {
      if(pOutputSEI)
        delete pOutputSEI;
      pOutputSEI = NULL;
    }

    param_360SCVP           param;
    unsigned char*          pOutputSEI;
};

TEST_F(I360SCVPTest_novelview, parseSEI)
{
    NovelViewSEI* sei_wr;
    sei_wr = (NovelViewSEI*)malloc(sizeof(NovelViewSEI));
    EXPECT_TRUE( sei_wr != NULL);
    if (!(sei_wr))
    {
        return;
    }
    memset(sei_wr, 0, sizeof(NovelViewSEI));

    int ret = 0;

    param.usedType = E_PARSER_ONENAL;

    param.novelViewSEI.cameraID_x = 2;
    param.novelViewSEI.cameraID_y = 3;

    param.novelViewSEI.focal_x = 484.638336;
    param.novelViewSEI.focal_y = 484.546051;
    param.novelViewSEI.center_x = 511.097839;
    param.novelViewSEI.center_y = 294.979370;

    param.novelViewSEI.codx = 0.0;
    param.novelViewSEI.cody = 0.0;

    param.novelViewSEI.roll = 2.757372;
    param.novelViewSEI.pitch = -0.269413;
    param.novelViewSEI.yaw = 0.011317;
    param.novelViewSEI.trans_x = -0.0298822;
    param.novelViewSEI.trans_y = -0.448803;
    param.novelViewSEI.trans_z = -1.19919;

    param.novelViewSEI.k1 = 0.537714303;
    param.novelViewSEI.k2 = -2.64278436;
    param.novelViewSEI.k3 = 1.53610420;
    param.novelViewSEI.k4 = 0.418025941;
    param.novelViewSEI.k5 = -2.47357488;
    param.novelViewSEI.k6 = 1.46605146;
    param.novelViewSEI.p1 = 0.00105964323;
    param.novelViewSEI.p2 = 5.81518434e-05;
    param.novelViewSEI.metric_radius = 4.56;

    void* pI360SCVP = I360SCVP_Init(&param);
    EXPECT_TRUE(pI360SCVP != NULL);
    if (!pI360SCVP)
    {
        free (sei_wr);
        sei_wr = NULL;
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    I360SCVP_GenerateNovelViewSEI(pI360SCVP,&param.novelViewSEI,param.pOutputSEI,&param.outputSEILen);

    ret = I360SCVP_ParseNovelViewSEI(pI360SCVP,sei_wr, param.pOutputSEI, param.outputSEILen);
    EXPECT_TRUE(ret == 0);
    if (ret)
    {
        free (sei_wr);
        sei_wr = NULL;
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    if((sei_wr->cameraID_x-param.novelViewSEI.cameraID_x>1e-5)||
       (sei_wr->cameraID_y-param.novelViewSEI.cameraID_y>1e-5)||
       (abs(sei_wr->focal_x-param.novelViewSEI.focal_x)>1e-4)||
       (abs(sei_wr->focal_y-param.novelViewSEI.focal_y)>1e-4)||
       (abs(sei_wr->center_x-param.novelViewSEI.center_x)>1e-5)||
       (abs(sei_wr->center_y-param.novelViewSEI.center_y)>1e-5)||

       (abs(sei_wr->roll-param.novelViewSEI.roll)>1e-5)||
       (abs(sei_wr->pitch-param.novelViewSEI.pitch)>1e-5)||
       (abs(sei_wr->yaw-param.novelViewSEI.yaw)>1e-5)||
       (abs(sei_wr->trans_x-param.novelViewSEI.trans_x)>1e-5)||
       (abs(sei_wr->trans_y-param.novelViewSEI.trans_y)>1e-5)||
       (abs(sei_wr->trans_z-param.novelViewSEI.trans_z)>1e-5)||

       (abs(sei_wr->codx-param.novelViewSEI.codx)>1e-5)||
       (abs(sei_wr->cody-param.novelViewSEI.cody)>1e-5)||
       (abs(sei_wr->k1-param.novelViewSEI.k1)>1e-5)||
       (abs(sei_wr->k2-param.novelViewSEI.k2)>1e-5)||
       (abs(sei_wr->k3-param.novelViewSEI.k3)>1e-5)||
       (abs(sei_wr->k4-param.novelViewSEI.k4)>1e-5)||
       (abs(sei_wr->k5-param.novelViewSEI.k5)>1e-5)||
       (abs(sei_wr->k6-param.novelViewSEI.k6)>1e-5)||

       (abs(sei_wr->p1-param.novelViewSEI.p1)>1e-5)||
       (abs(sei_wr->p2-param.novelViewSEI.p2)>1e-5)||
       (abs(sei_wr->metric_radius-param.novelViewSEI.metric_radius)>1e-5))
       ret = 1;


    free (sei_wr);
    sei_wr = NULL;
    I360SCVP_unInit(pI360SCVP);
    EXPECT_TRUE(ret == 0);
}
}
