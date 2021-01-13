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

 *
 */

//!
//! \file     GLFWRenderContext.h
//! \brief    Defines class for GLFWRenderContext.
//!

#ifdef _LINUX_OS_

#ifndef _GLFWRenderContext_H_
#define _GLFWRenderContext_H_

#include "../../player_lib/Common/Common.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include "../../player_lib/Render/RenderContext.h"
#include <GLFW/glfw3.h>

VCD_NS_BEGIN

class GLFWRenderContext
 : public RenderContext
{
public:
    GLFWRenderContext();
    GLFWRenderContext(struct RenderConfig config);
    virtual ~GLFWRenderContext();
    //! \brief swap buffer
    //!
    //! \param  [in] window
    //          window handle
    //!         [in] param
    //!         parameter for the swap buffer
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus SwapBuffers(void * window, int param);
    //! \brief get pose and status according to inputs
    //!
    //! \param  [out] yaw
    //          current pose : yaw
    //!         [out] pitch
    //!         current pose : pitch
    //!         [out] status
    //!         player status
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus GetStatusAndPose(HeadPose *pose, uint32_t* status);
    //! \brief initialize render context
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual void* InitContext();
    //! \brief check whether the render is running
    //!
    //! \return bool
    //!         isrunning or not
    //!
    virtual bool isRunning();
    //! \brief Auto change the viewport position
    //!
    //! \param  [out] float *
    //          horizontal Angle
    //!         [out] float *
    //!         vertical Angle
    //! \return void
    //!
    void AutoChangePos(float *hPos, float *vPos);
    //! \brief Get motion to high quality xml parameters
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus GetMotionOptionParams();

    //! \brief Set full resolution for calculating zoomFactor.
    //!
    //! \param  [in] uint32_t
    //          full picture width
    //!         [out] uint32_t
    //!         full picture height
    //!
    virtual void SetFullResolution(uint32_t width, uint32_t height)
    {
        m_fullWidth = width;
        m_fullHeight = height;
        SetMaxSpeed();
    };

private:

    //! \brief get pose and status according to inputs for 3D
    //!
    //! \param  [out] HeadPose
    //          current pose
    //!         [out] status
    //!         player status
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus GetStatusAndPoseFor3D(HeadPose *pose, uint32_t* status);

    //! \brief get pose and status according to inputs for 2D
    //!
    //! \param  [out] HeadPose
    //          current pose
    //!         [out] status
    //!         player status
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus GetStatusAndPoseFor2D(HeadPose *pose, uint32_t* status);
    //!
    //! \brief Set max speed for key motion.
    //!
    void SetMaxSpeed();

    bool m_needMotionTest;              //<! need motion test or not
    struct MotionConfig m_motionConfig; //<! test params for motion to high quality test
    float m_zoomCnt;                      //<! count for key event for zoom in/out.
    uint32_t m_fullWidth;               //<! max picture width for calculating zoomFactor.
    uint32_t m_fullHeight;              //<! max picture height for calculating zoomFactor.
    float m_zoomFactor;                 //<! defined as window w * window h / (content in max picture w * content in max picture h)
};

VCD_NS_END
#endif /* _GLFWRenderContext_H_ */
#endif