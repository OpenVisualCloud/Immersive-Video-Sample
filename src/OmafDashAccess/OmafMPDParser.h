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
//! \file:   MPDParser.h
//! \brief:
//! \detail:
//! on May 22, 2019, 4:09 PM
//!

#ifndef OMAFMPDPARSER_H
#define OMAFMPDPARSER_H

#include "OmafDashParser/OmafXMLParser.h"
#include "OmafMediaStream.h"
#include "general.h"

#include <mutex>

using namespace VCD::OMAF;
using namespace VCD::VRVideo;

VCD_OMAF_BEGIN

typedef enum {
  MPD_NONE = 0,
  MPD_STATIC,
  MPD_DYNAMIC,
} MPD_TYPE;

typedef std::vector<AdaptationSetElement*> ADAPTATIONSETS;
typedef std::vector<OmafMediaStream*> OMAFSTREAMS;
typedef std::vector<OmafAdaptationSet*> OMAFADAPTATIONSETS;
typedef std::map<std::string, OMAFADAPTATIONSETS> TYPE_OMAFADAPTATIONSETS;

//!
//! \class:   OmafMPDParser
//! \brief:   the parser for MPD file using libdash
//!
class OmafMPDParser {
 public:
  //!
  //! \brief  construct
  //!
  OmafMPDParser();

  //!
  //! \brief  de-construct
  //!
  virtual ~OmafMPDParser();

 public:
  //!
  //! \brief  parse MPD and get construct media streams
  //!
  int ParseMPD(std::string mpd_file, OMAFSTREAMS& listStream);

  //!
  //! \brief  update MPD and get construct media streams for live if needed.
  //!
  int UpdateMPD(OMAFSTREAMS& listStream);

  //!
  //! \brief  Get MPD information.
  //!
  MPDInfo* GetMPDInfo();

  //!
  //! \brief  Set cache dir.
  //!
  void SetCacheDir(string cache_dir) { mCacheDir = cache_dir; };

  void SetExtractorEnabled(bool isExtractorEnabled) { mExtractorEnabled = isExtractorEnabled; };

  bool GetExtractorEnabled() { return mExtractorEnabled; };
  void SetOmafDashParams(const OmafDashParams& params) { omaf_dash_params_ = params; }
  ProjectionFormat GetProjectionFmt() { return mPF; };

  uint32_t GetVideoQualityRanksNum() { return mQualityRanksNum; };

  std::map<int32_t, TwoDQualityInfo> GetTwoDQualityInfos() { return mTwoDQualityInfos; };

 private:
  //!
  //! \brief construct media streams.
  //!
  int ParseStreams(OMAFSTREAMS& listStream);

  //!
  //! \brief Parse MPD information
  //!
  int ParseMPDInfo();

  //!
  //! \brief group all adaptationSet based on the dependency.
  //!
  int GroupAdaptationSet(PeriodElement* pPeriod, TYPE_OMAFADAPTATIONSETS& mapAdaptationSets);

  //!
  //! \brief build up OmafMediaStreams based on the grouped AdaptationSets.
  //!
  int BuildStreams(TYPE_OMAFADAPTATIONSETS mapAdaptationSets, OMAFSTREAMS& listStream);

  //!
  //! \brief Create OmafAdaptationSet based on libDash AdaptationSetElement.
  //! \param [in] pAS AdaptationSetElement
  //! \return
  //!
  OmafAdaptationSet* CreateAdaptationSet(AdaptationSetElement* pAS, ProjectionFormat pf);

  //!
  //! \brief Judge the type of the AdaptationSet.
  //!
  bool ExtractorJudgement(AdaptationSetElement* pAS);

private:
    OmafMPDParser& operator=(const OmafMPDParser& other) { return *this; };
    OmafMPDParser(const OmafMPDParser& other) { /* do not create copies */ };

 private:
  OmafXMLParser* mParser = nullptr;
  MPDElement* mMpd = nullptr;  //!< the PTR for libdash MPD
  std::string mMPDURL;         //!< url of MPD
  // ThreadLock*                    mLock;
  std::mutex mLock;
  MPDInfo* mMPDInfo;  //!< the information of MPD
  std::vector<BaseUrlElement*> mBaseUrls;
  ProjectionFormat mPF;            //!< the projection format of the video content
  std::string mCacheDir;           //!< cache directory
  bool mExtractorEnabled = false;  //!< if extractor track is enabled
  OmafDashParams omaf_dash_params_;
  OmafAdaptationSet *mTmpAS;
  OmafMediaStream* mTmpStream;
  uint32_t mQualityRanksNum;
  std::map<int32_t, TwoDQualityInfo> mTwoDQualityInfos;
};

VCD_OMAF_END;

#endif /* MPDPARSER_H */
