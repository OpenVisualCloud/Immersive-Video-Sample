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
#include "../Common/Common.h"
#ifdef _LINUX_OS_
#include "SurfaceRender.h"
#endif
#include "RenderTarget.h"
#include "../MediaSource/MediaSource.h"
#include "../MediaSource/RenderSourceFactory.h"
#include "RenderContext.h"
#include "../../../utils/Threadable.h"

VCD_NS_BEGIN

class RenderManager
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
    virtual RenderStatus Initialize(MediaSource* source, RenderSourceFactory *rsFactory, RenderContext* context);

    //! \brief The render function
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus Render(int64_t pts, int64_t *corr_pts);

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
    RenderStatus ChangeViewport(HeadPose *pose, uint64_t pts);
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
    RenderStatus SetViewport(HeadPose *pose);
    //! \brief get yaw and pitch
    //!
    //! \param  [out] float
    //!         yaw angle
    //!         [out] pitch
    //!         pitch angle
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus GetViewport(float *yaw, float *pitch);

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
    void GetStatusAndPose(HeadPose *pose, uint32_t *status);

    RenderSourceFactory* getRenderSourceFactory(){return m_rsFactory;};

    int UpdateDisplayTex();

    void UpdateFrames(int64_t pts); // for android

    void SetOutputTexture(uint32_t tex_id);

    int* GetTransformTypeArray()
    {
        int face_num = 6;
        int* data = new int[face_num];
        std::map<uint32_t, uint8_t> transform_type = m_renderTarget->GetTransformType();

        for (auto it : transform_type)
        {
            if (it.first >=0 && it.first < face_num)
            {
                data[it.first] = it.second;
            }
        }
        return data;
    };

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
    //!
    //! \brief  Create the render target
    //!
    //! \param  [in] type
    //!         ERP or CubeMap
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_SUCCESS if success, else fail reason
    //!
    RenderStatus CreateRenderTarget(int32_t projFormat);

private:
    RenderManager& operator=(const RenderManager& other) { return *this; };
    RenderManager(const RenderManager& other) { /* do not create copies */ };

private:
    ViewPortManager        *m_viewPortManager;
    MediaSource            *m_mediaSource;
    RenderSourceFactory    *m_rsFactory;
    RenderTarget           *m_renderTarget;
#ifdef _LINUX_OS_
    SurfaceRender          *m_surfaceRender;
#endif
    RenderContext          *m_renderContext;
    struct RenderConfig     m_renderConfig;
    ThreadLock              m_poseLock;
    uint32_t                m_outputTexture;
};

VCD_NS_END
#endif /* _RENDERMANAGER_H_ */
