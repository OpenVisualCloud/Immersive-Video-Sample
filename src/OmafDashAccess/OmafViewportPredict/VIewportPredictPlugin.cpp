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

//! \file:   ViewportPredictPlugin.cpp
//! \brief:  Defines structure and functions in viewport predict plugin.
//! \detail:
//!
//! Created on April 7, 2020, 2:39 PM
//!

#include "ViewportPredictPlugin.h"
#include <dlfcn.h>

VCD_OMAF_BEGIN

ViewportPredictPlugin::ViewportPredictPlugin()
{
    m_libHandler      = NULL;
    m_predictHandler  = NULL;
    m_predictFunc     = NULL;
    m_initFunc        = NULL;
}

ViewportPredictPlugin::~ViewportPredictPlugin()
{
    dlclose(m_libHandler);
}

int ViewportPredictPlugin::LoadPlugin(const char* lib_path)
{
    if (NULL == lib_path)
    {
        return ERROR_NULL_PTR;
    }
    m_libHandler = dlopen(lib_path, RTLD_LAZY);
    if (!m_libHandler)
    {
        LOG(ERROR)<<"failed to open predict library path!"<<endl;
        return ERROR_NULL_PTR;
    }
    m_initFunc = (INIT_FUNC)dlsym(m_libHandler, "ViewportPredict_Init");
    if (dlerror() != NULL)
    {
        LOG(ERROR)<<"failed to load ViewportPredict_Init func!"<<endl;
        dlclose(m_libHandler);
        return ERROR_INVALID;
    }
    m_predictFunc = (PREDICTPOSE_FUNC)dlsym(m_libHandler, "ViewportPredict_PredictPose");
    if (dlerror() != NULL)
    {
        LOG(ERROR)<<"failed to load ViewportPredict_PredictPose func!"<<endl;
        dlclose(m_libHandler);
        return ERROR_INVALID;
    }
    return ERROR_NONE;
}

int ViewportPredictPlugin::Intialize(uint32_t pose_interval, uint32_t pre_pose_count, uint32_t predict_interval)
{
    Handler predict_handler = m_initFunc(pose_interval, pre_pose_count, predict_interval);
    if (NULL == predict_handler)
    {
        LOG(ERROR)<<"handler init failed!"<<std::endl;
        return ERROR_NULL_PTR;
    }
    m_predictHandler = predict_handler;
    return ERROR_NONE;
}

ViewportAngle* ViewportPredictPlugin::Predict(std::list<ViewportAngle> pose_history)
{
    if (pose_history.size() == 0)
    {
        LOG(ERROR)<<"pose history is empty now!"<<endl;
        return NULL;
    }
    ViewportAngle* predict_angle = m_predictFunc(m_predictHandler, pose_history);
    if (predict_angle == NULL)
    {
        LOG(ERROR)<<"predictPose_func return an invalid value!"<<endl;
        return NULL;
    }
    return predict_angle;
}

VCD_OMAF_END