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
//! \file     FFmpegMediaSource.h
//! \brief    Defines class for FFmpegMediaSource.
//!
#ifndef _FFMPEGMEDIASOURCE_H_
#define _FFMPEGMEDIASOURCE_H_

#include "MediaSource.h"

VCD_NS_BEGIN

class FFmpegMediaSource
: public MediaSource
{
public:
    FFmpegMediaSource();
    virtual ~FFmpegMediaSource();
    //! \brief Initial in DashMediaInfo
    //!
    //! \param  [in] struct RenderConfig
    //!         render configuration
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
    //! \brief Get Media Source Info
    //!
    //! \return struct MediaSourceInfo
    //!
    virtual struct MediaSourceInfo GetMediaSourceInfo();
    //! \brief Get SourceMetaData
    //!
    //! \return void*
    //!
    virtual void* GetSourceMetaData();
    //! \brief Check is file ends
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

private:

    struct SourceData m_ffmpegSourceData;

    RenderStatus ClearFFmpegSourceData();

    RenderStatus InitializeFFmpegSourceData();

};

VCD_NS_END
#endif /* _FFMPEGMEDIASOURCE_H_ */

