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
//! \file     MediaPlayer_Linux.h
//! \brief    Define Player.
//!

#ifdef _LINUX_OS_

#ifndef _MEDIAPLAYER_LINUX_H_
#define _MEDIAPLAYER_LINUX_H_

#include "../Common/Common.h"
#include "../Render/RenderManager.h"
#include "../Render/RenderContext.h"
#include "../../../utils/Threadable.h"

VCD_NS_BEGIN

class MediaPlayer_Linux
{
public:
    MediaPlayer_Linux();

    RenderStatus Create(struct RenderConfig config);

    ~MediaPlayer_Linux();

    //! \brief main loop in player control and playback
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus Play();

    //! \brief pause
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus Pause();

    //! \brief Stop
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus Resume();

    //! \brief pause
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus Stop();

    //! \brief main loop in player control and playback
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus Seek();

    //! \brief open media and initialize
    //!
    //! \param  [in] struct RenderConfig
    //!         render configuration
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus Start(void *render_context);

    //! \brief close media and un-initialize
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus Close();

    uint32_t GetStatus() { return m_status; };

    bool IsPlaying() { return m_status == PLAY; };

    HeadPose GetCurrentPosition()
    {
        HeadPose pose;
        pose.yaw = 0;
        pose.pitch = 0;
        m_renderManager->GetViewport(&pose.yaw, &pose.pitch);
        return pose;
    };

    uint32_t GetWidth() { return m_mediaSource->GetMediaSourceInfo().width; };

    uint32_t GetHeight() { return m_mediaSource->GetMediaSourceInfo().height; };

    void SetTexture(uint32_t texture) { m_renderManager->SetOutputTexture(texture); };

private:
    MediaPlayer_Linux& operator=(const MediaPlayer_Linux& other) { return *this; };
    MediaPlayer_Linux(const MediaPlayer_Linux& other) { /* do not create copies */ };

private:
    uint32_t                  m_status;
    RenderManager            *m_renderManager;
    MediaSource              *m_mediaSource;
    RenderSourceFactory      *m_rsFactory;
    RenderContext            *m_renderContext;
    RenderConfig              m_renderConfig;
    MediaInfo                 m_mediaInfo;

};

VCD_NS_END
#endif /* _MEDIAPLAYER_LINUX_H_ */
#endif