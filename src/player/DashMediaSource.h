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
#ifndef _ENABLE_WEBRTC_SOURCE_

#ifndef _DASHMEDIASOURCE_H_
#define _DASHMEDIASOURCE_H_

#include "MediaSource.h"
#include "../utils/Threadable.h"
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
    virtual RenderStatus Initialize(struct RenderConfig renderConfig);
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
    //! \brief Set Media Source Info
    //!
    //! \param  [in] void *
    //!         mediaInfo
    //!
    //! \return void *
    //!
    virtual RenderStatus SetMediaSourceInfo(void *mediaInfo);
    //! \brief Get media source information
    //!
    //! \return struct MediaSourceInfo
    //!         media information
    //!
    virtual struct MediaSourceInfo GetMediaSourceInfo();
    //! \brief Get SourceMetaData
    //!
    //! \return void*
    //!
    virtual void* GetSourceMetaData();
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
    virtual RenderStatus ChangeViewport(float yaw, float pitch);
    //! \brief set region information
    //!
    //! \param  [in] struct RegionInfo*
    //!         regionInfo
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus SetRegionInfo(struct RegionInfo* regionInfo);
    //! \brief delete buffer data
    //!
    //! \param  [in] uint8_t **
    //!         buffer
    //!
    virtual void DeleteBuffer(uint8_t **buffer);
    //! \brief delete Region Wise Packing data
    //!
    //! \param  [in] RegionWisePacking *
    //!         rwpk
    //!
    virtual void ClearRWPK(RegionWisePacking *rwpk);
    //!
    //! \brief  Thread functionality Pure virtual function  , it will be re implemented in derived classes
    //!
    virtual void Run();

private:

    void                               *m_handler;//Dash Source handle

    std::list<struct FrameInfo *>       m_frameBuffer;

    int32_t                             m_status;

    pthread_mutex_t                     m_frameMutex;

    std::list<RegionWisePacking>        m_rwpkList;

    struct SourceData m_dashSourceData;

    RenderStatus ClearDashSourceData();

    RenderStatus InitializeDashSourceData();

    RenderStatus ClearPackedData(struct FrameInfo *frameInfo);
    //! \brief Get packet buffers from the DashStreaming lib.
    //!
    //! \param  [out] AVPacket *
    //!         packet
    //!         [out] RegionWisePacking *
    //!         rwpk information
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus GetPacket(AVPacket *pkt, RegionWisePacking *rwpk);
    //! \brief Get a frame from the Media Source
    //!
    //! \param  [out] uint8_t **
    //!         the frame buffer
    //!
    //! \return DecoderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    DecoderStatus GetOneFrame(uint8_t **buffer);
    //! \brief flush frames from the Media Source
    //!
    //! \param  [out] uint8_t **
    //!         the frame buffer
    //!
    //! \return DecoderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    DecoderStatus FlushFrames(uint8_t **buffer);
};

VCD_NS_END
#endif /* _DASHMEDIASOURCE_H_ */

#endif /* _NON_ENABLE_WEBRTC_SOURCE_ */
