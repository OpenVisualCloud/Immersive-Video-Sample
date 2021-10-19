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
//! \file     DashMediaSource.h
//! \brief    Defines class for DashMediaSource.
//!
#ifdef _ENABLE_DASH_SOURCE_

#ifndef _DASHMEDIASOURCE_H_
#define _DASHMEDIASOURCE_H_

#include "MediaSource.h"
#include "../Decoder/DecoderManager.h"
#include "../../../utils/Threadable.h"
#include <list>

VCD_NS_BEGIN

class DashMediaSource
: public MediaSource , public Threadable
{
public:
    DashMediaSource();
    virtual ~DashMediaSource();

    //! \brief Initial in DashMediaInfo
    //!
    //! \param  [in] struct RenderConfig
    //!         render Configuration
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus Initialize(struct RenderConfig renderConfig, RenderSourceFactory *rsFactory=NULL);

    virtual RenderStatus Start();

    //! \brief Check is player ends
    //!
    //! \return bool
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
    virtual RenderStatus ChangeViewport(HeadPose *pose);

    //! \brief set region information
    //!
    //! \param  [in] struct RegionInfo*
    //!         regionInfo
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus SetRegionInfo(struct RegionInfo* regionInfo){return RENDER_STATUS_OK;};

    //! \brief UpdateFrames
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, RENDER_EOS if reach EOS
    //!
    virtual RenderStatus UpdateFrames(uint64_t pts, int64_t *corr_pts);

    //!
    //! \brief  Thread functionality Pure virtual function  , it will be re implemented in derived classes
    //!
    virtual void Run();

    //! \brief Get a frame from the Media Source
    //!
    //!         [out] uint8_t **
    //!         the frame buffer
    //!         [out] struct RegionInfo *
    //!         regionInfo
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus GetFrame(uint8_t **buffer, struct RegionInfo *regionInfo);

    //! \brief SeekTo
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, RENDER_EOS if reach EOS
    //!
    virtual RenderStatus SeekTo(uint64_t pts);


    //! \brief Pause
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success
    //!
    virtual RenderStatus Pause();

    //! \brief Play
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success
    //!
    virtual RenderStatus Play();

    virtual void SetActiveStream( int32_t video_id, int32_t audio_id );
private:
    //! \brief Set Media Source Info
    //!
    //! \param  [in] void *
    //!         mediaInfo
    //!
    //! \return void *
    //!
    RenderStatus SetMediaInfo(void *mediaInfo);

    void ProcessVideoPacket();

    void ProcessAudioPacket();

    RenderStatus GetStreamDumpedOptionParams();

private:
    DashMediaSource& operator=(const DashMediaSource& other) { return *this; };
    DashMediaSource(const DashMediaSource& other) { /* do not create copies */ };

private:

    void                               *m_handler;//Dash Source handle
    pthread_mutex_t                     m_frameMutex;
    DecoderManager                     *m_DecoderManager;
    ThreadLock                          m_Lock;
    bool                                m_needStreamDumped;
    vector<FILE*>                       m_dumpedFile;
    FILE*                               m_singleFile;
    uint32_t                            m_maxVideoWidth;
    uint32_t                            m_maxVideoHeight;
};

VCD_NS_END
#endif /* _DASHMEDIASOURCE_H_ */

#endif  // _ENABLE_DASH_SOURCE_
