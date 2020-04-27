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

//!

//! \file:   ViewportPredict_LR.cpp
//! \brief:  the class for viewport predict of linear regression.
//! \detail: it's class to predict viewport by linear regression.
//!
//! Created on April 2, 2020, 1:18 PM
//!


#include "ViewportPredict_LR.h"
#include <cmath>

VCD_OMAF_BEGIN

#define MAX_POSE_LIST_SIZE 50

float coef[4][10] = {-5.717383840296463848e-01, 3.705255734543102530e-01, 5.716572006233263670e-01, -3.944310273491463681e-02, -4.832487263440977676e-01, -4.418513020258272306e-01, -1.289878993569788523e+00, -1.353452610038038184e+00, -3.385166623810006992e-01, 4.480950098001700965e+00, \
                     -3.868609386457786403e-01, 2.138918222791050816e-01, 3.721943871177658680e-01, -8.630072895813600820e-02, -4.651027735858629386e-01, -3.093338045202858044e-01, -9.637826502252642147e-01, -1.064032550702946889e+00, -3.844128780707756210e-01, 3.907968845116745360e+00, \
                     -5.665274064500415430e-02, 8.696585858711358696e-02, 1.167398365825514928e-01, -1.538006281042666457e-01, -2.168047731336137651e-01, -2.304593204324957567e-01, -7.288238106554234541e-01, -8.832580502280420465e-01, -4.029940226638547007e-01, 3.207586792184790703e+00, \
                     -1.564989792043467720e-01, 6.810824709374668773e-02, 1.751593122220721499e-01, -1.495233839082101834e-01, -2.373940873076129110e-01, -2.369707848752707624e-01, -7.359318135242699510e-01, -8.197740423063037962e-01, -4.323319782309776871e-01, 3.343831371804256491e+00};
float intercept[4] = {-3.699505400208219497e-02, -1.511185971911371828e-02, 2.460604843495132199e-01, -1.663855805280387012e-02};

ViewportPredict_LR::ViewportPredict_LR()
{
}
ViewportPredict_LR::~ViewportPredict_LR() {}
ViewportAngle* ViewportPredict_LR::PredictPose(std::list<ViewportAngle> pose_history)
{
    ViewportAngle *predicted_angle = NULL;
    if (pose_history.empty())
    {
        return predicted_angle;
    }
    predicted_angle = new ViewportAngle;
    PredictModel(pose_history, &predicted_angle->yaw, &predicted_angle->pitch);
    return predicted_angle;
}

void ViewportPredict_LR::PredictModel(std::list<ViewportAngle> pose_history, float *yaw, float *pitch)
{
    vector<ViewportAngle> process_pose_array(10);
    std::copy(pose_history.begin(), pose_history.end(), process_pose_array.begin());
    float cos_predict_yaw = intercept[0];
    float sin_predict_yaw = intercept[1];
    float cos_predict_pitch = intercept[2];
    float sin_predict_pitch = intercept[3];
    for (uint32_t i=0;i<10;i++)
    {
        cos_predict_yaw += coef[0][i] * cos(process_pose_array[i].yaw * M_PI / 180);
        sin_predict_yaw += coef[1][i] * sin(process_pose_array[i].yaw * M_PI / 180);
        cos_predict_pitch += coef[2][i] * cos(process_pose_array[i].pitch * M_PI / 180);
        sin_predict_pitch += coef[3][i] * sin(process_pose_array[i].pitch * M_PI / 180);
    }
    float yaw_in_radian = atan2(sin_predict_yaw, cos_predict_yaw);
    float pitch_in_radian = atan2(sin_predict_pitch, cos_predict_pitch);
    *yaw = yaw_in_radian / M_PI * 180;
    *pitch = pitch_in_radian / M_PI * 180;
}
VCD_OMAF_END
