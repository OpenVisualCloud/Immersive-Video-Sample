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
 * File:   OmafAdaptationSet.cpp
 * Author: media
 *
 * Created on May 24, 2019, 10:19 AM
 */

#include "OmafAdaptationSet.h"
#include "OmafReaderManager.h"

#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
#include "../trace/E2E_latency_tp.h"
#endif
#endif
#include <sys/time.h>
//#include <sys/timeb.h>

VCD_OMAF_BEGIN

using namespace VCD::OMAF;

OmafAdaptationSet::OmafAdaptationSet() {
  mAdaptationSet = nullptr;
  mRepresentation = nullptr;
  mInitSegment = nullptr;
  mSRD = nullptr;
  mPreselID = nullptr;
  mTwoDQuality = nullptr;
  mSrqr = nullptr;
  mCC = nullptr;
  mEnable = true;
  m_bMain = false;
  mActiveSegNum = 1;
  mSegNum = 1;
  mStartSegNum = 1;
  mReEnable = false;
  mPF = PF_UNKNOWN;
  mSegmentDuration = 0;
  mTrackNumber = 0;
  mStartNumber = 1;
  mID = 0;
  mType = MediaType_NONE;
  mFpt = FP_UNKNOWN;
  mRwpkType = RWPK_UNKNOWN;
  mTileInfo = NULL;
  mIsExtractorTrack = false;
  mGopSize = 0;
  memset(&mVideoInfo, 0, sizeof(VideoInfo));
  memset(&mAudioInfo, 0, sizeof(AudioInfo));
}

OmafAdaptationSet::~OmafAdaptationSet() {
  this->ClearSegList();

  // SAFE_DELETE(mInitSegment);

  if (mBaseURL.size()) {
    mBaseURL.clear();
  }
}

OmafAdaptationSet::OmafAdaptationSet( AdaptationSetElement* pAdaptationSet, ProjectionFormat pf, bool isExtractorTrack ):OmafAdaptationSet()
{
    mPF = pf;
    mIsExtractorTrack = isExtractorTrack;
    Initialize(pAdaptationSet);
}

int OmafAdaptationSet::Initialize(AdaptationSetElement* pAdaptationSet) {
  mAdaptationSet = pAdaptationSet;

  SelectRepresentation();

  mMimeType = mAdaptationSet->GetMimeType();
  std::string type = GetSubstr(mMimeType, '/', true);

  if (type == "video")
  {
      mSrqr = mAdaptationSet->GetSphereQuality();
      mSRD = mAdaptationSet->GetSRD();
      mPreselID = mAdaptationSet->GetPreselection();
      mRwpkType = mAdaptationSet->GetRwpkType();
      mCC = mAdaptationSet->GetContentCoverage();
      //mID = stoi(mAdaptationSet->GetId());

      if ((mPF == ProjectionFormat::PF_CUBEMAP) && !IsExtractor())
      {
          mTileInfo = new TileDef;
          if (!mTileInfo)
              return OMAF_ERROR_NULL_PTR;
          if (NULL == mSRD)
          {
            OMAF_LOG(LOG_ERROR, "SRD information is invalid for track %d!\n", mID);
            return OMAF_ERROR_NULL_PTR;
          }
          mTileInfo->x = mSRD->get_X();
          mTileInfo->y = mSRD->get_Y();
      }

      for (auto it = mRepresentation->GetDependencyIDs().begin(); it != mRepresentation->GetDependencyIDs().end(); it++) {
        std::string id = *it;
        mDependIDs.push_back(atoi(id.c_str()));
      }
  }

  mID = stoi(mAdaptationSet->GetId());
  OMAF_LOG(LOG_INFO, "ID of AS %d\n", mID);
  if (mID ==  0 && !mAdaptationSet->GetGopSize().empty()) {// main
      mGopSize = stoi(mAdaptationSet->GetGopSize());
      OMAF_LOG(LOG_INFO, "get gop size %d\n", mGopSize);
  }
  SegmentElement* segment = mRepresentation->GetSegment();

  if (nullptr != segment) {
    mStartNumber = segment->GetStartNumber();
    mSegmentDuration = segment->GetDuration() / segment->GetTimescale();
    OMAF_LOG(LOG_INFO, "Segment duration %ld\n", mSegmentDuration);
  }

  // mAudioInfo.sample_rate = parse_int(
  // mRepresentation->GetAudioSamplingRate().c_str() ); mAudioInfo.channels    =
  // mRepresentation->GetAudioChannelConfiguration().size();
  // mAudioInfo.channel_bytes = 2;

  if (type == "video")
  {
      mVideoInfo.bit_rate = mRepresentation->GetBandwidth();
      mVideoInfo.height = mRepresentation->GetHeight();
      mVideoInfo.width = mRepresentation->GetWidth();
      mVideoInfo.frame_Rate.num = atoi(GetSubstr(mRepresentation->GetFrameRate(), '/', true).c_str());
      mVideoInfo.frame_Rate.den = atoi(GetSubstr(mRepresentation->GetFrameRate(), '/', false).c_str());
      mVideoInfo.sar.num = atoi(GetSubstr(mRepresentation->GetSar(), ':', true).c_str());
      mVideoInfo.sar.den = atoi(GetSubstr(mRepresentation->GetSar(), ':', false).c_str());
      mType = MediaType_Video;
      JudgeMainAdaptationSet();
  }
  else if (type == "audio")
  {
      AudioChannelConfigurationElement* audioElement = mRepresentation->GetAudioChlCfg();
      if (!audioElement)
      {
          OMAF_LOG(LOG_ERROR, "Failed to get audio channel configuration element from MPD parsing!\n");
          return OMAF_ERROR_INVALID_DATA;
      }

      mAudioInfo.channels = audioElement->GetChannelCfg();
      //mAudioInfo.channel_bytes =
      mAudioInfo.sample_rate = mRepresentation->GetAudioSamplingRate();
      OMAF_LOG(LOG_INFO, "Audio sample rate %u and channel cfg %u\n", mAudioInfo.sample_rate, mAudioInfo.channels);

      mType = MediaType_Audio;
  }

  mCodec = mAdaptationSet->GetCodecs();

  return ERROR_NONE;
}

int OmafAdaptationSet::SelectRepresentation() {
  std::vector<RepresentationElement*> pRep = mAdaptationSet->GetRepresentations();

  /// FIX; so far choose the first rep in the Representation list
  this->mRepresentation = pRep[0];

  return ERROR_NONE;
}

void OmafAdaptationSet::JudgeMainAdaptationSet() {
  if (nullptr == mAdaptationSet || !mSRD) return;

  if (mType == MediaType_Video) {
    if (this->mSRD->get_H() == 0 && mSRD->get_W() == 0) {
      m_bMain = true;
      return;
    }
  } else {
    m_bMain = true;
    return;
  }
  m_bMain = false;
}

int OmafAdaptationSet::LoadLocalInitSegment() {
  int ret = ERROR_NONE;

  for (auto it = mBaseURL.begin(); it != mBaseURL.end(); it++) {
    BaseUrlElement* baseURL = *it;
    std::string url = baseURL->GetPath();
  }

  SegmentElement* seg = mRepresentation->GetSegment();
  if (nullptr == seg) {
    OMAF_LOG(LOG_ERROR, "Create Initial SegmentElement for AdaptationSet: %d failed\n", this->mID);
    return ERROR_NULL_PTR;
  }

  auto repID = mRepresentation->GetId();
#if 0
 mInitSegment = new OmafSegment(seg, mSegNum, true);
#else
  DashSegmentSourceParams params;
  params.dash_url_ = seg->GenerateCompleteURL(mBaseURL, repID, 0);
  params.priority_ = TaskPriority::NORMAL;
  params.timeline_point_ = static_cast<int64_t>(mSegNum);
  mInitSegment = std::make_shared<OmafSegment>(params, mSegNum, true);
#endif
  if (nullptr == mInitSegment) {
    OMAF_LOG(LOG_ERROR, "New Initial OmafSegment for AdaptationSet: %d failed\n", this->mID);
    return ERROR_NULL_PTR;
  }

  OMAF_LOG(LOG_INFO, "Load Initial OmafSegment for AdaptationSet %d\n", this->mID );

  return ret;
}

int OmafAdaptationSet::LoadLocalSegment() {
  int ret = ERROR_NONE;

  if (!mEnable) {
    mActiveSegNum++;
    mSegNum++;
    return ret;
  }

  SegmentElement* seg = mRepresentation->GetSegment();

  if (nullptr == seg) {
    OMAF_LOG(LOG_ERROR, "Create Initial SegmentElement for AdaptationSet: %d failed\n", this->mID);
    return ERROR_NULL_PTR;
  }

  auto repID = mRepresentation->GetId();
#if 0
  OmafSegment* pSegment = new OmafSegment(seg, mSegNum, false);
#else
  DashSegmentSourceParams params;
  params.dash_url_ = seg->GenerateCompleteURL(mBaseURL, repID, 0);
  params.priority_ = TaskPriority::NORMAL;
  params.timeline_point_ = static_cast<int64_t>(mSegNum);
  OmafSegment::Ptr pSegment = std::make_shared<OmafSegment>(params, mSegNum, false);
#endif

  if (pSegment.get() != nullptr) {
  if (this->mInitSegment == nullptr) return ERROR_NULL_PTR;
  pSegment->SetInitSegID(this->mInitSegment->GetInitSegID());
  pSegment->SetSegID(mSegNum);
  pSegment->SetTrackId(this->mInitSegment->GetTrackId());
  pSegment->SetMediaType(mType);

  if ((mType == MediaType_Video) && (typeid(*this) != typeid(OmafExtractor))) {
    auto qualityRanking = GetRepresentationQualityRanking();
    pSegment->SetQualityRanking(qualityRanking);
    SRDInfo srdInfo;
    srdInfo.left = mSRD->get_X();
    srdInfo.top = mSRD->get_Y();
    srdInfo.width = mSRD->get_W();
    srdInfo.height = mSRD->get_H();
    pSegment->SetSRDInfo(srdInfo);
  }
  else if (mType == MediaType_Audio)
  {
    pSegment->SetAudioChlNum(mAudioInfo.channels);
    pSegment->SetAudioSampleRate(mAudioInfo.sample_rate);
  }

  OMAF_LOG(LOG_INFO, "Load OmafSegment for AdaptationSet %d\n", this->mID );

  mSegments.push_back(std::move(pSegment));

  mActiveSegNum++;
  mSegNum++;
  return ret;
  }
  else {
    OMAF_LOG(LOG_ERROR, "Create OmafSegment for AdaptationSet: %d Number: %d failed\n", this->mID, mActiveSegNum);

    return ERROR_NULL_PTR;
  }
}

int OmafAdaptationSet::LoadAssignedInitSegment(std::string assignedSegment) {
  int ret = ERROR_NONE;

  ret = LoadLocalInitSegment();
  if (ret) return ret;

  OmafSegment::Ptr initSeg = GetInitSegment();
  if (!initSeg) {
    OMAF_LOG(LOG_ERROR, "Failed to get local init segment\n");
    return ERROR_NOT_FOUND;
  }

  initSeg->SetSegmentCacheFile(assignedSegment);
  initSeg->SetSegStored();

  return ret;
}

OmafSegment::Ptr OmafAdaptationSet::LoadAssignedSegment(std::string assignedSegment) {
  int ret = ERROR_NONE;
  ret = LoadLocalSegment();
  if (ret) {
    OMAF_LOG(LOG_ERROR, "Failed to load local segment\n");
    return nullptr;
  }

  OmafSegment::Ptr newSeg = GetLocalNextSegment();
  if (!newSeg) {
    OMAF_LOG(LOG_ERROR, "Failed to get local segment\n");
    return nullptr;
  }

  OmafSegment::Ptr initSeg = GetInitSegment();
  if (!initSeg) {
    OMAF_LOG(LOG_ERROR, "Failed to get local init segment\n");
    return nullptr;
  }

  newSeg->SetSegmentCacheFile(assignedSegment);
  newSeg->SetSegStored();

  return newSeg;
}

/////Download relative methods

int OmafAdaptationSet::DownloadInitializeSegment() {
  int ret = ERROR_NONE;

  if (omaf_reader_mgr_ == nullptr) {
    OMAF_LOG(LOG_ERROR, "The omaf reader manager is empty!\n");
    return ERROR_NULL_PTR;
  }

  SegmentElement* seg = mRepresentation->GetSegment();
  if (nullptr == seg) {
    OMAF_LOG(LOG_ERROR, "Create Initial SegmentElement for AdaptationSet: %d failed\n", this->mID);
    return ERROR_NULL_PTR;
  }

  auto repID = mRepresentation->GetId();
#if 0
    ret = seg->InitDownload(mBaseURL, repID, 0);

    if( ERROR_NONE != ret ){
        SAFE_DELETE(seg);
        OMAF_LOG(LOG_ERROR, "Fail to Init OmafSegment Download for AdaptationSet: %d\n", this->mID);
    }
    mInitSegment = new OmafSegment(seg, mSegNum, true);
#else

  DashSegmentSourceParams params;
  params.dash_url_ = seg->GenerateCompleteURL(mBaseURL, repID, 0);
  params.priority_ = TaskPriority::NORMAL;
  params.timeline_point_ = static_cast<int64_t>(mSegNum);

  mInitSegment = std::make_shared<OmafSegment>(params, mSegNum, true);

#endif

  if (nullptr == mInitSegment) {
    OMAF_LOG(LOG_ERROR, "Failed to create Initial OmafSegment for AdaptationSet: %d\n", this->mID);
    return ERROR_NULL_PTR;
  }

  ret = omaf_reader_mgr_->OpenInitSegment(mInitSegment);

  if (ERROR_NONE != ret) {
    mInitSegment.reset();
    OMAF_LOG(LOG_ERROR, "Fail to Download Initial OmafSegment for AdaptationSet: %d\n", this->mID);
  }

  OMAF_LOG(LOG_INFO, "Download Initial OmafSegment for AdaptationSet %d\n", this->mID);

  return ret;
}

int OmafAdaptationSet::DownloadSegment() {
  int ret = ERROR_NONE;

  if (!mEnable) {
    mActiveSegNum++;
    mSegNum++;
    return ret;
  }
  OMAF_LOG(LOG_INFO, "Download OmafSegment id %d for AdaptationSet: %d\n", mSegNum, this->mID);

  if (omaf_reader_mgr_ == nullptr) {
    OMAF_LOG(LOG_ERROR, "The omaf reader manager is empty!\n");
    return ERROR_NULL_PTR;
  }

  SegmentElement* seg = mRepresentation->GetSegment();

  if (nullptr == seg) {
    OMAF_LOG(LOG_ERROR, "Create Initial SegmentElement for AdaptationSet: %d failed\n", this->mID);
    return ERROR_NULL_PTR;
  }

  auto repID = mRepresentation->GetId();
#if 0
  ret = seg->InitDownload(mBaseURL, repID, mActiveSegNum);

  if (ERROR_NONE != ret) {
    SAFE_DELETE(seg);
    OMAF_LOG(LOG_ERROR, "Fail to Init OmafSegment Download for AdaptationSet: %d\n", this->mID);
  }

  OmafSegment* pSegment = new OmafSegment(seg, mSegNum, false, mReEnable);
#else
  DashSegmentSourceParams params;

  params.dash_url_ = seg->GenerateCompleteURL(mBaseURL, repID, mActiveSegNum);
  params.priority_ = TaskPriority::NORMAL;
  params.timeline_point_ = static_cast<int64_t>(mSegNum);

  OmafSegment::Ptr pSegment = std::make_shared<OmafSegment>(params, mSegNum, false);

#endif
  // reset the re-enable flag, since it will be updated with different viewport
  if (mReEnable) mReEnable = false;

  if (pSegment.get() != nullptr) {
    if (this->mInitSegment.get() == nullptr) return ERROR_NULL_PTR;
    pSegment->SetInitSegID(this->mInitSegment->GetInitSegID());
    pSegment->SetMediaType(mType);
    if ((mType == MediaType_Video) && (typeid(*this) != typeid(OmafExtractor))) {
      auto qualityRanking = GetRepresentationQualityRanking();
      pSegment->SetQualityRanking(qualityRanking);
      SRDInfo srdInfo;
      srdInfo.left = mSRD->get_X();
      srdInfo.top = mSRD->get_Y();
      srdInfo.width = mSRD->get_W();
      srdInfo.height = mSRD->get_H();
      pSegment->SetSRDInfo(srdInfo);
    }
    else if (mType == MediaType_Audio)
    {
      pSegment->SetAudioChlNum(mAudioInfo.channels);
      pSegment->SetAudioSampleRate(mAudioInfo.sample_rate);
    }

    pSegment->SetSegID(mSegNum);
    pSegment->SetTrackId(this->mInitSegment->GetTrackId());
    ret = omaf_reader_mgr_->OpenSegment(std::move(pSegment), IsExtractor());

    if (ERROR_NONE != ret) {
      OMAF_LOG(LOG_ERROR, "Fail to Download OmafSegment for AdaptationSet: %d\n", this->mID);
    }

    //  pthread_mutex_lock(&mMutex);
    // NOTE: won't record segments in adaption set since GetNextSegment() not be
    // called
    //       , and this will lead to memory growth.
    // mSegments.push_back(pSegment);
    // pthread_mutex_unlock(&mMutex);

    mActiveSegNum++;
    mSegNum++;

    return ret;
  }
  else {
    OMAF_LOG(LOG_ERROR, "Create OmafSegment for AdaptationSet: %d Number: %d failed\n", this->mID, mActiveSegNum);

    return ERROR_NULL_PTR;
  }
}

int OmafAdaptationSet::DownloadAssignedSegment(uint32_t trackID, uint32_t segID)
{
  int ret = ERROR_NONE;

  int realSegNum = segID + mStartSegNum - 1;

  OMAF_LOG(LOG_INFO, "[FrameSequences][CatchUp][Download]: Download Catchup OmafSegment id %d for AdaptationSet: %d\n", segID, trackID);
  // ANDROID_LOGD("Download Catchup OmafSegment id %d for AdaptationSet: %d\n", segID, trackID);

  if (omaf_reader_mgr_ == nullptr) {
    OMAF_LOG(LOG_ERROR, "The omaf reader manager is empty!\n");
    return ERROR_NULL_PTR;
  }

  SegmentElement* seg = mRepresentation->GetSegment();

  if (nullptr == seg) {
    OMAF_LOG(LOG_ERROR, "Create Initial SegmentElement for AdaptationSet: %d failed\n", trackID);
    return ERROR_NULL_PTR;
  }

  auto repID = mRepresentation->GetId();

  DashSegmentSourceParams params;

  params.dash_url_ = seg->GenerateCompleteURL(mBaseURL, repID, realSegNum);
  params.priority_ = TaskPriority::NORMAL;
  params.timeline_point_ = static_cast<int64_t>(segID);

  OmafSegment::Ptr pSegment = std::make_shared<OmafSegment>(params, segID, false);

  // reset the re-enable flag, since it will be updated with different viewport
  // if (mReEnable) mReEnable = false;

  if (pSegment.get() != nullptr) {
    if (this->mInitSegment.get() == nullptr) return ERROR_NULL_PTR;
    pSegment->SetInitSegID(this->mInitSegment->GetInitSegID());
    pSegment->SetMediaType(mType);
    if ((mType == MediaType_Video) && (typeid(*this) != typeid(OmafExtractor))) {
      auto qualityRanking = GetRepresentationQualityRanking();
      pSegment->SetQualityRanking(qualityRanking);
      SRDInfo srdInfo;
      srdInfo.left = mSRD->get_X();
      srdInfo.top = mSRD->get_Y();
      srdInfo.width = mSRD->get_W();
      srdInfo.height = mSRD->get_H();
      pSegment->SetSRDInfo(srdInfo);
    }
    else if (mType == MediaType_Audio)
    {
      pSegment->SetAudioChlNum(mAudioInfo.channels);
      pSegment->SetAudioSampleRate(mAudioInfo.sample_rate);
    }

    pSegment->SetSegID(segID);
    pSegment->SetTrackId(trackID);
    bool isCatchup = true;
    ret = omaf_reader_mgr_->OpenSegment(std::move(pSegment), false, isCatchup);

    if (ERROR_NONE != ret) {
      OMAF_LOG(LOG_ERROR, "Fail to Download Catch up OmafSegment for AdaptationSet: %d\n", trackID);
    }

    return ret;
  }
  else {
    OMAF_LOG(LOG_ERROR, "Create Catch up OmafSegment for AdaptationSet: %d Number: %d failed\n", trackID, segID);

    return ERROR_NULL_PTR;
  }
}

std::string OmafAdaptationSet::GetUrl(const SegmentSyncNode& node) const {
  SegmentElement* seg = mRepresentation->GetSegment();

  if (nullptr == seg) {
    OMAF_LOG(LOG_ERROR, "Create Initial SegmentElement for AdaptationSet: %d failed\n", this->mID);
    return std::string();
  }

  auto repID = mRepresentation->GetId();
  return seg->GenerateCompleteURL(mBaseURL, repID, static_cast<int32_t>(node.segment_value.number_));
}

/////read relative methods
int OmafAdaptationSet::UpdateStartNumberByTime(uint64_t nAvailableStartTime) {
  time_t gTime;
  struct tm* t;
  struct timeval now;
  struct timezone tz;
  gettimeofday(&now, &tz);
  // struct timeb timeBuffer;
  // ftime(&timeBuffer);
  // now.tv_sec = (long)(timeBuffer.time);
  // now.tv_usec = timeBuffer.millitm * 1000;
  gTime = now.tv_sec;
  t = gmtime(&gTime);

  uint64_t current = timegm(t);
  current *= 1000;

  if (current < nAvailableStartTime) {
    OMAF_LOG(LOG_ERROR, "Unreasonable current time %lld which is earlier than available time %lld\n", current, nAvailableStartTime);

    return -1;
  }
  mActiveSegNum = (current - nAvailableStartTime) / (mSegmentDuration * 1000) + mStartNumber;
  mStartSegNum = mActiveSegNum;

  OMAF_LOG(LOG_INFO, "Current time= %lld and available time= %lld.\n", current, nAvailableStartTime);
  OMAF_LOG(LOG_INFO, "Set start segment index= %d\n", mActiveSegNum);
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
  tracepoint(E2E_latency_tp_provider,
             da_ssi_info,
             mActiveSegNum);
#endif
#endif
  return mActiveSegNum;
}

OmafSegment::Ptr OmafAdaptationSet::GetNextSegment() {
  OmafSegment::Ptr seg;

  std::lock_guard<std::mutex> lock(mMutex);
  seg = std::move(mSegments.front());
  mSegments.pop_front();

  return seg;
}

OmafSegment::Ptr OmafAdaptationSet::GetLocalNextSegment() {
  OmafSegment::Ptr seg;

  seg = std::move(mSegments.front());
  mSegments.pop_front();

  return seg;
}

void OmafAdaptationSet::ClearSegList() {
  std::list<OmafSegment*>::iterator it;
  std::lock_guard<std::mutex> lock(mMutex);
  for (auto it = mSegments.begin(); it != mSegments.end(); it++) {
    // delete *it;
    *it = nullptr;
  }
  mSegments.clear();
}

int OmafAdaptationSet::SeekTo(int seg_num) {
  mActiveSegNum = seg_num;
  ClearSegList();
  return ERROR_NONE;
}

VCD_OMAF_END
