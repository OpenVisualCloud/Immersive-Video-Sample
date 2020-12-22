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
//! \file     MediaPlayerAPI.h
//! \brief    Define Player. @obsolete: android and linux are seperated to expose their interfaces. The API is just for reference.
//!
#ifndef _MEDIAPLAYERAPI_H_
#define _MEDIAPLAYERAPI_H_

#include "../Common/RenderType.h"

class MediaPlayer
{
public:
    MediaPlayer(){};

    virtual ~MediaPlayer() = default;

    virtual RenderStatus Create(struct RenderConfig config) = 0;
    //! \brief main loop in player control and playback
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus Play() = 0;

    //! \brief pause
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus Pause() = 0;

    //! \brief Stop
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus Resume() = 0;

    //! \brief pause
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus Stop() = 0;

    //! \brief main loop in player control and playback
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus Seek() = 0;

    //! \brief open media and initialize
    //!
    //! \param  [in] struct RenderConfig
    //!         render configuration
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus Start(void *render_context) = 0;

    //! \brief close media and un-initialize
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus Close() = 0;

    //! \brief get player status
    //!
    //! \return uint32_t
    //!         status
    //!
    virtual uint32_t GetStatus() = 0;

    virtual bool IsPlaying() = 0;

    virtual HeadPose GetCurrentPosition() = 0;

    virtual uint32_t GetWidth() = 0;

    virtual uint32_t GetHeight() = 0;

    virtual void SetTexture(uint32_t texture) = 0;

};

#endif /* _MEDIAPLAYERAPI_H_ */
