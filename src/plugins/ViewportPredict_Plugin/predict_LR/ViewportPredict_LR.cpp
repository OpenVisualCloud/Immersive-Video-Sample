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

VCD_OMAF_BEGIN

#define MAX_POSE_LIST_SIZE 50

ViewportPredict_LR::ViewportPredict_LR() {}
ViewportPredict_LR::~ViewportPredict_LR() {}
ViewportAngle* ViewportPredict_LR::PredictPose(std::list<ViewportAngle> pose_history)
{
    ViewportAngle *predicted_angle = NULL;
    if (pose_history.empty())
    {
        return predicted_angle;
    }
    //LR algorithm to get the predicted viewport angle.
    return predicted_angle;
}
VCD_OMAF_END
