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
//! \file     MediaPlayer.h
//! \brief    Define Player.
//!
#ifndef _MEDIAPLAYER_H_
#define _MEDIAPLAYER_H_

#include "Common.h"
#include "Render/RenderManager.h"
#include <GLFW/glfw3.h>
#include "Render/RenderContext.h"
#include "../utils/Threadable.h"

VCD_NS_BEGIN

class MediaPlayer : public Threadable
{
public:
    MediaPlayer();
    MediaPlayer(struct RenderConfig config);

    virtual ~MediaPlayer();

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
    RenderStatus Open();

    //! \brief close media and un-initialize
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus Close();

    //! \brief get player status
    //!
    //! \return uint32_t
    //!         status
    //!
    uint32_t GetStatus();

    //!
    //! \brief  Thread functionality Pure virtual function  , it will be re implemented in derived classes
    //!
    virtual void Run();

private:
    //!
    //! \brief  Thread functionality Pure virtual function  , it will be re implemented in derived classes
    //!
    RenderStatus PlayOneVideo(int64_t pts);

    //!
    //! \brief  Thread functionality Pure virtual function  , it will be re implemented in derived classes
    //!
    RenderStatus PlayOneAudio(int64_t pts);

    RenderStatus UpdateUserInput();

    bool HasAudio();
    bool HasVideo();

private:
    MediaPlayer& operator=(const MediaPlayer& other) { return *this; };
    MediaPlayer(const MediaPlayer& other) { /* do not create copies */ };

private:
    ThreadStatus              m_status;
    RenderManager            *m_renderManager;
    MediaSource              *m_mediaSource;
    RenderSourceFactory      *m_rsFactory;
    RenderContext            *m_renderContext;
    RenderConfig              m_renderConfig;
    MediaInfo                 m_mediaInfo;

};

VCD_NS_END
#endif /* _MEDIAPLAYER_H_ */
