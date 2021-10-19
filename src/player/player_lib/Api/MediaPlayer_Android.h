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
//! \file     MediaPlayer_Android.h
//! \brief    Define Media Player in Android.
//!

#ifdef _ANDROID_OS_

#ifndef _MEDIAPLAYER_ANDROID_H_
#define _MEDIAPLAYER_ANDROID_H_

#include "../Common/Common.h"
#include "../Render/RenderManager.h"
#include "MediaPlayerAPI.h"

VCD_NS_BEGIN

class MediaPlayer_Android
{
public:
    MediaPlayer_Android();

    ~MediaPlayer_Android();

    RenderStatus Create(struct RenderConfig config);
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
    RenderStatus Start();

    //! \brief close media and un-initialize
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus Close();

    uint32_t GetStatus() {
        if (m_mediaSource->GetStatus() == STATUS_STOPPED) {
            return STOPPED;
        }
        else
            return m_status;
    };

    bool IsPlaying() { return m_status == PLAY; };

    void SetCurrentPosition(HeadPose pose)
    {
        m_renderManager->SetViewport(&pose);
        m_renderManager->ChangeViewport(&pose, pose.pts);
        return;
    };

    uint32_t GetWidth() { return m_mediaSource->GetMediaInfo().mVideoInfo[0].width; };

    uint32_t GetHeight() { return m_mediaSource->GetMediaInfo().mVideoInfo[0].height; };

    int32_t GetProjectionFormat() { return m_mediaSource->GetMediaInfo().mVideoInfo[0].mProjFormat; };

    int UpdateDisplayTex(int render_count);

    void SetDecodeSurface(jobject surface, int tex_id, int video_id)
    {
        m_mediaSource->SetDecodeSurface(surface, tex_id, video_id);
    };

    void SetDisplaySurface(int tex_id) { m_renderManager->SetOutputTexture(tex_id); };

    int* GetTransformType() { return m_renderManager->GetTransformTypeArray(); };

private:
    MediaPlayer_Android& operator=(const MediaPlayer_Android& other) { return *this; };
    MediaPlayer_Android(const MediaPlayer_Android& other) { /* do not create copies */ };

private:
    uint32_t                  m_status;
    RenderManager            *m_renderManager;
    MediaSource              *m_mediaSource;
    RenderSourceFactory      *m_rsFactory;
    RenderConfig              m_renderConfig;
    MediaInfo                 m_mediaInfo;


};

VCD_NS_END
#endif /* _MEDIAPLAYER_ANDROID_H_ */
#endif