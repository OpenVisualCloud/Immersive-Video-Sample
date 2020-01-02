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
//! \file     MediaSource.h
//! \brief    Defines class for MediaSource.
//!
#ifndef _MEDIASOURCE_H_
#define _MEDIASOURCE_H_

#include "Common.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

VCD_NS_BEGIN

class MediaSource
{
public:
    MediaSource();
    virtual ~MediaSource();
    //! \brief Initial in DashMediaInfo
    //!
    //! \param  [in] struct RenderConfig
    //!         render configuration
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus Initialize(struct RenderConfig renderConfig) = 0;
    //! \brief Get a frame from the Media Source
    //!
    //!         [out] uint8_t **
    //!         the frame buffer
    //!         [out] struct RegionInfo *
    //!         regionInfo
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus GetFrame(uint8_t **buffer, struct RegionInfo *regionInfo) = 0;
    //! \brief Set Media Source Info
    //!
    //! \param  [in] void *
    //!         mediaInfo
    //!
    //! \return void *
    //!
    virtual RenderStatus SetMediaSourceInfo(void *mediaInfo) = 0;
    //! \brief Get Media Source Info
    //!
    //! \return struct MediaSourceInfo
    //!
    virtual struct MediaSourceInfo GetMediaSourceInfo() = 0;
    //! \brief Get SourceMetaData
    //!
    //! \return void*
    //!
    virtual void* GetSourceMetaData() = 0;
    //! \brief Check is file ends
    //!
    //! \return bool
    //!
    virtual bool IsEOS() = 0;
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
    virtual RenderStatus ChangeViewport(float yaw, float pitch) = 0;
    //! \brief set region information
    //!
    //! \param  [in] struct RegionInfo*
    //!         regionInfo
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus SetRegionInfo(struct RegionInfo* regionInfo) = 0;
    //! \brief delete buffer data
    //!
    //! \param  [in] uint8_t **
    //!         buffer
    //!
    virtual void DeleteBuffer(uint8_t **buffer) = 0;
    //! \brief delete Region Wise Packing data
    //!
    //! \param  [in] RegionWisePacking *
    //!         rwpk
    //!
    virtual void ClearRWPK(RegionWisePacking *rwpk) = 0;
    //! \brief get isAllValid
    //!
    //! \return bool
    //!         isAllValid
    //!
    bool getIsAllValid() {return isAllValid;};

protected:

    struct MediaSourceInfo m_mediaSourceInfo;
    MediaSourceType::Enum m_sourceType;//vod or live
    bool isAllValid;

};

VCD_NS_END
#endif /* _MEDIASOURCE_H_ */

