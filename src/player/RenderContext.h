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
//! \file     RenderContext.h
//! \brief    Defines class for RenderContext.
//!
#ifndef _RenderContext_H_
#define _RenderContext_H_

#include "Common.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

VCD_NS_BEGIN

class RenderContext
{
public:
    RenderContext();
    virtual ~RenderContext();
    //! \brief swap buffer
    //!
    //! \param  [in] window
    //          window handle
    //!         [in] param
    //!         parameter for the swap buffer
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus SwapBuffers(void * window, int param) = 0;
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
    virtual RenderStatus GetStatusAndPose(float *yaw, float *pitch, uint32_t* status) = 0;
    //! \brief initialize render context
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus InitContext() = 0;
    //! \brief get window handle
    //!
    //! \return void *
    //!         m_window handle
    //!
    void *GetWindow() {return m_window;}
    //! \brief check whether the render is running
    //!
    //! \return bool
    //!         isrunning or not
    //!
    virtual bool isRunning() = 0;
    //! \brief get render projection matrix
    //!
    //! \return glm::mat4
    //!         render projection matrix
    //!
    glm::mat4 GetProjectionMatrix() {return m_projectionMatrix;}
    //! \brief get render view model matrix
    //!
    //! \return glm::mat4
    //!         render view model matrix
    //!
    glm::mat4 GetViewModelMatrix() {return m_viewModelMatrix;}

protected:
    enum RenderContextType m_renderContextType;
    void *m_window;
    uint32_t m_windowWidth;
    uint32_t m_windowHeight;
    float m_hFOV;
    float m_vFOV;

    glm::mat4 m_projectionMatrix;
    glm::mat4 m_viewModelMatrix;
    // Initial horizontal angle : toward -Z
    float m_horizontalAngle;
    // Initial vertical angle : none
    float m_verticalAngle;

    float m_speed; // 3 units / second
    float m_mouseSpeed;
};

VCD_NS_END
#endif /* _RenderContext_H_ */

