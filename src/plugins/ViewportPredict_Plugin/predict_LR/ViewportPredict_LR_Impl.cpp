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

//! \file:   ViewportPredict_Impl.cpp
//! \brief:  the class for viewport predict
//! \detail: it's class to predict viewport
//!
//! Created on April 2, 2020, 1:18 PM
//!

#include "../../../utils/ViewportPredict_API.h"
#include "../predict_Base/ViewportPredict.h"
#include "ViewportPredict_LR.h"

VCD_USE_VROMAF;

Handler ViewportPredict_Init(uint32_t pose_interval, uint32_t pre_pose_count, uint32_t predict_interval)
{
    ViewportPredict *predictor = NULL;
    predictor = new ViewportPredict_LR();
    predictor->Initialize(pose_interval, pre_pose_count, predict_interval);
    return (Handler)((long)predictor);
}

ViewportAngle* ViewportPredict_PredictPose(Handler hdl, std::list<ViewportAngle> pose_history)
{
    ViewportPredict *predictor = (ViewportPredict *)hdl;
    ViewportAngle* angle = predictor->PredictPose(pose_history);
    return angle;
}
