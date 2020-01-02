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
//! \file     RenderManager.h
//! \brief    Defines base class for RenderManager.
//!
#ifndef _RENDERMANAGER_H_
#define _RENDERMANAGER_H_

#include "ViewPortManager.h"
#include "Common.h"
#include "SurfaceRender.h"
#include "RenderTarget.h"
#include "MediaSource.h"
#include "RenderSource.h"
#include "RenderContext.h"
#include "../utils/Threadable.h"

VCD_NS_BEGIN

class RenderManager : public Threadable
{

public:
    RenderManager(struct RenderConfig config);
    virtual ~RenderManager();

public:
    //!
    //! \brief  Initialize the RenderManager
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_SUCCESS if success, else fail reason
    //!
    virtual RenderStatus Initialize();

    //!
    //! \brief  Do the prepare work before render
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_SUCCESS if success, else fail reason
    //!
    virtual RenderStatus PrepareRender();

    //! \brief The render function
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus Render();

    //!
    //! \brief check player is end or not
    //!
    //! \return bool
    //!         the player is end or not
    //!
    virtual bool IsEOS();

    //! \brief set yaw and pitch to change Viewport
    //!
    //! \param  [in] float
    //!         yaw angle
    //!         [in] pitch
    //!         pitch angle
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus ChangeViewport(float yaw, float pitch);
    //! \brief set yaw and pitch
    //!
    //! \param  [in] float
    //!         yaw angle
    //!         [in] pitch
    //!         pitch angle
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus SetViewport(float yaw, float pitch);
    //!
    //! \brief  Thread functionality Pure virtual function  , it will be re implemented in derived classes
    //!
    virtual void Run();

    //!
    //! \brief  Get Render Configuration
    //!
    //! \return struct RenderConfig
    //!         render Configuration
    //!
    struct RenderConfig GetRenderConfig();

    //! \brief Compute render Matrices
    //!
    //! \param  [in] float
    //!         yaw angle
    //!         [in] pitch
    //!         pitch angle
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    void GetStatusAndPose(float *yaw, float *pitch, uint32_t *status);

private:
    //!
    //! \brief  Create the SurfaceRender
    //!
    //! \param  [in] type
    //!         ERP or CubeMap
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_SUCCESS if success, else fail reason
    //!
    RenderStatus CreateRender(int32_t projFormat);

private:
    ViewPortManager *m_viewPortManager;
    RenderBackend   *m_renderBackend;
    MediaSource     *m_mediaSource;
    RenderSource    *m_renderSource;
    RenderTarget    *m_renderTarget;
    SurfaceRender   *m_surfaceRender;
    RenderContext   *m_renderContext;
    struct RenderConfig m_renderConfig;
    pthread_mutex_t  m_poseMutex;
    int32_t          m_status;
};

VCD_NS_END
#endif /* _RENDERMANAGER_H_ */
