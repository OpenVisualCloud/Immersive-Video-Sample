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
//! \file:   OmafTracksSelector.h
//! \brief:  Tracks selector base class definition
//! \detail: Define the operation of tracks selector base class based on viewport
//! Created on May 28, 2019, 1:19 PM
//!

#ifndef OMAFTRACKSSELECTOR_H
#define OMAFTRACKSSELECTOR_H

#include "360SCVPViewportAPI.h"
#include "OmafMediaStream.h"
#include "OmafViewportPredict/ViewportPredictPlugin.h"
#include "general.h"
#include <mutex>

using namespace VCD::OMAF;

VCD_OMAF_BEGIN

#define POSE_SIZE 10

constexpr uint64_t ptsInterval[4] = {0, 8, 15, 23}; // 30 38 45 53

typedef struct POSEINFO {
  HeadPose *pose;
  uint64_t time;
} PoseInfo;

class OmafTracksSelector {
 public:
  //!
  //! \brief  construct
  //!
  OmafTracksSelector(int size = POSE_SIZE);

  //!
  //! \brief  de-construct
  //!
  virtual ~OmafTracksSelector();

  //!
  //! \brief  Select tracks for the stream based on the latest pose. each time
  //!         the selector will select tracks based on the latest pose. the
  //!         information stored in mPoseHistory can be used for prediction for
  //!         further movement
  //!
  virtual int SelectTracks(OmafMediaStream* pStream, bool isTimed) = 0;

  //!
  //! \brief  Enable tracks adaptation sets according current selected tracks
  //!
  virtual int UpdateEnabledTracks(OmafMediaStream* pStream) = 0;

  //!
  //! \brief  Get current tracks map after select tracks
  //!
  virtual map<int, OmafAdaptationSet*> GetCurrentTracksMap() = 0;

  //!
  //! \brief  Set Init viewport
  //!
  int SetInitialViewport(std::vector<Viewport *> &pView, HeadSetInfo *headSetInfo, OmafMediaStream *pStream);

  //!
  //! \brief  Update Viewport; each time pose update will be recorded, but only
  //!         the latest will be used when SelectTracks is called.
  //!
  int UpdateViewport(HeadPose *pose);

  //!
  //! \brief  Load viewport prediction plugin
  //!
  int EnablePosePrediction(std::string predictPluginName, std::string libPath, bool enableExtractor);

  //!
  //! \brief  Get the priority of the segment
  //!
  // virtual int GetSegmentPriority(OmafSegment *segment) = 0;

  void SetProjectionFmt(ProjectionFormat projFmt) { mProjFmt = projFmt; };

  //!
  //! \brief  Set total video qualities number
  //!
  void SetVideoQualityRanksNum(uint32_t qualitiesNum) { mQualityRanksNum = qualitiesNum; };

  //!
  //! \brief  Set <qualityRanking, TwoDQualityInfo> map for all planar video sources
  //!
  void SetTwoDQualityInfos(std::map<int32_t, TwoDQualityInfo> twoDQualities) { mTwoDQualityInfos = std::move(twoDQualities); };

  //!
  //! \brief  Set segment duration in microsecond
  //!
  void SetSegmentDuration(uint32_t segDur) { mSegmentDur = segDur; };

  //!
  //! \brief  Set 360SCVP library plugin
  //!
  void SetI360SCVPPlugin(PluginDef i360scvp_plugin)
  {
      mI360ScvpPlugin.pluginType = i360scvp_plugin.pluginType;
      mI360ScvpPlugin.pluginFormat = i360scvp_plugin.pluginFormat;
      mI360ScvpPlugin.pluginLibPath = i360scvp_plugin.pluginLibPath;
  };

  //!
  //! \brief  Compare current tracks and prev tracks and get the different tracks.
  //!
  std::map<uint32_t, std::map<int, OmafAdaptationSet*>> CompareTracksAndGetDifference(OmafMediaStream* pStream, uint64_t *currentTimeLine);

  //!
  //! \brief  Get the different tracks (tracks1 - tracks2)
  //!
  TracksMap GetDifferentTracks(TracksMap track1, TracksMap track2);

private:
    OmafTracksSelector& operator=(const OmafTracksSelector& other) { return *this; };
    OmafTracksSelector(const OmafTracksSelector& other) { /* do not create copies */ };

 private:
  //!
  //! \brief  Initialize viewport prediction plugin
  //!
  int InitializePredictPlugins();

 protected:
  std::list<HeadPose*> mPoseHistory;
  std::map<uint64_t, std::map<int, OmafAdaptationSet*>> m_prevTimedTracksMap;
  int mSize;
  std::mutex mMutex;
  std::mutex mCurrentMutex;
  HeadPose *mPose;
  void *m360ViewPortHandle;
  param_360SCVP *mParamViewport;
  bool mUsePrediction;
  std::string mPredictPluginName;
  std::string mLibPath;
  std::map<std::string, ViewportPredictPlugin *> mPredictPluginMap;
  ProjectionFormat mProjFmt;
  uint32_t         mQualityRanksNum;
  map<int32_t, TwoDQualityInfo> mTwoDQualityInfos;
  map<int32_t, int32_t>         mTwoDStreamQualityMap;
  uint32_t                      mSegmentDur;
  PluginDef                     mI360ScvpPlugin;
  uint64_t                      mLastCatchupPTS;
};

VCD_OMAF_END;

#endif /* OMAFTRACKSSELECTOR_H */
