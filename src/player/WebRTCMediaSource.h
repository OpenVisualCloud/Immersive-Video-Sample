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
//! \file     WebRTCMediaSource.h
//! \brief    Defines class for WebRTCMediaSource.
//!
#ifdef _ENABLE_WEBRTC_SOURCE_

#ifndef _WebRTCMediaSource_H_
#define _WebRTCMediaSource_H_

#include "MediaSource.h"

#include <mutex>
#include <condition_variable>
#include <deque>

#include "owt/base/exception.h"
#include "owt/base/videorendererinterface.h"
#include "owt/base/videodecoderinterface.h"
#include "owt/conference/conferenceclient.h"

#include "360SCVPAPI.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

VCD_NS_BEGIN


class WebRTCVideoFrame
{
public:
    WebRTCVideoFrame(AVFrame *frame, uint8_t *extra_data, int32_t extra_data_length);

    virtual ~WebRTCVideoFrame();

    bool isValid() {return m_frame != NULL;};

    uint8_t *m_buffer[3];
    uint8_t *m_rwpk_sei;
    size_t m_rwpk_sei_length;

private:
    AVFrame *m_frame;
};

class WebRTCVideoRenderer
{
public:
    virtual ~WebRTCVideoRenderer() {}

    virtual int32_t RenderFrame(AVFrame *avFrame, uint8_t *extra_data, int32_t extra_data_length) = 0;
};

class WebRTCMediaSource : public MediaSource, public WebRTCVideoRenderer
{
    static WebRTCMediaSource *s_CurObj;

    static void join_on_success_callback(std::shared_ptr<owt::conference::ConferenceInfo> info);
    static void join_on_failure_callback(std::unique_ptr<owt::base::Exception> err);

    static void subscribe_on_success_callback(std::shared_ptr<owt::conference::ConferenceSubscription> subscription);
    static void subscribe_on_failure_callback(std::unique_ptr<owt::base::Exception> err);

public:
    WebRTCMediaSource();
    virtual ~WebRTCMediaSource();
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
    //! \brief Initial in DashMediaInfo
    //!
    //! \param  [in] const char *
    //!         media url
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus Initialize(struct RenderConfig renderConfig);
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
    //! \brief delete Region Wise Packing data
    //!
    //! \param  [in] RegionWisePacking *
    //!         rwpk
    //!
    virtual void ClearRWPK(RegionWisePacking *rwpk);
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual void DeleteBuffer(uint8_t **buffer);

    int getParam(int *flag, int* projType);

    int32_t RenderFrame(AVFrame *frame, uint8_t *extra_data, int32_t extra_data_length) override;

private:
    std::string m_serverAddress;
    std::shared_ptr<owt::conference::ConferenceClient> m_room;
    std::string m_roomId;
    std::shared_ptr<owt::conference::RemoteMixedStream> m_mixed_stream;
    std::string m_subId;
    int m_yaw;
    int m_pitch;
    std::mutex m_mutex;
    std::condition_variable m_cond;

    std::deque<std::shared_ptr<WebRTCVideoFrame>> m_webrtc_render_frame_queue;
    std::deque<std::shared_ptr<WebRTCVideoFrame>> m_free_queue;

    param_360SCVP m_parserRWPKParam;
    void*         m_parserRWPKHandle;
    RegionWisePacking  m_RWPK;

    bool m_ready;
    static uint32_t fullwidth,fullheight;
    uint32_t lowwidth,lowheight,packedwidth,packedheight,frameRate,frameNum;
    ProjectType projType;
};

class SimpleBuffer {
public:
    SimpleBuffer();
    virtual ~SimpleBuffer();

    void insert(const uint8_t *data, int size);
    void resize(int new_size) {m_size = new_size <= m_max_size ? new_size : 0;}

    uint8_t *data() {return m_data;}
    int size() {return m_size;}

private:
    uint8_t *m_data;
    int m_size;
    int m_max_size;
};

class WebRTCFFmpegVideoDecoder : public owt::base::VideoDecoderInterface {
public:
    WebRTCFFmpegVideoDecoder(WebRTCVideoRenderer *renderer);
    ~WebRTCFFmpegVideoDecoder();

    bool InitDecodeContext(owt::base::VideoCodec video_codec) override;
    owt::base::VideoDecoderInterface* Copy() override;
    bool Release() override;

    bool OnEncodedFrame(std::unique_ptr<owt::base::VideoEncodedFrame> frame) override;

protected:
    bool createDecoder(const owt::base::VideoCodec video_codec);

private:
    AVCodecContext *m_decCtx;
    AVFrame *m_decFrame;
    AVPacket m_packet;

    bool m_needKeyFrame;

    std::shared_ptr<SimpleBuffer> m_bitstream_buf;
    std::deque<std::shared_ptr<SimpleBuffer>> m_sei_queue;

    WebRTCVideoRenderer *m_renderer;

    char m_errbuff[500];
    char *ff_err2str(int errRet);

    uint32_t m_statistics_frames;
    uint64_t m_statistics_last_timestamp;
};

VCD_NS_END;
#endif /* _WebRTCMediaSource_H_ */

#endif /* _ENABLE_WEBRTC_SOURCE_ */
