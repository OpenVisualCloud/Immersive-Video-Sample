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

//! \file:   ViewportPredictPlugin.h
//! \brief:  Defines structure and functions in viewport predict plugin.
//! \detail:
//!
//! Created on April 7, 2020, 2:39 PM
//!

#ifndef VIEWPORTPREDICTPLUGIN_H
#define VIEWPORTPREDICTPLUGIN_H

#include "../general.h"
#include <stdlib.h>

VCD_OMAF_BEGIN

typedef void* Handler;
typedef Handler (*INIT_FUNC)(uint32_t,uint32_t,uint32_t);
typedef ViewportAngle* (*PREDICTPOSE_FUNC)(Handler, std::list<ViewportAngle>);

class ViewportPredictPlugin
{
public:
    //!
    //! \brief  construct
    //!
    ViewportPredictPlugin();
    //!
    //! \brief  de-construct
    //!
    ~ViewportPredictPlugin();
    //! \brief load viewport predict plugin by lib path
    //!
    //! \param  [in] const char *
    //!              lib path
    //! \return int
    //!         ERROR code
    //!
    int LoadPlugin(const char* lib_path);
    //! \brief intialize plugin
    //!
    //! \param  [in] uint32_t
    //!              pose interval
    //!         [in] uint32_t
    //!              previous pose count
    //!         [in] uint32_t
    //!              predict interval
    //!
    //! \return int
    //!         ERROR code
    //!
    int Intialize(uint32_t pose_interval, uint32_t pre_pose_count, uint32_t predict_interval);
    //! \brief viewport prediction process
    //!
    //! \param  [in] std::list<ViewportAngle>
    //!              pose history
    //! \return ViewportAngle*
    //!              return predicted viewport pose
    //!
    ViewportAngle* Predict(std::list<ViewportAngle> pose_history);
private:
    Handler          m_libHandler;
    Handler          m_predictHandler;
    INIT_FUNC        m_initFunc;
    PREDICTPOSE_FUNC m_predictFunc;
};

VCD_OMAF_END;
#endif /* VIEWPORTPREDICTPLUGIN_H */