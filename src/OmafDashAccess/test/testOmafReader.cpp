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
 */

//!
//! \file:   testOmafReader.cpp
//! \brief:  Omaf reader class unit test
//!
//! Created on July 17, 2019, 6:04 AM
//!

#include "gtest/gtest.h"
#include "../OmafMPDParser.h"
#include "../OmafReader.h"
#include "../OmafMP4VRReader.h"

#include <list>

VCD_USE_VROMAF;
VCD_USE_VRVIDEO;

// comments
// this is not a good unit-test cases :(

namespace {
class OmafReaderTest : public testing::Test {
 public:
  virtual void SetUp() {
    std::string mpdUrl = "./segs_for_readertest/Test.mpd";

    m_mpdParser = new OmafMPDParser();
    if (!m_mpdParser) return;
    m_mpdParser->SetExtractorEnabled(true);

    int ret = m_mpdParser->ParseMPD(mpdUrl, m_listStream);
    EXPECT_TRUE(ret == ERROR_NONE);
    // m_mpdParser->GetMPDInfo();

    m_reader = new OmafMP4VRReader();
    if (!m_reader) return;
  }
  virtual void TearDown() {
    SAFE_DELETE(m_mpdParser);
    SAFE_DELETE(m_reader);
  }
  uint32_t buildDashTrackId(uint32_t id) noexcept { return id & static_cast<uint32_t>(0xffff); }
  uint32_t buildReaderTrackId(uint32_t trackId, uint32_t initSegId) noexcept { return (initSegId << 16) | trackId; }
  std::shared_ptr<TrackInformation> findTrackInformation(OmafReader *reader, uint32_t tackId) noexcept {
    try {
      std::vector<TrackInformation *> track_infos;

      OMAF_STATUS ret = reader->getTrackInformations(track_infos);
      if (ERROR_NONE != ret) {
        OMAF_LOG(LOG_ERROR, "Failed to get the trackinformation list from reader, code=%d\n", ret);
        return nullptr;
      }

      // get the required track information and release the old data
      std::shared_ptr<TrackInformation> track_info;
      for (auto &track : track_infos) {
        if (track != nullptr) {
          if (buildDashTrackId(track->trackId) == tackId) {
            track_info = std::make_shared<TrackInformation>();
            *(track_info.get()) = *track;
          }
          delete track;
        }

        track = nullptr;
      }
      track_infos.clear();

      return std::move(track_info);
    } catch (const std::exception &ex) {
      OMAF_LOG(LOG_ERROR, "Exception when find the track information! ex: %s\n", ex.what() );
      return nullptr;
    }
  }

  bool findSampleIndexRange(std::shared_ptr<TrackInformation> track_info, uint32_t segid, size_t &begin,
                            size_t &end) noexcept {
    try {
      if (track_info.get() == nullptr) {
        return false;
      }
      bool found = false;
      for (size_t index = 0; index < track_info->sampleProperties.size; index++) {
        if (segid == track_info->sampleProperties[index].segmentId) {
          end++;
          if (!found) {
            found = true;
            begin = track_info->sampleProperties[index].sampleId;
          }
        }
      }
      return found;
    } catch (const std::exception &ex) {
      OMAF_LOG(LOG_ERROR, "Exception when find the start index! ex: %s\n", ex.what() );
      return false;
    }
  }

  OmafMPDParser *m_mpdParser;
  OMAFSTREAMS m_listStream;
  OmafReader *m_reader;
};

TEST_F(OmafReaderTest, ParseInitialSegment) {
  int ret = ERROR_NONE;
  uint32_t initSegID = 0;
  std::vector<TrackInformation *> trackInfos;

  std::string cacheFileName;
  char storedFileName[1024];

  for (auto it = m_listStream.begin(); it != m_listStream.end(); it++) {
    OmafMediaStream *stream = (OmafMediaStream *)(*it);
    EXPECT_TRUE(stream != NULL);

    std::map<int, OmafAdaptationSet *> normalAS = stream->GetMediaAdaptationSet();
    std::map<int, OmafExtractor *> extractorAS = stream->GetExtractors();

    for (auto itAS = normalAS.begin(); itAS != normalAS.end(); itAS++) {
      OmafAdaptationSet *pAS = (OmafAdaptationSet *)(itAS->second);
      EXPECT_TRUE(pAS != NULL);

      ret = pAS->LoadLocalInitSegment();
      EXPECT_TRUE(ret == ERROR_NONE);

      OmafSegment::Ptr initSeg = pAS->GetInitSegment();
      EXPECT_TRUE(initSeg != NULL);

      memset(storedFileName, 0, 1024);
      std::string repId = pAS->GetRepresentationId();
      snprintf(storedFileName, 1024, "./segs_for_readertest/%s.init.mp4", repId.c_str());

      cacheFileName = storedFileName;

      initSeg->SetSegmentCacheFile(cacheFileName);
      initSeg->SetSegStored();
      ret = m_reader->parseInitializationSegment(initSeg.get(), initSegID);
      EXPECT_TRUE(ret == ERROR_NONE);

      initSegID++;
    }

    for (auto itAS = extractorAS.begin(); itAS != extractorAS.end(); itAS++) {
      OmafExtractor *extractor = (OmafExtractor *)(itAS->second);
      EXPECT_TRUE(extractor != NULL);

      ret = extractor->LoadLocalInitSegment();
      EXPECT_TRUE(ret == ERROR_NONE);

      OmafSegment::Ptr initSeg = extractor->GetInitSegment();
      EXPECT_TRUE(initSeg != NULL);

      memset(storedFileName, 0, 1024);
      std::string repId = extractor->GetRepresentationId();
      snprintf(storedFileName, 1024, "./segs_for_readertest/%s.init.mp4", repId.c_str());

      cacheFileName = storedFileName;

      initSeg->SetSegmentCacheFile(cacheFileName);
      initSeg->SetSegStored();
      ret = m_reader->parseInitializationSegment(initSeg.get(), initSegID);
      EXPECT_TRUE(ret == ERROR_NONE);

      initSegID++;
    }
  }

  EXPECT_TRUE(initSegID == 18);

  try {
    // 1. get the track information

    m_reader->getTrackInformations(trackInfos);
    // 2. go through the track information
    for (auto track : trackInfos) {
      if (track == nullptr) {
        OMAF_LOG(LOG_ERROR, "Meet empty track!\n");
        continue;
      }

      // FIXME there would has bug, if more than one stream.
      // or we need update the logic for more than one stream
      for (auto &pStream : m_listStream) {
        // 2.1.1 check the adaptation set
        std::map<int, OmafAdaptationSet *> pMediaAS = pStream->GetMediaAdaptationSet();
        for (auto as : pMediaAS) {
          OmafAdaptationSet *pAS = (OmafAdaptationSet *)as.second;
          // FIXME GetInitSegID or GetSegID
          if (pAS->GetInitSegment()->GetInitSegID() == track->initSegmentId) {
            auto dash_track_id = buildDashTrackId(track->trackId);
            pAS->SetTrackNumber(static_cast<int>(dash_track_id));
            pAS->GetInitSegment()->SetTrackId(dash_track_id);

            OMAF_LOG(LOG_INFO, "Initse id=%u, trackid=%u\n", track->initSegmentId, dash_track_id );
            break;
          }
        }  // end for adaptation set loop

        // 2.1.3 check the extractors
        std::map<int, OmafExtractor *> pExtratorAS = pStream->GetExtractors();
        for (auto &extractor : pExtratorAS) {
          OmafExtractor *pExAS = extractor.second;
          // FIXME GetInitSegID or GetSegID
          if (pExAS->GetInitSegment()->GetInitSegID() == track->initSegmentId) {
            auto dash_track_id = buildDashTrackId(track->trackId);
            pExAS->SetTrackNumber(static_cast<int>(dash_track_id));
            pExAS->GetInitSegment()->SetTrackId(dash_track_id);

            OMAF_LOG(LOG_INFO, "Initse id=%u, trackid=%u\n", track->initSegmentId, dash_track_id);
            break;
          }
        }  // end for extractors loop
      }    // end stream loop
    }      // end for track loop

  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Failed to parse the init segment, ex: %s\n", ex.what());
  }

  /*
      uint16_t idx = 0;
      for ( auto it = trackInfos.begin(); it != trackInfos.end(); it++)
      {
          TrackInformation trackInfo = *it;
          idx++;

          //EXPECT_TRUE(trackInfo.trackId == idx);
          //EXPECT_TRUE(trackInfo.initSegId == (idx - 1));
          //EXPECT_TRUE(trackInfo.alternateGroupID ==
          //printf("trackInfo.alternateGroupID %d \n", trackInfo.alternateGroupId);
          //printf("trackInfo.features %d \n", trackInfo.features);
          //printf("trackInfo.vrFeatures %d \n", trackInfo.vrFeatures);
          //printf("trackInfo.maxSampleSize %d \n", trackInfo.maxSampleSize);
          //printf("trackInfo.timeScale %d \n", trackInfo.timeScale);
          //printf("trackInfo.hasTypeInformation %d \n", trackInfo.hasTypeInformation);
          //EXPECT_TRUE(trackInfo.frameRate.den == 1);
          //EXPECT_TRUE(trackInfo.frameRate.num == 25);

          //EXPECT_TRUE(trackInfo.trackURI.size() != 0);
          for (auto it1 = trackInfo.trackURI.begin();
              it1 != trackInfo.trackURI.end(); it1++)
          {
              //printf("The character in trackURI is %c \n", (*it1));
          }

          //EXPECT_TRUE(trackInfo.alternateTrackIds.size() != 0);
          for (auto it1 = trackInfo.alternateTrackIds.begin();
              it1 != trackInfo.alternateTrackIds.end(); it1++)
          {
              //printf("The value in alternateTrackIds is %d \n", (*it1));
          }

          //EXPECT_TRUE(trackInfo.referenceTrackIds.size() != 0);
          for (auto it1 = trackInfo.referenceTrackIds.begin();
              it1 != trackInfo.referenceTrackIds.end(); it1++)
          {
              TypeToTrackIDs referenceTrackId = *it1;
              //printf("referenceTrackId.type is %s \n", referenceTrackId.type);
              //EXPECT_TRUE(referenceTrackId.trackIds.size() != 0);
              for (auto it2 = referenceTrackId.trackIds.begin();
                  it2 != referenceTrackId.trackIds.end(); it2++)
              {
                  //printf("reference track id is %d \n", (*it2));
              }
          }

          //EXPECT_TRUE(trackInfo.trackGroupIds.size() != 0);
          for (auto it1 = trackInfo.trackGroupIds.begin();
              it1 != trackInfo.trackGroupIds.end(); it1++)
          {
              TypeToTrackIDs trackGroupId = *it1;
              //printf("trackGroupId.type is %s \n", trackGroupId.type);
              //EXPECT_TRUE(trackGroupId.trackIds.size() != 0);
              for (auto it2 = trackGroupId.trackIds.begin();
                  it2 != trackGroupId.trackIds.end(); it2++)
              {
                  //printf("track id in trackGroupIds is %d \n", (*it2));
              }
          }

          //EXPECT_TRUE(trackInfo.sampleProperties.size() != 0);
          for (auto it1 = trackInfo.sampleProperties.begin();
              it1 != trackInfo.sampleProperties.end(); it1++)
          {
              SampleInformation info = *it1;
              //printf("sample info earliestTimestamp %ld \n", info.earliestTimestamp);
              //printf("sample info earliestTimestampTS %ld \n", info.earliestTimestampTS);
              //printf("sample info sampleDescriptionIndex %d \n", info.sampleDescriptionIndex);
              //printf("sample info initSegmentId %d \n", info.initSegmentId);
              //printf("sample info sampleDurationTS %ld \n", info.sampleDurationTS);
              //printf("sample info sampleEntryType %s \n", info.sampleEntryType);
              //printf("sample info sampleId %d \n", info.sampleId);
              //printf("sample info segmentId %d \n", info.segmentId);
              //printf("sample info sampleType %d \n", info.sampleType);
              //printf("sample info sampleFlags flagsAsUInt %d \n", info.sampleFlags.flagsAsUInt);
              //printf("sample info sampleFlags flags is_leading %d \n", info.sampleFlags.flags.is_leading);
              //printf("sample info sampleFlags flags reserved %d \n", info.sampleFlags.flags.reserved);
              //printf("sample info sampleFlags flags sample_degradation_priority %d \n",
     info.sampleFlags.flags.sample_degradation_priority);
              //printf("sample info sampleFlags flags sample_has_redundancy %d \n",
     info.sampleFlags.flags.sample_has_redundancy);
              //printf("sample info sampleFlags flags sample_depends_on %d \n",
     info.sampleFlags.flags.sample_depends_on);
              //printf("sample info sampleFlags flags sample_is_non_sync_sample %d \n",
     info.sampleFlags.flags.sample_is_non_sync_sample);
              //printf("sample info sampleFlags flags sample_padding_value %d \n",
     info.sampleFlags.flags.sample_padding_value);
          }
      }
  */
  std::list<OmafSegment::Ptr> cached_segments;  // dirty implmentation

  for (auto it = m_listStream.begin(); it != m_listStream.end(); it++) {
    OmafMediaStream *stream = (OmafMediaStream *)(*it);
    EXPECT_TRUE(stream != NULL);

    std::map<int, OmafAdaptationSet *> normalAS = stream->GetMediaAdaptationSet();
    std::map<int, OmafExtractor *> extractorAS = stream->GetExtractors();

    for (auto itAS = normalAS.begin(); itAS != normalAS.end(); itAS++) {
      OmafAdaptationSet *pAS = (OmafAdaptationSet *)(itAS->second);
      EXPECT_TRUE(pAS != NULL);

      pAS->Enable(true);

      ret = pAS->LoadLocalSegment();
      EXPECT_TRUE(ret == ERROR_NONE);

      OmafSegment::Ptr newSeg = pAS->GetLocalNextSegment();
      EXPECT_TRUE(newSeg != NULL);

      memset(storedFileName, 0, 1024);
      std::string repId = pAS->GetRepresentationId();
      snprintf(storedFileName, 1024, "./segs_for_readertest/%s.1.mp4", repId.c_str());
      cacheFileName = storedFileName;
      OMAF_LOG(LOG_INFO, "segment file=%s\n", cacheFileName.c_str());
      OMAF_LOG(LOG_INFO, "init seg=%u\n", newSeg->GetInitSegID());
      OMAF_LOG(LOG_INFO, "segid=%d\n", newSeg->GetSegID());
      newSeg->SetSegmentCacheFile(cacheFileName);
      newSeg->SetSegStored();

      ret = m_reader->parseSegment(newSeg.get(), newSeg->GetInitSegID(), newSeg->GetSegID());

      EXPECT_TRUE(ret == ERROR_NONE);
      cached_segments.push_back(std::move(newSeg));
      // ret = m_reader->getTrackInformations(trackInfos);
      // EXPECT_TRUE(ret == ERROR_NONE);
    }

    for (auto itAS = extractorAS.begin(); itAS != extractorAS.end(); itAS++) {
      OmafExtractor *extractor = (OmafExtractor *)(itAS->second);
      EXPECT_TRUE(extractor != NULL);

      extractor->Enable(true);

      ret = extractor->LoadLocalSegment();
      EXPECT_TRUE(ret == ERROR_NONE);

      OmafSegment::Ptr newSeg = extractor->GetLocalNextSegment();
      EXPECT_TRUE(newSeg != NULL);

      memset(storedFileName, 0, 1024);
      std::string repId = extractor->GetRepresentationId();
      snprintf(storedFileName, 1024, "./segs_for_readertest/%s.1.mp4", repId.c_str());

      cacheFileName = storedFileName;
      OMAF_LOG(LOG_INFO, "segment file=%s\n", cacheFileName.c_str());
      OMAF_LOG(LOG_INFO, "init seg=%u\n", newSeg->GetInitSegID());
      OMAF_LOG(LOG_INFO, "segid=%d\n", newSeg->GetSegID());
      newSeg->SetSegmentCacheFile(cacheFileName);
      newSeg->SetSegStored();

      ret = m_reader->parseSegment(newSeg.get(), newSeg->GetInitSegID(), newSeg->GetSegID());
      EXPECT_TRUE(ret == ERROR_NONE);
      cached_segments.push_back(std::move(newSeg));
      // only parse one extractor
      break;
    }
  }

  ret = m_reader->getTrackInformations(trackInfos);
  EXPECT_TRUE(ret == ERROR_NONE);

  FILE *fp = NULL;
  char fileName[256];
  memset(fileName, 0, 256);

  uint8_t vps[256] = {0};
  uint8_t sps[256] = {0};
  uint8_t pps[256] = {0};
  uint8_t vpsLen = 0;
  uint8_t spsLen = 0;
  uint8_t ppsLen = 0;
  for (auto it = m_listStream.begin(); it != m_listStream.end(); it++) {
    OmafMediaStream *stream = (OmafMediaStream *)(*it);
    std::map<int, OmafExtractor *> extractorAS = stream->GetExtractors();
    for (auto &it : stream->GetExtractors()) {
      OmafExtractor *extractor = it.second;
      // for (uint32_t initSegIndex = 10; initSegIndex < 18; initSegIndex++) {
      // uint32_t correspondTrackIdx = initSegIndex + 990;
      // uint32_t trackIdx = (initSegIndex << 16) | correspondTrackIdx;

      auto trackId = extractor->GetTrackNumber();
      auto initsegid = extractor->GetInitSegment()->GetInitSegID();
      auto trackIdx = buildReaderTrackId(trackId, initsegid);
      OMAF_LOG(LOG_INFO, "The trackid=%d\n", trackId);
      OMAF_LOG(LOG_INFO, "init segid=%d\n",initsegid);
      OMAF_LOG(LOG_INFO, "reader trackid=%d\n", trackIdx);

      snprintf(fileName, 256, "Viewport%d.h265", initsegid - 999);
      fp = fopen(fileName, "wb+");
      EXPECT_TRUE(fp != NULL);
      if (!fp) continue;

      auto trackInf = findTrackInformation(m_reader, trackId);
      size_t begin = 0;
      size_t end = 0;
      auto segId = 1;
      if (findSampleIndexRange(trackInf, segId, begin, end)) {
        OMAF_LOG(LOG_INFO, "The begin index=%lld, end=%lld\n", begin, end);
      }

      uint32_t sampleIdx = begin;
      for (; sampleIdx < end; sampleIdx++) {
        uint32_t packetSize = ((7680 * 3840 * 3) / 2) / 2;
        MediaPacket *packet = new MediaPacket();
        if (NULL == packet) {
          break;
        }
        packet->ReAllocatePacket(packetSize);

        if (!packet->Payload()) {
          delete packet;
          break;
        }

        ret = m_reader->getExtractorTrackSampleData(trackIdx, sampleIdx, (char *)(packet->Payload()), packetSize);
        OMAF_LOG(LOG_INFO, "Extractor track sample data, ret=%d\n", ret);
        EXPECT_TRUE(ret == ERROR_NONE);

        if (sampleIdx == 0) {
          std::vector<VCD::OMAF::DecoderSpecificInfo> parameterSets;
          ret = m_reader->getDecoderConfiguration(trackIdx, sampleIdx, parameterSets);
          EXPECT_TRUE(ret == ERROR_NONE);

          for (auto const &parameter : parameterSets) {
            if (parameter.codecSpecInfoType == VCD::MP4::HEVC_VPS) {
              vpsLen = parameter.codecSpecInfoBits.size;
              for (uint32_t i = 0; i < parameter.codecSpecInfoBits.size; i++) {
                vps[i] = parameter.codecSpecInfoBits[i];
              }
            }

            if (parameter.codecSpecInfoType == VCD::MP4::HEVC_SPS) {
              spsLen = parameter.codecSpecInfoBits.size;
              for (uint32_t i = 0; i < parameter.codecSpecInfoBits.size; i++) {
                sps[i] = parameter.codecSpecInfoBits[i];
              }
            }

            if (parameter.codecSpecInfoType == VCD::MP4::HEVC_PPS) {
              ppsLen = parameter.codecSpecInfoBits.size;
              for (uint32_t i = 0; i < parameter.codecSpecInfoBits.size; i++) {
                pps[i] = parameter.codecSpecInfoBits[i];
              }
            }
          }

          fwrite(vps, 1, vpsLen, fp);
          fwrite(sps, 1, spsLen, fp);
          fwrite(pps, 1, ppsLen, fp);
        }

        fwrite((uint8_t *)(packet->Payload()), 1, packetSize, fp);

        delete packet;
        packet = NULL;
      }
      // }
      break;
    }

    fclose(fp);
    fp = NULL;
  }
}  // namespace
}  // namespace
