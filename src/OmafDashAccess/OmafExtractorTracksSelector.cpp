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

/*
 * File:   OmafExtractorTracksSelector.cpp
 * Author: media
 *
 * Created on May 28, 2019, 1:19 PM
 */

#include "OmafExtractorTracksSelector.h"
#include <math.h>
#include <cfloat>
#include <chrono>
#include <cstdint>
#include "OmafMediaStream.h"
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
#include "../trace/MtHQ_tp.h"
#endif
#endif

VCD_OMAF_BEGIN

OmafExtractorTracksSelector::~OmafExtractorTracksSelector() { mCurrentExtractor = nullptr; }

int OmafExtractorTracksSelector::SelectTracks(OmafMediaStream* pStream) {
  OmafExtractor* pSelectedExtrator = NULL;
  if (mUsePrediction) {
    ListExtractor predict_extractors = GetExtractorByPosePrediction(pStream);
    if (predict_extractors.empty()) {
      pSelectedExtrator = GetExtractorByPose(pStream);
    } else
      pSelectedExtrator = predict_extractors.front();
  } else {
    pSelectedExtrator = GetExtractorByPose(pStream);
  }

  if (NULL == pSelectedExtrator && !mCurrentExtractor) return ERROR_NULL_PTR;

  bool isExtractorChanged = false;
  // not first time and changed and change to different extractor
  if (mCurrentExtractor && pSelectedExtrator && mCurrentExtractor != pSelectedExtrator) {
    isExtractorChanged = true;
  }

  mCurrentExtractor = pSelectedExtrator ? pSelectedExtrator : mCurrentExtractor;

  ListExtractor extractors;

  extractors.push_front(mCurrentExtractor);

  if (isExtractorChanged || extractors.size() > 1)  //?
  {
    list<int> trackIDs;
    for (auto& it : extractors) {
      trackIDs.push_back(it->GetTrackNumber());
    }
    // READERMANAGER::GetInstance()->RemoveTrackFromPacketQueue(trackIDs);
  }

  int ret = pStream->UpdateEnabledExtractors(extractors);

  return ret;
}

bool OmafExtractorTracksSelector::IsDifferentPose(HeadPose* pose1, HeadPose* pose2) {
  // return false if two pose is same
  if (abs(pose1->yaw - pose2->yaw) < 1e-3 && abs(pose1->pitch - pose2->pitch) < 1e-3) {
    LOG(INFO) << "pose has not changed!" << std::endl;
    return false;
  }
  return true;
}

OmafExtractor* OmafExtractorTracksSelector::GetExtractorByPose(OmafMediaStream* pStream) {
  HeadPose* previousPose = NULL;
  int64_t historySize = 0;
  {
    std::lock_guard<std::mutex> lock(mMutex);
    if (mPoseHistory.size() == 0) {
      return NULL;
    }

    previousPose = mPose;

    mPose = mPoseHistory.front().pose;
    mPoseHistory.pop_front();

    if (!mPose) {
      return nullptr;
    }

    historySize = mPoseHistory.size();
  }

  // won't get viewport if pose hasn't changed
  if (previousPose && mPose && !IsDifferentPose(previousPose, mPose) && historySize > 1 && !mUsePrediction) {
    LOG(INFO) << "pose hasn't changed!" << endl;
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
    // trace
    tracepoint(mthq_tp_provider, T2_detect_pose_change, 0);
#endif
#endif
    return NULL;
  }

  // to select extractor;
  LOG(INFO) << "Start to select extractor tracks!" << endl;
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
        // trace
  tracepoint(mthq_tp_provider, T1_select_tracks, "extractortrack");
#endif
#endif
  OmafExtractor* selectedExtractor = SelectExtractor(pStream, mPose);
  if (selectedExtractor && previousPose) {
    LOG(INFO) << "pose has changed from (" << previousPose->yaw << "," << previousPose->pitch << ") to (" << mPose->yaw
              << "," << mPose->pitch << ") ! extractor id is: " << selectedExtractor->GetID() << endl;
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
    // trace
    tracepoint(mthq_tp_provider, T2_detect_pose_change, 1);
#endif
#endif
  }

  if (previousPose != mPose) SAFE_DELETE(previousPose);

  return selectedExtractor;
}

OmafExtractor* OmafExtractorTracksSelector::SelectExtractor(OmafMediaStream* pStream, HeadPose* pose) {
  // to select extractor;
  int ret = I360SCVP_setViewPort(m360ViewPortHandle, pose->yaw, pose->pitch);
  if (ret != 0) return NULL;
  ret = I360SCVP_process(mParamViewport, m360ViewPortHandle);
  if (ret != 0) return NULL;

  // get Content Coverage from 360SCVP library
  CCDef* outCC = new CCDef;
  ret = I360SCVP_getContentCoverage(m360ViewPortHandle, outCC);
  if (ret != 0)
  {
    SAFE_DELETE(outCC);
    return NULL;
  }

  // get the extractor with largest intersection
  OmafExtractor* selectedExtractor = GetNearestExtractor(pStream, outCC);

  SAFE_DELETE(outCC);

  return selectedExtractor;
}

OmafExtractor* OmafExtractorTracksSelector::GetNearestExtractor(OmafMediaStream* pStream, CCDef* outCC) {
  // calculate which extractor should be chosen
  OmafExtractor* selectedExtractor = nullptr;
  float leastDistance = FLT_MAX;
  std::map<int, OmafExtractor*> extractors = pStream->GetExtractors();
  for (auto& ie : extractors) {
    ContentCoverage* cc = ie.second->GetContentCoverage();
    if (!cc) continue;

    int32_t ca = cc->coverage_infos[0].centre_azimuth;
    int32_t ce = cc->coverage_infos[0].centre_elevation;

    // for now, every extractor has the same azimuth_range and elevation_range
    // , so we just calculate least Euclidean distance between centres to find the
    // extractor with largest intersection
    float distance = sqrt(pow((ca - outCC->centreAzimuth), 2) + pow((ce - outCC->centreElevation), 2));
    if (distance < leastDistance) {
      leastDistance = distance;
      selectedExtractor = ie.second;
    }
  }

  return selectedExtractor;
}

ListExtractor OmafExtractorTracksSelector::GetExtractorByPosePrediction(OmafMediaStream* pStream) {
  ListExtractor extractors;
  int64_t historySize = 0;
  {
    std::lock_guard<std::mutex> lock(mMutex);
    if (mPoseHistory.size() <= 1) {
      return extractors;
    }
  }

  HeadPose* previousPose = mPose;
  {
    std::lock_guard<std::mutex> lock(mMutex);
    mPose = mPoseHistory.front().pose;
    mPoseHistory.pop_front();
    if (!mPose) {
      return extractors;
    }
    historySize = mPoseHistory.size();
  }
  // won't get viewport if pose hasn't changed
  if (previousPose && mPose && !IsDifferentPose(previousPose, mPose) && historySize > 1) {
    LOG(INFO) << "pose hasn't changed!" << endl;
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
    // trace
    tracepoint(mthq_tp_provider, T2_detect_pose_change, 0);
#endif
#endif
    SAFE_DELETE(previousPose);
    return extractors;
  }
  // if viewport changed, then predict viewport using pose history.
  if (mPredictPluginMap.size() == 0)
  {
      LOG(ERROR)<<"predict plugin map is empty!"<<endl;
      return extractors;
  }
  // 1. figure out the pts of predicted angle
  uint32_t current_segment_num = pStream->GetSegmentNumber();

  DashStreamInfo *stream_info = pStream->GetStreamInfo();
  if (stream_info == nullptr) return extractors;

  int32_t stream_frame_rate = stream_info->framerate_num / stream_info->framerate_den;
  uint64_t first_predict_pts = current_segment_num > 0 ? (current_segment_num - 1) * stream_frame_rate : 0;
  // 2. predict process
  ViewportPredictPlugin *plugin = mPredictPluginMap.at(mPredictPluginName);
  std::map<uint64_t, ViewportAngle*> predict_angles;
  plugin->Predict(first_predict_pts, predict_angles);
  if (predict_angles.empty())
  {
      LOG(INFO)<<"predictPose_func return an invalid value!"<<endl;
      return extractors;
  }
  // to select extractor, only support SingleViewport mode in prediction.
  HeadPose* predictPose = new HeadPose;
  ViewportAngle *angle = predict_angles[first_predict_pts];
  predictPose->yaw = angle->yaw;
  predictPose->pitch = angle->pitch;
  LOG(INFO) << "Start to select extractor tracks!" << endl;
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
        // trace
  tracepoint(mthq_tp_provider, T1_select_tracks, "extractortrack");
#endif
#endif
  OmafExtractor* selectedExtractor = SelectExtractor(pStream, predictPose);
  if (selectedExtractor && previousPose) {
    extractors.push_back(selectedExtractor);
    LOG(INFO) << "pose has changed from (" << previousPose->yaw << "," << previousPose->pitch << ") to (" << mPose->yaw
              << "," << mPose->pitch << ") ! extractor id is: " << selectedExtractor->GetID() << endl;
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
    // trace
    tracepoint(mthq_tp_provider, T2_detect_pose_change, 1);
#endif
#endif
  }
  SAFE_DELETE(previousPose);
  SAFE_DELETE(predictPose);
  predict_angles.clear();
  return extractors;
}

VCD_OMAF_END
