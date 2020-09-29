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

#include "OmafDashSource.h"
#include <dirent.h>
#include <math.h>
#include <string.h>
#include "OmafExtractorTracksSelector.h"
#include "OmafReaderManager.h"
#include "OmafTileTracksSelector.h"
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
#include <sys/time.h>
#include "../trace/Bandwidth_tp.h"
#include "../trace/MtHQ_tp.h"
#endif
#endif

VCD_OMAF_BEGIN

#define MAX_CACHE_SIZE 100 * 1024 * 1024

OmafDashSource::OmafDashSource() {
  mMPDParser = nullptr;
  mStatus = STATUS_CREATED;
  mViewPortChanged = false;
  memset(&mHeadSetInfo, 0, sizeof(mHeadSetInfo));
  memset(&mPose, 0, sizeof(mPose));
  mLoop = false;
  mEOS = false;
  // mSelector = new OmafExtractorSelector();
  m_selector = nullptr;
  mMPDinfo = nullptr;
  dcount = 1;
  mPreExtractorID = 0;
  m_stitch = nullptr;
}

OmafDashSource::~OmafDashSource() {
  SAFE_DELETE(mMPDParser);
  // SAFE_DELETE(mSelector);
  SAFE_DELETE(m_selector);
  SAFE_DELETE(mMPDinfo);
  mViewPorts.clear();
  ClearStreams();
  SAFE_DELETE(m_stitch);
}

int OmafDashSource::SyncTime(std::string url) {
  // base URL should be "http://IP:port/FilePrefix/"
  std::size_t posf = url.find(":");
  std::size_t poss = url.find(":", posf + 1);
  if (poss == string::npos) {
    OMAF_LOG(LOG_ERROR, "Failed to find IP port in baseURL!\n");
    return ERROR_INVALID;
  }
  std::size_t pos = url.find("/", poss + 1);
  if (pos == string::npos) {
    OMAF_LOG(LOG_ERROR, "Failed to find file prefix in baseURL!\n");
    return ERROR_INVALID;
  }

  std::string addr = url.substr(0, pos);
  bool isHttps = addr.find("https") == std::string::npos ? false : true;
  string curlOption = isHttps ? "-k" : "-s";

  // get remote machine time by getting http server header file with curl
  string cmd = "sudo date --set=\"$(curl " + curlOption + " --head " + addr +
               " | grep \"Date:\" |sed 's/Date: [A-Z][a-z][a-z], //g'| sed 's/\r//')\"";
  int ret = system(cmd.c_str());
  if (ret) OMAF_LOG(LOG_WARNING, "Please run as root user!\n");

  return ret;
}

int OmafDashSource::OpenMedia(std::string url, std::string cacheDir, void* externalLog, bool enableExtractor,
                              bool enablePredictor, std::string predictPluginName, std::string libPath) {
  if (externalLog)
    logCallBack = (LogFunction)externalLog;
  else
    logCallBack = GlogFunction;

  DIR* dir = opendir(cacheDir.c_str());
  if (dir) {
    closedir(dir);
  } else {
    OMAF_LOG(LOG_INFO, "Failed to open the cache path: %s, create a folder with this path!\n", cacheDir.c_str());
    int checkdir = mkdir(cacheDir.c_str(), 0777);
    if (checkdir) {
      OMAF_LOG(LOG_ERROR, "Uable to create cache path: %s\n", cacheDir.c_str());
      return ERROR_INVALID;
    }
  }
  // FIXME, support more platform
  if (cacheDir.size() && cacheDir[cacheDir.size() - 1] == '/') {
    cacheDir = cacheDir.substr(0, cacheDir.size() - 1);
  }
  OMAF_LOG(LOG_INFO, "Now open media !\n");
  const char* strHTTP = "http://";
  const char* strHTTPS = "https://";
  uint32_t httpLen = strlen(strHTTP);
  uint32_t httpsLen = strlen(strHTTPS);

  bool isLocalMedia = false;

  if (0 != strncmp(url.c_str(), strHTTP, httpLen) && 0 != strncmp(url.c_str(), strHTTPS, httpsLen)) {
    isLocalMedia = true;
  }

  /// init download manager
  SAFE_DELETE(mMPDParser);
  int ret = ERROR_NONE;
  DownloadManager* pDM = DOWNLOADMANAGER::GetInstance();

  if (!isLocalMedia) {
    pDM->SetMaxCacheSize(MAX_CACHE_SIZE);

    pDM->SetCacheFolder(cacheDir);

    OmafDashSegmentHttpClient::Ptr http_source =
        OmafDashSegmentHttpClient::create(omaf_dash_params_.max_parallel_transfers_);
    if (http_source) {
      http_source->setProxy(omaf_dash_params_.http_proxy_);
      http_source->setParams(omaf_dash_params_.http_params_);
      if (omaf_dash_params_.stats_params_.enable_) {
        http_source->setStatisticsWindows(omaf_dash_params_.stats_params_.window_size_ms_);
      }
    }
    else
    {
      OMAF_LOG(LOG_ERROR, "http source failed to create!\n");
      return ERROR_NULL_PTR;
    }
    ret = http_source->start();
    if (ERROR_NONE != ret) {
      OMAF_LOG(LOG_ERROR, "Failed to start the client for omaf dash segment http source, err=%d\n", ret);
      return ret;
    }

    dash_client_ = std::move(http_source);
  }

  mMPDParser = new OmafMPDParser();
  if (nullptr == mMPDParser) return ERROR_NULL_PTR;
  mMPDParser->SetOmafDashParams(omaf_dash_params_);
  mMPDParser->SetExtractorEnabled(enableExtractor);

  OMAFSTREAMS listStream;
  mMPDParser->SetCacheDir(cacheDir);
  ret = mMPDParser->ParseMPD(url, listStream);
  if (ret == OMAF_INVALID_EXTRACTOR_ENABLEMENT) {
    enableExtractor = false;  //! enableExtractor;
    mMPDParser->SetExtractorEnabled(enableExtractor);
  } else if (ret != ERROR_NONE)
    return ret;

  mMPDinfo = this->GetMPDInfo();

  ProjectionFormat projFmt = mMPDParser->GetProjectionFmt();
  std::string projStr;
  if (projFmt == ProjectionFormat::PF_ERP) {
    projStr = "ERP";
  } else if (projFmt == ProjectionFormat::PF_CUBEMAP) {
    projStr = "CubeMap";
  } else {
    OMAF_LOG(LOG_ERROR, "Invalid projection format !\n");
    return OMAF_ERROR_INVALID_PROJECTIONTYPE;
  }
  OMAF_LOG(LOG_INFO, "The DASH Source is from %s projection !\n", projStr.c_str());
  if (!isLocalMedia) {
    // base URL should be "http://IP:port/FilePrefix/"
    std::size_t pos = mMPDinfo->baseURL[0].find(":");
    pos = mMPDinfo->baseURL[0].find(":", pos + 1);
    if (pos == string::npos) {
      OMAF_LOG(LOG_ERROR, "Failed to find IP port in baseURL!\n");
      return ERROR_INVALID;
    }
    pos = mMPDinfo->baseURL[0].find("/", pos + 1);
    if (pos == string::npos) {
      OMAF_LOG(LOG_ERROR, "Failed to find file prefix in baseURL!\n");
      return ERROR_INVALID;
    }

    std::string prefix = mMPDinfo->baseURL[0].substr(pos + 1, mMPDinfo->baseURL[0].length() - (pos + 1));
    pDM->SetFilePrefix(prefix);
    pDM->SetUseCache((cacheDir == "") ? false : true);

    // sync local time according to the remote mechine for live mode
    if (mMPDinfo->type == TYPE_LIVE) {
      SyncTime(mMPDinfo->baseURL[0]);
    }
  }

  // create the reader manager
  {
    OmafReaderManager::OmafReaderParams params;
    if (mMPDinfo->type == TYPE_STATIC) {
      params.stream_type_ = DASH_STREAM_STATIC;
      params.duration_ = mMPDinfo->media_presentation_duration;
    } else {
      params.stream_type_ = DASH_STREAM_DYNMIC;
    }

    if (enableExtractor) {
      params.mode_ = OmafDashMode::EXTRACTOR;
    } else {
      params.mode_ = OmafDashMode::LATER_BINDING;
    }

    OMAF_LOG(LOG_INFO, "media stream type=%s\n", mMPDinfo->type.c_str());
    OMAF_LOG(LOG_INFO, "media stream duration=%lld\n", mMPDinfo->media_presentation_duration);
    OMAF_LOG(LOG_INFO, "media stream extractor=%d\n", enableExtractor);

    OmafReaderManager::Ptr omaf_reader_mgr = std::make_shared<OmafReaderManager>(dash_client_, params);
    ret = omaf_reader_mgr->Initialize(this);
    if (ERROR_NONE != ret) {
      OMAF_LOG(LOG_ERROR, "Failed to start the omaf reader manager, err=%d\n", ret);
      return ret;
    }
    omaf_reader_mgr_ = std::move(omaf_reader_mgr);
  }

  int id = 0;
  for (auto& stream : listStream) {
    this->mMapStream[id] = stream;
    stream->SetStreamID(id);
    // stream->SetEnabledExtractor(enableExtractor);
    stream->SetOmafReaderMgr(omaf_reader_mgr_);
    if (!enableExtractor)
      stream->SetMaxStitchResolution(omaf_dash_params_.max_decode_width_, omaf_dash_params_.max_decode_height_);
    id++;
  }

  if (enableExtractor) {
    m_selector = new OmafExtractorTracksSelector();
    if (!m_selector) {
      OMAF_LOG(LOG_ERROR, "Failed to create extractor tracks selector !\n");
      return ERROR_NULL_PTR;
    }
  } else {
    m_selector = new OmafTileTracksSelector();
    if (!m_selector) {
      OMAF_LOG(LOG_ERROR, "Failed to create tile tracks selector !\n");
      return ERROR_NULL_PTR;
    }
  }

  m_selector->SetProjectionFmt(projFmt);
  if (enablePredictor) m_selector->EnablePosePrediction(predictPluginName, libPath, enableExtractor);
  // Setup initial Viewport and select Adaption Set
  auto it = mMapStream.begin();
  if (it == mMapStream.end()) {
    return ERROR_INVALID;
  }
  ret = m_selector->SetInitialViewport(mViewPorts, &mHeadSetInfo, (it->second));
  if (ret != ERROR_NONE) return ret;

  // set status
  this->SetStatus(STATUS_READY);

  // READERMANAGER::GetInstance()->Initialize(this);
  // READERMANAGER::GetInstance()->SetExtractorEnabled(enableExtractor);

  /// if MPD is static one, don't create thread to download
  if (!isLocalMedia) {
    StartThread();
  }

  return ERROR_NONE;
}

int OmafDashSource::GetTrackCount() {
  int cnt = 0;
  std::map<int, OmafMediaStream*>::iterator it;
  for (it = this->mMapStream.begin(); it != this->mMapStream.end(); it++) {
    OmafMediaStream* pStream = (OmafMediaStream*)it->second;
    cnt += pStream->GetTrackCount();
  }

  return cnt;
}

void OmafDashSource::StopThread() {
  this->SetStatus(STATUS_EXITING);
  this->Join();
}

int OmafDashSource::CloseMedia() {
  if (STATUS_STOPPED != this->GetStatus()) this->StopThread();

  // READERMANAGER::GetInstance()->Close();
  if (omaf_reader_mgr_ != nullptr) {
    omaf_reader_mgr_->Close();
  }

  for (auto it : mMapStream) {
    OmafMediaStream* stream = it.second;
    stream->Close();
  }

  return ERROR_NONE;
}

int OmafDashSource::GetPacket(int streamID, std::list<MediaPacket*>* pkts, bool needParams, bool clearBuf) {
  OmafMediaStream* pStream = this->GetStream(streamID);

  MediaPacket* pkt = nullptr;

  int currentExtractorID = 0;

  if (pStream->HasExtractor()) {
    std::list<OmafExtractor*> extractors = pStream->GetEnabledExtractor();
    int enabledSize = pStream->GetExtractorSize();
    int totalSize = pStream->GetTotalExtractorSize();
    for (auto it = extractors.begin(); it != extractors.end(); it++) {
      OmafExtractor* pExt = (OmafExtractor*)(*it);
      int trackNumber = pExt->GetTrackNumber();
      if (enabledSize < totalSize)  // normal track
      {
        size_t remainSize = 0;
        currentExtractorID = pExt->GetTrackNumber();
        omaf_reader_mgr_->GetPacketQueueSize(mPreExtractorID, remainSize);
        if (mPreExtractorID != currentExtractorID) {
          if (remainSize > 0)  // if there exit remaining data in previous Extractor, then need to pop up them all.
          {
            trackNumber = mPreExtractorID;
            OMAF_LOG(LOG_INFO, "Remaining data in previous track id have to be got! remainSize is %lld\n", remainSize);
          } else  // if there is no data in previous track, then fetch data in current track.
          {
            mPreExtractorID = currentExtractorID;
          }
        }
      }
      int ret = omaf_reader_mgr_->GetNextPacket(trackNumber, pkt, needParams);
      if (ret == ERROR_NONE) {
        pkts->push_back(pkt);
      }
    }
  } else {
    std::map<int, OmafAdaptationSet*> mapAS = pStream->GetMediaAdaptationSet();
    // std::map<int, OmafAdaptationSet*> mapSelectedAS = pStream->GetSelectedTileTracks();
    if (mapAS.size() == 1) {
      OMAF_LOG(LOG_INFO, "There is only one tile for the video stream !\n");
      for (auto as_it = mapAS.begin(); as_it != mapAS.end(); as_it++) {
        OmafAdaptationSet* pAS = (OmafAdaptationSet*)(as_it->second);
        // int ret = READERMANAGER::GetInstance()->GetNextFrame(pAS->GetTrackNumber(), pkt, needParams);
        int ret = omaf_reader_mgr_->GetNextPacket(pAS->GetTrackNumber(), pkt, needParams);
        if (ret == ERROR_NONE) pkts->push_back(pkt);
      }
    } else {
      std::list<MediaPacket*> mergedPackets;
      pStream->SetNeedVideoParams(needParams);
      mergedPackets = pStream->GetOutTilesMergedPackets();
      // OMAF_LOG(LOG_INFO, " merged packets has the size of %lld\n", mergedPackets.size());
      std::list<MediaPacket*>::iterator itPacket;
      for (itPacket = mergedPackets.begin(); itPacket != mergedPackets.end(); itPacket++) {
        MediaPacket* onePacket = *itPacket;
        pkts->push_back(onePacket);
      }
      mergedPackets.clear();
    }
  }

  return ERROR_NONE;
}

int OmafDashSource::GetStatistic(DashStatisticInfo* dsInfo) {
#if 0
  DownloadManager *pDM = DOWNLOADMANAGER::GetInstance();
  dsInfo->avg_bandwidth = pDM->GetAverageBitrate();
  dsInfo->immediate_bandwidth = pDM->GetImmediateBitrate();
#else
  if (dsInfo && dash_client_) {
    std::unique_ptr<OmafDashSegmentClient::PerfStatistics> perf_stats = dash_client_->statistics();
    if (perf_stats) {
      dsInfo->avg_bandwidth = static_cast<int32_t>(perf_stats->download_speed_bps_);
    }
  }

#endif
  return ERROR_NONE;
}

int OmafDashSource::SetupHeadSetInfo(HeadSetInfo* clientInfo) {
  memcpy_s(&mHeadSetInfo, sizeof(HeadSetInfo), clientInfo, sizeof(HeadSetInfo));
  return ERROR_NONE;
}

int OmafDashSource::ChangeViewport(HeadPose* pose) {
  int ret = m_selector->UpdateViewport(pose);

  return ret;
}

int OmafDashSource::GetMediaInfo(DashMediaInfo* media_info) {
  MPDInfo* mInfo = this->GetMPDInfo();
  if (!mInfo) return ERROR_NULL_PTR;

  media_info->duration = mInfo->media_presentation_duration;
  media_info->stream_count = this->GetStreamCount();
  if (mInfo->type == TYPE_STATIC) {
    media_info->streaming_type = DASH_STREAM_STATIC;
  } else {
    media_info->streaming_type = DASH_STREAM_DYNMIC;
  }

  for (int i = 0; i < media_info->stream_count; i++) {
    DashStreamInfo* pStreamInfo = this->mMapStream[i]->GetStreamInfo();
    media_info->stream_info[i].bit_rate = pStreamInfo->bit_rate;
    media_info->stream_info[i].height = pStreamInfo->height;
    media_info->stream_info[i].width = pStreamInfo->width;
    media_info->stream_info[i].stream_type = pStreamInfo->stream_type;
    media_info->stream_info[i].framerate_den = pStreamInfo->framerate_den;
    media_info->stream_info[i].framerate_num = pStreamInfo->framerate_num;
    media_info->stream_info[i].channel_bytes = pStreamInfo->channel_bytes;
    media_info->stream_info[i].channels = pStreamInfo->channels;
    media_info->stream_info[i].sample_rate = pStreamInfo->sample_rate;
    media_info->stream_info[i].mProjFormat = pStreamInfo->mProjFormat;
    media_info->stream_info[i].codec = new char[1024];
    media_info->stream_info[i].mime_type = new char[1024];
    media_info->stream_info[i].source_number = pStreamInfo->source_number;
    media_info->stream_info[i].source_resolution = pStreamInfo->source_resolution;
    media_info->stream_info[i].segmentDuration = pStreamInfo->segmentDuration;
    media_info->stream_info[i].tileRowNum = pStreamInfo->tileRowNum;
    media_info->stream_info[i].tileColNum = pStreamInfo->tileColNum;
    memcpy_s(const_cast<char*>(media_info->stream_info[i].codec), 1024, pStreamInfo->codec, 1024);
    memcpy_s(const_cast<char*>(media_info->stream_info[i].mime_type), 1024, pStreamInfo->mime_type, 1024);
  }

  return ERROR_NONE;
}

void OmafDashSource::Run() {
  if (mMPDinfo->type == TYPE_LIVE) {
    thread_dynamic();
    return;
  }
  thread_static();
}

int OmafDashSource::TimedDownloadSegment(bool bFirst) {
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
  // trace
  struct timeval currTime;
  gettimeofday(&currTime, nullptr);
  uint64_t timeUs = currTime.tv_sec * 1000000 + currTime.tv_usec;
  tracepoint(bandwidth_tp_provider, download_info, timeUs, dcount);
  tracepoint(mthq_tp_provider, T3_start_download_time, dcount);
#endif
#endif

  std::map<int, OmafMediaStream*>::iterator it;
  for (it = this->mMapStream.begin(); it != this->mMapStream.end(); it++) {
    OmafMediaStream* pStream = it->second;
    if (bFirst) {
      if (mMPDinfo->type == TYPE_LIVE) {
        pStream->UpdateStartNumber(mMPDinfo->availabilityStartTime);
        if (omaf_dash_params_.syncer_params_.enable_) {
          pStream->SetupSegmentSyncer(omaf_dash_params_);
        }
      }
    }
    pStream->DownloadSegments();
  }
  OMAF_LOG(LOG_INFO, "Start to download segments and id is %d\n", dcount++);

#if 0
  std::unique_ptr<OmafDashSegmentClient::PerfStatistics> perf = dash_client_->statistics();
  if (perf) {
    OMAF_LOG(LOG_INFO, perf->to_string());
  }
#endif
  return ERROR_NONE;
}

int OmafDashSource::StartReadThread() {
  int ret = TimedSelectSegements();
  if (ERROR_NONE != ret) return ret;

  uint32_t cnt = 0;
  while (cnt < mMapStream.size()) {
    cnt = 0;
    for (auto it : mMapStream) {
      if ((it.second)->IsExtractorEnabled()) {
        int enableSize = it.second->GetExtractorSize();
        int totalSize = it.second->GetTotalExtractorSize();
        if (enableSize < totalSize) cnt++;
      } else {
        cnt++;
      }
    }
  }
  // READERMANAGER::GetInstance()->StartThread();
  // omaf_reader_mgr_->StartThread();

  return ERROR_NONE;
}

int OmafDashSource::SelectSpecialSegments(int extractorTrackIdx) {
  int ret = ERROR_NONE;

  std::map<int, OmafMediaStream*>::iterator it;
  for (it = this->mMapStream.begin(); it != this->mMapStream.end(); it++) {
    OmafMediaStream* pStream = it->second;

    pStream->ClearEnabledExtractors();
    OmafExtractor* specialExtractor = pStream->AddEnabledExtractor(extractorTrackIdx);
    if (!specialExtractor) return OMAF_ERROR_INVALID_DATA;
  }
  return ret;
}

int OmafDashSource::TimedSelectSegements() {
  int ret = ERROR_NONE;
  if (nullptr == m_selector) return ERROR_NULL_PTR;

  std::map<int, OmafMediaStream*>::iterator it;
  for (it = this->mMapStream.begin(); it != this->mMapStream.end(); it++) {
    OmafMediaStream* pStream = it->second;
    pStream->SetSegmentNumber(dcount);
    ret = m_selector->SelectTracks(pStream);
    if (ERROR_NONE != ret) break;
  }
  return ret;
}

void OmafDashSource::ClearStreams() {
  std::map<int, OmafMediaStream*>::iterator it;
  for (it = this->mMapStream.begin(); it != this->mMapStream.end(); it++) {
    OmafMediaStream* pStream = (OmafMediaStream*)it->second;
    delete pStream;
  }
  mMapStream.clear();
}

void OmafDashSource::SeekToSeg(int seg_num) {
  if (mMPDinfo->type != TYPE_STATIC) return;
  int nStream = GetStreamCount();
  for (int i = 0; i < nStream; i++) {
    OmafMediaStream* pStream = GetStream(i);

    pStream->SeekTo(seg_num);
  }
  return;
}

int OmafDashSource::SetEOS(bool eos) {
  std::map<int, OmafMediaStream*>::iterator it;
  for (it = mMapStream.begin(); it != mMapStream.end(); it++) {
    OmafMediaStream* pStream = (OmafMediaStream*)it->second;
    pStream->SetEOS(eos);
  }

  return ERROR_NONE;
}

int OmafDashSource::DownloadInitSeg() {
  int nStream = GetStreamCount();

  if (0 == nStream) {
    return ERROR_NO_STREAM;
  }

  int ret = ERROR_NONE;
  /// download initial mp4 for each stream
  for (int i = 0; i < nStream; i++) {
    OmafMediaStream* pStream = GetStream(i);
    bool isExtractorEnabled = pStream->IsExtractorEnabled();
    if (isExtractorEnabled) {
      std::list<OmafExtractor*> listExtarctors;
      std::map<int, OmafExtractor*> mapExtractors = pStream->GetExtractors();
      for (auto& it : mapExtractors) {
        listExtarctors.push_back(it.second);
      }

      ret = pStream->UpdateEnabledExtractors(listExtarctors);
      if (ERROR_NONE != ret) {
        return ERROR_INVALID;
      }
    }
    ret = pStream->DownloadInitSegment();
  }

  return ERROR_NONE;
}

uint64_t OmafDashSource::GetSegmentDuration(int stream_id) { return mMapStream[stream_id]->GetSegmentDuration(); }

void OmafDashSource::thread_dynamic() {
  int ret = ERROR_NONE;
  bool go_on = true;

  if (STATUS_READY != GetStatus()) {
    return;
  }

  SetStatus(STATUS_RUNNING);

  /// download initial mp4 for each stream
  if (ERROR_NONE != DownloadInitSeg()) {
    SetStatus(STATUS_STOPPED);
    return;
  }
  uint32_t wait_time = 3000;
  uint32_t current_wait_time = 0;
  bool isInitSegParsed = omaf_reader_mgr_->IsInitSegmentsParsed();
  while (!isInitSegParsed) {
    ::usleep(1000);
    current_wait_time++;
    if (current_wait_time > wait_time)
    {
      SetStatus(STATUS_STOPPED);
      OMAF_LOG(LOG_ERROR, " Time out for waiting init segment parse!\n");
      return;
    }
    isInitSegParsed = omaf_reader_mgr_->IsInitSegmentsParsed();
  }

  while ((ERROR_NONE != StartReadThread())) {
    ::usleep(1000);
  }

  uint32_t uLastUpdateTime = sys_clock();
  uint32_t uLastSegTime = 0;
  bool bFirst = false;
  /// main loop: update mpd; download segment according to timeline
  while (go_on) {
    if (STATUS_EXITING == GetStatus()) {
      break;
    }

    // Update viewport and select Adaption Set according to pose change
    ret = TimedSelectSegements();

    if (ERROR_NONE != ret) continue;

    uint32_t timer = sys_clock() - uLastUpdateTime;

    if (mMPDinfo->minimum_update_period && (timer > mMPDinfo->minimum_update_period)) {
      TimedUpdateMPD();
      uLastUpdateTime = sys_clock();
    }

    if (0 == uLastSegTime) {
      uLastSegTime = sys_clock();
      bFirst = true;
    } else {
      bFirst = false;
    }

    TimedDownloadSegment(bFirst);

    uint32_t interval = sys_clock() - uLastSegTime;

    /// 1/2 segment duration ahead of time to fetch segment.
    // uint32_t wait_time = (info.max_segment_duration * 3) / 4 - interval;
    uint32_t wait_time = mMPDinfo->max_segment_duration > interval ? mMPDinfo->max_segment_duration - interval : 0;

    ::usleep(wait_time * 1000);

    uLastSegTime = sys_clock();
  }

  SetStatus(STATUS_STOPPED);

  return;
}

void OmafDashSource::thread_static() {
  int ret = ERROR_NONE;
  bool go_on = true;

  if (STATUS_READY != GetStatus()) {
    return;
  }

  SetStatus(STATUS_RUNNING);

  /// download initial mp4 for each stream
  if (ERROR_NONE != DownloadInitSeg()) {
    SetStatus(STATUS_STOPPED);
    return;
  }
  uint32_t wait_time = 3000;
  uint32_t current_wait_time = 0;
  bool isInitSegParsed = omaf_reader_mgr_->IsInitSegmentsParsed();
  while (!isInitSegParsed) {
    ::usleep(1000);
    current_wait_time++;
    if (current_wait_time > wait_time)
    {
      SetStatus(STATUS_STOPPED);
      OMAF_LOG(LOG_ERROR, " Time out for waiting init segment parse!\n");
      return;
    }
    isInitSegParsed = omaf_reader_mgr_->IsInitSegmentsParsed();
  }

  while ((ERROR_NONE != StartReadThread())) {
    ::usleep(1000);
  }

  // -0.1 for framerate.den is 1001
  if (GetSegmentDuration(0) == 0) {
    return;
  }

  double segmentDuration = (double)GetSegmentDuration(0);
  int total_seg =
      segmentDuration > 0 ? (ceil((double)mMPDinfo->media_presentation_duration / segmentDuration / 1000)) : 0;

  int seg_count = 0;

  uint32_t uLastSegTime = 0;
  bool bFirst = false;
  /// main loop: update mpd; download segment according to timeline
  while (go_on) {
    if (STATUS_EXITING == GetStatus()) {
      break;
    }

    // Update viewport and select Adaption Set according to pose change
    ret = TimedSelectSegements();

    if (ERROR_NONE != ret) continue;

    if (0 == uLastSegTime) {
      uLastSegTime = sys_clock();
      bFirst = true;
    } else {
      bFirst = false;
    }

    TimedDownloadSegment(bFirst);

    uint32_t interval = sys_clock() - uLastSegTime;

    /// one segment duration ahead of time to fetch segment.
    // uint32_t wait_time = (mMPDinfo->max_segment_duration / 2 > interval) ? (mMPDinfo->max_segment_duration -
    // interval) : 0;
    uint32_t wait_time = mMPDinfo->max_segment_duration > interval ? mMPDinfo->max_segment_duration - interval : 0;

    ::usleep(wait_time * 1000);

    uLastSegTime = sys_clock();

    seg_count++;

    if (seg_count >= total_seg) {
      seg_count = 0;
      if (mLoop) {
        SeekToSeg(1);
      } else {
        mEOS = true;
        SetEOS(true);
        break;
      }
    }
  }

  SetStatus(STATUS_STOPPED);

  return;
}

int OmafDashSource::TimedUpdateMPD() { return ERROR_NONE; }

VCD_OMAF_END
