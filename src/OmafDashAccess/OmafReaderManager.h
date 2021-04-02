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
//! \file:   OmafReaderManager.h
//! \brief:
//! \detail:
//!
//! Created on May 28, 2019, 1:41 PM
//!

#ifndef OMAFMP4READERMGR_H
#define OMAFMP4READERMGR_H

#include "MediaPacket.h"
#include "general.h"

#include "OmafMediaSource.h"
#include "OmafReader.h"

#include <atomic>
#include <chrono>
#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

VCD_OMAF_BEGIN

using PacketQueue = std::list<MediaPacket *>;

enum class OmafDashMode { EXTRACTOR = 0, LATER_BINDING = 1 };

class OmafSegmentNode;
class OmafPacketParams;
class OmafAudioPacketParams;

struct _omafSegmentNodeTimedSet {
  int64_t timeline_point_ = -1;
  std::chrono::steady_clock::time_point create_time_;
  std::list<std::shared_ptr<OmafSegmentNode>> segment_nodes_;
};

using OmafSegmentNodeTimedSet = struct _omafSegmentNodeTimedSet;

class OmafReaderManager : public VCD::NonCopyable, public enable_shared_from_this<OmafReaderManager> {
  friend OmafSegmentNode;

 public:
  struct _params {
    OmafDashMode mode_;
    DashStreamType stream_type_ = DASH_STREAM_DYNMIC;
    size_t duration_ = 0;
    int32_t segment_timeout_ms_ = 3000;  // ms
    ProjectionFormat proj_fmt_  = ProjectionFormat::PF_ERP;
  };

  using OmafReaderParams = struct _params;

 public:
  using Ptr = std::shared_ptr<OmafReaderManager>;

 public:
  OmafReaderManager(std::shared_ptr<OmafDashSegmentClient> client, OmafReaderParams params)
      : dash_client_(client), work_params_(params) {}
  virtual ~OmafReaderManager() { Close(); }

 public:
  //!  \brief initialize the reader with MediaSource
  //!
  OMAF_STATUS Initialize(OmafMediaSource *pSource) noexcept;

  //!  \brief close the reader
  //!
  OMAF_STATUS Close() noexcept;

  //!  \brief add init Segment for reading after it is downloaded
  //!
  OMAF_STATUS OpenInitSegment(std::shared_ptr<OmafSegment> pInitSeg) noexcept;
  OMAF_STATUS OpenLocalInitSegment(std::shared_ptr<OmafSegment> pInitSeg) noexcept;

  //!  \brief add Segment for reading after it is downloaded
  //!
  OMAF_STATUS OpenSegment(std::shared_ptr<OmafSegment> pSeg, bool isExtractor = false, bool isCatchup = false) noexcept;
  OMAF_STATUS OpenLocalSegment(std::shared_ptr<OmafSegment> pSeg, bool isExtractor = false) noexcept;

  //!  \brief Get Next packet from packet queue. each track has a packet queue
  //!
  OMAF_STATUS GetNextPacket(uint32_t trackID, MediaPacket *&pPacket, bool requireParams) noexcept;

  //! \brief Get Next packet with assigned track index and PTS from packet queue.
  OMAF_STATUS GetNextPacketWithPTS(uint32_t trackID, uint64_t pts, MediaPacket *&pPacket, bool requireParams) noexcept;

  //!  \brief Get mPacketQueue[trackID] size
  //!
  OMAF_STATUS GetPacketQueueSize(uint32_t trackID, size_t &size) noexcept;

  //!  \brief Get initial segments parse status.
  //!
  inline bool IsInitSegmentsParsed() { return bInitSeg_all_ready_.load(); };

  uint64_t GetOldestPacketPTSForTrack(int trackId);
  void RemoveOutdatedPacketForTrack(int trackId, uint64_t currPTS);
  void RemoveOutdatedCatchupPacketForTrack(int trackId, uint64_t currPTS);
  size_t GetSamplesNumPerSegmentForTimeLine(uint64_t currTimeLine)
  {
      size_t samples_num = 0;
      {
          std::lock_guard<std::mutex> lock(segment_samples_mutex_);
          std::map<uint64_t, size_t>::iterator it;
          it = samples_num_per_seg_.find(currTimeLine);
          if (it != samples_num_per_seg_.end())
          {
              samples_num = it->second;
          }
      }

      return samples_num;
  }

 private:
  void threadRunner() noexcept;
  std::shared_ptr<OmafSegmentNode> findReadySegmentNode() noexcept;
  void clearOlderSegmentSet(int64_t timeline_point) noexcept;
  bool checkEOS(int64_t segment_num) noexcept;
  bool isEmpty(std::mutex &mutex, const std::list<OmafSegmentNodeTimedSet> &nodes) noexcept;

 private:
  inline int initSegParsedCount(void) noexcept { return initSeg_ready_count_.load(); }
  void buildInitSegmentInfo(void) noexcept;
  void setupTrackIdMap(void) noexcept;

 private:
  void initSegmentStateChange(std::shared_ptr<OmafSegment>, OmafSegment::State) noexcept;
  void normalSegmentStateChange(std::shared_ptr<OmafSegment>, OmafSegment::State) noexcept;

  std::shared_ptr<OmafPacketParams> getPacketParams(uint32_t qualityRanking) noexcept {
    return omaf_packet_params_[qualityRanking];
  }
  void setPacketParams(uint32_t qualityRanking, std::shared_ptr<OmafPacketParams> params) {
    omaf_packet_params_[qualityRanking] = std::move(params);
  }

  std::shared_ptr<OmafPacketParams> getPacketParamsForExtractors(uint32_t extractorTrackIdx) noexcept {
    return packet_params_for_extractors_[extractorTrackIdx];
  }
  void setPacketParamsForExtractors(uint32_t extractorTrackIdx, std::shared_ptr<OmafPacketParams> params) {
    packet_params_for_extractors_[extractorTrackIdx] = std::move(params);
  }

  std::shared_ptr<OmafAudioPacketParams> getPacketParamsForAudio(uint32_t audioTrackIdx) noexcept {
    return packet_params_for_audio_[audioTrackIdx];
  }
  void setPacketParamsForAudio(uint32_t audioTrackIdx, std::shared_ptr<OmafAudioPacketParams> params) {
    packet_params_for_audio_[audioTrackIdx] = std::move(params);
  }

 private:
  std::shared_ptr<OmafDashSegmentClient> dash_client_;

  OmafReaderParams work_params_;
  int64_t timeline_point_ = -1;
  // omaf reader
  std::thread segment_reader_worker_;
  bool breader_working_ = false;

  std::mutex segment_samples_mutex_;
  std::map<uint64_t, size_t> samples_num_per_seg_;
  //size_t samples_num_per_seg_ = 0;
  std::shared_ptr<OmafReader> reader_;

  std::mutex segment_opening_mutex_;
  std::list<OmafSegmentNodeTimedSet> segment_opening_list_;
  std::mutex segment_opened_mutex_;
  std::condition_variable segment_opened_cv_;
  std::list<OmafSegmentNodeTimedSet> segment_opened_list_;
  std::mutex segment_parsed_mutex_;
  std::condition_variable segment_parsed_cv_;
  std::list<OmafSegmentNodeTimedSet> segment_parsed_list_;

  OmafMediaSource *media_source_ = nullptr;
  std::map<uint32_t, std::shared_ptr<OmafPacketParams>> omaf_packet_params_;

  std::map<uint32_t, std::shared_ptr<OmafPacketParams>> packet_params_for_extractors_;

  std::map<uint32_t, std::shared_ptr<OmafAudioPacketParams>> packet_params_for_audio_;

  std::mutex initSeg_mutex_;

  //<! ID pair for InitSegID to TrackID;
  std::map<uint32_t, uint32_t> initSeg_trackIds_map_;
  //<! ID pair for TrackID to InitSegID;
  std::map<uint32_t, uint32_t> trackIds_initSeg_map_;
  // map < initSeg_id, depends_initSeg_ids>
  std::map<uint32_t, std::vector<uint32_t>> initSegId_depends_map_;

  std::atomic_int initSeg_ready_count_{0};
  std::atomic_bool bInitSeg_all_ready_{false};
};
// using READERMANAGER = Singleton<OmafReaderManager>;
VCD_OMAF_END

#endif  // OMAFMP4READERMGR_H
