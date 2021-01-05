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

#include "OmafDashRangeSync.h"
#include "OmafReaderManager.h"
#include "OmafTileTracksSelector.h"
#include "OmafMediaStream.h"
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
#include "../trace/MtHQ_tp.h"
#endif
#endif

VCD_OMAF_BEGIN

OmafMediaStream::OmafMediaStream() {
  mMainAdaptationSet = NULL;
  mExtratorAdaptationSet = NULL;
  m_pStreamInfo = NULL;
  m_bEOS = false;
  mStreamID = 0;
  m_hasTileTracksSelected = false;
  m_stitchThread = 0;
  m_enabledExtractor = true;
  m_stitch = NULL;
  m_needParams = false;
  m_currFrameIdx = 0;
  m_status = STATUS_UNKNOWN;
  m_activeSegmentNum = 0;
  m_tileSelTimeLine  = 0;
}

OmafMediaStream::~OmafMediaStream() {
  SAFE_DELETE(m_pStreamInfo->codec);
  SAFE_DELETE(m_pStreamInfo->mime_type);
  if (m_pStreamInfo->stream_type == MediaType_Video)
  {
    SAFE_DELETE(m_pStreamInfo->source_resolution);
    SAFE_FREE(mMainAdaptationSet);
  }
  SAFE_FREE(m_pStreamInfo);
  std::map<uint64_t, std::map<int, OmafAdaptationSet*>>::iterator itSel;
  for (itSel = m_selectedTileTracks.begin(); itSel != m_selectedTileTracks.end(); )
  {
    (itSel->second).clear();
    m_selectedTileTracks.erase(itSel++);
  }
  m_selectedTileTracks.clear();
  if (mMediaAdaptationSet.size()) {
    for (auto& it : mMediaAdaptationSet) {
      SAFE_DELETE(it.second);
      mMediaAdaptationSet.erase(it.first);
    }
    mMediaAdaptationSet.clear();
  }
  if (mExtractors.size()) {
    for (auto& it : mExtractors) {
      SAFE_DELETE(it.second);
      mExtractors.erase(it.first);
    }
    mExtractors.clear();
  }
  if (m_stitchThread) {
    pthread_join(m_stitchThread, NULL);
    m_stitchThread = 0;
  }

  if (m_mergedPackets.size()) {
    std::list<std::list<MediaPacket*>>::iterator it;
    for (it = m_mergedPackets.begin(); it != m_mergedPackets.end();) {
      std::list<MediaPacket*> packets = *it;
      if (packets.size()) {
        std::list<MediaPacket*>::iterator itPacket;
        for (itPacket = packets.begin(); itPacket != packets.end();) {
          MediaPacket* packet = *itPacket;
          SAFE_DELETE(packet);
          packets.erase(itPacket++);
        }
        packets.clear();
      }
      m_mergedPackets.erase(it++);
    }
    m_mergedPackets.clear();
  }
  SAFE_DELETE(m_stitch);
  m_sources.clear();
}

void OmafMediaStream::SetOmafReaderMgr(std::shared_ptr<OmafReaderManager> mgr) noexcept {
  omaf_reader_mgr_ = std::move(mgr);

  std::lock_guard<std::mutex> lock(mMutex);
  for (auto it = mMediaAdaptationSet.begin(); it != mMediaAdaptationSet.end(); it++) {
    OmafAdaptationSet* pAS = (OmafAdaptationSet*)(it->second);
    pAS->SetOmafReaderMgr(omaf_reader_mgr_);
  }

  if (m_enabledExtractor) {
    for (auto extrator_it = mExtractors.begin(); extrator_it != mExtractors.end(); extrator_it++) {
      OmafExtractor* extractor = (OmafExtractor*)(extrator_it->second);
      extractor->SetOmafReaderMgr(omaf_reader_mgr_);
    }
  }
}

void OmafMediaStream::Close() {
  if (m_status != STATUS_STOPPED) {
    m_status = STATUS_STOPPED;
    if (m_stitchThread) {
      pthread_join(m_stitchThread, NULL);
      m_stitchThread = 0;
    }
  }
}

int OmafMediaStream::AddExtractor(OmafExtractor* pAS) {
  if (NULL != pAS) mExtractors[pAS->GetID()] = pAS;

  return ERROR_NONE;
}

int OmafMediaStream::AddAdaptationSet(OmafAdaptationSet* pAS) {
  if (NULL != pAS) {
    mMediaAdaptationSet[pAS->GetID()] = pAS;
  }
  return ERROR_NONE;
}

int OmafMediaStream::InitStream(std::string type) {
  if (NULL == m_pStreamInfo) m_pStreamInfo = (DashStreamInfo*)malloc(sizeof(DashStreamInfo));

  if (NULL == m_pStreamInfo) {
    return ERROR_NULL_PTR;
  }

  if (type == "video") {
    m_pStreamInfo->stream_type = MediaType_Video;
  } else if (type == "audio") {
    m_pStreamInfo->stream_type = MediaType_Audio;
  } else {
    return ERROR_INVALID;
  }
  if (type == "video")
  {
      if (!m_enabledExtractor && !m_stitch) {
        m_stitch = new OmafTilesStitch();
        if (!m_stitch) return OMAF_ERROR_NULL_PTR;
      }
  }
  UpdateStreamInfo();

  if (type == "video")
  {
      SetupExtratorDependency();

      if (!m_enabledExtractor) {
        int32_t ret = StartTilesStitching();
        if (ret) {
          OMAF_LOG(LOG_ERROR, "Failed to start tiles stitching !\n");
          return ret;
        }
      }
  }

  return ERROR_NONE;
}

OMAF_STATUS OmafMediaStream::UpdateStreamInfo() {
  if (!mMediaAdaptationSet.size()) return OMAF_ERROR_INVALID_DATA;

  if (m_enabledExtractor) {
    if (NULL != mMainAdaptationSet && NULL != mExtratorAdaptationSet) {
      VideoInfo vi = mMainAdaptationSet->GetVideoInfo();
      AudioInfo ai = mMainAdaptationSet->GetAudioInfo();

      m_pStreamInfo->bit_rate = vi.bit_rate;

      m_pStreamInfo->framerate_den = vi.frame_Rate.den;
      m_pStreamInfo->framerate_num = vi.frame_Rate.num;
      m_pStreamInfo->width = mExtratorAdaptationSet->GetQualityRanking()->srqr_quality_infos[0].orig_width;
      m_pStreamInfo->height = mExtratorAdaptationSet->GetQualityRanking()->srqr_quality_infos[0].orig_height;
      m_pStreamInfo->mime_type = new char[1024];
      m_pStreamInfo->codec = new char[1024];
      memcpy_s(const_cast<char*>(m_pStreamInfo->mime_type), 1024, mMainAdaptationSet->GetMimeType().c_str(), 1024);
      memcpy_s(const_cast<char*>(m_pStreamInfo->codec), 1024, mMainAdaptationSet->GetCodec()[0].c_str(), 1024);
      m_pStreamInfo->mFpt = (int32_t)mMainAdaptationSet->GetFramePackingType();
      m_pStreamInfo->mProjFormat = (int32_t)mMainAdaptationSet->GetProjectionFormat();
      m_pStreamInfo->segmentDuration = mMainAdaptationSet->GetSegmentDuration();

      m_pStreamInfo->channel_bytes = ai.channel_bytes;
      m_pStreamInfo->channels = ai.channels;
      m_pStreamInfo->sample_rate = ai.sample_rate;

      int sourceNumber = mExtratorAdaptationSet->GetQualityRanking()->srqr_quality_infos.size();
      m_pStreamInfo->source_number = sourceNumber;
      m_pStreamInfo->source_resolution = new SourceResolution[sourceNumber];
      for (int i = 0; i < sourceNumber; i++) {
        m_pStreamInfo->source_resolution[i].qualityRanking = static_cast<QualityRank>(
            mExtratorAdaptationSet->GetQualityRanking()->srqr_quality_infos[i].quality_ranking);
        m_pStreamInfo->source_resolution[i].width =
            mExtratorAdaptationSet->GetQualityRanking()->srqr_quality_infos[i].orig_width;
        m_pStreamInfo->source_resolution[i].height =
            mExtratorAdaptationSet->GetQualityRanking()->srqr_quality_infos[i].orig_height;
      }
      std::map<int, OmafAdaptationSet*>::iterator itAS;
      for (itAS = mMediaAdaptationSet.begin(); itAS != mMediaAdaptationSet.end(); itAS++) {
        if (mMainAdaptationSet == (itAS->second)) break;
      }
      if (itAS != mMediaAdaptationSet.end()) {
        mMediaAdaptationSet.erase(itAS);
      }
    }
  } else {
    if ((m_pStreamInfo->stream_type == MediaType_Video) && (NULL != mMainAdaptationSet)) {
      VideoInfo vi = mMainAdaptationSet->GetVideoInfo();
      //AudioInfo ai = mMainAdaptationSet->GetAudioInfo();

      m_pStreamInfo->bit_rate = vi.bit_rate;

      m_pStreamInfo->framerate_den = vi.frame_Rate.den;
      m_pStreamInfo->framerate_num = vi.frame_Rate.num;
      m_pStreamInfo->height = vi.height;  // mExtratorAdaptationSet->GetVideoInfo().height;
      m_pStreamInfo->width = vi.width;    // mExtratorAdaptationSet->GetVideoInfo().width;
      m_pStreamInfo->mime_type = new char[1024];
      m_pStreamInfo->codec = new char[1024];
      memcpy_s(const_cast<char*>(m_pStreamInfo->mime_type), 1024, mMainAdaptationSet->GetMimeType().c_str(), 1024);
      memcpy_s(const_cast<char*>(m_pStreamInfo->codec), 1024, mMainAdaptationSet->GetCodec()[0].c_str(), 1024);
      m_pStreamInfo->mFpt = (int32_t)mMainAdaptationSet->GetFramePackingType();
      m_pStreamInfo->mProjFormat = (int32_t)mMainAdaptationSet->GetProjectionFormat();
      m_pStreamInfo->segmentDuration = mMainAdaptationSet->GetSegmentDuration();

      //m_pStreamInfo->channel_bytes = ai.channel_bytes;
      //m_pStreamInfo->channels = ai.channels;
      //m_pStreamInfo->sample_rate = ai.sample_rate;

      std::set<QualityRank> allQualities;
      std::map<int, OmafAdaptationSet*>::iterator itAS;
      for (itAS = mMediaAdaptationSet.begin(); itAS != mMediaAdaptationSet.end(); itAS++) {
        if (mMainAdaptationSet == (itAS->second)) break;
      }
      if (itAS != mMediaAdaptationSet.end()) {
        mMediaAdaptationSet.erase(itAS);
      }

      for (itAS = mMediaAdaptationSet.begin(); itAS != mMediaAdaptationSet.end(); itAS++) {
        OmafAdaptationSet* adaptationSet = itAS->second;
        auto qualityRanking = adaptationSet->GetRepresentationQualityRanking();
        allQualities.insert(qualityRanking);
      }
      std::set<QualityRank>::reverse_iterator itQuality;
      for (itQuality = allQualities.rbegin(); itQuality != allQualities.rend(); itQuality++) {
        auto quality = *itQuality;
        int32_t width = 0;
        int32_t height = 0;
        for (itAS = mMediaAdaptationSet.begin(); itAS != mMediaAdaptationSet.end(); itAS++) {
          OmafAdaptationSet* adaptationSet = itAS->second;
          OmafSrd* srd = adaptationSet->GetSRD();
          int32_t tileWidth = srd->get_W();
          int32_t tileHeight = srd->get_H();
          int32_t tileLeft = srd->get_X();
          int32_t tileTop = srd->get_Y();
          uint32_t qualityRanking = adaptationSet->GetRepresentationQualityRanking();
          if (qualityRanking == quality) {
            if (tileTop == 0) {
              width += tileWidth;
            }

            if (tileLeft == 0) {
              height += tileHeight;
            }
          }
        }

        SourceInfo oneSrc;
        oneSrc.qualityRanking = quality;
        oneSrc.width = width;
        oneSrc.height = height;
        m_sources.insert(make_pair(quality, oneSrc));
      }

      int sourceNumber = m_sources.size();
      m_pStreamInfo->source_number = sourceNumber;
      m_pStreamInfo->source_resolution = new SourceResolution[sourceNumber];
      std::map<uint32_t, SourceInfo>::iterator itSrc;
      itSrc = m_sources.begin();
      for (int i = 0; ((i < sourceNumber) && (itSrc != m_sources.end())); i++) {
        SourceInfo oneSrc = itSrc->second;

        m_pStreamInfo->source_resolution[i].qualityRanking = oneSrc.qualityRanking;
        m_pStreamInfo->source_resolution[i].width = oneSrc.width;
        m_pStreamInfo->source_resolution[i].height = oneSrc.height;
        itSrc++;
      }
    }
    else if (m_pStreamInfo->stream_type == MediaType_Audio)
    {
      AudioInfo ai = mMainAdaptationSet->GetAudioInfo();
      m_pStreamInfo->channel_bytes = ai.channel_bytes;
      m_pStreamInfo->channels = ai.channels;
      m_pStreamInfo->sample_rate = ai.sample_rate;

      std::map<int, OmafAdaptationSet*>::iterator itAS;
      itAS = mMediaAdaptationSet.begin();
      OmafAdaptationSet *as = itAS->second;

      m_pStreamInfo->mime_type = new char[1024];
      m_pStreamInfo->codec = new char[1024];
      memcpy_s(const_cast<char*>(m_pStreamInfo->mime_type), 1024, as->GetMimeType().c_str(), 1024);
      memcpy_s(const_cast<char*>(m_pStreamInfo->codec), 1024, as->GetCodec()[0].c_str(), 1024);
      m_pStreamInfo->segmentDuration = as->GetSegmentDuration();
      OMAF_LOG(LOG_INFO, "Audio mime type %s\n", m_pStreamInfo->mime_type);
      OMAF_LOG(LOG_INFO, "Audio codec %s\n", m_pStreamInfo->codec);
      OMAF_LOG(LOG_INFO, "Audio segment duration %ld\n", m_pStreamInfo->segmentDuration);
    }
  }

  if (m_pStreamInfo->stream_type == MediaType_Video)
  {
      uint32_t rowNum = 0, colNum = 0;
      for (auto& it : mMediaAdaptationSet) {
        OmafAdaptationSet* as = it.second;
        QualityRank qr = as->GetRepresentationQualityRanking();
        // only calculate tile segmentation of the stream with highest resolution
        if (qr == HIGHEST_QUALITY_RANKING) {
          OmafSrd* srd = as->GetSRD();
          if (srd->get_X() == 0) rowNum++;
          if (srd->get_Y() == 0) colNum++;
        }
      }
      m_pStreamInfo->tileRowNum = rowNum;
      m_pStreamInfo->tileColNum = colNum;

      if (m_pStreamInfo->mProjFormat == VCD::OMAF::ProjectionFormat::PF_CUBEMAP) {
        std::map<int, OmafAdaptationSet*>::iterator itAS;
        for (itAS = mMediaAdaptationSet.begin(); itAS != mMediaAdaptationSet.end(); itAS++) {
          OmafAdaptationSet* adaptationSet = itAS->second;
          uint32_t qualityRanking = adaptationSet->GetRepresentationQualityRanking();
          if (qualityRanking == HIGHEST_QUALITY_RANKING) {
            OmafSrd* srd = adaptationSet->GetSRD();
            TileDef* oneTile = adaptationSet->GetTileInfo();
            if (!oneTile) {
              OMAF_LOG(LOG_ERROR, "Un-matched projection format !\n");
              return OMAF_ERROR_INVALID_PROJECTIONTYPE;
            }
            int32_t globalX = oneTile->x;
            int32_t globalY = oneTile->y;
            int32_t faceWidth = m_pStreamInfo->width / 3;
            int32_t faceHeight = m_pStreamInfo->height / 2;
            int32_t faceColId = globalX / faceWidth;
            int32_t faceRowId = globalY / faceHeight;
            int32_t localX = globalX % faceWidth;
            int32_t localY = globalY % faceHeight;
            int32_t tileWidth = srd->get_W();
            int32_t tileHeight = srd->get_H();
            if (faceRowId == 0) {
              if (faceColId == 0) {
                oneTile->faceId = 2;
                oneTile->x = localX;
                oneTile->y = localY;
              } else if (faceColId == 1) {
                oneTile->faceId = 0;
                oneTile->x = localX;
                oneTile->y = localY;
              } else if (faceColId == 2) {
                oneTile->faceId = 3;
                oneTile->x = localX;
                oneTile->y = localY;
              }
            } else if (faceRowId == 1) {
              if (faceColId == 0) {
                oneTile->faceId = 5;
                oneTile->y = localX;
                oneTile->x = faceHeight - tileHeight - localY;
              } else if (faceColId == 1) {
                oneTile->faceId = 1;
                oneTile->x = localX;
                oneTile->y = localY;
              } else if (faceColId == 2) {
                oneTile->faceId = 4;
                oneTile->y = faceWidth - tileWidth - localX;
                oneTile->x = localY;
              }
            }
          }
        }
      }
  }

  return ERROR_NONE;
}

void OmafMediaStream::SetupExtratorDependency() {
  for (auto extrator_it = mExtractors.begin(); extrator_it != mExtractors.end(); extrator_it++) {
    OmafExtractor* extractor = (OmafExtractor*)(extrator_it->second);
    for (auto it = mMediaAdaptationSet.begin(); it != mMediaAdaptationSet.end(); it++) {
      OmafAdaptationSet* pAS = (OmafAdaptationSet*)(it->second);
      extractor->AddDependAS(pAS);
    }
  }
}

int OmafMediaStream::SetupSegmentSyncer(const OmafDashParams& params) {
  OmafDashRangeSync::Ptr syncer;
  OMAF_LOG(LOG_INFO, "Setup segment window syncer!\n");
  auto as = mMediaAdaptationSet.begin();
  if (as != mMediaAdaptationSet.end()) {
    OMAF_LOG(LOG_INFO, "Create one dash window syncer!\n");
    syncer = make_omaf_syncer(*as->second, [this](SegmentSyncNode node) {
      std::lock_guard<std::mutex> lock(this->mMutex);
      OMAF_LOG(LOG_INFO, "Syncer segment number to value=%lld\n", node.segment_value.number_);
      for (auto it = this->mMediaAdaptationSet.begin(); it != this->mMediaAdaptationSet.end(); it++) {
        OmafAdaptationSet* pAS = (OmafAdaptationSet*)(it->second);
        pAS->UpdateSegmentNumber(node.segment_value.number_);
      }

      for (auto extrator_it = this->mExtractors.begin(); extrator_it != this->mExtractors.end(); extrator_it++) {
        OmafExtractor* extractor = (OmafExtractor*)(extrator_it->second);
        extractor->UpdateSegmentNumber(node.segment_value.number_);
      }
    });
  }

  if (syncer) {
    syncer_helper_.addSyncer(syncer);

    CurlParams curl_params;
    curl_params.http_params_ = params.http_params_;
    curl_params.http_proxy_ = params.http_proxy_;
    syncer_helper_.start(curl_params);
  }

  return ERROR_NONE;
}

int OmafMediaStream::UpdateStartNumber(uint64_t nAvailableStartTime) {
  int ret = ERROR_NONE;

  std::lock_guard<std::mutex> lock(mMutex);
  for (auto it = mMediaAdaptationSet.begin(); it != mMediaAdaptationSet.end(); it++) {
    OmafAdaptationSet* pAS = (OmafAdaptationSet*)(it->second);
    pAS->UpdateStartNumberByTime(nAvailableStartTime);
  }

  for (auto extrator_it = mExtractors.begin(); extrator_it != mExtractors.end(); extrator_it++) {
    OmafExtractor* extractor = (OmafExtractor*)(extrator_it->second);
    extractor->UpdateStartNumberByTime(nAvailableStartTime);
  }
  return ret;
}

int OmafMediaStream::DownloadInitSegment() {
  std::lock_guard<std::mutex> lock(mMutex);
  for (auto it = mMediaAdaptationSet.begin(); it != mMediaAdaptationSet.end(); it++) {
    OmafAdaptationSet* pAS = (OmafAdaptationSet*)(it->second);
    pAS->DownloadInitializeSegment();
  }

  if (m_enabledExtractor) {
    for (auto extrator_it = mExtractors.begin(); extrator_it != mExtractors.end(); extrator_it++) {
      OmafExtractor* extractor = (OmafExtractor*)(extrator_it->second);
      extractor->DownloadInitializeSegment();
    }
  }

  return ERROR_NONE;
}

int OmafMediaStream::DownloadSegments() {
  int ret = ERROR_NONE;
  std::lock_guard<std::mutex> lock(mMutex);
  for (auto it = mMediaAdaptationSet.begin(); it != mMediaAdaptationSet.end(); it++) {
    OmafAdaptationSet* pAS = (OmafAdaptationSet*)(it->second);
    pAS->DownloadSegment();
  }

  // NOTE: this function should be in the same thread with UpdateEnabledExtractors
  //       , otherwise mCurrentExtractors need a mutex lock
  // pthread_mutex_lock(&mCurrentMutex);
  for (auto extrator_it = mExtractors.begin(); extrator_it != mExtractors.end(); extrator_it++) {
    OmafExtractor* extractor = (OmafExtractor*)(extrator_it->second);
    extractor->DownloadSegment();
  }
  // pthread_mutex_unlock(&mCurrentMutex);
  return ret;
}

int OmafMediaStream::SeekTo(int seg_num) {
  int ret = ERROR_NONE;
  std::lock_guard<std::mutex> lock(mMutex);
  for (auto it = mMediaAdaptationSet.begin(); it != mMediaAdaptationSet.end(); it++) {
    OmafAdaptationSet* pAS = (OmafAdaptationSet*)(it->second);
    pAS->SeekTo(seg_num);
  }

  for (auto extrator_it = mExtractors.begin(); extrator_it != mExtractors.end(); extrator_it++) {
    OmafExtractor* extractor = (OmafExtractor*)(extrator_it->second);
    extractor->SeekTo(seg_num);
  }
  return ret;
}

int OmafMediaStream::UpdateEnabledExtractors(std::list<OmafExtractor*> extractors) {
  if (extractors.empty()) return ERROR_INVALID;

  int ret = ERROR_NONE;

  {
    std::lock_guard<std::mutex> lock(mMutex);
    for (auto as_it1 = mMediaAdaptationSet.begin(); as_it1 != mMediaAdaptationSet.end(); as_it1++) {
      OmafAdaptationSet* pAS = (OmafAdaptationSet*)(as_it1->second);
      pAS->Enable(false);
    }
    for (auto extrator_it = mExtractors.begin(); extrator_it != mExtractors.end(); extrator_it++) {
      OmafExtractor* extractor = (OmafExtractor*)(extrator_it->second);
      extractor->Enable(false);
    }

    {
      std::lock_guard<std::mutex> lock(mCurrentMutex);
      mCurrentExtractors.clear();
      for (auto it = extractors.begin(); it != extractors.end(); it++) {
        OmafExtractor* tmp = (OmafExtractor*)(*it);
        tmp->Enable(true);
        mCurrentExtractors.push_back(tmp);
        std::map<int, OmafAdaptationSet*> AS = tmp->GetDependAdaptationSets();
        for (auto as_it = AS.begin(); as_it != AS.end(); as_it++) {
          OmafAdaptationSet* pAS = (OmafAdaptationSet*)(as_it->second);
          pAS->Enable(true);
        }
      }
    }
  }

  return ret;
}

int OmafMediaStream::EnableAllAudioTracks() {
  int ret = ERROR_NONE;

  {
    std::lock_guard<std::mutex> lock(mMutex);
    for (auto as_it1 = mMediaAdaptationSet.begin(); as_it1 != mMediaAdaptationSet.end(); as_it1++) {
      OmafAdaptationSet* pAS = (OmafAdaptationSet*)(as_it1->second);
      pAS->Enable(true);
    }
  }

  return ret;
}
int OmafMediaStream::UpdateEnabledTileTracks(std::map<int, OmafAdaptationSet*> selectedTiles) {
  if (selectedTiles.empty()) return ERROR_INVALID;

  int ret = ERROR_NONE;

  {
    std::lock_guard<std::mutex> lock(mMutex);
    for (auto as_it1 = mMediaAdaptationSet.begin(); as_it1 != mMediaAdaptationSet.end(); as_it1++) {
      OmafAdaptationSet* pAS = (OmafAdaptationSet*)(as_it1->second);
      pAS->Enable(false);
    }
    for (auto extrator_it = mExtractors.begin(); extrator_it != mExtractors.end(); extrator_it++) {
      OmafExtractor* extractor = (OmafExtractor*)(extrator_it->second);
      extractor->Enable(false);
    }

    {
      std::lock_guard<std::mutex> lock(mCurrentMutex);
      //m_selectedTileTracks.clear();
      OMAF_LOG(LOG_INFO, "Will insert tiles selection for time line %ld\n", m_tileSelTimeLine);
      std::map<int, OmafAdaptationSet*> oneSelection;
      for (auto itAS = selectedTiles.begin(); itAS != selectedTiles.end(); itAS++) {
        OmafAdaptationSet* adaptationSet = itAS->second;
        adaptationSet->Enable(true);
        OMAF_LOG(LOG_INFO, "Insert track %d for time line %ld\n", itAS->first, m_tileSelTimeLine);
        oneSelection.insert(make_pair(itAS->first, itAS->second));
      }
      //m_selectedTileTracks.push_back(oneSelection);
      m_selectedTileTracks.insert(make_pair(m_tileSelTimeLine, oneSelection));
      m_tileSelTimeLine++;
      m_hasTileTracksSelected = true;
    }
  }

  return ret;
}

int OmafMediaStream::GetTrackCount() {
  int tracksCnt = 0;
  if (m_enabledExtractor) {
    tracksCnt = this->mMediaAdaptationSet.size() + this->mExtractors.size();
  } else {
    tracksCnt = this->mMediaAdaptationSet.size();
  }
  return tracksCnt;
}

int32_t OmafMediaStream::StartTilesStitching() {
  int32_t ret = pthread_create(&m_stitchThread, NULL, TilesStitchingThread, this);
  if (ret) {
    OMAF_LOG(LOG_ERROR, "Failed to create tiles stitching thread !\n");
    return OMAF_ERROR_CREATE_THREAD;
  }

  return ERROR_NONE;
}

void* OmafMediaStream::TilesStitchingThread(void* pThis) {
  OmafMediaStream* pStream = (OmafMediaStream*)pThis;

  pStream->TilesStitching();

  return NULL;
}

static bool IsSelectionChanged(TracksMap selection1, TracksMap selection2) {
  bool isChanged = false;

  if (selection1.size() && selection2.size()) {
    if (selection1.size() != selection2.size()) {
      isChanged = true;
    } else {
      std::map<int, OmafAdaptationSet*>::iterator it1;
      for (it1 = selection1.begin(); it1 != selection1.end(); it1++) {
        OmafAdaptationSet* as1 = it1->second;
        std::map<int, OmafAdaptationSet*>::iterator it2;
        for (it2 = selection2.begin(); it2 != selection2.end(); it2++) {
          OmafAdaptationSet* as2 = it2->second;
          if (as1 == as2) {
            break;
          }
        }
        if (it2 == selection2.end()) {
          isChanged = true;
          break;
        }
      }
    }
  }

  return isChanged;
}

int32_t OmafMediaStream::TilesStitching() {
  if (!m_stitch) {
    OMAF_LOG(LOG_ERROR, "Tiles stitching handle hasn't been created !\n");
    return OMAF_ERROR_NULL_PTR;
  }
  int ret = ERROR_NONE;
  bool selectedFlag = false;
  uint32_t wait_time = 30000;
  uint32_t current_wait_time = 0;

  do
  {
    {
      std::lock_guard<std::mutex> lock(mCurrentMutex);
      selectedFlag = m_hasTileTracksSelected;
    }
    usleep(100);
    current_wait_time++;
    if (current_wait_time > wait_time)
    {
      OMAF_LOG(LOG_ERROR, "Time out for tile track select!\n");
      return ERROR_INVALID;
    }
  }while (!selectedFlag);

  uint64_t currFramePTS = 0;
  uint64_t currSegTimeLine = 0;
  std::map<int, OmafAdaptationSet*> mapSelectedAS;
  bool isEOS = false;
  uint32_t waitTimes = 1000;
  uint32_t selectionWaitTimes = 10000;
  bool prevPoseChanged = false;
  std::map<int, OmafAdaptationSet*> prevSelectedAS;
  bool segmentEnded = false;
  size_t samplesNumPerSeg = 0;
  size_t aveSamplesNumPerSeg = 0;
  bool skipFrames = false;
  bool beginNewSeg = false;
  while (!isEOS && m_status != STATUS_STOPPED) {
    beginNewSeg = false;

    if (aveSamplesNumPerSeg && !skipFrames)
        currFramePTS++;

    skipFrames = false;
    if (aveSamplesNumPerSeg)
    {
        if ((currFramePTS / aveSamplesNumPerSeg + 1) > currSegTimeLine)
        {
            beginNewSeg = true;
        }
        currSegTimeLine = currFramePTS / aveSamplesNumPerSeg + 1;
    }
    // begin to generate tiles merged media packets for each frame
    OMAF_LOG(LOG_INFO, "Begin stitch frame %ld from segment %ld\n", currFramePTS, currSegTimeLine);
    OMAF_LOG(LOG_INFO, "Begin new seg %d and samples num per seg %ld\n", beginNewSeg, samplesNumPerSeg);
    uint32_t currWaitTimes = 0;
    std::map<int, OmafAdaptationSet*> updatedSelectedAS;

    if (prevSelectedAS.empty() || beginNewSeg)
    {
        if (prevSelectedAS.empty())
        {
          while (currWaitTimes < selectionWaitTimes)
          {
            {
              std::lock_guard<std::mutex> lock(mCurrentMutex);
              if (m_selectedTileTracks.size() >= 2)
                  break;
            }
            usleep(50);
            currWaitTimes++;
          }
          if (currWaitTimes >= selectionWaitTimes)
          {
            OMAF_LOG(LOG_ERROR, "Wait too much time for tiles selection, timed out !\n");
            break;
          }
          currWaitTimes = 0;

          {
            std::lock_guard<std::mutex> lock(mCurrentMutex);

            //m_selectedTileTracks.pop_front(); //At the beginning, there are two same tiles selection in m_selectedTileTracks due to previous process in StartReadThread, so remove repeated one
            updatedSelectedAS = m_selectedTileTracks[1]; //At the beginning, there are two same tiles selection in m_selectedTileTracks due to previous process in StartReadThread, so remove repeated one
          }
          currSegTimeLine = 1;
        }
        else
        {
          while (currWaitTimes < selectionWaitTimes)
          {
            {
              std::lock_guard<std::mutex> lock(mCurrentMutex);
              std::map<uint64_t, std::map<int, OmafAdaptationSet*>>::iterator it;
              it = m_selectedTileTracks.find(currSegTimeLine);
              if (it != m_selectedTileTracks.end())
                  break;
            }

            usleep((m_pStreamInfo->segmentDuration * 1000000) / selectionWaitTimes);
            currWaitTimes++;
          }

          {
            std::lock_guard<std::mutex> lock(mCurrentMutex);
            if (currWaitTimes < selectionWaitTimes)
            {
              updatedSelectedAS = m_selectedTileTracks[currSegTimeLine];
            }
            else
            {
              if (m_status == STATUS_STOPPED)
              {
                OMAF_LOG(LOG_INFO, "Status Stopped !\n");
                break;
              }
              else
              {
                updatedSelectedAS = prevSelectedAS;
                OMAF_LOG(LOG_WARNING, "Tile tracks selection result for current time line hasn't come, Still use previous selected AS !\n");
              }
            }
          }
          currWaitTimes = 0;
        }
        mapSelectedAS = updatedSelectedAS;
        OMAF_LOG(LOG_INFO, "For frame next to frame %ld, Use updated viewport !\n", currFramePTS);
    }
    else
    {
        mapSelectedAS = prevSelectedAS;
        OMAF_LOG(LOG_INFO, "For frame next to frame %ld, Use last viewport !\n", currFramePTS);
    }

    prevPoseChanged = prevSelectedAS.empty() ? false : IsSelectionChanged(mapSelectedAS, prevSelectedAS);

    prevSelectedAS = mapSelectedAS;
    bool hasPktOutdated = false;
    std::map<uint32_t, MediaPacket*> selectedPackets;
    for (auto as_it = mapSelectedAS.begin(); as_it != mapSelectedAS.end(); as_it++) {
      OmafAdaptationSet* pAS = (OmafAdaptationSet*)(as_it->second);
      int32_t trackID = pAS->GetTrackNumber();
      MediaPacket* onePacket = NULL;
      if (!(m_stitch->IsInitialized())) m_needParams = true;

      if (prevPoseChanged) m_needParams = true;

      // ret = READERMANAGER::GetInstance()->GetNextFrame(trackID, onePacket, m_needParams);
      if (as_it != mapSelectedAS.begin()) {
        uint64_t pts = omaf_reader_mgr_->GetOldestPacketPTSForTrack(trackID);
        if (pts > currFramePTS) {
          OMAF_LOG(LOG_INFO, "For current PTS %ld :\n", currFramePTS);
          OMAF_LOG(LOG_INFO, "Outdated PTS %ld from track %d\n", pts, trackID);
          hasPktOutdated = true;
          if (samplesNumPerSeg == aveSamplesNumPerSeg)
          {
              if (pts % samplesNumPerSeg)
              {
                  pts = ((pts / samplesNumPerSeg) + 1) * samplesNumPerSeg;
              }
          }
          else //most likely current segment is the last segment
          {
              pts = aveSamplesNumPerSeg * (currSegTimeLine - 1) + samplesNumPerSeg;
          }

          currFramePTS = pts;
          beginNewSeg = true;
          skipFrames = true;
          break;
        } else if (pts < currFramePTS) {

          if (pts == 0)
          {
              while((!pts) && (currWaitTimes < waitTimes) && (m_status != STATUS_STOPPED))
              {
                  usleep(((m_pStreamInfo->segmentDuration * 1000000) / 2) / waitTimes);
                  currWaitTimes++;
                  pts = omaf_reader_mgr_->GetOldestPacketPTSForTrack(trackID);
              }
              if (currWaitTimes >= waitTimes)
              {
                  OMAF_LOG(LOG_INFO, "Wait times has timed out for frame %ld from track %d\n", currFramePTS, trackID);
              }
              currWaitTimes = 0;
              if (pts > currFramePTS)
              {
                  OMAF_LOG(LOG_INFO, "After wait for a moment, outdated PTS %ld from track %d\n", pts, trackID);
                  hasPktOutdated = true;
                  if (samplesNumPerSeg == aveSamplesNumPerSeg)
                  {
                      if (pts % samplesNumPerSeg)
                      {
                          pts = ((pts / samplesNumPerSeg) + 1) * samplesNumPerSeg;
                      }
                  }
                  else //most likely current segment is the last segment
                  {
                      pts = aveSamplesNumPerSeg * (currSegTimeLine - 1) + samplesNumPerSeg;
                  }

                  currFramePTS = pts;
                  skipFrames = true;
                  break;
              } else if (pts < currFramePTS) {
                  omaf_reader_mgr_->RemoveOutdatedPacketForTrack(trackID, currFramePTS);
                  pts = omaf_reader_mgr_->GetOldestPacketPTSForTrack(trackID);
                  if (pts > currFramePTS)
                  {
                      OMAF_LOG(LOG_INFO, "After wait for a moment, outdated PTS %ld from track %d\n", pts, trackID);
                      hasPktOutdated = true;
                      if (samplesNumPerSeg == aveSamplesNumPerSeg)
                      {
                          if (pts % samplesNumPerSeg)
                          {
                              pts = ((pts / samplesNumPerSeg) + 1) * samplesNumPerSeg;
                          }
                      }
                      else //most likely current segment is the last segment
                      {
                          pts = aveSamplesNumPerSeg * (currSegTimeLine - 1) + samplesNumPerSeg;
                      }

                      currFramePTS = pts;
                      skipFrames = true;
                      break;
                  }
                  else if (pts < currFramePTS)
                  {
                      OMAF_LOG(LOG_INFO, "After waiting for a while, pts %ld still diff from current PTS %ld\n", pts, currFramePTS);
                      hasPktOutdated = true;
                      break;
                  }
              }
          } else {
              omaf_reader_mgr_->RemoveOutdatedPacketForTrack(trackID, currFramePTS);
              pts = omaf_reader_mgr_->GetOldestPacketPTSForTrack(trackID);

              if (pts > currFramePTS)
              {
                  OMAF_LOG(LOG_INFO, "After wait for a moment, outdated PTS %ld from track %d\n", pts, trackID);
                  hasPktOutdated = true;
                  if (samplesNumPerSeg == aveSamplesNumPerSeg)
                  {
                      if (pts % samplesNumPerSeg)
                      {
                          pts = ((pts / samplesNumPerSeg) + 1) * samplesNumPerSeg;
                      }
                  }
                  else //most likely current segment is the last segment
                  {
                      pts = aveSamplesNumPerSeg * (currSegTimeLine - 1) + samplesNumPerSeg;
                  }

                  currFramePTS = pts;
                  skipFrames = true;
                  break;
              }
              else if (pts < currFramePTS)
              {
                  OMAF_LOG(LOG_INFO, "After waiting for a while, pts %ld still diff from current PTS %ld\n", pts, currFramePTS);
                  hasPktOutdated = true;
                  break;
              }
          }
        }
      }

      ret = omaf_reader_mgr_->GetNextPacketWithPTS(trackID, currFramePTS, onePacket, m_needParams);

      OMAF_LOG(LOG_INFO, "Get next packet !\n");
      currWaitTimes = 0;

      while ((ret == ERROR_NULL_PACKET) && (currWaitTimes < waitTimes) && m_status != STATUS_STOPPED) {

        usleep((m_pStreamInfo->segmentDuration * 1000000) / waitTimes);
        currWaitTimes++;
        //OMAF_LOG(LOG_INFO, "To get packet %ld for track %d\n", currFramePTS, trackID);
        ret = omaf_reader_mgr_->GetNextPacketWithPTS(trackID, currFramePTS, onePacket, m_needParams);
      }

      if (ret == ERROR_NONE) {
        if (onePacket->GetEOS()) {
          OMAF_LOG(LOG_INFO, "EOS has been gotten !\n");
          isEOS = true;
          selectedPackets.insert(std::make_pair((uint32_t)(trackID), onePacket));
          break;
        }
        samplesNumPerSeg = omaf_reader_mgr_->GetSamplesNumPerSegmentForTimeLine(currSegTimeLine);
        if (!aveSamplesNumPerSeg)
        {
            aveSamplesNumPerSeg = samplesNumPerSeg;
        }
        if (as_it == mapSelectedAS.begin()) {
          segmentEnded = onePacket->GetSegmentEnded();
          OMAF_LOG(LOG_INFO, "For frame %ld, segmentEnded %d\n", currFramePTS, segmentEnded);
        }
        OMAF_LOG(LOG_INFO, "To insert packet %ld for track %d\n", currFramePTS, trackID);
        selectedPackets.insert(std::make_pair((uint32_t)(trackID), onePacket));
      } else if (ret == ERROR_NULL_PACKET) {
        hasPktOutdated = true;
        OMAF_LOG(LOG_INFO, "Still can't get frame %ld for track %d\n", currFramePTS, trackID);
        break;
      }
    }

    if (hasPktOutdated) {
      std::list<MediaPacket *> allPackets;
      for (auto it1 = selectedPackets.begin(); it1 != selectedPackets.end();) {
        MediaPacket* pkt = it1->second;
        std::list<MediaPacket *>::iterator pktIter;
        pktIter = std::find(allPackets.begin(), allPackets.end(), pkt);
        if (pktIter == allPackets.end())
        {
          allPackets.push_back(pkt);
          SAFE_DELETE(pkt);
        }
        selectedPackets.erase(it1++);
      }
      selectedPackets.clear();
      allPackets.clear();
      if (currFramePTS > 0)
      {
          std::map<int, OmafAdaptationSet*>::iterator itAS;
          for (itAS = mMediaAdaptationSet.begin(); itAS != mMediaAdaptationSet.end(); itAS++)
          {
              OmafAdaptationSet *oneAS = itAS->second;
              int32_t trkID = oneAS->GetTrackNumber();
              omaf_reader_mgr_->RemoveOutdatedPacketForTrack(trkID, (currFramePTS));
          }
      }
      if (!skipFrames)
      {
          if (beginNewSeg)
          {
              OMAF_LOG(LOG_INFO, "Current frame %ld is key frame but has outdated, drop frames till next key frame !\n", currFramePTS);
              currFramePTS += samplesNumPerSeg;
              skipFrames = true;
              usleep((m_pStreamInfo->segmentDuration * 1000000) / 2);
          }
          else
          {
              OMAF_LOG(LOG_INFO, "Frame %ld can't be stitched, move to next segment !\n", currFramePTS);

              if (samplesNumPerSeg == aveSamplesNumPerSeg)
              {
                  currFramePTS = ((currFramePTS / samplesNumPerSeg) + 1) * samplesNumPerSeg;
              }
              else //most likely current segment is the last segment
              {
                  currFramePTS = aveSamplesNumPerSeg * (currSegTimeLine - 1) + samplesNumPerSeg;
              }
              skipFrames = true;
              usleep((m_pStreamInfo->segmentDuration * 1000000) / 2);
          }
      }

      continue;
    }

    OMAF_LOG(LOG_INFO, "Start to stitch packets! and pts is %ld\n", currFramePTS);
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
        // trace
        tracepoint(mthq_tp_provider, T6_stitch_start_time, currFramePTS);
#endif
#endif
    if (!isEOS && (selectedPackets.size() != mapSelectedAS.size()) && (currWaitTimes >= waitTimes)) {
      OMAF_LOG(LOG_INFO, "Incorrect selected tile tracks packets number for tiles stitching !\n");

      std::list<MediaPacket *> allPackets;
      for (auto it1 = selectedPackets.begin(); it1 != selectedPackets.end();) {
        MediaPacket* pkt = it1->second;
	std::list<MediaPacket *>::iterator pktIter;
        pktIter = std::find(allPackets.begin(), allPackets.end(), pkt);
	if (pktIter == allPackets.end())
        {
          allPackets.push_back(pkt);
          SAFE_DELETE(pkt);
	}
        selectedPackets.erase(it1++);
      }
      selectedPackets.clear();
      allPackets.clear();
      if (currFramePTS > 0)
      {
          std::map<int, OmafAdaptationSet*>::iterator itAS;
          for (itAS = mMediaAdaptationSet.begin(); itAS != mMediaAdaptationSet.end(); itAS++)
          {
              OmafAdaptationSet *oneAS = itAS->second;
              int32_t trkID = oneAS->GetTrackNumber();
              omaf_reader_mgr_->RemoveOutdatedPacketForTrack(trkID, (currFramePTS));
          }
      }

      continue;
    }

    if (!isEOS && !(m_stitch->IsInitialized())) {
      ret = m_stitch->Initialize(selectedPackets, m_needParams,
                                 (VCD::OMAF::ProjectionFormat)(m_pStreamInfo->mProjFormat), m_sources);
      if (ret) {
        OMAF_LOG(LOG_ERROR, "Failed to initialize stitch class !\n");
	std::list<MediaPacket *> allPackets;
        for (auto it1 = selectedPackets.begin(); it1 != selectedPackets.end();) {
          MediaPacket* pkt = it1->second;
	  std::list<MediaPacket *>::iterator pktIter;
          pktIter = std::find(allPackets.begin(), allPackets.end(), pkt);
          if (pktIter == allPackets.end())
          {
            allPackets.push_back(pkt);
            SAFE_DELETE(pkt);
	  }
          selectedPackets.erase(it1++);
        }
	allPackets.clear();
        selectedPackets.clear();
        return ret;
      }
    } else {
      if (!isEOS && m_status != STATUS_STOPPED) {
        ret = m_stitch->UpdateSelectedTiles(selectedPackets, m_needParams);
        if (ret) {
          OMAF_LOG(LOG_ERROR, "Failed to update media packets for tiles merge !\n");
	  std::list<MediaPacket *> allPackets;
          for (auto it1 = selectedPackets.begin(); it1 != selectedPackets.end();) {
            MediaPacket* pkt = it1->second;
	    std::list<MediaPacket *>::iterator pktIter;
            pktIter = std::find(allPackets.begin(), allPackets.end(), pkt);
            if (pktIter == allPackets.end())
            {
	      allPackets.push_back(pkt);
              SAFE_DELETE(pkt);
            }
            selectedPackets.erase(it1++);
          }
	  allPackets.clear();
          selectedPackets.clear();
          return ret;
        }

        if (currFramePTS > 0)
        {
            std::map<int, OmafAdaptationSet*>::iterator itAS;
            for (itAS = mMediaAdaptationSet.begin(); itAS != mMediaAdaptationSet.end(); itAS++)
            {
                OmafAdaptationSet *oneAS = itAS->second;
                int32_t trkID = oneAS->GetTrackNumber();
                omaf_reader_mgr_->RemoveOutdatedPacketForTrack(trkID, (currFramePTS));
            }
        }
      }
    }

    std::list<MediaPacket*> mergedPackets;

    if (isEOS) {
      std::map<uint32_t, MediaPacket*>::iterator itPacket1;
      for (itPacket1 = selectedPackets.begin(); itPacket1 != selectedPackets.end(); itPacket1++) {
        MediaPacket* packet = itPacket1->second;
        mergedPackets.push_back(packet);
      }
    } else {
      mergedPackets = m_stitch->GetTilesMergedPackets();
    }

    {
      std::lock_guard<std::mutex> lock(m_packetsMutex);
      m_mergedPackets.push_back(mergedPackets);
    }
    std::list<MediaPacket*>::iterator it = mergedPackets.begin();
    MediaPacket *one = *it;
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
    // trace
    tracepoint(mthq_tp_provider, T7_stitch_end_time, one->GetSegID(), currFramePTS, mergedPackets.size());
#endif
#endif
    OMAF_LOG(LOG_INFO, "Finish to stitch packets for packet segment id %d\n", one->GetSegID());
    OMAF_LOG(LOG_INFO, "packet pts is %ld and video number is %lld\n", one->GetPTS(), mergedPackets.size());
    selectedPackets.clear();
    prevPoseChanged = false;
  }

  return ERROR_NONE;
}

std::list<MediaPacket*> OmafMediaStream::GetOutTilesMergedPackets() {
  std::list<MediaPacket*> outPackets;
  std::lock_guard<std::mutex> lock(m_packetsMutex);
  if (m_mergedPackets.size()) {
    outPackets = m_mergedPackets.front();
    m_mergedPackets.pop_front();
  }
  // correct the video id
  uint32_t video_id = 0;
  for (auto packet : outPackets) {
    packet->SetVideoID(video_id++);
  }
  return outPackets;
}

VCD_OMAF_END;
