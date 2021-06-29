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
//! \file     DashMediaSource.cpp
//! \brief    Implement class for DashMediaSource.
//!
#ifdef _ENABLE_DASH_SOURCE_
#include "DashMediaSource.h"
#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>
#include "../../../utils/tinyxml2.h"
#include "../Common/RenderType.h"
#include "OmafDashAccessApi.h"
#ifndef _ANDROID_OS_
#ifdef _USE_TRACE_
#include "../../../trace/MtHQ_tp.h"
#include "../../../trace/Bandwidth_tp.h"
#endif
#endif
#define MAX_LIST_NUMBER 30
#define MIN_LIST_REMAIN 2
#define DECODE_THREAD_COUNT 16
#define MAX_PACKETS 16
#define WAIT_PACKET_TIME_OUT 10000 // 10s
#define TEST_GET_PACKET_ONLY 0
#define MAX_DUMP_FILE_NUM 10
using namespace tinyxml2;

VCD_NS_BEGIN

DashMediaSource::DashMediaSource() {
  pthread_mutex_init(&m_frameMutex, NULL);
  m_status = STATUS_UNKNOWN;
  m_handler = NULL;
  m_DecoderManager = NULL;
  m_needStreamDumped = false;
  m_maxVideoWidth = 0;
  m_maxVideoHeight = 0;
#ifndef _ANDROID_OS_
  GetStreamDumpedOptionParams();
#endif
  m_singleFile = NULL;
  if (m_needStreamDumped) {
    for (uint32_t i = 0; i < MAX_DUMP_FILE_NUM; i++)
    {
      m_singleFile = fopen(string("video" + to_string(i) + ".h265").c_str(), "wb");
      if (NULL == m_singleFile) LOG(ERROR) << "Failed to open stream dumped file!" << endl;
      m_dumpedFile.push_back(std::move(m_singleFile));
      m_singleFile = nullptr;
    }
  }
}

DashMediaSource::~DashMediaSource() {
  m_status = STATUS_STOPPED;
  SAFE_DELETE(m_DecoderManager);
  if (m_needStreamDumped && !m_dumpedFile.empty()) {
    for (FILE* file : m_dumpedFile)
    {
      if (file)
      {
        fclose(file);
        file = NULL;
      }
    }
    m_dumpedFile.clear();
  }
  if (m_singleFile)
  {
    fclose(m_singleFile);
    m_singleFile = NULL;
  }
  OmafAccess_CloseMedia(m_handler);
  OmafAccess_Close(m_handler);
}

RenderStatus DashMediaSource::Initialize(struct RenderConfig renderConfig, RenderSourceFactory *rsFactory) {
  if (nullptr == renderConfig.url) {
    return RENDER_ERROR;
  }
  // 1.initial DashStreaming
  DashStreamingClient *pCtxDashStreaming = (DashStreamingClient *)malloc(sizeof(DashStreamingClient));
  if (NULL == pCtxDashStreaming) {
    return RENDER_ERROR;
  }
  pCtxDashStreaming->media_url = renderConfig.url;
  pCtxDashStreaming->cache_path = renderConfig.cachePath;
  pCtxDashStreaming->source_type = MultiResSource;
  pCtxDashStreaming->enable_extractor = renderConfig.enableExtractor;
  pCtxDashStreaming->log_callback = NULL;

  // init the omaf params
  memset(&pCtxDashStreaming->omaf_params, 0, sizeof(pCtxDashStreaming->omaf_params));
  pCtxDashStreaming->omaf_params.http_params.ssl_verify_host = 0;
  pCtxDashStreaming->omaf_params.http_params.ssl_verify_peer = 0;
  pCtxDashStreaming->omaf_params.http_params.conn_timeout = -1;  // not set
  pCtxDashStreaming->omaf_params.http_params.retry_times = 3;
  pCtxDashStreaming->omaf_params.http_params.total_timeout = -1;  // not set

  pCtxDashStreaming->omaf_params.max_parallel_transfers = 256;
  pCtxDashStreaming->omaf_params.segment_open_timeout_ms = 3000;           // ms
  pCtxDashStreaming->omaf_params.statistic_params.enable = 0;              // enable statistic
  pCtxDashStreaming->omaf_params.statistic_params.window_size_ms = 10000;  // ms

  pCtxDashStreaming->omaf_params.synchronizer_params.enable = 0;               //  enable dash segment number syncer
  pCtxDashStreaming->omaf_params.synchronizer_params.segment_range_size = 20;  // 20
  pCtxDashStreaming->omaf_params.max_decode_width = renderConfig.maxVideoDecodeWidth;
  pCtxDashStreaming->omaf_params.max_decode_height = renderConfig.maxVideoDecodeHeight;
  pCtxDashStreaming->omaf_params.enable_in_time_viewport_update = renderConfig.enableInTimeViewportUpdate;
  pCtxDashStreaming->omaf_params.max_response_times_in_seg = renderConfig.maxResponseTimesInOneSeg;
  pCtxDashStreaming->omaf_params.max_catchup_width = renderConfig.maxCatchupWidth;
  pCtxDashStreaming->omaf_params.max_catchup_height = renderConfig.maxCatchupHeight;
  m_maxVideoWidth = renderConfig.maxVideoDecodeWidth;
  m_maxVideoHeight = renderConfig.maxVideoDecodeHeight;
  PluginDef def;
  def.pluginLibPath = renderConfig.pathof360SCVPPlugin;
  pCtxDashStreaming->plugin_def = def;
  m_handler = OmafAccess_Init(pCtxDashStreaming);
  if (NULL == m_handler) {
    LOG(ERROR) << "handler init failed!" << std::endl;
    free(pCtxDashStreaming);
    pCtxDashStreaming = NULL;
    return RENDER_ERROR;
  }
  // 2. initial viewport.
  HeadSetInfo clientInfo;
  clientInfo.pose = (HeadPose *)malloc(sizeof(HeadPose));
  if (NULL == clientInfo.pose) {
    LOG(ERROR) << "client info malloc failed!" << std::endl;
    free(pCtxDashStreaming);
    pCtxDashStreaming = NULL;
    return RENDER_ERROR;
  }
  clientInfo.pose->yaw = 0;
  clientInfo.pose->pitch = 0;
  clientInfo.pose->centerX = 0;
  clientInfo.pose->centerY = 0;
  clientInfo.pose->speed = 0.0f;
  clientInfo.pose->zoomFactor = 1.0f;
  clientInfo.pose->viewOrient.mode = ORIENT_NONE;
  clientInfo.pose->viewOrient.orientation = 0.0f;
  clientInfo.pose->pts = 0;
  clientInfo.viewPort_hFOV = renderConfig.viewportHFOV;
  clientInfo.viewPort_vFOV = renderConfig.viewportVFOV;
  clientInfo.viewPort_Width = renderConfig.viewportWidth;
  clientInfo.viewPort_Height = renderConfig.viewportHeight;
  OmafAccess_SetupHeadSetInfo(m_handler, &clientInfo);
  // 3.load media source
  if (ERROR_NONE != OmafAccess_OpenMedia(m_handler, pCtxDashStreaming, renderConfig.enablePredictor,
                                         (char *)renderConfig.predictPluginName,
                                         (char *)renderConfig.libPath)) {
    LOG(ERROR) << "Open media failed!" << std::endl;
    free(pCtxDashStreaming);
    pCtxDashStreaming = NULL;
    free(clientInfo.pose);
    clientInfo.pose = NULL;
    return RENDER_ERROR;
  }
  // 4. add extra information in mediaInfo for render.
  DashMediaInfo mediaInfo;
  OmafAccess_GetMediaInfo(m_handler, &mediaInfo);

  m_rsFactory = rsFactory;

  SetMediaInfo(&mediaInfo);

  SAFE_DELETE(m_DecoderManager);
  m_DecoderManager = new DecoderManager();
#ifdef _ANDROID_OS_
  if (m_nativeSurface.empty()) ANDROID_LOGD("native surface is null in dash media source");

  for (uint32_t i = 0; i < m_nativeSurface.size(); i++)
  {
    // ANDROID_LOGD("dash media source : set surface at i : %d surface is %p", m_nativeSurface[i].first, m_nativeSurface[i].second);
    m_DecoderManager->SetSurface(i, m_nativeSurface[i].first, m_nativeSurface[i].second);
  }
#endif
  DecodeInfo decode_info;
  decode_info.frameRate_den = mediaInfo.stream_info[0].framerate_den;
  decode_info.frameRate_num = mediaInfo.stream_info[0].framerate_num;
  decode_info.segment_duration = mediaInfo.stream_info[0].segmentDuration;
  m_DecoderManager->SetDecodeInfo(decode_info);
  RenderStatus ret = m_DecoderManager->Initialize(m_rsFactory);
  if (RENDER_STATUS_OK != ret) {
    LOG(INFO) << "m_DecoderManager::Initialize failed" << std::endl;
  }

  m_sourceType = (MediaSourceType::Enum)mediaInfo.streaming_type;
  StartThread();
  m_status = STATUS_CREATED;
  free(pCtxDashStreaming);
  pCtxDashStreaming = NULL;
  free(clientInfo.pose);
  clientInfo.pose = NULL;
  SAFE_DELETE_ARRAY(mediaInfo.stream_info[0].codec);
  SAFE_DELETE_ARRAY(mediaInfo.stream_info[0].mime_type);
  return RENDER_STATUS_OK;
}

RenderStatus DashMediaSource::Start()
{
  if (OmafAccess_StartStreaming(m_handler) != ERROR_NONE)
  {
    return RENDER_ERROR;
  }
  return RENDER_STATUS_OK;
}

RenderStatus DashMediaSource::GetStreamDumpedOptionParams() {
  XMLDocument config;
  config.LoadFile("config.xml");
  XMLElement *info = config.RootElement();
  if (NULL == info)
  {
    LOG(ERROR) << " XML parse failed! " << std::endl;
    return RENDER_ERROR;
  }
  XMLElement *dumpedElem = info->FirstChildElement("StreamDumpedOption");
  if (NULL == dumpedElem) {
    m_needStreamDumped = false;
    return RENDER_ERROR;
  }
  m_needStreamDumped = atoi(dumpedElem->GetText());
  return RENDER_STATUS_OK;
}

void DashMediaSource::SetActiveStream(int32_t video_id, int32_t audio_id) {
  mMediaInfo.SetActiveAudio(audio_id);
  mMediaInfo.SetActiveVideo(video_id);
}

RenderStatus DashMediaSource::SetMediaInfo(void *mediaInfo) {
  DashMediaInfo *dashMediaInfo = (DashMediaInfo *)mediaInfo;

  mMediaInfo.mDuration = dashMediaInfo->duration;
  mMediaInfo.mStreamingType = dashMediaInfo->streaming_type;
  VideoInfo vi;
  AudioInfo ai;
  int32_t vidx = 0;
  int32_t aidx = 0;
  for (int i = 0; i < dashMediaInfo->stream_count; i++) {
    switch (dashMediaInfo->stream_info[i].stream_type) {
      case MediaType_Video:
        vi.streamID = i;
        vi.bit_rate = dashMediaInfo->stream_info[i].bit_rate;
        vi.codec = dashMediaInfo->stream_info[i].codec;
        vi.codec_type = dashMediaInfo->stream_info[i].codec_type;
        vi.framerate_den = dashMediaInfo->stream_info[i].framerate_den;
        vi.framerate_num = dashMediaInfo->stream_info[i].framerate_num;
        vi.height = dashMediaInfo->stream_info[i].height;
        vi.mFpt = dashMediaInfo->stream_info[i].mFpt;
        vi.mime_type = dashMediaInfo->stream_info[i].mime_type;
        vi.mProjFormat = dashMediaInfo->stream_info[i].mProjFormat;
        vi.width = dashMediaInfo->stream_info[i].width;
        vi.sourceHighTileRow = dashMediaInfo->stream_info[i].tileRowNum;
        vi.sourceHighTileCol = dashMediaInfo->stream_info[i].tileColNum;
        vi.mPixFmt = PixelFormat::PIX_FMT_YUV420P;
        vi.source_number = dashMediaInfo->stream_info[i].source_number;
        vi.source_resolution = dashMediaInfo->stream_info[i].source_resolution;
        mMediaInfo.AddVideoInfo(vidx, vi);
        vidx++;
        break;
      case MediaType_Audio:
        ai.streamID = i;
        ai.bit_rate = dashMediaInfo->stream_info[i].bit_rate;
        ai.channel_bytes = dashMediaInfo->stream_info[i].channel_bytes;
        ai.channels = dashMediaInfo->stream_info[i].channels;
        ai.codec = dashMediaInfo->stream_info[i].codec;
        ai.codec_type = dashMediaInfo->stream_info[i].codec_type;
        ai.mime_type = dashMediaInfo->stream_info[i].mime_type;
        ai.sample_rate = dashMediaInfo->stream_info[i].sample_rate;
        mMediaInfo.AddAudioInfo(aidx, ai);
        aidx++;
        break;
      default:
        break;
    }
  }
  mMediaInfo.GetActiveAudioInfo(ai);
  mMediaInfo.GetActiveVideoInfo(vi);

  if (NULL != this->m_rsFactory) {
    m_rsFactory->SetSourceResolution(vi.source_number, vi.source_resolution);
    m_rsFactory->SetProjectionFormat(vi.mProjFormat);
    m_rsFactory->SetHighTileRow(vi.sourceHighTileRow);
    m_rsFactory->SetHighTileCol(vi.sourceHighTileCol);
  }

  LOG(INFO) << "------------------------------------------" << std::endl;
  LOG(INFO) << "Player [config]: fps               " << vi.framerate_num / vi.framerate_den << std::endl;
  // LOG(INFO)<<"Player [config]: render resolution
  // "<<m_mediaSourceInfo.sourceWH->width[0]<<"x"<<m_mediaSourceInfo.sourceWH->height[0]<<std::endl;
  LOG(INFO) << "Player [config]: packed resolution " << vi.width << "x" << vi.height << std::endl;
  LOG(INFO) << "------------------------------------------" << std::endl;
  // trace
  if (dashMediaInfo->streaming_type != 1 && dashMediaInfo->streaming_type != 2) {
    LOG(ERROR) << "dash mode is invalid!" << std::endl;
  }
#ifndef _ANDROID_OS_
#ifdef _USE_TRACE_
  int32_t frameNum = round(float(mMediaInfo.mDuration) / 1000 * (vi.framerate_num / vi.framerate_den));
  const char *dash_mode = (dashMediaInfo->streaming_type == 1) ? "static" : "dynamic";
  tracepoint(mthq_tp_provider, stream_information, (char *)dash_mode, vi.mProjFormat, dashMediaInfo->stream_info[0].segmentDuration,
             dashMediaInfo->duration, vi.framerate_num / vi.framerate_den, frameNum, vi.width, vi.height);
  tracepoint(bandwidth_tp_provider, segmentation_info, (char *)dash_mode, dashMediaInfo->stream_info[0].segmentDuration, vi.framerate_num / vi.framerate_den, (uint32_t)dashMediaInfo->stream_count, (uint64_t*)&(dashMediaInfo->stream_info[0].bit_rate), frameNum, mMediaInfo.mDuration/1000);
#endif
#endif
  return RENDER_STATUS_OK;
}

bool DashMediaSource::IsEOS() {
  if (STATUS_TIMEOUT == m_status)
  {
    LOG(INFO) << "The status is time out! EOS occurs!" << endl;
    return true;
  }
  if (m_sourceType == 1)  // vod
  {
    // return m_bEOS;
    return m_DecoderManager->GetEOS();
  }
  return false;  // vod return false or live always false
}

RenderStatus DashMediaSource::ChangeViewport(HeadPose *pose) {
  LOG(INFO) << "ChangeViewport pose centX " << pose->centerX << " pose centY " << pose->centerY << endl;
  OmafAccess_ChangeViewport(m_handler, pose);
  return RENDER_STATUS_OK;
}

void DashMediaSource::ProcessVideoPacket() {
  VideoInfo vi;
  mMediaInfo.GetActiveVideoInfo(vi);
  // 1. get one packet from DashStreaming lib.
  DashPacket dashPkt[MAX_PACKETS];
  memset(dashPkt, 0, MAX_PACKETS * sizeof(DashPacket));
  int dashPktNum = 0;
  static bool needHeaders = true;
  static uint64_t currentWaitTime = 0;
  uint64_t pts = 0;
  int ret =
      OmafAccess_GetPacket(m_handler, vi.streamID, &(dashPkt[0]), &dashPktNum, (uint64_t *)&pts, needHeaders, false);
  if (ERROR_NONE != ret) {
    // LOG(INFO) << "Get packet failed: stream_id:" << vi.streamID << ", ret:" << ret << std::endl;
    currentWaitTime++;
    if (currentWaitTime > WAIT_PACKET_TIME_OUT) // wait 5s but get packet failed
    {
      m_status = STATUS_TIMEOUT;
#ifdef _ANDROID_OS_
      ANDROID_LOGD("Wait too long to get packet from Omaf Dash Access library! Force to quit!");
#endif
      LOG(ERROR) << " Wait too long to get packet from Omaf Dash Access library! Force to quit! " << std::endl;
    }
    return;
  }
  currentWaitTime = 0;
  if (dashPkt[0].bEOS && !dashPkt[0].bCatchup) {
    m_status = STATUS_STOPPED;
  }
  for (uint32_t i = 0; i < dashPktNum; i++) {
    LOG(INFO) << "[FrameSequences][Packet]: Get packet has done! and pts is " << dashPkt[i].pts  << " video id " << dashPkt[i].videoID << " catch up flag is " << dashPkt[i].bCatchup << std::endl;
    // ANDROID_LOGD("Get packet has done! and pts is %lld, video id %d\n", dashPkt[i].pts, dashPkt[i].videoID);
  }
  if (!dashPkt[0].bCatchup) {
#ifndef _ANDROID_OS_
#ifdef _USE_TRACE_
  // trace
  tracepoint(mthq_tp_provider, T8_get_packet, dashPkt[0].pts);
#endif
#endif
  }
  if (m_needStreamDumped && !m_dumpedFile.empty()) {
    for (uint32_t i = 0; i < dashPktNum; i++)
    {
        fwrite(dashPkt[i].buf, 1, dashPkt[i].size, m_dumpedFile[dashPkt[i].videoID]);
    }
  }
#ifdef _ANDROID_OS_
  if (dashPktNum == 1 && (dashPkt[0].width > m_maxVideoWidth || dashPkt[0].height > m_maxVideoHeight) && !dashPkt[0].bCatchup) {//ET mode
    ANDROID_LOGD("Cannot start VR Player due to codec capacity! please check maxDecWidth/maxDecHeight settings! w %d, h %d", dashPkt[0].width, dashPkt[0].height);
    m_status = STATUS_STOPPED;
    return;
  }
#endif
#if !TEST_GET_PACKET_ONLY
  if (NULL != m_DecoderManager) {
    RenderStatus ret = m_DecoderManager->SendVideoPackets(&(dashPkt[0]), dashPktNum);
    // needHeaders = false;
    if (RENDER_STATUS_OK != ret) {
      LOG(INFO) << "m_DecoderManager::SendVideoPackets: stream_id:" << vi.streamID << " segment id" << dashPkt[0].segID
                << std::endl;
    }
  }
#endif
  for (int i = 0; i < dashPktNum; i++) {
    SAFE_FREE(dashPkt[i].buf);
    if (dashPkt[i].rwpk) SAFE_DELETE_ARRAY(dashPkt[i].rwpk->rectRegionPacking);
    SAFE_DELETE(dashPkt[i].rwpk);
    SAFE_DELETE_ARRAY(dashPkt[i].qtyResolution);
  }
}

void DashMediaSource::ProcessAudioPacket() {
  AudioInfo ai;
  mMediaInfo.GetActiveAudioInfo(ai);
}

void DashMediaSource::Run() {
  if (NULL == m_handler) {
    return;
  }

  m_status = STATUS_PLAYING;
  while (m_status != STATUS_STOPPED && m_status != STATUS_TIMEOUT) {
    {
      ScopeLock lock(m_Lock);
      ProcessVideoPacket();
    }
    usleep(1000);
  }
}

RenderStatus DashMediaSource::UpdateFrames(uint64_t pts, int64_t *corr_pts) {
  if (NULL == m_DecoderManager) return RENDER_NO_MATCHED_DECODER;

  RenderStatus ret = m_DecoderManager->UpdateVideoFrames(pts, corr_pts);

  if (RENDER_STATUS_OK != ret) {
    LOG(INFO) << "DashMediaSource::UpdateFrames failed with code:" << ret << std::endl;
  }

  return ret;
}

RenderStatus DashMediaSource::GetFrame(uint8_t **buffer, struct RegionInfo *regionInfo) {
  *buffer = NULL;
  return RENDER_STATUS_OK;
}

RenderStatus DashMediaSource::SeekTo(uint64_t pts) {
  if (NULL == m_handler) return RENDER_NULL_HANDLE;
  if (NULL == m_DecoderManager) return RENDER_NULL_HANDLE;

  RenderStatus ret = RENDER_STATUS_OK;
  if (mMediaInfo.mStreamingType == 2)  // if live mode, pause isn't supported
  {
    ScopeLock lock(m_Lock);
    m_status = STATUS_PAUSED;
    ret = m_DecoderManager->ResetDecoders();
    if (RENDER_STATUS_OK != ret) return RENDER_SEEK_FAILURE;

    int res = OmafAccess_SeekMedia(m_handler, pts);
    if (ERROR_NONE != res) {
      LOG(INFO) << "DashMediaSource::SeekTo failed with code:" << res << std::endl;
      return RENDER_SEEK_FAILURE;
    }
    m_status = STATUS_PLAYING;
  }
  return ret;
}

RenderStatus DashMediaSource::Pause() {
  if (mMediaInfo.mStreamingType == 2)  // if live mode, pause isn't supported
  {
    ScopeLock lock(m_Lock);
    m_status = STATUS_PAUSED;
  }
  return RENDER_STATUS_OK;
}

RenderStatus DashMediaSource::Play() {
  ScopeLock lock(m_Lock);
  m_status = STATUS_PLAYING;
  return RENDER_STATUS_OK;
}

VCD_NS_END
#endif  // _ENABLE_DASH_SOURCE_
