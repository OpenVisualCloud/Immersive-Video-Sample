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
#include <vector>
#include <map>

VCD_OMAF_BEGIN

typedef void* Handler;
// define function point of plugin interfaces
typedef Handler (*INIT_FUNC)(PredictOption);
typedef int32_t (*SETVIEWPORT_FUNC)(Handler, ViewportAngle*);
typedef int32_t (*PREDICTPOSE_FUNC)(Handler, uint64_t, std::map<uint64_t, ViewportAngle*>&, float*);
typedef int32_t (*DESTROY_FUNC)(Handler);

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
    //! \param  [in] PredictOption
    //!              predict option
    //!
    //! \return int
    //!         ERROR code
    //!
    int Intialize(PredictOption option);
    //! \brief set original viewport angle
    //!
    //! \param  [in] ViewportAngle*
    //!              original viewport angle
    //!
    int SetViewport(ViewportAngle *angle);
    //! \brief viewport prediction process
    //!
    //! \param  [in] uint64_t
    //!              first pts of predict angle
    //!         [in] std::map<uint64_t, ViewportAngle*>&
    //!              output predict angle list
    //! \return int
    //!         ERROR code
    //!
    int Predict(uint64_t pre_first_pts, std::map<uint64_t, ViewportAngle*>& predict_viewport_list, float *possibilityOfHalting);
    //!
    //! \brief viewport prediction destroy function
    //!
    int Destroy();

private:
    Handler          m_libHandler;
    Handler          m_predictHandler;
    INIT_FUNC        m_initFunc;
    SETVIEWPORT_FUNC m_setViewportFunc;
    PREDICTPOSE_FUNC m_predictFunc;
    DESTROY_FUNC     m_destroyFunc;
};

VCD_OMAF_END;
#endif /* VIEWPORTPREDICTPLUGIN_H */
