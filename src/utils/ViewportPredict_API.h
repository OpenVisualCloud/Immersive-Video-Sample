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

//! \file:   ViewportPredict_API.h
//! \brief:  the class for viewport predict API.
//! \detail: it's class to predict viewport API.
//!
//! Created on April 2, 2020, 1:18 PM
//!

#ifndef _VIEWPORTPREDICT_H_
#define _VIEWPORTPREDICT_H_

#include <stdlib.h>
#include <map>
#include "data_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* Handler;

//! \brief Initialze the viewport prediction algorithm
//!
//! \param  [in] PredictOption
//!              predict option
//!
Handler ViewportPredict_Init(PredictOption option);
//! \brief set original viewport angle
//!
//! \param  [in] Handler
//!              viewport prediction handler
//!         [in] ViewportAngle*
//!              original viewport angle
//!
int32_t ViewportPredict_SetViewport(Handler hdl, ViewportAngle *angle);
//! \brief viewport prediction process
//!
//! \param  [in] Handler
//!              viewport prediction handler
//!         [in] uint64_t
//!              first pts of predict angle
//!         [out] std::map<uint64_t, ViewportAngle*>&
//!              predicted viewport map
//!
int32_t ViewportPredict_PredictPose(Handler hdl, uint64_t pre_first_pts, std::map<uint64_t, ViewportAngle*>& predict_viewport_list, float *possibilityOfHalting);

//!
//! \brief uninit the viewport prediction algorithm
//!
int32_t ViewportPredict_unInit(Handler hdl);

#ifdef __cplusplus
}
#endif

#endif /* _VIEWPORTPREDICT_H_ */