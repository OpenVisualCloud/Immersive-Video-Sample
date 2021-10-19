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

#include "../Decoder/DecoderManager.h"
#include "MediaSource.h"

#include <condition_variable>
#include <mutex>

#include "owt/base/exception.h"
#include "owt/base/videodecoderinterface.h"
#include "owt/base/videorendererinterface.h"
#include "owt/conference/conferenceclient.h"

#include "360SCVPAPI.h"

VCD_NS_BEGIN

class WebRTCVideoPacketListener {
 public:
  virtual ~WebRTCVideoPacketListener() {}

  virtual bool OnVideoPacket(
      std::unique_ptr<owt::base::VideoEncodedFrame> frame) = 0;
};

class WebRTCMediaSource : public MediaSource, public WebRTCVideoPacketListener {
  static WebRTCMediaSource* s_CurObj;

  static void join_on_success_callback(
      std::shared_ptr<owt::conference::ConferenceInfo> info);
  static void join_on_failure_callback(
      std::unique_ptr<owt::base::Exception> err);

  static void subscribe_on_success_callback(
      std::shared_ptr<owt::conference::ConferenceSubscription> subscription);
  static void subscribe_on_failure_callback(
      std::unique_ptr<owt::base::Exception> err);

 public:
  WebRTCMediaSource();
  virtual ~WebRTCMediaSource();

  //! \brief Initial in MediaInfo
  //!
  //!         render configuration
  //! \return RenderStatus
  //!         RENDER_STATUS_OK if success, else fail reason
  //!
  virtual RenderStatus Initialize(
      struct RenderConfig renderConfig,
      RenderSourceFactory* rsFactory = NULL) override;

  //! \brief Get a Video frame from the Media Source
  //!
  //! \param  [out] uint8_t **
  //!         the frame buffer
  //!         [out] struct RegionInfo *
  //!         regionInfo
  //! \return RenderStatus
  //!         RENDER_STATUS_OK if success, else fail reason
  //!
  virtual RenderStatus GetFrame(uint8_t** buffer,
                                struct RegionInfo* regionInfo) override;

  //! \brief Get an Audio frame from the Media Source
  //!
  //! \param  [out] uint8_t **
  //!         the frame buffer
  //!         [out] struct RegionInfo *
  //!         regionInfo
  //! \return RenderStatus
  //!         RENDER_STATUS_OK if success, else fail reason
  //!
  virtual RenderStatus GetAudioFrame(int64_t pts, uint8_t** buffer) override;

  //! \brief Check is file ends
  //!
  //! \return bool
  //!
  virtual bool IsEOS() override;

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
  virtual RenderStatus ChangeViewport(HeadPose* pose) override;
  virtual RenderStatus Start() override;

  //! \brief UpdateFrames
  //!
  //! \return RenderStatus
  //!         RENDER_STATUS_OK if success, RENDER_EOS if reach EOS
  //!
  virtual RenderStatus UpdateFrames(uint64_t pts, int64_t *corr_pts) override;

  //! \brief SeekTo
  //!
  //! \return RenderStatus
  //!         RENDER_STATUS_OK if success, RENDER_EOS if reach EOS
  //!
  virtual RenderStatus SeekTo(uint64_t pts) override;

  //! \brief Pause
  //!
  //! \return RenderStatus
  //!         RENDER_STATUS_OK if success
  //!
  virtual RenderStatus Pause() override;

  //! \brief Play
  //!
  //! \return RenderStatus
  //!         RENDER_STATUS_OK if success
  //!
  virtual RenderStatus Play() override;

  bool OnVideoPacket(
      std::unique_ptr<owt::base::VideoEncodedFrame> frame) override;

 private:
  void parseOptions();
  void setMediaInfo();
  void initDump();
  void dump(const uint8_t* buf, int len, FILE* fp);

  int32_t m_source_width;
  int32_t m_source_height;
  int32_t m_source_framerate_den;
  int32_t m_source_framerate_num;

  std::string m_serverAddress;
  std::shared_ptr<owt::conference::ConferenceClient> m_room;
  std::string m_roomId;
  std::shared_ptr<owt::conference::RemoteMixedStream> m_mixed_stream;
  std::string m_subId;
  int m_yaw;
  int m_pitch;
  uint32_t m_frame_count;
  std::mutex m_mutex;
  std::condition_variable m_cond;

  owt::base::VideoDecoderInterface* m_rtcp_feedback;

  std::shared_ptr<DecoderManager> m_DecoderManager;

  // 360scvp
  param_360SCVP m_parserRWPKParam;
  void* m_parserRWPKHandle;

  bool m_ready;
  bool m_enableBsDump;
  FILE* m_bsDumpfp;
};

class SimpleBuffer {
 public:
  SimpleBuffer();
  virtual ~SimpleBuffer();

  void insert(const uint8_t* data, int size);
  void resize(int new_size) { m_size = new_size <= m_max_size ? new_size : 0; }

  uint8_t* data() { return m_data; }
  int size() { return m_size; }

 private:
  uint8_t* m_data;
  int m_size;
  int m_max_size;
};

class WebRTCVideoDecoderAdapter : public owt::base::VideoDecoderInterface {
 public:
  WebRTCVideoDecoderAdapter(WebRTCVideoPacketListener* listener);
  ~WebRTCVideoDecoderAdapter();

  bool InitDecodeContext(owt::base::VideoCodec video_codec) override;
  owt::base::VideoDecoderInterface* Copy() override;
  bool Release() override;

  bool OnEncodedFrame(
      std::unique_ptr<owt::base::VideoEncodedFrame> frame) override;

 private:
  int32_t m_ref_count;

  WebRTCVideoPacketListener* m_listener;
};

VCD_NS_END;
#endif /* _WebRTCMediaSource_H_ */

#endif /* _ENABLE_WEBRTC_SOURCE_ */
