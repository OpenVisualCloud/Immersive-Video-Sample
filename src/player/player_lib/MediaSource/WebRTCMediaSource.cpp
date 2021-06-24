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
//! \file     WebRTCMediaSource.cpp
//! \brief    Implement class for WebRTCMediaSource.
//!
#ifdef _ENABLE_WEBRTC_SOURCE_

#include "WebRTCMediaSource.h"
#include "../Common/Common.h"

#include <algorithm>

#include "http.h"
#include "owt/base/commontypes.h"
#include "owt/base/globalconfiguration.h"
#include "owt/base/network.h"
#include "owt/base/options.h"
#include "owt/conference/conferenceclient.h"
#include "owt/conference/remotemixedstream.h"

#include "../utils/tinyxml2.h"

VCD_NS_BEGIN
using namespace tinyxml2;

static int isValidStartCode(uint8_t* data, int length) {
  if (length < 3)
    return -1;

  if (data[0] == 0 && data[1] == 0 && data[2] == 1)
    return 3;

  if (length >= 4 && data[0] == 0 && data[1] == 0 && data[2] == 0 &&
      data[3] == 1)
    return 4;

  return 0;
}

static int findFirstNALU(uint8_t* data, int length, int* offset, int* size) {
  int i = 0;
  int ret = -1;
  int start_code_len = 0;
  int nalu_offset = 0;
  int nalu_type = -1;
  int nalu_size = 0;

  while (true) {
    ret = isValidStartCode(data + i, length - i);
    if (ret < 0)
      return -1;
    else if (ret > 0) {
      start_code_len = ret;
      nalu_offset = i;
      break;
    }

    ++i;
  }

  i += start_code_len;
  while (true) {
    ret = isValidStartCode(data + i, length - i);
    if (ret < 0) {
      nalu_size = length - nalu_offset;
      break;
    } else if (ret > 0) {
      nalu_size = i - nalu_offset;
      break;
    }

    ++i;
  }

  *offset = nalu_offset;
  *size = nalu_size;
  return (data[nalu_offset + start_code_len] & 0x7e) >> 1;
}

static int filterNALs(std::shared_ptr<SimpleBuffer> bitstream_buf,
                      const std::vector<int>& remove_types,
                      std::shared_ptr<SimpleBuffer> sei_buf) {
  int remove_nals_size = 0;

  uint8_t* buffer_start = bitstream_buf->data();
  int buffer_length = bitstream_buf->size();
  int nalu_offset = 0;
  int nalu_size = 0;
  int nalu_type;

  while (buffer_length > 0) {
    nalu_type =
        findFirstNALU(buffer_start, buffer_length, &nalu_offset, &nalu_size);
    if (nalu_type < 0)
      break;

    if (std::find(remove_types.begin(), remove_types.end(), nalu_type) !=
        remove_types.end()) {
      // copy
      sei_buf->insert(buffer_start + nalu_offset, nalu_size);

      // next
      memmove(buffer_start, buffer_start + nalu_offset + nalu_size,
              buffer_length - nalu_offset - nalu_size);
      buffer_length -= nalu_offset + nalu_size;

      remove_nals_size += nalu_offset + nalu_size;
      continue;
    }

    buffer_start += (nalu_offset + nalu_size);
    buffer_length -= (nalu_offset + nalu_size);
  }

  bitstream_buf->resize(bitstream_buf->size() - remove_nals_size);
  return bitstream_buf->size();
}

static void filter_RWPK_SEI(std::shared_ptr<SimpleBuffer> bitstream_buf,
                            std::shared_ptr<SimpleBuffer> sei_buf) {
  std::vector<int> sei_types;
  sei_types.push_back(38);
  sei_types.push_back(39);
  sei_types.push_back(30);

  filterNALs(bitstream_buf, sei_types, sei_buf);
}

RegionWisePacking* alloc_RegionWisePacking() {
  RegionWisePacking* rwpk = new RegionWisePacking();
  assert(rwpk != NULL);
  memset(rwpk, 0, sizeof(RegionWisePacking));

  rwpk->rectRegionPacking =
      new RectangularRegionWisePacking[DEFAULT_REGION_NUM];
  assert(rwpk->rectRegionPacking != NULL);

  return rwpk;
}

void free_RegionWisePacking(RegionWisePacking* rwpk) {
  if (rwpk)
    SAFE_DELETE_ARRAY(rwpk->rectRegionPacking);
  SAFE_DELETE(rwpk);
}

WebRTCMediaSource* WebRTCMediaSource::s_CurObj;

void WebRTCMediaSource::subscribe_on_success_callback(
    std::shared_ptr<owt::conference::ConferenceSubscription> sc) {
  LOG(INFO) << "subscribe_on_success_callback" << std::endl;

  s_CurObj->m_subId = sc->Id();

  std::unique_lock<std::mutex> ulock(s_CurObj->m_mutex);
  s_CurObj->m_ready = true;
  s_CurObj->m_cond.notify_all();
}

void WebRTCMediaSource::subscribe_on_failure_callback(
    std::unique_ptr<owt::base::Exception> err) {
  LOG(ERROR) << "subscribe_on_failure_callback: " << err->Message()
             << std::endl;
  exit(1);
}

void WebRTCMediaSource::join_on_success_callback(
    std::shared_ptr<owt::conference::ConferenceInfo> info) {
  LOG(INFO) << "join_on_success_callback" << std::endl;

  std::vector<std::shared_ptr<owt::base::RemoteStream>> remoteStreams =
      info->RemoteStreams();
  for (auto& remoteStream : remoteStreams) {
    if (remoteStream->Source().video == owt::base::VideoSourceInfo::kMixed) {
      s_CurObj->m_mixed_stream =
          static_pointer_cast<owt::conference::RemoteMixedStream>(remoteStream);
      break;
    }
  }

  if (!s_CurObj->m_mixed_stream) {
    LOG(ERROR) << "No mixed stream!" << std::endl;
    exit(1);
  }

  owt::base::VideoCodecParameters codecParams;
  codecParams.name = owt::base::VideoCodec::kH265;

  owt::conference::SubscribeOptions options;
  options.video.codecs.push_back(codecParams);

  options.video.resolution.width = s_CurObj->m_source_width;
  options.video.resolution.height = s_CurObj->m_source_height;

  LOG(INFO) << "Subscribe: " << options.video.resolution.width << "x"
            << options.video.resolution.height << std::endl;
  s_CurObj->m_roomId = info->Id();
  s_CurObj->m_room->Subscribe(s_CurObj->m_mixed_stream, options,
                              subscribe_on_success_callback,
                              subscribe_on_failure_callback);
}

void WebRTCMediaSource::join_on_failure_callback(
    std::unique_ptr<owt::base::Exception> err) {
  LOG(ERROR) << "join_on_failure_callback: " << err->Message() << std::endl;
  exit(1);
}

WebRTCMediaSource::WebRTCMediaSource()
    : m_source_width(0),
      m_source_height(0),
      m_source_framerate_den(1),
      m_source_framerate_num(30),
      m_frame_count(0),
      m_yaw(0),
      m_pitch(0),
      m_rtcp_feedback(nullptr),
      m_parserRWPKHandle(nullptr),
      m_ready(false),
      m_enableBsDump(false),
      m_bsDumpfp(nullptr) {

  LOG(INFO) << __FUNCTION__ << std::endl;

  s_CurObj = this;
  parseOptions();
}

WebRTCMediaSource::~WebRTCMediaSource() {
  LOG(INFO) << __FUNCTION__ << std::endl;

  SourceResolution *source_resolution = m_rsFactory->GetSourceResolution();
  SAFE_DELETE_ARRAY(source_resolution);

  if (m_parserRWPKHandle) {
    I360SCVP_unInit(m_parserRWPKHandle);
    m_parserRWPKHandle = nullptr;
  }

  if (m_bsDumpfp) {
    fclose(m_bsDumpfp);
    m_bsDumpfp = nullptr;
  }
}

void WebRTCMediaSource::parseOptions() {
  XMLDocument config;
  config.LoadFile("config.xml");
  XMLElement* info = config.RootElement();

  if (!strcmp(info->FirstChildElement("resolution")->GetText(), "8k")) {
    m_source_width = 7680;
    m_source_height = 3840;
  } else if (!strcmp(info->FirstChildElement("resolution")->GetText(), "4k")) {
    m_source_width = 3840;
    m_source_height = 2048;
  } else {
    LOG(ERROR) << "Resolution must be 4k or 8k!" << std::endl;
    exit(-1);
  }

  const char* server_url = info->FirstChildElement("server_url")->GetText();
  if (!server_url) {
    LOG(ERROR) << "Invalid server_url!" << std::endl;
    exit(-1);
  }
  m_serverAddress = std::string(server_url);

  if (!strcmp(info->FirstChildElement("enableDump")->GetText(), "true")) {
    m_enableBsDump = true;
  }

}

void WebRTCMediaSource::setMediaInfo() {
  mMediaInfo.mDuration = -1;
  mMediaInfo.mStreamingType = 2;

  VideoInfo vi;
  int32_t vidx = 0;

  vi.codec = "";
  vi.codec_type = VideoCodec_HEVC;
  vi.streamID = 0;
  vi.bit_rate = 30000000;
  vi.framerate_den = m_source_framerate_den;
  vi.framerate_num = m_source_framerate_num;
  vi.mFpt = 0;
  vi.mime_type = "";
  vi.mProjFormat = 0;
  vi.mPixFmt = PixelFormat::PIX_FMT_YUV420P;

  vi.width = m_source_width;
  vi.height = m_source_height;
  if (m_source_width == 7680 && m_source_height == 3840) {
    vi.sourceHighTileRow = 6;
    vi.sourceHighTileCol = 12;
  } else if (m_source_width == 3840 && m_source_height == 2048) {
    vi.sourceHighTileRow = 8;
    vi.sourceHighTileCol = 10;
  } else {
    vi.sourceHighTileRow = 0;
    vi.sourceHighTileCol = 0;

    LOG(ERROR) << "Unsupported resolution " << m_source_width << "x"
               << m_source_height << std::endl;
  }

  mMediaInfo.AddVideoInfo(vidx, vi);
  mMediaInfo.GetActiveVideoInfo(vi);

  if (NULL != this->m_rsFactory) {
    SourceResolution* source_resolution = new SourceResolution[1];
    source_resolution[0].qualityRanking = HIGHEST_QUALITY_RANKING;
    source_resolution[0].width = vi.width;
    source_resolution[0].height = vi.height;
    m_rsFactory->SetSourceResolution(1, source_resolution);

    m_rsFactory->SetHighTileRow(vi.sourceHighTileRow);
    m_rsFactory->SetHighTileCol(vi.sourceHighTileCol);
    m_rsFactory->SetProjectionFormat(vi.mProjFormat);
  }
}

void WebRTCMediaSource::initDump() {
  if (m_enableBsDump) {
    char dumpFileName[128];
    snprintf(dumpFileName, 128, "/tmp/webrtcsource-%p.%s", this, "hevc");
    m_bsDumpfp = fopen(dumpFileName, "wb");
    if (m_bsDumpfp) {
      LOG(INFO) << "Enable bitstream dump " << dumpFileName << std::endl;
    } else {
      LOG(ERROR) << "Can not open dump file " << dumpFileName << std::endl;
    }
  }
}

void WebRTCMediaSource::dump(const uint8_t* buf, int len, FILE* fp) {
  if (fp) {
    fwrite(buf, 1, len, fp);
    fflush(fp);
  }
}

RenderStatus WebRTCMediaSource::Initialize(struct RenderConfig renderConfig,
                                           RenderSourceFactory* rsFactory) {
  m_rsFactory = rsFactory;
  setMediaInfo();
  initDump();

  m_DecoderManager = std::make_shared<DecoderManager>();
  RenderStatus ret = m_DecoderManager->Initialize(rsFactory);
  if (RENDER_STATUS_OK != ret) {
    LOG(INFO) << "m_DecoderManager::Initialize failed" << std::endl;

    m_DecoderManager.reset();
  }

  memset(&m_parserRWPKParam, 0, sizeof(m_parserRWPKParam));
  m_parserRWPKParam.usedType = E_PARSER_FOR_CLIENT;
  m_parserRWPKHandle = I360SCVP_Init(&m_parserRWPKParam);

  owt::base::GlobalConfiguration::SetEncodedVideoFrameEnabled(true);
  unique_ptr<owt::base::VideoDecoderInterface> decoder(
      new WebRTCVideoDecoderAdapter(this));
  m_rtcp_feedback = decoder.get();
  owt::base::GlobalConfiguration::SetCustomizedVideoDecoderEnabled(
      std::move(decoder));

  owt::conference::ConferenceClientConfiguration configuration;

  owt::base::IceServer ice;
  ice.urls.push_back("stun:61.152.239.56");
  ice.username = "";
  ice.password = "";
  std::vector<owt::base::IceServer> ice_servers;
  ice_servers.push_back(ice);

  configuration.ice_servers = ice_servers;

  string roomId = "";
  string token = CHttp::getToken(m_serverAddress, roomId);

  if (token == "") {
    LOG(ERROR) << "invalid token!" << std::endl;
    return RENDER_ERROR;
  }
  m_room = owt::conference::ConferenceClient::Create(configuration);

  m_room->Join(token, join_on_success_callback, join_on_failure_callback);

  {
    std::unique_lock<std::mutex> ulock(m_mutex);
    while (!m_ready)
      m_cond.wait(ulock);
  }

  isAllValid = true;
  LOG(INFO) << "Initialized!" << std::endl;
  return RENDER_STATUS_OK;
}

RenderStatus WebRTCMediaSource::GetFrame(uint8_t** buffer,
                                         struct RegionInfo* regionInfo) {
  *buffer = NULL;
  return RENDER_STATUS_OK;
}

RenderStatus WebRTCMediaSource::GetAudioFrame(int64_t pts, uint8_t** buffer) {
  return RENDER_ERROR;
}

bool WebRTCMediaSource::IsEOS() {
  return false;
}

RenderStatus WebRTCMediaSource::ChangeViewport(HeadPose* pose) {
  if ((int)pose->yaw == m_yaw && (int)pose->pitch == m_pitch)
    return RENDER_STATUS_OK;

  LOG(INFO) << "yaw: " << pose->yaw << ", pitch: " << pose->pitch << std::endl;

  m_yaw = pose->yaw;
  m_pitch = pose->pitch;

  int yawValue = m_yaw + 180;
  int pitchValue = m_pitch + 90;

  if (m_rtcp_feedback) {
    m_rtcp_feedback->SendFOVFeedback(yawValue, pitchValue);
  }

  return RENDER_STATUS_OK;
}

RenderStatus WebRTCMediaSource::Start() {

  return RENDER_STATUS_OK;
}


RenderStatus WebRTCMediaSource::UpdateFrames(uint64_t pts, int64_t *corr_pts) {
  if (NULL == m_DecoderManager)
    return RENDER_NO_MATCHED_DECODER;

  RenderStatus ret = m_DecoderManager->UpdateVideoFrames(pts, corr_pts);
  if (RENDER_STATUS_OK != ret) {
    LOG(INFO) << "UpdateFrames failed with code:" << ret << std::endl;
  }

  return ret;
}

RenderStatus WebRTCMediaSource::SeekTo(uint64_t pts) {
  return RENDER_STATUS_OK;
}

RenderStatus WebRTCMediaSource::Pause() {
  return RENDER_STATUS_OK;
}

RenderStatus WebRTCMediaSource::Play() {
  return RENDER_STATUS_OK;
}

bool WebRTCMediaSource::OnVideoPacket(
    std::unique_ptr<owt::base::VideoEncodedFrame> frame) {
  LOG(INFO) << __FUNCTION__ << std::endl;

  if (NULL == m_DecoderManager)
    return true;

  std::shared_ptr<SimpleBuffer> bitstream_buf =
      std::make_shared<SimpleBuffer>();
  bitstream_buf->insert(frame->buffer, frame->length);
  if (m_enableBsDump) {
    dump(frame->buffer, frame->length, m_bsDumpfp);
  }

  std::shared_ptr<SimpleBuffer> sei_buf = std::make_shared<SimpleBuffer>();
  filter_RWPK_SEI(bitstream_buf, sei_buf);
  if (sei_buf->size() <= 0) {
    LOG(ERROR) << "No valid rwpk sei in bitstream!" << std::endl;
    return false;
  }

  RegionWisePacking* rwpk = alloc_RegionWisePacking();
  assert(rwpk != NULL);

  I360SCVP_ParseRWPK(m_parserRWPKHandle, rwpk, sei_buf->data(),
                     sei_buf->size());

  // just workaround
  int temp = rwpk->lowResPicHeight;
  rwpk->lowResPicHeight = rwpk->lowResPicWidth;
  rwpk->lowResPicWidth = temp;

  DashPacket dashPkt;
  dashPkt.videoID = 0;
  dashPkt.video_codec = VideoCodec_HEVC;
  dashPkt.segID = 0;
  dashPkt.numQuality = 2;

  dashPkt.qtyResolution = new SourceResolution[dashPkt.numQuality];
  dashPkt.qtyResolution[0].height = rwpk->packedPicHeight;
  dashPkt.qtyResolution[0].width = rwpk->packedPicWidth - rwpk->lowResPicWidth;
  dashPkt.qtyResolution[0].left = 0;
  dashPkt.qtyResolution[0].top = 0;
  dashPkt.qtyResolution[0].qualityRanking = static_cast<QualityRank>(1);

  dashPkt.qtyResolution[1].height = rwpk->lowResPicHeight;
  dashPkt.qtyResolution[1].width = rwpk->lowResPicWidth;
  dashPkt.qtyResolution[1].left = rwpk->packedPicWidth - rwpk->lowResPicWidth;
  dashPkt.qtyResolution[1].top = 0;
  dashPkt.qtyResolution[1].qualityRanking = static_cast<QualityRank>(2);

  dashPkt.height = rwpk->packedPicHeight;
  dashPkt.width = rwpk->packedPicWidth;

  char* buf = (char*)malloc(bitstream_buf->size());
  memcpy(buf, bitstream_buf->data(), bitstream_buf->size());

  dashPkt.buf = buf;
  dashPkt.size = bitstream_buf->size();
  dashPkt.pts = m_frame_count++; // TODO: consider how to use the frame->time_stamp propertly in future.
  dashPkt.rwpk = rwpk;
  dashPkt.bEOS = false;
  dashPkt.bCatchup = false;

  RenderStatus ret = m_DecoderManager->SendVideoPackets(&dashPkt, 1);
  if (RENDER_STATUS_OK != ret) {
    LOG(ERROR) << "m_DecoderManager::SendVideoPackets" << std::endl;
  }

  SAFE_FREE(dashPkt.buf);
  SAFE_DELETE_ARRAY(dashPkt.qtyResolution);
  free_RegionWisePacking(dashPkt.rwpk);
  return true;
}

SimpleBuffer::SimpleBuffer() : m_data(NULL), m_size(0), m_max_size(0) {}

SimpleBuffer::~SimpleBuffer() {
  if (m_data)
    free(m_data);
}

void SimpleBuffer::insert(const uint8_t* data, int size) {
  if (!m_data) {
    m_max_size = 1024 * 4;
    m_data = (uint8_t*)malloc(m_max_size);
  }

  if (size > m_max_size - m_size) {
    int new_max_size = m_max_size;
    while (size > new_max_size - m_size) {
      new_max_size += 1024;
    }
    m_data = (uint8_t*)realloc(m_data, new_max_size);
    m_max_size = new_max_size;
  }

  memcpy(m_data + m_size, data, size);
  m_size += size;
}

WebRTCVideoDecoderAdapter::WebRTCVideoDecoderAdapter(
    WebRTCVideoPacketListener* listener)
    : m_ref_count(0), m_listener(listener) {}

owt::base::VideoDecoderInterface* WebRTCVideoDecoderAdapter::Copy() {
  assert(m_ref_count == 0);
  m_ref_count++;
  return this;
}

bool WebRTCVideoDecoderAdapter::InitDecodeContext(
    owt::base::VideoCodec video_codec) {
  LOG(INFO) << __FUNCTION__ << std::endl;
  return true;
}

bool WebRTCVideoDecoderAdapter::Release() {
  LOG(INFO) << __FUNCTION__ << std::endl;
  m_ref_count--;
  return true;
}

WebRTCVideoDecoderAdapter::~WebRTCVideoDecoderAdapter() {
  LOG(INFO) << __FUNCTION__ << std::endl;
}

bool WebRTCVideoDecoderAdapter::OnEncodedFrame(
    std::unique_ptr<owt::base::VideoEncodedFrame> frame) {
  if (m_listener)
    return m_listener->OnVideoPacket(std::move(frame));

  return true;
}

VCD_NS_END

#endif /* _ENABLE_WEBRTC_SOURCE_ */
