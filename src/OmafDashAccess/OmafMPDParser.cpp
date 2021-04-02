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

#include "OmafMPDParser.h"
#include <typeinfo>
#include "OmafExtractor.h"

VCD_OMAF_BEGIN

OmafMPDParser::OmafMPDParser() {
  mParser = nullptr;
  this->mMpd = nullptr;
  this->mMPDURL = "";
  this->mCacheDir = "";
  // this->mLock = new ThreadLock();
  mMPDInfo = nullptr;
  mPF = PF_UNKNOWN;
  this->mTmpAS = nullptr;
  this->mTmpStream = nullptr;
  this->mQualityRanksNum = 0;
}

OmafMPDParser::~OmafMPDParser() {
  SAFE_DELETE(mParser);
  mTwoDQualityInfos.clear();
  // SAFE_DELETE(mMpd);
  // SAFE_DELETE(mLock);
}

int OmafMPDParser::ParseMPD(std::string mpd_file, OMAFSTREAMS& listStream) {
  int ret = ERROR_NONE;

  if (nullptr == mParser) {
    mParser = new OmafXMLParser();
    mParser->SetOmafHttpParams(omaf_dash_params_.http_proxy_, omaf_dash_params_.http_params_);
  }

  // mLock->lock();
  std::lock_guard<std::mutex> lock(mLock);

  mMPDURL = mpd_file;

  OMAF_LOG(LOG_INFO, "To parse the mpd file: %s\n", mMPDURL.c_str());

  ODStatus st = mParser->Generate(const_cast<char*>(mMPDURL.c_str()), mCacheDir);
  if (st != OD_STATUS_SUCCESS) {
    // mLock->unlock();
    OMAF_LOG(LOG_ERROR, "Failed to load MPD file: %s\n", mpd_file.c_str());
    return st;
  }

  mMpd = mParser->GetGeneratedMPD();
  if (nullptr == mMpd) {
    OMAF_LOG(LOG_ERROR, "Failed to get the generated mpd!\n");
    return ERROR_PARSE;
  }

  ret = ParseMPDInfo();
  if (ret != ERROR_NONE) {
    OMAF_LOG(LOG_ERROR, "Failed to parse MPD file: %s\n", mpd_file.c_str());
    return ret;
  }

  ret = ParseStreams(listStream);
  if (ret != ERROR_NONE) {
    if (ret != OMAF_INVALID_EXTRACTOR_ENABLEMENT)
      OMAF_LOG(LOG_ERROR, "Failed to parse media streams from MPD file: %s\n", mpd_file.c_str());
    return ret;
  }

  // mLock->unlock();

  return ERROR_NONE;
}

int OmafMPDParser::ParseMPDInfo() {
  mMPDInfo = new MPDInfo;
  if (!mMPDInfo) return ERROR_NULL_PTR;

  auto baseUrl = mMpd->GetBaseUrls().back();
  mMPDInfo->mpdPathBaseUrl = baseUrl->GetPath();
  mMPDInfo->profiles = mMpd->GetProfiles();
  mMPDInfo->type = mMpd->GetType();

  if (!mMpd->GetMediaPresentationDuration().empty()) {
    mMPDInfo->media_presentation_duration = parse_duration(mMpd->GetMediaPresentationDuration().c_str());
  }

  if (!mMpd->GetAvailabilityStartTime().empty()) {
    mMPDInfo->availabilityStartTime = parse_date(mMpd->GetAvailabilityStartTime().c_str());
  }
  if (!mMpd->GetAvailabilityEndTime().empty()) {
    mMPDInfo->availabilityEndTime = parse_date(mMpd->GetAvailabilityEndTime().c_str());
  }
  if (!mMpd->GetMaxSegmentDuration().empty()) {
    mMPDInfo->max_segment_duration = parse_duration(mMpd->GetMaxSegmentDuration().c_str());
  }
  if (!mMpd->GetMinBufferTime().empty()) {
    mMPDInfo->min_buffer_time = parse_duration(mMpd->GetMinBufferTime().c_str());
  }
  if (!mMpd->GetMinimumUpdatePeriod().empty()) {
    mMPDInfo->minimum_update_period = parse_duration(mMpd->GetMinimumUpdatePeriod().c_str());
  }
  if (!mMpd->GetSuggestedPresentationDelay().empty()) {
    mMPDInfo->suggested_presentation_delay = parse_int(mMpd->GetSuggestedPresentationDelay().c_str());
  }
  if (!mMpd->GetTimeShiftBufferDepth().empty()) {
    mMPDInfo->time_shift_buffer_depth = parse_duration(mMpd->GetTimeShiftBufferDepth().c_str());
  }

  mBaseUrls = mMpd->GetBaseUrls();
  // Get all base urls except the last one
  for (uint32_t i = 0; i < mBaseUrls.size() - 1; i++) {
    mMPDInfo->baseURL.push_back(mBaseUrls[i]->GetPath());
  }

  mPF = mMpd->GetProjectionFormat();

  return ERROR_NONE;
}

int OmafMPDParser::UpdateMPD(OMAFSTREAMS& listStream) { return ParseMPD(this->mMPDURL, listStream); }

MPDInfo* OmafMPDParser::GetMPDInfo() { return this->mMPDInfo; }

//!
//! \brief construct media streams.
//!
int OmafMPDParser::ParseStreams(OMAFSTREAMS& listStream) {
  int ret = ERROR_NONE;

  std::vector<PeriodElement*> Periods = mMpd->GetPeriods();
  if (Periods.size() == 0) return ERROR_NO_VALUE;

  // processing only the first period;
  PeriodElement* pPeroid = Periods[0];

  TYPE_OMAFADAPTATIONSETS adapt_sets;

  ret = GroupAdaptationSet(pPeroid, adapt_sets);

  ret = BuildStreams(adapt_sets, listStream);

  return ret;
}

int OmafMPDParser::GroupAdaptationSet(PeriodElement* pPeriod, TYPE_OMAFADAPTATIONSETS& mapAdaptationSets) {
  ADAPTATIONSETS AdaptationSets = pPeriod->GetAdaptationSets();

  /// so far, we supposed that there will be only one viewpoint in the mpd,
  /// so all Adaptation sets are belong to the same audio-visual content.
  /// FIXIT, if there are multiple viewpoints.
  for (auto it = AdaptationSets.begin(); it != AdaptationSets.end(); it++) {
    AdaptationSetElement* pAS = (AdaptationSetElement*)(*it);
    mTmpAS = CreateAdaptationSet(pAS, mPF);
    if (mTmpAS == NULL) return ERROR_INVALID;
    /// catalog the Adaptation according to the media type: video, audio, etc
    std::string type = GetSubstr(mTmpAS->GetMimeType(), '/', true);
    OMAF_LOG(LOG_INFO, "Create one AS with type %s\n", type.c_str());

    mapAdaptationSets[type].push_back(mTmpAS);
  }

  return ERROR_NONE;
}

int OmafMPDParser::BuildStreams(TYPE_OMAFADAPTATIONSETS mapAdaptationSets, OMAFSTREAMS& listStream) {
  int ret = ERROR_NONE;
  uint32_t allExtractorCnt = 0;
  uint32_t videoStrNum = 0;
  std::set<QualityRank> allVideoQualities;
  std::map<std::string, OmafMediaStream*> streamsMap;
  for (auto it = mapAdaptationSets.begin(); it != mapAdaptationSets.end(); it++) {
    OMAFADAPTATIONSETS ASs = it->second;
    std::vector<OmafAdaptationSet*>::iterator mainASit;
    std::string type = it->first;
    mTmpStream = new OmafMediaStream();
    if (mTmpStream == NULL) return ERROR_INVALID;
    if (strncmp(type.c_str(), "video", 5) == 0)
    {
        mainASit = ASs.begin();
        videoStrNum++;
    }

    for (auto as_it = ASs.begin(); as_it != ASs.end(); as_it++) {
      OmafAdaptationSet* pOmafAs = (OmafAdaptationSet*)(*as_it);
      pOmafAs->SetBaseURL(mBaseUrls);
      if (typeid(*pOmafAs) == typeid(OmafExtractor)) {
        if (mExtractorEnabled) {
          OmafExtractor* tmpOmafAs = (OmafExtractor*)pOmafAs;
          mTmpStream->AddExtractor(tmpOmafAs);
          mTmpStream->SetExtratorAdaptationSet(tmpOmafAs);
        }
      } else {
        mTmpStream->AddAdaptationSet(pOmafAs);
        if (strncmp(type.c_str(), "video", 5) == 0)
        {
            if (pOmafAs->IsMain()) {
              pOmafAs->SetProjectionFormat(mPF);
              mTmpStream->SetMainAdaptationSet(pOmafAs);
              mainASit = as_it;
              pOmafAs->SetTwoDQualityInfos();
              mTwoDQualityInfos = pOmafAs->GetTwoDQualityInfos();
            } else {
              QualityRank oneQuality = pOmafAs->GetRepresentationQualityRanking();
              allVideoQualities.insert(oneQuality);
            }
        }
        else if (strncmp(type.c_str(), "audio", 5) == 0)
        {
            if (as_it == ASs.begin())
            {
              mTmpStream->SetMainAdaptationSet(pOmafAs);
              mainASit = as_it;
            }
        }
      }
    }

    std::map<int, OmafExtractor*> extractors = mTmpStream->GetExtractors();
    if (extractors.size()) {
      allExtractorCnt++;
    }

    // remove main AS from AdaptationSets for it has no real data
    if (strncmp(type.c_str(), "video", 5) == 0)
    {
        ASs.erase(mainASit);
    }

    streamsMap.insert(std::make_pair(type, mTmpStream));
  }

  mQualityRanksNum = allVideoQualities.size();
  OMAF_LOG(LOG_INFO, "allExtractorCnt %u\n", allExtractorCnt);
  OMAF_LOG(LOG_INFO, "video streams num %u\n", videoStrNum);
  OMAF_LOG(LOG_INFO, "video quality ranks num %u\n", mQualityRanksNum);
  //if (allExtractorCnt < mapAdaptationSets.size()) {
  if (allExtractorCnt < videoStrNum) {
    if (mExtractorEnabled) {
      OMAF_LOG(LOG_INFO, "There isn't extractor track from MPD parsing, extractor track enablement should be false !\n");
      mExtractorEnabled = false;
      ret = OMAF_INVALID_EXTRACTOR_ENABLEMENT;
    }
  }
  std::map<std::string, OmafMediaStream*>::iterator itStream;
  for (itStream = streamsMap.begin(); itStream != streamsMap.end(); itStream++) {
    std::string type = itStream->first;
    OmafMediaStream* stream = itStream->second;
    if (strncmp(type.c_str(), "video", 5) == 0)
    {
        stream->SetEnabledExtractor(mExtractorEnabled);
    }
    else
    {
        stream->SetEnabledExtractor(false);
    }
    stream->SetEnableCatchUp(omaf_dash_params_.enable_in_time_viewport_update);
    stream->SetCatchupThreadNum(omaf_dash_params_.max_response_times_in_seg);
    stream->InitStream(type);
    listStream.push_back(stream);
  }
  return ret;
}

OmafAdaptationSet* OmafMPDParser::CreateAdaptationSet(AdaptationSetElement* pAS, ProjectionFormat pf) {
  if (ExtractorJudgement(pAS)) {
    return new OmafExtractor(pAS, pf, true);
  }
  return new OmafAdaptationSet(pAS, pf, false);
}

bool OmafMPDParser::ExtractorJudgement(AdaptationSetElement* pAS) {
  PreselValue* sel = pAS->GetPreselection();
  if (sel) return true;


  /// FIXME, if @DependencyID has multiple dependency ID, then set it as extractor.
  std::vector<std::string> depIDs = pAS->GetRepresentations()[0]->GetDependencyIDs();
  if (depIDs.size() > 0) {
    return true;
  }

  return false;
}

VCD_OMAF_END
