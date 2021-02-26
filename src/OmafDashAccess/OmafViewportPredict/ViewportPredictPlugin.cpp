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
    m_setViewportFunc = NULL;
    m_destroyFunc     = NULL;
}

ViewportPredictPlugin::~ViewportPredictPlugin()
{
    if (m_libHandler)
    {
        dlclose(m_libHandler);
        m_libHandler = NULL;
    }
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
        OMAF_LOG(LOG_ERROR,"failed to open predict library path!\n");
        return ERROR_NULL_PTR;
    }
    m_initFunc = (INIT_FUNC)dlsym(m_libHandler, "ViewportPredict_Init");
    if (dlerror() != NULL)
    {
        OMAF_LOG(LOG_ERROR,"failed to load ViewportPredict_Init func!\n");
        dlclose(m_libHandler);
        return ERROR_INVALID;
    }
    m_setViewportFunc = (SETVIEWPORT_FUNC)dlsym(m_libHandler, "ViewportPredict_SetViewport");
    if (dlerror() != NULL)
    {
        OMAF_LOG(LOG_ERROR,"failed to load ViewportPredict_SetViewport func!\n");
        dlclose(m_libHandler);
        return ERROR_INVALID;
    }
    m_predictFunc = (PREDICTPOSE_FUNC)dlsym(m_libHandler, "ViewportPredict_PredictPose");
    if (dlerror() != NULL)
    {
        OMAF_LOG(LOG_ERROR,"failed to load ViewportPredict_PredictPose func!\n");
        dlclose(m_libHandler);
        return ERROR_INVALID;
    }
    m_destroyFunc = (DESTROY_FUNC)dlsym(m_libHandler, "ViewportPredict_unInit");
    if (dlerror() != NULL)
    {
        OMAF_LOG(LOG_ERROR,"failed to load ViewportPredict_unInit func!\n");
        dlclose(m_libHandler);
        return ERROR_INVALID;
    }
    return ERROR_NONE;
}

int ViewportPredictPlugin::Intialize(PredictOption option)
{
    Handler predict_handler = m_initFunc(option);
    if (NULL == predict_handler)
    {
        OMAF_LOG(LOG_ERROR,"handler init failed!\n");
        return ERROR_NULL_PTR;
    }
    m_predictHandler = predict_handler;
    return ERROR_NONE;
}

int ViewportPredictPlugin::SetViewport(ViewportAngle *angle)
{
    if (angle == nullptr)
    {
        OMAF_LOG(LOG_ERROR, " Viewport angle is null!\n");
        return ERROR_NULL_PTR;
    }
    return m_setViewportFunc(m_predictHandler, angle);
}

int ViewportPredictPlugin::Predict(uint64_t pre_first_pts, std::map<uint64_t, ViewportAngle*>& predict_viewport_list, float *possibilityOfHalting)
{
    return m_predictFunc(m_predictHandler, pre_first_pts, predict_viewport_list, possibilityOfHalting);
}

int ViewportPredictPlugin::Destroy()
{
    return m_destroyFunc(m_predictHandler);
}

VCD_OMAF_END
