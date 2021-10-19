/*
 * Copyright (c) 2018, Intel Corporation
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
 */

/*
 * File:   VRDashStreamingAPI.h
 * Author: Zhang, Andrew
 *
 * Created on January 15, 2019, 1:11 PM
 */

#include <cstdlib>
#include <math.h>

//#include "../utils/GlogWrapper.h"
#include "OmafDashAccessApi.h"
#include "OmafDashSource.h"
#include "OmafMediaSource.h"
#include "OmafTypes.h"
#include "general.h"
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
#include "../trace/Bandwidth_tp.h"
#include "../trace/E2E_latency_tp.h"
#endif
#endif

using namespace std;

VCD_USE_VROMAF;
VCD_USE_VRVIDEO;

Handler OmafAccess_Init(DashStreamingClient *pCtx) {
  if (pCtx == nullptr) {
    return nullptr;
  }
  OmafMediaSource *pSource = new OmafDashSource();

  VCD::OMAF::OmafDashParams omaf_dash_params;
  const OmafParams &omaf_params = pCtx->omaf_params;
  // for download
  if (omaf_params.proxy.http_proxy) {
    omaf_dash_params.http_proxy_.http_proxy_ = std::string(omaf_params.proxy.http_proxy);
  }
  if (omaf_params.proxy.https_proxy) {
    omaf_dash_params.http_proxy_.https_proxy_ = std::string(omaf_params.proxy.https_proxy);
  }
  if (omaf_params.proxy.no_proxy) {
    omaf_dash_params.http_proxy_.no_proxy_ = std::string(omaf_params.proxy.no_proxy);
  }
  if (omaf_params.proxy.proxy_user) {
    omaf_dash_params.http_proxy_.proxy_user_ = std::string(omaf_params.proxy.proxy_user);
  }
  if (omaf_params.proxy.proxy_passwd) {
    omaf_dash_params.http_proxy_.proxy_passwd_ = std::string(omaf_params.proxy.proxy_passwd);
  }

  if (omaf_params.http_params.conn_timeout > 0) {
    omaf_dash_params.http_params_.conn_timeout_ = omaf_params.http_params.conn_timeout;
  }
  if (omaf_params.http_params.total_timeout > 0) {
    omaf_dash_params.http_params_.total_timeout_ = omaf_params.http_params.total_timeout;
  }

  if (omaf_params.http_params.retry_times > 0) {
    omaf_dash_params.http_params_.retry_times_ = omaf_params.http_params.retry_times;
  }

  omaf_dash_params.http_params_.bssl_verify_peer_ = omaf_params.http_params.ssl_verify_peer == 0 ? false : true;

  omaf_dash_params.http_params_.bssl_verify_host_ = omaf_params.http_params.ssl_verify_host == 0 ? false : true;

  omaf_dash_params.prediector_params_.enable_ = omaf_params.predictor_params.enable == 0 ? false : true;

  if (omaf_params.predictor_params.name) {
    omaf_dash_params.prediector_params_.name_ = std::string(omaf_params.predictor_params.name);
  }
  if (omaf_params.predictor_params.libpath) {
    omaf_dash_params.prediector_params_.libpath_ = std::string(omaf_params.predictor_params.libpath);
  }

  omaf_dash_params.stats_params_.enable_ = omaf_params.statistic_params.enable == 0 ? false : true;
  if (omaf_dash_params.stats_params_.enable_) {
    omaf_dash_params.stats_params_.window_size_ms_ = omaf_params.statistic_params.window_size_ms;
  }

  omaf_dash_params.syncer_params_.enable_ = omaf_params.synchronizer_params.enable == 0 ? false : true;
  if (omaf_dash_params.syncer_params_.enable_) {
    omaf_dash_params.syncer_params_.segment_range_size_ = omaf_params.synchronizer_params.segment_range_size;
  }

  if (omaf_params.max_parallel_transfers > 0) {
    omaf_dash_params.max_parallel_transfers_ = omaf_params.max_parallel_transfers;
  }

  if (omaf_params.segment_open_timeout_ms > 0) {
    omaf_dash_params.segment_open_timeout_ms_ = omaf_params.segment_open_timeout_ms;
  }
  // for stitch
  if (omaf_params.max_decode_width > 0) {
    omaf_dash_params.max_decode_width_ = omaf_params.max_decode_width;
  }
  if (omaf_params.max_decode_height > 0) {
    omaf_dash_params.max_decode_height_ = omaf_params.max_decode_height;
  }
  // for catch up
  omaf_dash_params.enable_in_time_viewport_update = omaf_params.enable_in_time_viewport_update;
  omaf_dash_params.max_response_times_in_seg = omaf_params.max_response_times_in_seg;
  omaf_dash_params.max_catchup_width = omaf_params.max_catchup_width;
  omaf_dash_params.max_catchup_height = omaf_params.max_catchup_height;

  OMAF_LOG(LOG_INFO,"Dash parameter %s\n", omaf_dash_params.to_string().c_str());
  pSource->SetOmafDashParams(omaf_dash_params);

  return (Handler)((long)pSource);
}

int OmafAccess_OpenMedia(Handler hdl, DashStreamingClient *pCtx, bool enablePredictor, char *predictPluginName,
                         char *libPath) {
  if (hdl == nullptr || pCtx == nullptr) {
    return ERROR_INVALID;
  }
  OmafMediaSource *pSource = (OmafMediaSource *)hdl;
  pSource->SetLoop(false);
  // for android ndk compile, transform char* to string is mandatory
  string media_url = pCtx->media_url;
  string cache_path = pCtx->cache_path;
  string s_predictPluginName = predictPluginName;
  string s_libPath = libPath;
  return pSource->OpenMedia(media_url, cache_path, pCtx->log_callback, pCtx->plugin_def, pCtx->enable_extractor, enablePredictor,
                            s_predictPluginName, s_libPath);
}

int OmafAccess_StartStreaming(Handler hdl)
{
  OmafMediaSource *pSource = (OmafMediaSource *)hdl;
  return pSource->StartStreaming();
}

int OmafAccess_CloseMedia(Handler hdl) {
  OmafMediaSource *pSource = (OmafMediaSource *)hdl;

  return pSource->CloseMedia();
}

int OmafAccess_SeekMedia(Handler hdl, uint64_t time) {
  OmafMediaSource *pSource = (OmafMediaSource *)hdl;

  return pSource->SeekTo(time);
}

int OmafAccess_GetMediaInfo(Handler hdl, DashMediaInfo *info) {
  OmafMediaSource *pSource = (OmafMediaSource *)hdl;
  pSource->GetMediaInfo(info);
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
  const char *dash_mode = (info->streaming_type == 1) ? "static" : "dynamic";
  int32_t frameNum = round(float(info->duration) / 1000 * ((float)info->stream_info[0].framerate_num / info->stream_info[0].framerate_den));
  tracepoint(bandwidth_tp_provider, segmentation_info, (char *)dash_mode, (int)info->stream_info[0].segmentDuration, (float)info->stream_info[0].framerate_num / info->stream_info[0].framerate_den, (uint32_t)info->stream_count, (uint64_t*)&(info->stream_info[0].bit_rate), frameNum, (int32_t)(info->duration / 1000));
#endif
#endif
  return ERROR_NONE;
}

int OmafAccess_GetPacket(Handler hdl, int stream_id, DashPacket *packet, int *size, uint64_t *pts, bool needParams,
                         bool clearBuf) {
  OmafMediaSource *pSource = (OmafMediaSource *)hdl;
  std::list<MediaPacket *> pkts;
  pSource->GetPacket(stream_id, &pkts, needParams, clearBuf);

  if (0 == pkts.size()) {
    return ERROR_NULL_PACKET;
  }

  *size = pkts.size();

  int i = 0;
  for (auto it = pkts.begin(); it != pkts.end(); it++) {
    MediaPacket *pPkt = (MediaPacket *)(*it);
    if (!pPkt) {
      *size -= 1;
      continue;
    }
    if (!(pPkt->GetEOS()) || pPkt->IsCatchup()) {
      if (pPkt->GetMediaType() == MediaType_Video)
      {
          RegionWisePacking *newRwpk = new RegionWisePacking;
          const RegionWisePacking &pRwpk = pPkt->GetRwpk();
          *newRwpk = pRwpk;
          newRwpk->rectRegionPacking = new RectangularRegionWisePacking[newRwpk->numRegions];
          memcpy_s(newRwpk->rectRegionPacking, pRwpk.numRegions * sizeof(RectangularRegionWisePacking),
                   pRwpk.rectRegionPacking, pRwpk.numRegions * sizeof(RectangularRegionWisePacking));
          SourceResolution *srcRes = new SourceResolution[pPkt->GetQualityNum()];
          memcpy_s(srcRes, pPkt->GetQualityNum() * sizeof(SourceResolution), pPkt->GetSourceResolutions(),
                   pPkt->GetQualityNum() * sizeof(SourceResolution));
          packet[i].rwpk = newRwpk;
          packet[i].buf = pPkt->MovePayload();
          packet[i].size = pPkt->Size();
          packet[i].segID = pPkt->GetSegID();
          packet[i].videoID = pPkt->GetVideoID();
          packet[i].video_codec = pPkt->GetCodecType();
          packet[i].pts = pPkt->GetPTS();
          packet[i].height = pPkt->GetVideoHeight();
          packet[i].width = pPkt->GetVideoWidth();
          packet[i].numQuality = pPkt->GetQualityNum();
          packet[i].qtyResolution = srcRes;
          packet[i].tileRowNum = pPkt->GetVideoTileRowNum();
          packet[i].tileColNum = pPkt->GetVideoTileColNum();
          packet[i].bEOS = pPkt->GetEOS();
          packet[i].bCatchup = pPkt->IsCatchup();
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
          string tag = "sgmtIdx:" + to_string(pPkt->GetSegID());
          tag += ";videoIdx:" + to_string(pPkt->GetVideoID());
          tracepoint(E2E_latency_tp_provider,
                     post_da_info,
                     pPkt->GetPTS(),
                     tag.c_str());
#endif
#endif
      }
      else if (pPkt->GetMediaType() == MediaType_Audio)
      {
          packet[i].buf = pPkt->MovePayload();
          packet[i].size = pPkt->Size();
          packet[i].segID = pPkt->GetSegID();
          packet[i].pts = pPkt->GetPTS();
          packet[i].bEOS = pPkt->GetEOS();
      }
    } else {
      packet[i].bEOS = true;
    }

    i++;

    delete pPkt;
    pPkt = NULL;
  }

  return ERROR_NONE;
}

int OmafAccess_SetupHeadSetInfo(Handler hdl, HeadSetInfo *clientInfo) {
  OmafMediaSource *pSource = (OmafMediaSource *)hdl;

  return pSource->SetupHeadSetInfo(clientInfo);
}

int OmafAccess_ChangeViewport(Handler hdl, HeadPose *pose) {
  OmafMediaSource *pSource = (OmafMediaSource *)hdl;
  return pSource->ChangeViewport(pose);
}

int OmafAccess_Statistic(Handler hdl, DashStatisticInfo *info) {
  OmafMediaSource *pSource = (OmafMediaSource *)hdl;

  return pSource->GetStatistic(info);
}

int OmafAccess_Close(Handler hdl) {
  OmafMediaSource *pSource = (OmafMediaSource *)hdl;
  delete pSource;

  // FIXME, when and where to do resource release
  // OmafCurlDownloader::releaseCurlModule();
  return ERROR_NONE;
}
