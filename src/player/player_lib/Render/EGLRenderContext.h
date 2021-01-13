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
//! \file     EGLRenderContext.h
//! \brief    Defines class for EGLRenderContext.
//!

#ifdef _LINUX_OS_

#ifndef _EGLRenderContext_H_
#define _EGLRenderContext_H_

#include "../Common/Common.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include "RenderContext.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

VCD_NS_BEGIN

class EGLRenderContext
 : public RenderContext
{
public:
    EGLRenderContext();
    EGLRenderContext(struct RenderConfig config);
    virtual ~EGLRenderContext();
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

protected:
    Window      m_win;
    EGLDisplay  m_eglDisplay;
    EGLContext  m_eglContext;
    EGLSurface  m_eglSurface;

};

VCD_NS_END
#endif /* _EGLRenderContext_H_ */
#endif // _LINUX_OS_