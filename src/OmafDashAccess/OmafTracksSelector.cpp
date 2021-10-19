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
//! \file:   OmafTracksSelector.cpp
//! \brief:  Tracks selector base class implementation
//!
//! Created on May 28, 2019, 1:19 PM
//!

#include "OmafTracksSelector.h"

#define MIN_PTS_INTERVAL_ 5

VCD_OMAF_BEGIN

OmafTracksSelector::OmafTracksSelector(int size) {
  mSize = size;
  m360ViewPortHandle = nullptr;
  mParamViewport = nullptr;
  mPose = nullptr;
  mUsePrediction = false;
  mPredictPluginName = "";
  mLibPath = "";
  mProjFmt = ProjectionFormat::PF_ERP;
  mSegmentDur = 0;
  mQualityRanksNum = 0;
  mLastCatchupPTS = 0;
  memset_s(&(mI360ScvpPlugin), sizeof(PluginDef), 0);
}

OmafTracksSelector::~OmafTracksSelector() {
  if (m360ViewPortHandle) {
    I360SCVP_unInit(m360ViewPortHandle);
    m360ViewPortHandle = nullptr;
  }

  if (mParamViewport)
  {
    if (mParamViewport->pStreamInfo)
    {
      delete [] (mParamViewport->pStreamInfo);
      mParamViewport->pStreamInfo = nullptr;
    }
  }
  SAFE_DELETE(mParamViewport);

  if (mPoseHistory.size()) {
    //for (auto pose : mPoseHistory) {
    //  SAFE_DELETE(pose);
    //}
    std::list<HeadPose*>::iterator itPose;
    for (itPose = mPoseHistory.begin(); itPose != mPoseHistory.end(); )
    {
        HeadPose *onePose = *itPose;
        SAFE_DELETE(onePose);
        mPoseHistory.erase(itPose++);
    }

    mPoseHistory.clear();
  }
  if (mUsePrediction && !mPredictPluginMap.empty())
  {
    ViewportPredictPlugin *plugin = mPredictPluginMap.at(mPredictPluginName);
    if (plugin != nullptr) plugin->Destroy();
  }

  if (mPredictPluginMap.size()) {
    for (auto &p : mPredictPluginMap) {
      SAFE_DELETE(p.second);
    }

    mPredictPluginMap.clear();
  }

  mUsePrediction = false;

  // SAFE_DELETE(mPose);

  mTwoDQualityInfos.clear();
}

int OmafTracksSelector::SetInitialViewport(std::vector<Viewport *> &pView, HeadSetInfo *headSetInfo,
                                           OmafMediaStream *pStream) {
  if (mProjFmt != ProjectionFormat::PF_PLANAR)
  {
    if (!headSetInfo || !headSetInfo->viewPort_hFOV || !headSetInfo->viewPort_vFOV || !headSetInfo->viewPort_Width ||
        !headSetInfo->viewPort_Height) {
      return ERROR_INVALID;
    }
  }
  else
  {
    if (!headSetInfo || !headSetInfo->viewPort_Width || !headSetInfo->viewPort_Height || !(mTwoDQualityInfos.size())) {
      return ERROR_INVALID;
    }
  }

  mParamViewport = new param_360SCVP;
  mParamViewport->usedType = E_VIEWPORT_ONLY;
  mParamViewport->logFunction = (void*)logCallBack;
  mParamViewport->pStreamInfo = NULL;
  if (mProjFmt == ProjectionFormat::PF_ERP) {
    mParamViewport->paramViewPort.viewportWidth = headSetInfo->viewPort_Width;
    mParamViewport->paramViewPort.viewportHeight = headSetInfo->viewPort_Height;
    mParamViewport->paramViewPort.viewPortPitch = headSetInfo->pose->pitch;
    mParamViewport->paramViewPort.viewPortYaw = headSetInfo->pose->yaw;
    mParamViewport->paramViewPort.viewPortFOVH = headSetInfo->viewPort_hFOV;
    mParamViewport->paramViewPort.viewPortFOVV = headSetInfo->viewPort_vFOV;
    mParamViewport->paramViewPort.geoTypeInput =
        (EGeometryType)(mProjFmt);                                    //(EGeometryType)headSetInfo->input_geoType;
    mParamViewport->paramViewPort.geoTypeOutput = E_SVIDEO_VIEWPORT;  //(EGeometryType)headSetInfo->output_geoType;
    mParamViewport->paramViewPort.tileNumRow = pStream->GetRowSize();
    mParamViewport->paramViewPort.tileNumCol = pStream->GetColSize();
    mParamViewport->paramViewPort.usageType = E_VIEWPORT_ONLY;
    mParamViewport->paramViewPort.faceWidth = pStream->GetStreamHighResWidth();
    mParamViewport->paramViewPort.faceHeight = pStream->GetStreamHighResHeight();
    mParamViewport->paramViewPort.paramVideoFP.cols = 1;
    mParamViewport->paramViewPort.paramVideoFP.rows = 1;
    mParamViewport->paramViewPort.paramVideoFP.faces[0][0].idFace = 0;
    mParamViewport->paramViewPort.paramVideoFP.faces[0][0].rotFace = NO_TRANSFORM;
  } else if (mProjFmt == ProjectionFormat::PF_CUBEMAP) {
    mParamViewport->paramViewPort.viewportWidth = headSetInfo->viewPort_Width;
    mParamViewport->paramViewPort.viewportHeight = headSetInfo->viewPort_Height;
    mParamViewport->paramViewPort.viewPortPitch = headSetInfo->pose->pitch;
    mParamViewport->paramViewPort.viewPortYaw = headSetInfo->pose->yaw;
    mParamViewport->paramViewPort.viewPortFOVH = headSetInfo->viewPort_hFOV;
    mParamViewport->paramViewPort.viewPortFOVV = headSetInfo->viewPort_vFOV;
    mParamViewport->paramViewPort.geoTypeInput =
        (EGeometryType)(mProjFmt);                                    //(EGeometryType)headSetInfo->input_geoType;
    mParamViewport->paramViewPort.geoTypeOutput = E_SVIDEO_VIEWPORT;  //(EGeometryType)headSetInfo->output_geoType;
    mParamViewport->paramViewPort.tileNumRow = pStream->GetRowSize() / 2;
    mParamViewport->paramViewPort.tileNumCol = pStream->GetColSize() / 3;
    mParamViewport->paramViewPort.usageType = E_VIEWPORT_ONLY;
    mParamViewport->paramViewPort.faceWidth = pStream->GetStreamHighResWidth() / 3;
    mParamViewport->paramViewPort.faceHeight = pStream->GetStreamHighResHeight() / 2;

    mParamViewport->paramViewPort.paramVideoFP.cols = 3;
    mParamViewport->paramViewPort.paramVideoFP.rows = 2;
    mParamViewport->paramViewPort.paramVideoFP.faces[0][0].idFace = 0;
    mParamViewport->paramViewPort.paramVideoFP.faces[0][0].rotFace = NO_TRANSFORM;
    mParamViewport->paramViewPort.paramVideoFP.faces[0][0].faceWidth = mParamViewport->paramViewPort.faceWidth;
    mParamViewport->paramViewPort.paramVideoFP.faces[0][0].faceHeight = mParamViewport->paramViewPort.faceHeight;
    mParamViewport->paramViewPort.paramVideoFP.faces[0][1].idFace = 1;
    mParamViewport->paramViewPort.paramVideoFP.faces[0][1].rotFace = NO_TRANSFORM;
    mParamViewport->paramViewPort.paramVideoFP.faces[0][2].idFace = 2;
    mParamViewport->paramViewPort.paramVideoFP.faces[0][2].rotFace = NO_TRANSFORM;
    mParamViewport->paramViewPort.paramVideoFP.faces[1][0].idFace = 3;
    mParamViewport->paramViewPort.paramVideoFP.faces[1][0].rotFace = NO_TRANSFORM;
    mParamViewport->paramViewPort.paramVideoFP.faces[1][1].idFace = 4;
    mParamViewport->paramViewPort.paramVideoFP.faces[1][1].rotFace = NO_TRANSFORM;
    mParamViewport->paramViewPort.paramVideoFP.faces[1][2].idFace = 5;
    mParamViewport->paramViewPort.paramVideoFP.faces[1][2].rotFace = NO_TRANSFORM;
  } else if (mProjFmt == ProjectionFormat::PF_PLANAR) {
    mParamViewport->paramViewPort.viewportWidth = headSetInfo->viewPort_Width;
    mParamViewport->paramViewPort.viewportHeight = headSetInfo->viewPort_Height;
    mParamViewport->paramViewPort.geoTypeInput = E_SVIDEO_PLANAR;
    mParamViewport->paramViewPort.geoTypeOutput = E_SVIDEO_VIEWPORT;
    mParamViewport->paramViewPort.usageType = E_VIEWPORT_ONLY;
    mParamViewport->sourceResolutionNum = mTwoDQualityInfos.size();
    mParamViewport->accessInterval = (float)(mSegmentDur);
    mParamViewport->pStreamInfo = new Stream_Info[mParamViewport->sourceResolutionNum];
    if (!(mParamViewport->pStreamInfo))
      return ERROR_NULL_PTR;

    uint8_t strIdx = 0;
    map<int32_t, TwoDQualityInfo>::iterator itQua;
    for(itQua = mTwoDQualityInfos.begin(); itQua != mTwoDQualityInfos.end(); itQua++)
    {
      TwoDQualityInfo oneQuality = itQua->second;
      if (oneQuality.quality_ranking == 1)
      {
        mParamViewport->paramViewPort.faceWidth = oneQuality.orig_width;
        mParamViewport->paramViewPort.faceHeight = oneQuality.orig_height;
      }
      mParamViewport->pStreamInfo[strIdx].FrameWidth  = oneQuality.orig_width;
      mParamViewport->pStreamInfo[strIdx].FrameHeight = oneQuality.orig_height;
      mParamViewport->pStreamInfo[strIdx].TileWidth   = oneQuality.region_width;
      mParamViewport->pStreamInfo[strIdx].TileHeight  = oneQuality.region_height;

      OMAF_LOG(LOG_INFO, "Planar video %d\n", strIdx);
      OMAF_LOG(LOG_INFO, "Width %d, Height %d\n", mParamViewport->pStreamInfo[strIdx].FrameWidth, mParamViewport->pStreamInfo[strIdx].FrameHeight);
      OMAF_LOG(LOG_INFO, "Tile Width %d, Tile Height %d\n", mParamViewport->pStreamInfo[strIdx].TileWidth, mParamViewport->pStreamInfo[strIdx].TileHeight);

      mTwoDStreamQualityMap.insert(std::make_pair(strIdx, oneQuality.quality_ranking));
      OMAF_LOG(LOG_INFO, "Insert one pair of stream index %d and its corresponding quality rankding %d\n", strIdx, oneQuality.quality_ranking);

      strIdx++;
    }

    mParamViewport->paramViewPort.paramVideoFP.cols = 1;
    mParamViewport->paramViewPort.paramVideoFP.rows = 1;

    mParamViewport->pluginDef = mI360ScvpPlugin;
    if (mParamViewport->pluginDef.pluginLibPath == NULL)
    {
        OMAF_LOG(LOG_ERROR, "No assigned 360SCVP Plugin for planar video tiles selection, exit !\n");
        return ERROR_NULL_PTR;
    }
    OMAF_LOG(LOG_INFO, "Used 360SCVP Plugin is %s\n", mParamViewport->pluginDef.pluginLibPath);
  }

  m360ViewPortHandle = I360SCVP_Init(mParamViewport);
  if (!m360ViewPortHandle) return ERROR_NULL_PTR;

  // set current Pose;
  mPose = new HeadPose;
  if (!mPose) return ERROR_NULL_PTR;

  memcpy_s(mPose, sizeof(HeadPose), headSetInfo->pose, sizeof(HeadPose));

  return UpdateViewport(mPose);
}

int OmafTracksSelector::UpdateViewport(HeadPose *pose) {
  if (!pose) return ERROR_NULL_PTR;

  std::lock_guard<std::mutex> lock(mMutex);
  HeadPose* input_pose = new HeadPose;

  if (!(input_pose)) return ERROR_NULL_PTR;
  memcpy_s(input_pose, sizeof(HeadPose), pose, sizeof(HeadPose));
  mPoseHistory.push_front(input_pose);
  if (mPoseHistory.size() > (uint32_t)(this->mSize)) {
    auto pit = mPoseHistory.back();
    SAFE_DELETE(pit);
    mPoseHistory.pop_back();
  }

  // if using viewport prediction, set real time viewports.
  if (mUsePrediction && !mPredictPluginMap.empty())
  {
    ViewportPredictPlugin *plugin = mPredictPluginMap.at(mPredictPluginName);
    ViewportAngle *angle = new ViewportAngle;
    angle->yaw = pose->yaw;
    angle->pitch = pose->pitch;
    angle->pts = pose->pts;
    plugin->SetViewport(angle);
  }

  return ERROR_NONE;
}

int OmafTracksSelector::EnablePosePrediction(std::string predictPluginName, std::string libPath, bool enableExtractor) {
  mUsePrediction = true;
  mPredictPluginName.assign(predictPluginName);
  mLibPath.assign(libPath);
  // 1. load plugin
  int ret = InitializePredictPlugins();
  if (ret != ERROR_NONE)
  {
    OMAF_LOG(LOG_ERROR, "Failed in loading predict plugin\n");
    return ret;
  }
  // 2. initial plugin
  ViewportPredictPlugin *plugin = mPredictPluginMap.at(mPredictPluginName);
  PredictOption option;
  option.usingFeedbackAngleAdjust = true;
  if (enableExtractor){
    option.mode = PredictionMode::SingleViewpoint;
  }
  else{
    option.mode = PredictionMode::MultiViewpoints;
  }
  ret = plugin->Intialize(option);
  if (ret != ERROR_NONE)
  {
    OMAF_LOG(LOG_ERROR, "Failed in initializing predict plugin\n");
    return ret;
  }
  return ERROR_NONE;
}

int OmafTracksSelector::InitializePredictPlugins() {
  if (mLibPath.empty() || mPredictPluginName.empty()) {
    OMAF_LOG(LOG_ERROR, "Viewport predict plugin path OR name is invalid!\n");
    return ERROR_INVALID;
  }
  ViewportPredictPlugin *plugin = new ViewportPredictPlugin();
  if (!plugin) return ERROR_NULL_PTR;

  std::string pluginPath = mLibPath + mPredictPluginName;
  int ret = plugin->LoadPlugin(pluginPath.c_str());
  if (ret != ERROR_NONE) {
    OMAF_LOG(LOG_ERROR, "Load plugin failed!\n");
    SAFE_DELETE(plugin);
    return ret;
  }
  mPredictPluginMap.insert(std::pair<std::string, ViewportPredictPlugin *>(mPredictPluginName, plugin));
  return ERROR_NONE;
}

std::map<uint32_t, std::map<int, OmafAdaptationSet*>> OmafTracksSelector::CompareTracksAndGetDifference(OmafMediaStream* pStream, uint64_t *currentTimeLine)
{
    //first: segID, second: TracksMap
    std::map<uint32_t, std::map<int, OmafAdaptationSet*>> diffTracksMap;
    //1. get current tile tracks timeline.
    {
    std::lock_guard<std::mutex> lock(mMutex);
    *currentTimeLine = mPoseHistory.empty() ? 0 : mPoseHistory.front()->pts;
    if (mLastCatchupPTS != 0 && *currentTimeLine - mLastCatchupPTS < MIN_PTS_INTERVAL_) { // too short time to get the difference
      return diffTracksMap;
    }
    }
    auto stream_info = pStream->GetStreamInfo();
    if (nullptr == stream_info) {
        OMAF_LOG(LOG_ERROR, "Stream informaiton is empty!\n");
        return diffTracksMap;
    }
    TracksMap currSelectedTracksMap = GetCurrentTracksMap();
    //2. get compared tile tracks according to current timeline.
    if (stream_info->framerate_den == 0) return diffTracksMap;
    uint32_t sampleNumPerSeg = pStream->GetSegmentDuration() * stream_info->framerate_num / stream_info->framerate_den;
    if (sampleNumPerSeg == 0) return diffTracksMap;
    uint64_t comTimeLine = *currentTimeLine / sampleNumPerSeg * sampleNumPerSeg;
    TracksMap comTracks = m_prevTimedTracksMap[comTimeLine];
    // LOG(INFO) << "comTimeLine " << comTimeLine << endl;
    // for (auto tks : comTracks) LOG(INFO) << comTimeLine << " " << tks.first << endl;

    //3. get different tracks.
    TracksMap diffTracks = GetDifferentTracks(currSelectedTracksMap, comTracks);
    //3.1 if no difference
    if (diffTracks.empty())
    {
        OMAF_LOG(LOG_INFO, "There is no difference between current tracks and former timed-download tracks\n");
        return diffTracksMap;
    }
    // for (auto df : diffTracks)
    // {
    //   OMAF_LOG(LOG_INFO, "Diff is %d\n", df.first);
    // }
    OMAF_LOG(LOG_INFO, "Additional tiles occurs at PTS %lld! Trigger catch-up path!\n", *currentTimeLine);
    mLastCatchupPTS = *currentTimeLine;
    //3.2 have difference - 3 conditions to handle
    uint32_t thresholdFrameNum = 10;
    if (*currentTimeLine - comTimeLine <= sampleNumPerSeg - thresholdFrameNum)
    {
        if (m_prevTimedTracksMap.find(comTimeLine + 1 * sampleNumPerSeg) == m_prevTimedTracksMap.end()) //Eg.C70 The moment that changes is in the front of the rendering in the segment.
        {
            uint32_t comSegID = comTimeLine / sampleNumPerSeg + 1;
            diffTracksMap.insert(make_pair(comSegID, diffTracks));
            OMAF_LOG(LOG_INFO, "Eg.C70 download seg id %d\n", comSegID);
        }
        else //Eg.C105 Need to download the additional tiles for 2 segments.
        {
            uint32_t comSegID1 = comTimeLine / sampleNumPerSeg + 1;
            uint32_t comSegID2 = comSegID1 + 1;
            diffTracksMap.insert(make_pair(comSegID1, diffTracks));
            TracksMap diffTracks2 = GetDifferentTracks(currSelectedTracksMap, m_prevTimedTracksMap[comTimeLine + sampleNumPerSeg]);
            if (!diffTracks2.empty())
            {
                diffTracksMap.insert(make_pair(comSegID2, diffTracks2));
            }
            else
            {
                OMAF_LOG(LOG_WARNING, "Abnormal happens in condition 2 for multi-segment additional downloading!\n");
            }
            OMAF_LOG(LOG_INFO, "Eg.C105 download two segs id is %d %d\n", comSegID1, comSegID2);
        }
    }
    else if (m_prevTimedTracksMap.find(comTimeLine + sampleNumPerSeg) != m_prevTimedTracksMap.end())//Eg.C25 The moment that changes is very close to the last rendering frame of the segment. So switch the compared segment to the next one.
    {
        uint32_t comSegID = (*currentTimeLine / sampleNumPerSeg + 1) + 1;
        TracksMap diffTracks = GetDifferentTracks(currSelectedTracksMap, m_prevTimedTracksMap[comTimeLine + sampleNumPerSeg]);
        diffTracksMap.insert(make_pair(comSegID, diffTracks));
        OMAF_LOG(LOG_INFO, "Eg.C25 download seg id %d\n", comSegID);
    }
    else // in this case, the change will be detected and render soon(Means the latency of (render - download) is less than thresholdFrameNum). Need not catch up. Eg.C25-D30-R30
    {
      if (m_prevTimedTracksMap.rbegin() != m_prevTimedTracksMap.rend()) {
        OMAF_LOG(LOG_WARNING, "Abnormal happens outside of all 3 conditions! The current time line is %lld and last timeline in prevTracks is %lld\n", *currentTimeLine, m_prevTimedTracksMap.rbegin()->first);
      }
    }

    return diffTracksMap;
}

TracksMap OmafTracksSelector::GetDifferentTracks(TracksMap tracks1, TracksMap tracks2) // tracks1 - tracks2
{
  TracksMap diffTracks;

  if (tracks1.empty() || tracks2.empty())
  {
    OMAF_LOG(LOG_INFO, "Tracks are empty!\n");
    return diffTracks;
  }
  //1. convert map to vector
  std::vector<pair<int, OmafAdaptationSet*>> convertedTracksList1, convertedTracksList2, convertedDiffTracks;
  std::copy(tracks1.begin(), tracks1.end(), back_inserter(convertedTracksList1));
  std::copy(tracks2.begin(), tracks2.end(), back_inserter(convertedTracksList2));
  //2. calculate convertedTracksList1 - convertedTracksList2
  set_difference(convertedTracksList1.begin(), convertedTracksList1.end(), convertedTracksList2.begin(), convertedTracksList2.end(), back_inserter(convertedDiffTracks));
  //3. convert vector to map
  for (auto track : convertedDiffTracks)
  {
    diffTracks.insert(make_pair(track.first, track.second));
  }
  return diffTracks;
}

VCD_OMAF_END
