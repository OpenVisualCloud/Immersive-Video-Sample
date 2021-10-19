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

int OmafExtractorTracksSelector::SelectTracks(OmafMediaStream* pStream, bool isTimed) {

  DashStreamInfo *streamInfo = pStream->GetStreamInfo();
  int ret = ERROR_NONE;
  if (streamInfo->stream_type == MediaType_Video)
  {
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
      {
      std::lock_guard<std::mutex> lock(mCurrentMutex);
      if (NULL == pSelectedExtrator && !mCurrentExtractor) return ERROR_NULL_PTR;

      bool isExtractorChanged = false;
      // not first time and changed and change to different extractor
      if (mCurrentExtractor && pSelectedExtrator && mCurrentExtractor != pSelectedExtrator) {
        isExtractorChanged = true;
      }

      mCurrentExtractor = pSelectedExtrator ? pSelectedExtrator : mCurrentExtractor;

      mCurrentExtractors.clear();

      mCurrentExtractors.push_front(mCurrentExtractor);

      if (isExtractorChanged || mCurrentExtractors.size() > 1)  //?
      {
        list<int> trackIDs;
        for (auto& it : mCurrentExtractors) {
          trackIDs.push_back(it->GetTrackNumber());
        }
        // READERMANAGER::GetInstance()->RemoveTrackFromPacketQueue(trackIDs);
      }
      if (isTimed)
      {
          int32_t stream_frame_rate = streamInfo->framerate_num / streamInfo->framerate_den;
          uint32_t sampleNumPerSeg = pStream->GetSegmentDuration() * stream_frame_rate;
          m_prevTimedTracksMap.insert(make_pair((static_cast<uint64_t>(pStream->GetSegmentNumber()) - 1) * sampleNumPerSeg, mCurrentExtractor->GetCurrentTracksMap()));
      }
      }
  }
  return ret;
}

int OmafExtractorTracksSelector::UpdateEnabledTracks(OmafMediaStream* pStream)
{
  DashStreamInfo *streamInfo = pStream->GetStreamInfo();
  int ret = ERROR_NONE;
  if (streamInfo->stream_type == MediaType_Video)
  {
    ret = pStream->UpdateEnabledExtractors(mCurrentExtractors);
  }
  else if (streamInfo->stream_type == MediaType_Audio)
  {
    ret = pStream->EnableAllAudioTracks();
  }
  return ret;
}

bool OmafExtractorTracksSelector::IsDifferentPose(HeadPose* pose1, HeadPose* pose2) {
  // return false if two pose is same
  if (abs(pose1->yaw - pose2->yaw) < 1e-3 && abs(pose1->pitch - pose2->pitch) < 1e-3) {
    OMAF_LOG(LOG_INFO, "pose has not changed!\n");
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

    mPose = mPoseHistory.front();
    // mPoseHistory.pop_front();

    if (!mPose) {
      return nullptr;
    }

    historySize = mPoseHistory.size();
  }

  // won't get viewport if pose hasn't changed
  if (previousPose && mPose && !IsDifferentPose(previousPose, mPose) && historySize > 1 && mPose->pts > 0 && !mUsePrediction) {
    OMAF_LOG(LOG_INFO, "pose hasn't changed!\n");
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
    // trace
    tracepoint(mthq_tp_provider, T2_detect_pose_change, 0);
#endif
#endif
    return NULL;
  }

  // to select extractor;
  OMAF_LOG(LOG_INFO, "Start to select extractor tracks!\n");
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
        // trace
  tracepoint(mthq_tp_provider, T1_select_tracks, "extractortrack");
#endif
#endif
  OmafExtractor* selectedExtractor = SelectExtractor(pStream, mPose);
  if (selectedExtractor && previousPose) {
    OMAF_LOG(LOG_INFO, "For extractor track %d\n", selectedExtractor->GetID());
    OMAF_LOG(LOG_INFO, "pose has changed from yaw %f, pitch %f\n", previousPose->yaw, previousPose->pitch);
    OMAF_LOG(LOG_INFO, "to yaw %f, pitch %f\n", mPose->yaw, mPose->pitch);
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
    // trace
    tracepoint(mthq_tp_provider, T2_detect_pose_change, 1);
#endif
#endif
  }

  // if (previousPose != mPose) SAFE_DELETE(previousPose);

  return selectedExtractor;
}

static bool IsIncluded(std::list<int> first, std::list<int> second)
{
    bool included = false;
    if (first.size() > second.size())
    {
        included = false;
    }
    else
    {
        uint64_t includedNum = 0;
        std::list<int>::iterator it1;
        for (it1 = first.begin(); it1 != first.end(); it1++)
        {
            int id1 = *it1;
            std::list<int>::iterator it2;
            for (it2 = second.begin(); it2 != second.end(); it2++)
            {
                 int id2 = *it2;
                 if (id2 == id1)
                 {
                     includedNum++;
                     break;
                 }
             }
         }
         if (includedNum == first.size())
         {
             included = true;
         }
         else
         {
             included = false;
         }
     }
     return included;
}

OmafExtractor* OmafExtractorTracksSelector::SelectExtractor(OmafMediaStream* pStream, HeadPose* pose) {
  // to select extractor;
  int ret = I360SCVP_setViewPort(m360ViewPortHandle, (float)(round(pose->yaw)), (float)(round(pose->pitch)));
  if (ret != 0) return NULL;
  ret = I360SCVP_process(mParamViewport, m360ViewPortHandle);
  if (ret != 0) return NULL;

  TileDef *tilesInViewport = new TileDef[1024];
  Param_ViewportOutput paramViewportOutput;
  int32_t selectedTilesNum = I360SCVP_getTilesInViewport(
          tilesInViewport, &paramViewportOutput, m360ViewPortHandle);
  std::list<int> selectedTracks;

  std::map<int, OmafAdaptationSet*> asMap = pStream->GetMediaAdaptationSet();
  std::map<int, OmafAdaptationSet*>::iterator itAS;

  if (mProjFmt == ProjectionFormat::PF_ERP)
  {
      for (int32_t index = 0; index < selectedTilesNum; index++)
      {
          int32_t left = tilesInViewport[index].x;
          int32_t top  = tilesInViewport[index].y;

          for (itAS = asMap.begin(); itAS != asMap.end(); itAS++)
          {
              OmafAdaptationSet *adaptationSet = itAS->second;
              OmafSrd *srd = adaptationSet->GetSRD();
              int32_t tileLeft = srd->get_X();
              int32_t tileTop  = srd->get_Y();
              uint32_t qualityRanking = adaptationSet->GetRepresentationQualityRanking();

              if ((qualityRanking == HIGHEST_QUALITY_RANKING) && (tileLeft == left) && (tileTop == top))
              {
                  int trackID = adaptationSet->GetID();
                  selectedTracks.push_back(trackID);
                  break;
              }
          }
      }
  }
  else if (mProjFmt == ProjectionFormat::PF_CUBEMAP)
  {
      for (int32_t index = 0; index < selectedTilesNum; index++)
      {
          int32_t left = tilesInViewport[index].x;
          int32_t top  = tilesInViewport[index].y;
          int32_t faceId = tilesInViewport[index].faceId;
          for (itAS = asMap.begin(); itAS != asMap.end(); itAS++)
          {
              OmafAdaptationSet *adaptationSet = itAS->second;
              uint32_t qualityRanking = adaptationSet->GetRepresentationQualityRanking();

              if (qualityRanking == HIGHEST_QUALITY_RANKING)
              {
                  TileDef *tileInfo = adaptationSet->GetTileInfo();
                  if (!tileInfo)
                  {
                      OMAF_LOG(LOG_ERROR, "NULL tile information for Cubemap !\n");
                      DELETE_ARRAY(tilesInViewport);
                      return NULL;
                  }
                  int32_t tileLeft = tileInfo->x;
                  int32_t tileTop  = tileInfo->y;
                  int32_t tileFaceId = tileInfo->faceId;
                  if ((tileLeft == left) && (tileTop == top) && (tileFaceId == faceId))
                  {
                      int trackID = adaptationSet->GetID();
                      selectedTracks.push_back(trackID);
                      break;
                  }
              }
          }
      }
  }
  // get Content Coverage from 360SCVP library
  CCDef* outCC = new CCDef;
  ret = I360SCVP_getContentCoverage(m360ViewPortHandle, outCC);
  if (ret != 0)
  {
    SAFE_DELETE(outCC);
    return NULL;
  }

  OmafExtractor* selectedExtractor = NULL;
  bool included = false;
  std::map<int, OmafExtractor*> extractors = pStream->GetExtractors();
  std::map<int, OmafExtractor*>::iterator ie;
  for (ie = extractors.begin(); ie != extractors.end(); ie++)
  {
      std::list<int> refTracks = ie->second->GetDependTrackID();
      included = IsIncluded(selectedTracks, refTracks);
      if (included)
          break;
  }
  if (ie == extractors.end())
  {
      OMAF_LOG(LOG_WARNING, "Couldn't find matched extractor track. There is error in packing !\n");
      selectedExtractor = GetNearestExtractor(pStream, outCC);
  }
  else
  {
      selectedExtractor = ie->second;
  }

  SAFE_DELETE(outCC);
  delete [] tilesInViewport;
  tilesInViewport = NULL;

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
    mPose = mPoseHistory.front();
    // mPoseHistory.pop_front();
    if (!mPose) {
      return extractors;
    }
    historySize = mPoseHistory.size();
  }
  // won't get viewport if pose hasn't changed
  if (previousPose && mPose && !IsDifferentPose(previousPose, mPose) && historySize > 1 && mPose->pts > 0) {
    OMAF_LOG(LOG_INFO, "pose hasn't changed!\n");
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
    // trace
    tracepoint(mthq_tp_provider, T2_detect_pose_change, 0);
#endif
#endif
    // SAFE_DELETE(previousPose);
    return extractors;
  }
  // if viewport changed, then predict viewport using pose history.
  if (mPredictPluginMap.size() == 0)
  {
      OMAF_LOG(LOG_ERROR,"predict plugin map is empty!\n");
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
  float possibilityOfHalting = 0;
  plugin->Predict(first_predict_pts, predict_angles, &possibilityOfHalting);
  // possibility of halting
  OMAF_LOG(LOG_INFO, "Possibility of halting is %f\n", possibilityOfHalting);
  if (predict_angles.empty())
  {
      OMAF_LOG(LOG_INFO, "predictPose_func return an invalid value!\n");
      return extractors;
  }
  // to select extractor, only support SingleViewport mode in prediction.
  HeadPose* predictPose = new HeadPose;
  ViewportAngle *angle = predict_angles[first_predict_pts];
  predictPose->yaw = angle->yaw;
  predictPose->pitch = angle->pitch;
  OMAF_LOG(LOG_INFO, "Start to select extractor tracks by prediction!\n");
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
        // trace
  tracepoint(mthq_tp_provider, T1_select_tracks, "extractortrack");
#endif
#endif
  OmafExtractor* selectedExtractor = SelectExtractor(pStream, predictPose);
  if (selectedExtractor && previousPose) {
    extractors.push_back(selectedExtractor);
    OMAF_LOG(LOG_INFO, "For extractor track %d\n", selectedExtractor->GetID());
    OMAF_LOG(LOG_INFO, "pose has changed from yaw %f, pitch %f\n", previousPose->yaw, previousPose->pitch);
    OMAF_LOG(LOG_INFO, "to yaw %f, pitch %f\n", mPose->yaw, mPose->pitch);
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
    // trace
    tracepoint(mthq_tp_provider, T2_detect_pose_change, 1);
#endif
#endif
  }
  for (auto pre_angle : predict_angles)
  {
    SAFE_DELETE(pre_angle.second);
  }
  // SAFE_DELETE(previousPose);
  SAFE_DELETE(predictPose);
  predict_angles.clear();
  return extractors;
}

VCD_OMAF_END
