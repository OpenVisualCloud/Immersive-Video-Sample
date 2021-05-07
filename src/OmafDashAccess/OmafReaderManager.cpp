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
 * File:   OmafReaderManager.cpp
 * Author: media
 *
 * Created on May 28, 2019, 1:41 PM
 */

#include "OmafReaderManager.h"

#include "OmafMP4VRReader.h"
#include "OmafMediaSource.h"
#include "OmafReader.h"
#include "common.h"

#include <math.h>
#include <functional>
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
#include "../trace/MtHQ_tp.h"
#include "../trace/Bandwidth_tp.h"
#endif
#endif

VCD_OMAF_BEGIN

// not support concurrency
class OmafPacketParams : public VCD::NonCopyable {
  friend OmafSegmentNode;

 public:
  using Ptr = std::shared_ptr<OmafPacketParams>;

 public:
  OmafPacketParams(){};
  virtual ~OmafPacketParams(){};

 public:
  int init(std::shared_ptr<OmafReader> reader, uint32_t reader_trackId, uint32_t sampleId) noexcept;

 private:
  std::vector<uint8_t> params_;

  //<! VPS data
  std::vector<uint8_t> vps_;

  //<! SPS data
  std::vector<uint8_t> sps_;

  //<! PPS data
  std::vector<uint8_t> pps_;

  //<! sample width
  uint32_t width_ = 0;

  //<! sample height
  uint32_t height_ = 0;

  bool binit_ = false;
};

class OmafAudioPacketParams : public VCD::NonCopyable {
  friend OmafSegmentNode;

  public:
  using Ptr = std::shared_ptr<OmafAudioPacketParams>;

  public:
  OmafAudioPacketParams(){};
  virtual ~OmafAudioPacketParams(){};

  public:
  int init(std::shared_ptr<OmafReader> reader, uint32_t reader_trackId, uint32_t sampleId) noexcept;

  void writeADTSHdr(uint32_t frameSize);

  private:
  int  unPackUnsignedIntValue(uint8_t bitsNum, uint32_t *value);
  void packOneBit(bool value);
  void packUnsignedIntValue(uint8_t bitsNum, uint32_t value);

  std::vector<uint8_t> params_;

  uint32_t             objType_ = 0;

  uint32_t             frequencyIdx_ = 0;

  uint32_t             channelCfg_ = 0;

  int32_t              curr_bit_pos_ = 0;

  bool                 binit_ = false;
};

// FIXME use better class name?
class OmafSegmentNode : public VCD::NonCopyable {
 public:
  using Ptr = std::shared_ptr<OmafSegmentNode>;
  using WPtr = std::weak_ptr<OmafSegmentNode>;

 public:
  OmafSegmentNode(std::shared_ptr<OmafReaderManager> mgr, OmafDashMode mode, ProjectionFormat projFmt, std::shared_ptr<OmafReader> reader,
                  std::shared_ptr<OmafSegment> segment, size_t depends_size = 0, bool isExtractor = false, bool isCatchup = false)
      : omaf_reader_mgr_(mgr),
        mode_(mode),
        projection_fmt_(projFmt),
        reader_(reader),
        segment_(segment),
        depends_size_(depends_size),
        bExtractor_(isExtractor),
        bCatchup_(isCatchup) {}

  virtual ~OmafSegmentNode() {
    OMAF_LOG(LOG_INFO, "Release the segment node %s packet size=%lld\n", to_string().c_str(), media_packets_.size());
    // relase packet
    std::lock_guard<std::mutex> lock(packet_mutex_);
    while (media_packets_.size()) {
      auto p = media_packets_.front();
      if (p) {
        delete p;
      }
      media_packets_.pop();
    }
  }

 public:
  int start(void) noexcept;
  int parse(void) noexcept;
  int stop(void) noexcept;

  // int getPacket(std::unique_ptr<MediaPacket> &pPacket, bool needParams) noexcept;
  int getPacket(MediaPacket *&pPacket, bool requireParams) noexcept;
  int getPacketWithPTS(MediaPacket *&pPacket, bool requireParams, uint64_t pts) noexcept;
  int packetQueueSize(void) const noexcept { return media_packets_.size(); }
  std::string to_string() const noexcept {
    std::stringstream ss;

    ss << "node, timeline=" << getTimelinePoint();

    if (segment_) {
      ss << ", " << segment_->to_string();
    }
    return ss.str();
  };

 public:
  uint32_t getSegId() const noexcept {
    if (segment_.get() != nullptr) {
      return segment_->GetSegID();
    }
    return 0;
  }

  uint32_t getTrackId() const noexcept {
    if (segment_.get() != nullptr) {
      return segment_->GetTrackId();
    }
    return 0;
  }

  uint32_t getInitSegId() const noexcept {
    if (segment_.get() != nullptr) {
      return segment_->GetInitSegID();
    }
    return 0;
  }
  int64_t getTimelinePoint() const noexcept {
    if (segment_.get() != nullptr) {
      return segment_->GetTimelinePoint();
    }
    return 0;
  }
  uint64_t getPTS() {
    std::lock_guard<std::mutex> lock(packet_mutex_);
    if (media_packets_.size()) {
      return media_packets_.front()->GetPTS();
    }
    return 0;
  }
  void clearPacketByPTS(uint64_t pts) {
    std::lock_guard<std::mutex> lock(packet_mutex_);
    while (media_packets_.size()) {
      auto &packet = media_packets_.front();
      if (packet && (packet->GetPTS() >= pts)) {
        break;
      }
      if (packet)
      {
          delete packet;
          packet = NULL;
      }
      media_packets_.pop();
    }
  }

  MediaType getMediaType() {
    if (segment_.get() != nullptr) {
      return segment_->GetMediaType();
    }
    return MediaType_NONE;
  }

  const std::chrono::steady_clock::time_point &startTime() const noexcept { return start_time_; };
  bool checkTimeout(int32_t ms) const noexcept {
    auto now = std::chrono::steady_clock::now();
    std::chrono::milliseconds time_span = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_);
    return time_span.count() >= ms ? true : false;
  }
  size_t dependsSize() const noexcept { return depends_size_; }
  bool isExtractor() const noexcept { return bExtractor_; }
  bool isCatchup() const noexcept { return bCatchup_; }
  OmafDashMode GetMode() const noexcept { return mode_; }
  bool isReady() const noexcept;
  size_t GetSamplesNum() { return samples_num_; };

  // FIXME, who own the media packets
  // const std::list<MediaPacket *> packets() const { return media_packets_; }

  void pushDepends(OmafSegmentNode::Ptr node) { depends_.push_back(std::move(node)); }

  bool operator==(OmafSegment::Ptr segment) { return this->segment_ == segment; }

 private:
  int parseSegmentStream(std::shared_ptr<OmafReader> reader) noexcept;
  int removeSegmentStream(std::shared_ptr<OmafReader> reader) noexcept;
  int cachePackets(std::shared_ptr<OmafReader> reader) noexcept;
  std::shared_ptr<TrackInformation> findTrackInformation(std::shared_ptr<OmafReader> reader) noexcept;
  bool findSampleIndexRange(std::shared_ptr<TrackInformation>, size_t &begin, size_t &end) noexcept;
  OmafPacketParams::Ptr getPacketParams() {
    auto reader_mgr = omaf_reader_mgr_.lock();
    if (reader_mgr) {
      return reader_mgr->getPacketParams(segment_->GetQualityRanking());
    }
    return nullptr;
  }
  void setPacketParams(OmafPacketParams::Ptr params) {
    auto reader_mgr = omaf_reader_mgr_.lock();
    if (reader_mgr) {
      reader_mgr->setPacketParams(segment_->GetQualityRanking(), std::move(params));
    }
  }

  OmafPacketParams::Ptr getPacketParamsForExtractors() {
    auto reader_mgr = omaf_reader_mgr_.lock();
    if (reader_mgr) {
      return reader_mgr->getPacketParamsForExtractors(segment_->GetTrackId());
    }
    return nullptr;
  }
  void setPacketParamsForExtractors(OmafPacketParams::Ptr params) {
    auto reader_mgr = omaf_reader_mgr_.lock();
    if (reader_mgr) {
      reader_mgr->setPacketParamsForExtractors(segment_->GetTrackId(), std::move(params));
    }
  }

  OmafAudioPacketParams::Ptr getPacketParamsForAudio() {
    auto reader_mgr = omaf_reader_mgr_.lock();
    if (reader_mgr) {
      return reader_mgr->getPacketParamsForAudio(segment_->GetTrackId());
    }
    return nullptr;
  }
  void setPacketParamsForAudio(OmafAudioPacketParams::Ptr params) {
    auto reader_mgr = omaf_reader_mgr_.lock();
    if (reader_mgr) {
      reader_mgr->setPacketParamsForAudio(segment_->GetTrackId(), std::move(params));
    }
  }

 private:
  std::weak_ptr<OmafReaderManager> omaf_reader_mgr_;

  // dash mode
  OmafDashMode mode_ = OmafDashMode::EXTRACTOR;

  ProjectionFormat projection_fmt_ = ProjectionFormat::PF_ERP;

  // omaf reader
  std::weak_ptr<OmafReader> reader_;

  // this dash source related segment
  OmafSegment::Ptr segment_;

  // use shared ptr or unique ptr
  std::vector<OmafSegmentNode::Ptr> depends_;

  std::chrono::steady_clock::time_point start_time_;

  // packet list
  // std::queue<std::unique_ptr<MediaPacket::Ptr>> media_packets_;
  std::queue<MediaPacket *> media_packets_;
  std::mutex packet_mutex_;

  // OmafPacketParams::Ptr packet_params;

  const size_t depends_size_ = 0;

  const bool bExtractor_ = false;

  const bool bCatchup_ = false;

  size_t samples_num_ = 0;
};

uint32_t buildDashTrackId(uint32_t id) noexcept { return id & static_cast<uint32_t>(0xffff); }

uint32_t buildReaderTrackId(uint32_t trackId, uint32_t initSegId) noexcept { return (initSegId << 16) | trackId; }

OMAF_STATUS OmafReaderManager::Initialize(OmafMediaSource *pSource) noexcept {
  try {
    if (pSource == nullptr) {
      OMAF_LOG(LOG_ERROR, "Invalid media source!\n");
      return ERROR_INVALID;
    }

    media_source_ = pSource;

    reader_ = std::make_shared<OmafMP4VRReader>();
    if (reader_.get() == nullptr) {
      OMAF_LOG(LOG_ERROR, "Failed to create the omaf mp4 vr reader!\n");
      return ERROR_INVALID;
    }
    breader_working_ = true;
    segment_reader_worker_ = std::thread(&OmafReaderManager::threadRunner, this);

    return ERROR_NONE;

  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Failed to init the client reader, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafReaderManager::Close() noexcept {
  try {
    OMAF_LOG(LOG_INFO, "Close the omaf reader manager!\n");
    breader_working_ = false;

    {
      std::lock_guard<std::mutex> lock(segment_opening_mutex_);
      segment_opening_list_.clear();
    }

    {
      std::lock_guard<std::mutex> lock(segment_opened_mutex_);
      segment_opened_list_.clear();
      segment_opened_cv_.notify_all();
    }

    {
      std::lock_guard<std::mutex> lock(segment_parsed_mutex_);
      segment_parsed_list_.clear();
    }

    if (segment_reader_worker_.joinable()) {
      breader_working_ = false;
      segment_reader_worker_.join();
    }

    return ERROR_NONE;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Failed to close the client reader, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafReaderManager::OpenInitSegment(std::shared_ptr<OmafSegment> pInitSeg) noexcept {
  try {
    if (pInitSeg.get() == nullptr) {
      OMAF_LOG(LOG_ERROR, "The init segment is empty!\n");
      return ERROR_INVALID;
    }

#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
    //trace
    const char *trackType = "init_track";
    uint64_t streamSize = pInitSeg->GetStreamSize();
    char tileRes[128] = { 0 };
    snprintf(tileRes, 128, "%s", "none");
    tracepoint(bandwidth_tp_provider, packed_segment_size, 0, trackType, tileRes, 0, streamSize);
#endif
#endif

    pInitSeg->RegisterStateChange([this](std::shared_ptr<OmafSegment> pInitSeg, OmafSegment::State state) {
      this->initSegmentStateChange(std::move(pInitSeg), state);
    });
    OMAF_STATUS ret = pInitSeg->Open(dash_client_);
    if (ret != ERROR_NONE) {
      OMAF_LOG(LOG_ERROR, "Failed to open the init segment, code= %d\n", ret);
      return ret;
    }

    return ERROR_NONE;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Failed to add the init segment, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafReaderManager::OpenLocalInitSegment(std::shared_ptr<OmafSegment> pInitSeg) noexcept {
  try {
    if (pInitSeg.get() == nullptr) {
      OMAF_LOG(LOG_ERROR, "The init segment is empty!\n");
      return ERROR_INVALID;
    }
    pInitSeg->SetState(OmafSegment::State::OPEN_SUCCES);
    this->initSegmentStateChange(std::move(pInitSeg), OmafSegment::State::OPEN_SUCCES);

    return ERROR_NONE;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Failed to add the local init segment, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafReaderManager::OpenSegment(std::shared_ptr<OmafSegment> pSeg, bool isExtracotr, bool isCatchup) noexcept {
  try {
    size_t depends_size = 0;
    auto d_it = initSegId_depends_map_.find(pSeg->GetInitSegID());
    if (d_it != initSegId_depends_map_.end()) {
      depends_size = d_it->second.size();
    }

    pSeg->RegisterStateChange([this](std::shared_ptr<OmafSegment> segment, OmafSegment::State state) {
      this->normalSegmentStateChange(std::move(segment), state);
    });

    OmafDashMode work_mode = work_params_.mode_;
    if (isCatchup) work_mode = OmafDashMode::LATER_BINDING;

    OmafSegmentNode::Ptr new_node = std::make_shared<OmafSegmentNode>(shared_from_this(), work_mode, work_params_.proj_fmt_, reader_,
                                                                      std::move(pSeg), depends_size, isExtracotr, isCatchup);
    {
      std::unique_lock<std::mutex> lock(segment_opening_mutex_);

      bool newTimeslide = true;
      for (auto &nodeset : segment_opening_list_) {
        if (nodeset.timeline_point_ == new_node->getTimelinePoint()) {
          nodeset.segment_nodes_.push_back(new_node);
          newTimeslide = false;
          break;
        }
      }  // end for segment_opening_list_

      if (newTimeslide) {
        OmafSegmentNodeTimedSet nodeset;
        nodeset.timeline_point_ = new_node->getTimelinePoint();
        nodeset.create_time_ = std::chrono::steady_clock::now();
        nodeset.segment_nodes_.push_back(new_node);
        segment_opening_list_.emplace_back(nodeset);
      }
    }

    OMAF_STATUS ret = new_node->start();
    if (ret != ERROR_NONE) {
      OMAF_LOG(LOG_ERROR, "Failed to open the segment, code= %d\n", ret);
      return ret;
    }
    return ERROR_NONE;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Failed to add segment, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafReaderManager::OpenLocalSegment(std::shared_ptr<OmafSegment> segment, bool isExtractor) noexcept {
  try {
    size_t depends_size = 0;
    auto d_it = initSegId_depends_map_.find(segment->GetInitSegID());
    if (d_it != initSegId_depends_map_.end()) {
      depends_size = d_it->second.size();
    }

    OmafSegmentNode::Ptr new_node = std::make_shared<OmafSegmentNode>(shared_from_this(), work_params_.mode_, work_params_.proj_fmt_, reader_,
                                                                      segment, depends_size, isExtractor);
    {
      std::unique_lock<std::mutex> lock(segment_opening_mutex_);

      bool newTimeslide = true;
      for (auto &nodeset : segment_opening_list_) {
        if (nodeset.timeline_point_ == new_node->getTimelinePoint()) {
          nodeset.segment_nodes_.push_back(new_node);
          newTimeslide = false;
          break;
        }
      }  // end for segment_opening_list_

      if (newTimeslide) {
        OmafSegmentNodeTimedSet nodeset;
        nodeset.timeline_point_ = new_node->getTimelinePoint();
        nodeset.create_time_ = std::chrono::steady_clock::now();
        nodeset.segment_nodes_.push_back(new_node);
        segment_opening_list_.emplace_back(nodeset);
      }
    }
    segment->SetState(OmafSegment::State::OPEN_SUCCES);
    this->normalSegmentStateChange(std::move(segment), OmafSegment::State::OPEN_SUCCES);

    return ERROR_NONE;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Failed to add segment, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafReaderManager::GetNextPacket(uint32_t trackID, MediaPacket *&pPacket, bool requireParams) noexcept {
  try {
    OMAF_STATUS ret = ERROR_NONE;

    bool bpacket_readed = false;
    {
      std::unique_lock<std::mutex> lock(segment_parsed_mutex_);

      // 1. read the required packet
      for (auto &nodeset : segment_parsed_list_) {
        std::list<OmafSegmentNode::Ptr>::iterator it = nodeset.segment_nodes_.begin();
        while (it != nodeset.segment_nodes_.end()) {
          auto &node = *it;
          //OMAF_LOG(LOG_INFO, "Require trackid=%u, node trackid=%u\n", trackID, node->getTrackId());
          if (node->getTrackId() == trackID) {
            ret = node->getPacket(pPacket, requireParams);
            if (ret == ERROR_NONE) {
              bpacket_readed = true;
            }
            timeline_point_ = node->getTimelinePoint();
            //OMAF_LOG(LOG_INFO, "timeline_point_ is %ld in GetNextPacket, bpacket_readed %d\n", timeline_point_, bpacket_readed);
            if (0 == node->packetQueueSize()) {
              //OMAF_LOG(LOG_INFO, "Node count=%d. %s\n", node.use_count(), node->to_string().c_str());
              nodeset.segment_nodes_.erase(it);
            }

            break;
          }

          it++;
        }
        if (bpacket_readed)
            break;

      }
    }
    if (!bpacket_readed) {
      // FIXME, this may a bug for using the timeline point as segment number
      if (work_params_.stream_type_ == DASH_STREAM_DYNMIC || !checkEOS(timeline_point_)) {
        pPacket = nullptr;

        ret = ERROR_NULL_PACKET;
      } else {
        ret = ERROR_NONE;
        pPacket = new MediaPacket();
        pPacket->SetEOS(true);
      }
    }

    // 2. sync timeline point for outside reading
    // drop all dashset whose timeline point is less than timeline point
    if (timeline_point_ != -1) {
      OMAF_LOG(LOG_INFO, "To clear the timeline point < %ld\n", timeline_point_ - 1);
      clearOlderSegmentSet(timeline_point_ - 1);
      std::lock_guard<std::mutex> lock(segment_parsed_mutex_);
      std::list<OmafSegmentNodeTimedSet>::iterator it = segment_parsed_list_.begin();
      while (it != segment_parsed_list_.end() && it->timeline_point_ < timeline_point_ - 1) {
        it = segment_parsed_list_.erase(it);  // 'it' will move to next when calling erase
      }
    }
    return ret;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Failed to read frame for trackid=%u, ex: %s\n", trackID, ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafReaderManager::GetNextPacketWithPTS(uint32_t trackID, uint64_t pts, MediaPacket *&pPacket, bool requireParams) noexcept {
  try {
    OMAF_STATUS ret = ERROR_NONE;

    bool bpacket_readed = false;
    {
      std::unique_lock<std::mutex> lock(segment_parsed_mutex_);

      // 1. read the required packet
      for (auto &nodeset : segment_parsed_list_) {//loop on different timeline
        uint32_t sample_size = GetSamplesNumPerSegmentForTimeLine(1);
        if (sample_size != 0 && nodeset.timeline_point_ == (int64_t)(pts / sample_size + 1)) {
        std::list<OmafSegmentNode::Ptr>::iterator it = nodeset.segment_nodes_.begin();
        while (it != nodeset.segment_nodes_.end()) { //loop on different node (track)
          auto &node = *it;
          // OMAF_LOG(LOG_INFO, "Require trackid=%u, node trackid=%u\n", trackID, node->getTrackId());
          if (node->getTrackId() == trackID) {
            // OMAF_LOG(LOG_INFO, "PACKET Get packet with pts %lld, track id %d\n", node->getPTS(), trackID);
            ret = node->getPacketWithPTS(pPacket, requireParams, pts);
            if (ret == ERROR_NONE) {
              // OMAF_LOG(LOG_INFO, "PACKET Get correct packet with pts %lld, track id %d\n", pts, trackID);
              bpacket_readed = true;
              timeline_point_ = node->getTimelinePoint();
            }

            if (0 == node->packetQueueSize()) {
              //OMAF_LOG(LOG_INFO, "Node count=%d. %s\n", node.use_count(), node->to_string().c_str());
              nodeset.segment_nodes_.erase(it);
            }

            break;
          }
          it++;
        }
        if (bpacket_readed)
        {
          break;
        }
      }
      }
    }
    if (!bpacket_readed) {
      // FIXME, this may a bug for using the timeline point as segment number
      if (work_params_.stream_type_ == DASH_STREAM_DYNMIC || !checkEOS(timeline_point_)) {
        pPacket = nullptr;

        ret = ERROR_NULL_PACKET;
      } else {
        ret = ERROR_NONE;
        pPacket = new MediaPacket();
        pPacket->SetEOS(true);
      }
    }

    // 2. sync timeline point for outside reading
    // drop all dashset whose timeline point is less than timeline point
    if (timeline_point_ != -1) {
      OMAF_LOG(LOG_INFO, "To clear the timeline point < %ld\n", timeline_point_ - 1);
      clearOlderSegmentSet(timeline_point_ - 1);
      std::lock_guard<std::mutex> lock(segment_parsed_mutex_);
      std::list<OmafSegmentNodeTimedSet>::iterator it = segment_parsed_list_.begin();
      while (it != segment_parsed_list_.end() && it->timeline_point_ < timeline_point_ - 1) {
        it = segment_parsed_list_.erase(it);  // 'it' will move to next when calling erase
      }
    }
    return ret;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Failed to read frame for trackid=%u, ex: %s\n", trackID, ex.what());
    return ERROR_INVALID;
  }
}

inline bool OmafReaderManager::isEmpty(std::mutex &mutex, const std::list<OmafSegmentNodeTimedSet> &nodes) noexcept {
  try {
    std::lock_guard<std::mutex> lock(mutex);
    if (nodes.empty()) {
      return true;
    }
    if (nodes.size() > 1) {
      return false;
    }

    const OmafSegmentNodeTimedSet &node_set = nodes.front();
    if (node_set.segment_nodes_.empty()) {
      return true;
    }

    return false;

  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Failed to check the empty, ex: %s\n", ex.what());
    return false;
  }
}

bool OmafReaderManager::checkEOS(int64_t segment_num) noexcept {
  try {
    if (media_source_ == nullptr) {
      return true;
    }

    bool eos = true;

    // set EOS when all stream meet eos
    for (int i = 0; i < media_source_->GetStreamCount(); i++) {
      OmafMediaStream *pStream = media_source_->GetStream(i);
      if (pStream) {
        uint64_t segmentDur = pStream->GetSegmentDuration();
        if (segmentDur == 0) {
          continue;
        }

        double tmpSegNum = static_cast<double>(work_params_.duration_) / 1000.0 / static_cast<double>(segmentDur);
        int64_t totalSegNum = static_cast<int64_t>(tmpSegNum);
        totalSegNum = abs(tmpSegNum - totalSegNum) < 1e-6 ? totalSegNum : totalSegNum + 1;
        if (segment_num < totalSegNum) {
          eos = false;
          break;
        }
      }
    }
    if (eos) {
      if (!isEmpty(segment_opening_mutex_, segment_opening_list_)) {
        OMAF_LOG(LOG_WARNING, "segment opening list is not empty!\n");
      }
    }
    if (eos) {
      if (!isEmpty(segment_opened_mutex_, segment_opened_list_)) {
        OMAF_LOG(LOG_WARNING, "segment opened list is not empty!\n");
      }
    }
    if (eos) {
      if (!isEmpty(segment_parsed_mutex_, segment_parsed_list_)) {
        OMAF_LOG(LOG_WARNING, "segment parsed list is not empty!\n");
      }
    }
    if (eos) {
      OMAF_LOG(LOG_INFO, "Meet EOS\n");
    }
    return eos;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Failed to check the eos, ex: %s\n", ex.what());
    return true;
  }
}

OMAF_STATUS OmafReaderManager::GetPacketQueueSize(uint32_t trackID, size_t &size) noexcept {
  try {
    std::unique_lock<std::mutex> lock(segment_parsed_mutex_);
    for (auto &nodeset : segment_parsed_list_) {
      for (auto &node : nodeset.segment_nodes_) {
        if (node->getTrackId() == trackID) {
          size = node->packetQueueSize();
          return ERROR_NONE;
        }
      }
    }

    return ERROR_INVALID;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Failed to read packet size for trackid=%u, ex: %s\n", trackID, ex.what());
    return ERROR_INVALID;
  }
}
uint64_t OmafReaderManager::GetOldestPacketPTSForTrack(int trackId) {
  try {
    uint64_t oldestPTS = 0;
    bool findPTS = false;
    std::unique_lock<std::mutex> lock(segment_parsed_mutex_);
    for (auto &nodeset : segment_parsed_list_) {
      for (auto &node : nodeset.segment_nodes_) {
        if (node->getTrackId() == static_cast<uint32_t>(trackId) && !node->isCatchup()) {
          //return node->getPTS();
          if (!findPTS)
          {
            oldestPTS = node->getPTS();
            findPTS = true;
          }
          else
          {
            if (oldestPTS > node->getPTS())
            {
              oldestPTS = node->getPTS();
            }
          }
        }
      }
    }

    return oldestPTS;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Failed to read packet size for trackid=%u, ex: %s\n", trackId, ex.what());
    return 0;
  }
}
void OmafReaderManager::RemoveOutdatedPacketForTrack(int trackId, uint64_t currPTS) {
  try {
    std::unique_lock<std::mutex> lock(segment_parsed_mutex_);
    for (auto &nodeset : segment_parsed_list_) {
      for (auto &node : nodeset.segment_nodes_) {
        if (node->getTrackId() == static_cast<uint32_t>(trackId) && !node->isCatchup()) {
          node->clearPacketByPTS(currPTS);
        }
      }
    }
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Failed to read packet size for trackid=%d, ex: %s\n", trackId, ex.what());
  }
}

void OmafReaderManager::RemoveOutdatedCatchupPacketForTrack(int trackId, uint64_t currPTS) {
  try {
    std::unique_lock<std::mutex> lock(segment_parsed_mutex_);
    for (auto &nodeset : segment_parsed_list_) {//for timeline
      uint64_t sample_num = GetSamplesNumPerSegmentForTimeLine(1);
      if (sample_num == 0) return;
      int64_t currtl = currPTS / sample_num + 1;
      if (nodeset.timeline_point_ != currtl) continue;//only delete the current timeline catchup packets
      for (auto &node : nodeset.segment_nodes_) {
        if (node->getTrackId() == static_cast<uint32_t>(trackId) && node->isCatchup()) {
          node->clearPacketByPTS(currPTS);
        }
      }
    }
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Failed to read packet size for trackid=%d, ex: %s\n", trackId, ex.what());
  }
}

void OmafReaderManager::initSegmentStateChange(std::shared_ptr<OmafSegment> pInitSeg,
                                               OmafSegment::State state) noexcept {
  try {
    if (state != OmafSegment::State::OPEN_SUCCES) {
      OMAF_LOG(LOG_ERROR, "Failed to open the init segment, state= %d\n", static_cast<int>(state));
      OMAF_LOG(LOG_ERROR," Track id is %u\n", pInitSeg->GetTrackId());
      return;
    }

    // 1. parse the segment
    OMAF_STATUS ret = reader_->parseInitializationSegment(pInitSeg.get(), pInitSeg->GetInitSegID());
    if (ret != ERROR_NONE) {
      OMAF_LOG(LOG_ERROR, "parse initialization segment failed! code= %d\n", ret);
      return;
    }

    initSeg_ready_count_++;

    OMAF_LOG(LOG_INFO, "Parsed init seg %u\n", pInitSeg->GetInitSegID());
    // 2 get the track information when all init segment parsed
    if (initSegParsedCount() == media_source_->GetTrackCount() && !IsInitSegmentsParsed()) {
      buildInitSegmentInfo();
    }
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Failed to parse the init segment, ex: %s\n", ex.what());
  }
}

void OmafReaderManager::buildInitSegmentInfo(void) noexcept {
  try {
    std::lock_guard<std::mutex> lock(initSeg_mutex_);
    // 1. get the track information
    std::vector<TrackInformation *> track_infos;
    reader_->getTrackInformations(track_infos);

    // 2. go through the track information
    for (auto track : track_infos) {
      if (track == nullptr) {
        OMAF_LOG(LOG_ERROR, "Meet empty track!\n");
        continue;
      }

      // FIXME there would has bug, if more than one stream.
      // or we need update the logic for more than one stream
      for (int i = 0; i < media_source_->GetStreamCount(); i++) {
        OmafMediaStream *pStream = media_source_->GetStream(i);

        // 2.1.1 check the adaptation set
        std::map<int, OmafAdaptationSet *> pMediaAS = pStream->GetMediaAdaptationSet();
        for (auto as : pMediaAS) {
          OmafAdaptationSet *pAS = (OmafAdaptationSet *)as.second;
          // FIXME GetInitSegID or GetSegID
          if (pAS->GetInitSegment()->GetInitSegID() == track->initSegmentId) {
            auto dash_track_id = buildDashTrackId(track->trackId);
            pAS->SetTrackNumber(static_cast<int>(dash_track_id));
            pAS->GetInitSegment()->SetTrackId(dash_track_id);
            initSeg_trackIds_map_[track->initSegmentId] = dash_track_id;
            trackIds_initSeg_map_[dash_track_id] = track->initSegmentId;
            OMAF_LOG(LOG_INFO, "Initse id=%u, trackid=%u\n", track->initSegmentId, dash_track_id);
            break;
          }
        }  // end for adaptation set loop

        // 2.1.2 if has init the track, then loop to next
        if (initSeg_trackIds_map_.find(track->initSegmentId) != initSeg_trackIds_map_.end()) {
          break;
        }

        // 2.1.3 check the extractors
        std::map<int, OmafExtractor *> pExtratorAS = pStream->GetExtractors();
        for (auto &extractor : pExtratorAS) {
          OmafExtractor *pExAS = extractor.second;
          // FIXME GetInitSegID or GetSegID
          if (pExAS->GetInitSegment()->GetInitSegID() == track->initSegmentId) {
            auto dash_track_id = buildDashTrackId(track->trackId);
            pExAS->SetTrackNumber(static_cast<int>(dash_track_id));
            pExAS->GetInitSegment()->SetTrackId(dash_track_id);
            initSeg_trackIds_map_[track->initSegmentId] = dash_track_id;
            trackIds_initSeg_map_[dash_track_id] = track->initSegmentId;
            OMAF_LOG(LOG_INFO, "Initse id=%u, trackid=%u\n", track->initSegmentId, dash_track_id);
            break;
          }
        }  // end for extractors loop
      }    // end stream loop
    }      // end for track loop
    bInitSeg_all_ready_ = true;

    // 2.2 setup the id map
    setupTrackIdMap();

    // 3.1 release the track informations
    for (auto &track : track_infos) {
      if (track) {
        delete track;
        track = nullptr;
      }
    }
    track_infos.clear();
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Failed to parse the init segment, ex: %s\n", ex.what());
  }
}

void OmafReaderManager::setupTrackIdMap(void) noexcept {
  try {
    for (int i = 0; i < media_source_->GetStreamCount(); i++) {
      OmafMediaStream *pStream = media_source_->GetStream(i);
      std::map<int, OmafExtractor *> pExtratorAS = pStream->GetExtractors();
      for (auto extractor : pExtratorAS) {
        OmafExtractor *pExAS = extractor.second;
        uint32_t track_id = pExAS->GetTrackNumber();
        std::list<int> depends_track_ids = pExAS->GetDependTrackID();
        if (trackIds_initSeg_map_.find(track_id) != trackIds_initSeg_map_.end()) {
          auto &initSeg_id = trackIds_initSeg_map_[track_id];
          initSegId_depends_map_[initSeg_id] = std::vector<uint32_t>();
          for (auto depend_id : depends_track_ids) {
            if (trackIds_initSeg_map_.find(depend_id) != trackIds_initSeg_map_.end()) {
              auto depend_initSeg_id = trackIds_initSeg_map_[depend_id];
              initSegId_depends_map_[initSeg_id].push_back(depend_initSeg_id);
            }
          }
        }
      }  // end for extractors
    }    // end for media stream
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Failed to set up track map, ex: %s\n", ex.what());
  }
}

void OmafReaderManager::normalSegmentStateChange(std::shared_ptr<OmafSegment> segment,
                                                 OmafSegment::State state) noexcept {
  try {
    // 0. check params
    if (segment.get() == nullptr) {
      OMAF_LOG(LOG_ERROR, "Empty segment!\n");
      return;
    }

    OmafSegmentNode::Ptr opened_dash_node;

    // 1. remove from the opening list
    {
      std::lock_guard<std::mutex> lock(segment_opening_mutex_);
      for (auto &nodeset : segment_opening_list_) {
        if (nodeset.timeline_point_ == segment->GetTimelinePoint()) {
          std::list<OmafSegmentNode::Ptr>::iterator it = nodeset.segment_nodes_.begin();
          while (it != nodeset.segment_nodes_.end()) {
            auto &node = (*(*it).get());
            if (node == segment) {
              opened_dash_node = std::move(*it);
              nodeset.segment_nodes_.erase(it);
              break;
            }
            it++;
          }
        }
      }
    }

    if (opened_dash_node.get() == nullptr) {
      OMAF_LOG(LOG_WARNING, "Can't find the dash node for coming segment!\n");
      return;
    }

#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
    //trace
    if (opened_dash_node.get() != nullptr) {
        if (opened_dash_node->isExtractor()) {
            const char *trackType = "extractor_track";
            uint64_t streamSize = segment->GetStreamSize();

            char tileRes[128] = { 0 };
            snprintf(tileRes, 128, "%s", "none");
            int trackIndex = segment->GetTrackId();
            uint32_t nSegID = segment->GetSegID();
            tracepoint(bandwidth_tp_provider, packed_segment_size, trackIndex, trackType, tileRes, nSegID, streamSize);
        }
        else {
            const char *trackType = "tile_track";
            uint64_t streamSize = segment->GetStreamSize();

            char tileRes[128] = { 0 };
            snprintf(tileRes, 128, "%s", "none");
            int trackIndex = segment->GetTrackId();
            uint32_t nSegID = segment->GetSegID();
            tracepoint(bandwidth_tp_provider, packed_segment_size, trackIndex, trackType, tileRes, nSegID, streamSize);
        }
    }
#endif
#endif


    // 2. append to the dash opened list
    {
      std::lock_guard<std::mutex> lock(segment_opened_mutex_);
      bool new_timeline_point = true;
      if (opened_dash_node->GetMode() == OmafDashMode::EXTRACTOR) {
        // this is a dash node built from extractor
        // build depends based on extractor
        if (opened_dash_node->isExtractor()) {
          for (auto &nodeset : segment_opened_list_) {
            if (nodeset.timeline_point_ == opened_dash_node->getTimelinePoint()) {
              new_timeline_point = false;
              std::list<OmafSegmentNode::Ptr>::iterator it = nodeset.segment_nodes_.begin();

              // put all depends node into depend
              const auto &depends = initSegId_depends_map_[opened_dash_node->getInitSegId()];

              while (it != nodeset.segment_nodes_.end()) {
                // check whether this dash node is belong the target dash's depends
                auto initSeg_id = (*it)->getInitSegId();
                auto is_in_depend = false;
                for (auto id : depends) {
                  if (id == initSeg_id) {
                    is_in_depend = true;
                    break;
                  }
                }  // end for id

                if (is_in_depend) {
                  // remove from the list and push to depends
                  opened_dash_node->pushDepends(std::move(*it));
                  // it will move to next when calling erase
                  it = nodeset.segment_nodes_.erase(it);
                } else {
                  it++;
                }
              }  // end while

              nodeset.segment_nodes_.push_back(std::move(opened_dash_node));
              break;
            }  // end for if same timeslide
          }    // end for dash_opened_list loop
        } else {
          // this is a dash node built from the general segment
          for (auto &nodeset : segment_opened_list_) {
            if (nodeset.timeline_point_ == opened_dash_node->getTimelinePoint()) {
              new_timeline_point = false;
              // while loop all node in the dash nodes,
              // the opend_dash_node maybe added to more than one dash node who built from extractor
              // the detail structure depend on the media stream's extractor logic
              // FIXME, if one segment belong to different extractor at the same time, the logix has bug now.
              // it should update the logic of parse in the dash_node
              //
              bool bfind_extractor = false;
              for (auto &node : nodeset.segment_nodes_) {
                // this is a extractor
                if (node->isExtractor()) {
                  const auto &depends = initSegId_depends_map_[node->getInitSegId()];
                  for (auto id : depends) {
                    // this dash node is in depends of the node
                    if (id == opened_dash_node->getInitSegId()) {
                      bfind_extractor = true;
                      node->pushDepends(std::move(opened_dash_node));
                      break;
                    }
                  }  // end for auto id
                }
              }  // end for auto node
              if (!bfind_extractor) {
                nodeset.segment_nodes_.push_back(std::move(opened_dash_node));
              }
              break;
            }
          }  // end for nodeset
        }
      } else {
        // not extractor mode
        for (auto &nodeset : segment_opened_list_) {
          if (nodeset.timeline_point_ == opened_dash_node->getTimelinePoint()) {
            nodeset.segment_nodes_.push_back(std::move(opened_dash_node));
            new_timeline_point = false;
            break;
          }
        }
      }

      if (new_timeline_point) {
        bool to_append = true;
        if (!segment_opened_list_.empty()) {
          auto &tail_nodeset = segment_opened_list_.back();
          // the nodeset queue's timeline should increase one by one
          if (opened_dash_node->getTimelinePoint() <= tail_nodeset.timeline_point_) {
            OMAF_LOG(LOG_WARNING, "Try to insert the timeline: %lld, which <= %lld\n", opened_dash_node->getTimelinePoint(), tail_nodeset.timeline_point_);
            to_append = false;
          }
        }
        if (to_append) {
          OmafSegmentNodeTimedSet nodeset;
          nodeset.timeline_point_ = opened_dash_node->getTimelinePoint();
          nodeset.create_time_ = std::chrono::steady_clock::now();
          nodeset.segment_nodes_.push_back(std::move(opened_dash_node));
          segment_opened_list_.emplace_back(nodeset);
        }
      }

      // TODO, refine the logic,
      // we may send the notify by checking all segment ready for extractor mode
      segment_opened_cv_.notify_all();
    }  // end of append to the dash opened list
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when set up track map, ex: %s\n", ex.what());
  }
}

void OmafReaderManager::threadRunner() noexcept {
  try {
    //OMAF_LOG(LOG_INFO, "Start the reader runner!\n");

    while (breader_working_) {
      // 1. find the ready segment/dash_node opend list
      OmafSegmentNode::Ptr ready_dash_node = findReadySegmentNode();

      // 1.1 no ready dash node, then wait
      if (ready_dash_node.get() == nullptr) {
        std::unique_lock<std::mutex> lock(segment_opened_mutex_);
        segment_opened_cv_.wait(lock);
        continue;
      }

      // 2. parse the ready segment/dash_node
      const int64_t timeline_point = ready_dash_node->getTimelinePoint();
      // if (ready_dash_node->isCatchup()) OMAF_LOG(LOG_INFO, "Catch up node found! timeline is %lld, track id %d\n", timeline_point, ready_dash_node->getTrackId());
      //OMAF_LOG(LOG_INFO, "Get ready segment! timeline=%lld\n", timeline_point);
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
      tracepoint(mthq_tp_provider, T4_parse_start_time, timeline_point);
#endif
#endif
      OMAF_STATUS ret = ready_dash_node->parse();
      // if (ready_dash_node->isCatchup()) OMAF_LOG(LOG_INFO, "Catch up node parsed! timeline is %lld, track id %d\n", timeline_point, ready_dash_node->getTrackId());

      if (ready_dash_node->getMediaType() == MediaType_Video)
      {
          std::lock_guard<std::mutex> lock(segment_samples_mutex_);
          std::map<uint64_t, size_t>::iterator it;
          it = samples_num_per_seg_.find(ready_dash_node->getTimelinePoint());
          if (it == samples_num_per_seg_.end())
          {
              samples_num_per_seg_.insert(std::make_pair(ready_dash_node->getTimelinePoint(), ready_dash_node->GetSamplesNum()));
          }
      }
      //samples_num_per_seg_ = ready_dash_node->GetSamplesNum();

      // 3. move the parsed segment/dash_node to parsed list
      if (ret == ERROR_NONE) {
        //OMAF_LOG(LOG_INFO, "Success to parsed dash segment! timeline=%lld\n", timeline_point);
#ifndef _ANDROID_NDK_OPTION_
#ifdef _USE_TRACE_
      tracepoint(mthq_tp_provider, T5_parse_end_time, timeline_point);
#endif
#endif
        std::unique_lock<std::mutex> lock(segment_parsed_mutex_);
        bool new_timeline_point = true;
        for (auto &nodeset : segment_parsed_list_) {
          if (nodeset.timeline_point_ == timeline_point) {
            // if (ready_dash_node->isCatchup())
            // LOG(INFO) << "Push catch up parsed node PTS " << nodeset.timeline_point_ << " with track id " << ready_dash_node->getTrackId() << " into parsed list" << endl;
            nodeset.segment_nodes_.push_back(std::move(ready_dash_node));
            new_timeline_point = false;
            break;
          }
        }
        if (new_timeline_point) {
          OmafSegmentNodeTimedSet nodeset;
          nodeset.timeline_point_ = timeline_point;
          nodeset.create_time_ = std::chrono::steady_clock::now();
          // if (ready_dash_node->isCatchup())
          //   LOG(INFO) << "Push catch up parsed node PTS " << nodeset.timeline_point_ << " with track id " << ready_dash_node->getTrackId() << " into parsed list" << endl;
          nodeset.segment_nodes_.push_back(std::move(ready_dash_node));
          segment_parsed_list_.emplace_back(nodeset);
        }
        segment_parsed_cv_.notify_all();
      } else {
        OMAF_LOG(LOG_ERROR, "Failed to parse %s\n", ready_dash_node->to_string().c_str());
      }

      // 4. clear dash set whose timeline point older than current ready segment/dash_node
      // we use simple logic to main the dash node sets
      // we will remove older dash nodes
      //clearOlderSegmentSet(timeline_point_);
    }
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception in reader runner, ex: %s\n", ex.what());
  }

  OMAF_LOG(LOG_INFO, "Exit from the reader runner!\n");
}

OmafSegmentNode::Ptr OmafReaderManager::findReadySegmentNode() noexcept {
  try {
    OmafSegmentNode::Ptr ready_dash_node;
    std::unique_lock<std::mutex> lock(segment_opened_mutex_);
    for (auto &nodeset : segment_opened_list_) {
      //OMAF_LOG(LOG_INFO, "To find the ready node set timeline=%lld\n", nodeset.timeline_point_);

      // 1.1.1 try to find the ready node
      std::list<OmafSegmentNode::Ptr>::iterator it = nodeset.segment_nodes_.begin();
      while (it != nodeset.segment_nodes_.end()) {
        auto &node = *it;
        if (node->isReady()) {
          if (node->GetMode() == OmafDashMode::EXTRACTOR) {
            if (node->isExtractor()) {
              ready_dash_node = std::move(node);
            }
            else if (node->getMediaType() == MediaType_Audio) {
              ready_dash_node = std::move(node);
              OMAF_LOG(LOG_INFO, "Found one ready audio track segment node!\n");
            }
          } else {
            ready_dash_node = std::move(node);
          }

          if (ready_dash_node.get() != nullptr) {
            break;
          }
        }
        it++;
      }  // end while

      // 1.1.2 find the ready node, exit and return
      if (ready_dash_node.get() != nullptr) {
        nodeset.segment_nodes_.erase(it);
        OMAF_LOG(LOG_INFO, "Get ready segment node with timeline %ld\n", nodeset.timeline_point_);
        break;
      }
      else {
        OMAF_LOG(LOG_INFO, "No ready segment node for timeline %ld\n", nodeset.timeline_point_);
      }

      // 1.2.1 no ready node found, then check whether timeout
      bool btimeout = true;
      for (auto node : nodeset.segment_nodes_) {
        if (!node->checkTimeout(work_params_.segment_timeout_ms_)) {
          btimeout = false;
          break;
        }
      }

      // 1.2.2 some node not timeout, still wait
      if (!btimeout) {
        // do nothing, wait the data ready
        break;
      }

      // 1.3 all nodes timeout, then move to next timeline point's nodeset
      continue;
    }  // end for nodeset loop

    return ready_dash_node;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when find the ready dash node, ex: %s\n", ex.what());
    return nullptr;
  }
}

void OmafReaderManager::clearOlderSegmentSet(int64_t timeline_point) noexcept {
  try {
    {
      // 1. clear the opening list to save the network bandwith
      std::lock_guard<std::mutex> lock(segment_opening_mutex_);
      std::list<OmafSegmentNodeTimedSet>::iterator it = segment_opening_list_.begin();
      while ((it != segment_opening_list_.end()) && (it->timeline_point_ < timeline_point)) {
        OMAF_LOG(LOG_INFO, "Removing older dash opening list, timeline=%lld\n", it->timeline_point_);
        for (auto &node : it->segment_nodes_) {
          node->stop();
        }
        it = segment_opening_list_.erase(it);
      }
    }

    {
      // 2. clear the opened dash node list
      std::lock_guard<std::mutex> lock(segment_opened_mutex_);
      std::list<OmafSegmentNodeTimedSet>::iterator it = segment_opened_list_.begin();
      while ((it != segment_opened_list_.end()) && (it->timeline_point_ < timeline_point)) {
        OMAF_LOG(LOG_INFO, "Removing older dash opened list, timeline=%lld\n", it->timeline_point_);
        it = segment_opened_list_.erase(it);
      }
    }
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when clear the older dash set, whose timeline is older than %lld, ex: %s\n", timeline_point, ex.what());
  }
}

// FIXME, dims and vps/sps/pps may not in the same sample
int OmafPacketParams::init(std::shared_ptr<OmafReader> reader, uint32_t reader_trackId, uint32_t sampleId) noexcept {
  try {
    // 1. read width and height
    OMAF_STATUS ret = reader->getDims(reader_trackId, sampleId, width_, height_);
    if (ret) {
      OMAF_LOG(LOG_ERROR, "Failed to get sample dims !\n");
      return ret;
    }
    if (!width_ || !height_) {
      OMAF_LOG(LOG_ERROR, "Failed to get the dims!\n");
      return OMAF_ERROR_INVALID_DATA;
    }

    // 2. read vps/sps/pps params
    std::vector<VCD::OMAF::DecoderSpecificInfo> parameterSets;
    ret = reader->getDecoderConfiguration(reader_trackId, sampleId, parameterSets);
    if (ret) {
      OMAF_LOG(LOG_ERROR, "Failed to get VPS/SPS/PPS !\n");
      return ret;
    }

    for (auto const &parameter : parameterSets) {
      if (parameter.codecSpecInfoType == VCD::MP4::HEVC_VPS) {
        vps_.resize(parameter.codecSpecInfoBits.size);
        for (size_t i = 0; i < parameter.codecSpecInfoBits.size; i++) {
          vps_[i] = parameter.codecSpecInfoBits[i];
        }
      }

      if (parameter.codecSpecInfoType == VCD::MP4::HEVC_SPS) {
        sps_.resize(parameter.codecSpecInfoBits.size);
        for (size_t i = 0; i < parameter.codecSpecInfoBits.size; i++) {
          sps_[i] = parameter.codecSpecInfoBits[i];
        }
      }

      if (parameter.codecSpecInfoType == VCD::MP4::HEVC_PPS) {
        pps_.resize(parameter.codecSpecInfoBits.size);
        for (uint32_t i = 0; i < parameter.codecSpecInfoBits.size; i++) {
          pps_[i] = parameter.codecSpecInfoBits[i];
        }
      }
    }

    binit_ = vps_.size() && sps_.size() && pps_.size();

    params_.resize(vps_.size() + sps_.size() + pps_.size());
    memcpy_s(params_.data(), params_.size(), vps_.data(), vps_.size());
    memcpy_s(params_.data() + vps_.size(), sps_.size(), sps_.data(), sps_.size());
    memcpy_s(params_.data() + vps_.size() + sps_.size(), pps_.size(), pps_.data(), pps_.size());

    return binit_ ? ERROR_NONE : ERROR_INVALID;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Failed to init the packet params, ex: %s\n", ex.what());
    binit_ = false;
    return ERROR_INVALID;
  }
}

int OmafAudioPacketParams::unPackUnsignedIntValue(uint8_t bitsNum, uint32_t *value)
{
  int ret = ERROR_NONE;
  if ((8 - ((uint8_t)(curr_bit_pos_) % 8)) > bitsNum)
  {
    uint8_t tempVal = params_[(uint8_t)(curr_bit_pos_)/8];
    tempVal <<= (curr_bit_pos_ % 8);
    tempVal >>= (8 - bitsNum);
    *value = (uint32_t)tempVal;
  }
  else if ((16 - ((uint8_t)(curr_bit_pos_) % 16)) > bitsNum)
  {
    uint16_t tempVal1 = (uint16_t)(params_[(uint8_t)(curr_bit_pos_)/8]);
    uint16_t tempVal2 = (uint16_t)(params_[(uint8_t)(curr_bit_pos_)/8 + 1]);
    uint16_t tempVal  = (tempVal1 << 8) | tempVal2;
    tempVal <<= curr_bit_pos_;
    tempVal >>= (16 - bitsNum);
    *value = (uint32_t)tempVal;
  }
  else
  {
    OMAF_LOG(LOG_ERROR, "Not supported bits number %d for bits reader !\n", bitsNum);
    ret = OMAF_ERROR_INVALID_DATA;
  }
  return ret;
}

int OmafAudioPacketParams::init(std::shared_ptr<OmafReader> reader, uint32_t reader_trackId, uint32_t sampleId) noexcept {
  try {
    std::vector<VCD::OMAF::DecoderSpecificInfo> parameterSets;
    int ret = reader->getDecoderConfiguration(reader_trackId, sampleId, parameterSets);
    if (ret) {
      OMAF_LOG(LOG_ERROR, "Failed to get audio specific info !\n");
      return ret;
    }

    for (auto const &parameter : parameterSets) {
      if (parameter.codecSpecInfoType == VCD::MP4::AudioSpecificConfig) {
        params_.resize(parameter.codecSpecInfoBits.size);
        for (size_t i = 0; i < parameter.codecSpecInfoBits.size; i++) {
          params_[i] = parameter.codecSpecInfoBits[i];
        }
      }
    }

    binit_ = params_.size();

    ret = unPackUnsignedIntValue(5, &objType_);
    if (ret)
      return ret;

    OMAF_LOG(LOG_INFO, "Parsed audio obj type %d\n", objType_);
    curr_bit_pos_ += 5;

    ret = unPackUnsignedIntValue(4, &frequencyIdx_);
    if (ret)
      return ret;

    OMAF_LOG(LOG_INFO, "Parsed audio sample rate idx %d\n", frequencyIdx_);
    curr_bit_pos_ += 4;

    ret = unPackUnsignedIntValue(4, &channelCfg_);
    if (ret)
      return ret;

    OMAF_LOG(LOG_INFO, "Parsed audio channel configuration %d\n", channelCfg_);
    curr_bit_pos_ += 4;

    return ERROR_NONE;
  } catch(const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Failed to init audio packet params, ex: %s\n", ex.what());
    binit_ = false;
    return ERROR_INVALID;
  }
}

void OmafAudioPacketParams::packOneBit(bool value)
{
  --curr_bit_pos_;
  if (curr_bit_pos_ == -1)
  {
    curr_bit_pos_ = 7;
    params_.push_back(0);
  }
  params_[params_.size() - 1] |= (uint8_t(value) << curr_bit_pos_);
}

void OmafAudioPacketParams::packUnsignedIntValue(uint8_t bitsNum, uint32_t value)
{
  for (int32_t num = (bitsNum - 1); num >= 0; --num)
  {
    packOneBit(((value >> num) & 1));
  }
}

void OmafAudioPacketParams::writeADTSHdr(uint32_t frameSize)
{
  params_.clear();
  curr_bit_pos_ = 0;

  packUnsignedIntValue(12, 0xfff);
  packUnsignedIntValue(1, 0);
  packUnsignedIntValue(2, 0);
  packUnsignedIntValue(1, 1);
  packUnsignedIntValue(2, objType_);
  packUnsignedIntValue(4, frequencyIdx_);
  packUnsignedIntValue(1, 0);
  packUnsignedIntValue(3, channelCfg_);
  packUnsignedIntValue(1, 0);
  packUnsignedIntValue(1, 0);

  packUnsignedIntValue(1, 0);
  packUnsignedIntValue(1, 0);
  packUnsignedIntValue(13, (frameSize + 7)); //ADTS Header size is 7 bytes
  packUnsignedIntValue(11, 0x7ff);
  packUnsignedIntValue(2, 0);
  OMAF_LOG(LOG_INFO, "ADTS header size %ld bytes\n", params_.size());
}

int OmafSegmentNode::parse() noexcept {
  try {
    clock_t lBefore = clock();
    clock_t lBefore2 = lBefore;
    double dResult;

    auto reader = reader_.lock();
    if (reader.get() == nullptr) {
      OMAF_LOG(LOG_ERROR, "The omaf reader is empty!\n");
      return ERROR_NULL_PTR;
    }

    OMAF_STATUS ret = ERROR_NONE;
    // 1.1 calling depends to parse the segment
    for (auto &node : depends_) {
      ret = node->parseSegmentStream(reader);
      if (ERROR_NONE != ret) {
        OMAF_LOG(LOG_ERROR, "Failed to parse the dependent node %s. Error code=%d\n", node->to_string().c_str(), ret);
        return ret;
      }
    }

    // 1.2 parse self
    ret = parseSegmentStream(reader);
    if (ERROR_NONE != ret) {
      OMAF_LOG(LOG_ERROR, "Failed to parse %s. Error code=%d\n", this->to_string().c_str(), ret);
      return ret;
    }
    dResult = (double)(clock() - lBefore) * 1000 / CLOCKS_PER_SEC;
    OMAF_LOG(LOG_INFO, "OmafSegmentNode parsing segment dependency and self time is %f ms\n", dResult);

    // 2 cache packets from the reader
    lBefore = clock();
    ret = cachePackets(reader);
    if (ERROR_NONE != ret) {
      OMAF_LOG(LOG_ERROR, "Failed to read packet from %s. Error code=%d\n", this->to_string().c_str(), ret);
      return ERROR_INVALID;
    }
    dResult = (double)(clock() - lBefore) * 1000 / CLOCKS_PER_SEC;
    OMAF_LOG(LOG_INFO, "OmafSegmentNode parsing cachePackets time is %f ms\n", dResult);

    // 3.1 remove self segment from reader
    lBefore = clock();
    ret = removeSegmentStream(reader);
    if (ERROR_NONE != ret) {
      OMAF_LOG(LOG_ERROR, "Failed to remove segment from reader %s. Error code=%d\n", this->to_string().c_str(), ret);
      return ERROR_INVALID;
    }

    // 3.2 remove segment of depends from reader
    for (auto &node : depends_) {
      ret = node->removeSegmentStream(reader);
      if (ERROR_NONE != ret) {
        OMAF_LOG(LOG_ERROR, "Failed to remove dependent segment form reader %s. Error code=%d\n", node->to_string().c_str(), ret);
        return ret;
      }
    }
    dResult = (double)(clock() - lBefore) * 1000 / CLOCKS_PER_SEC;
    OMAF_LOG(LOG_INFO, "OmafSegmentNode parsing remove segment dependency and self is %f ms\n", dResult);
    dResult = (double)(clock() - lBefore2) * 1000 / CLOCKS_PER_SEC;
    OMAF_LOG(LOG_INFO, "OmafSegmentNode parsing time in total is %f ms\n", dResult);
    return ERROR_NONE;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when parse the segment! ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

int OmafSegmentNode::start(void) noexcept {
  try {
    OMAF_STATUS ret = ERROR_NONE;
    if (segment_.get() == nullptr) {
      OMAF_LOG(LOG_ERROR, "Try to open the empty segment!\n");
      return ERROR_INVALID;
    }

    auto reader_mgr = omaf_reader_mgr_.lock();
    if (reader_mgr.get() == nullptr) {
      OMAF_LOG(LOG_ERROR, "The reader manager is empty!\n");
      return ERROR_NULL_PTR;
    }

    start_time_ = std::chrono::steady_clock::now();
    ret = segment_->Open(reader_mgr->dash_client_);
    if (ret != ERROR_NONE) {
      OMAF_LOG(LOG_ERROR, "Failed to open the segment!\n");
      return ret;
    }

    return ERROR_NONE;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when open the segment, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

int OmafSegmentNode::stop(void) noexcept {
  try {
    OMAF_STATUS ret = ERROR_NONE;
    if (segment_.get() == nullptr) {
      OMAF_LOG(LOG_ERROR, "Try to stop the empty segment!\n");
      return ERROR_INVALID;
    }
    ret = this->segment_->Stop();
    if (ret != ERROR_NONE) {
      OMAF_LOG(LOG_WARNING, "Failed to stop the segment!\n");
    }

    for (auto &node : depends_) {
      node->stop();
    }
    return ret;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when stop the segment, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

int OmafSegmentNode::parseSegmentStream(std::shared_ptr<OmafReader> reader) noexcept {
  try {
    return reader->parseSegment(segment_.get(), segment_->GetInitSegID(), segment_->GetSegID());
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when parse the segment! ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

int OmafSegmentNode::removeSegmentStream(std::shared_ptr<OmafReader> reader) noexcept {
  try {
    return reader->invalidateSegment(segment_->GetInitSegID(), segment_->GetSegID());

  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when remove the segment from reader! ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

// int OmafSegmentNode::getPacket(std::unique_ptr<MediaPacket> &pPacket, bool needParams) {
int OmafSegmentNode::getPacket(MediaPacket *&pPacket, bool requireParams) noexcept {
  try {
    std::lock_guard<std::mutex> lock(packet_mutex_);
    if (media_packets_.size() <= 0) {
      OMAF_LOG(LOG_INFO, "There is no packets\n");
      return ERROR_NULL_PACKET;
    }

    pPacket = media_packets_.front();
    media_packets_.pop();
    if (pPacket->GetMediaType() == MediaType_Video)
    {
      if (requireParams) {
        auto packet_params = (bExtractor_ == true) ? getPacketParamsForExtractors() : getPacketParams();
        if (packet_params.get() == nullptr || !packet_params->binit_) {
          OMAF_LOG(LOG_ERROR, "Invalid VPS/SPS/PPS in getting packet !\n");
          return OMAF_ERROR_INVALID_DATA;
        }
        pPacket->InsertParams(packet_params->params_);
        pPacket->SetVPSLen(packet_params->vps_.size());
        pPacket->SetSPSLen(packet_params->sps_.size());
        pPacket->SetPPSLen(packet_params->pps_.size());
        pPacket->SetVideoHeaderSize(packet_params->params_.size());
        pPacket->SetCatchupFlag(this->bCatchup_);
      }
    }
    else if (pPacket->GetMediaType() == MediaType_Audio)
    {
      if (requireParams) {
        pPacket->InsertADTSHdr();
      }
    }

    return ERROR_NONE;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when read the frame! ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

int OmafSegmentNode::getPacketWithPTS(MediaPacket *&pPacket, bool requireParams, uint64_t pts) noexcept {
  try {
    std::lock_guard<std::mutex> lock(packet_mutex_);
    if (media_packets_.size() <= 0) {
      OMAF_LOG(LOG_INFO, "There is no packets\n");
      return ERROR_NULL_PACKET;
    }

    int64_t pkt_pts = -1;

    do {
      if (media_packets_.size() > 0) {
        pPacket = media_packets_.front();
        pkt_pts = pPacket->GetPTS();
        media_packets_.pop();
      }
      else {
        usleep(5000);
      }
    } while (pkt_pts < (int64_t)pts);

    if (pPacket->GetMediaType() == MediaType_Video)
    {
      if (requireParams) {
        auto packet_params = (bExtractor_ == true) ? getPacketParamsForExtractors() : getPacketParams();
        if (packet_params.get() == nullptr || !packet_params->binit_) {
          OMAF_LOG(LOG_ERROR, "Invalid VPS/SPS/PPS in getting packet !\n");
          return OMAF_ERROR_INVALID_DATA;
        }
        pPacket->InsertParams(packet_params->params_);
        pPacket->SetVPSLen(packet_params->vps_.size());
        pPacket->SetSPSLen(packet_params->sps_.size());
        pPacket->SetPPSLen(packet_params->pps_.size());
        pPacket->SetVideoHeaderSize(packet_params->params_.size());
        pPacket->SetCatchupFlag(this->bCatchup_);
      }
    }
    else if (pPacket->GetMediaType() == MediaType_Audio)
    {
      if (requireParams) {
        pPacket->InsertADTSHdr();
      }
    }

    return ERROR_NONE;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when read the frame! ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

int OmafSegmentNode::cachePackets(std::shared_ptr<OmafReader> reader) noexcept {
  try {
    OMAF_STATUS ret = ERROR_NONE;
    std::shared_ptr<TrackInformation> track_info = findTrackInformation(reader);
    if (track_info.get() == nullptr) {
      OMAF_LOG(LOG_FATAL, "Failed to find the sepcial track information.%d\n", this->to_string().c_str());
      return ERROR_INVALID;
    }

    size_t sample_begin = 0;
    size_t sample_end = 0;
    if (!findSampleIndexRange(track_info, sample_begin, sample_end)) {
      OMAF_LOG(LOG_ERROR, "Failed to find the sample range for segment. %s\n", this->to_string().c_str());
      return ERROR_INVALID;
    }
    samples_num_ = sample_end - sample_begin;
    OMAF_LOG(LOG_INFO, "segment %s has samples num %ld\n", this->to_string().c_str(), samples_num_);
#if 0
    if (sample_begin < 1) {
      LOG(FATAL) << "The begin sample id is less than 1, whose value =" << sample_begin << "." << this->to_string()
                );
      return ERROR_INVALID;
    }
#endif
    if (segment_->GetMediaType() == MediaType_Video) {
      auto packet_params = (bExtractor_ == true) ? getPacketParamsForExtractors() : getPacketParams();
      for (size_t sample = sample_begin; sample < sample_end; sample++) {
        uint32_t reader_track_id = buildReaderTrackId(segment_->GetTrackId(), segment_->GetInitSegID());

        if (packet_params.get() == nullptr) {
          packet_params = std::make_shared<OmafPacketParams>();
        }
        if (!packet_params->binit_) {
          ret = packet_params->init(reader, reader_track_id, sample);
          if (ret != ERROR_NONE) {
            OMAF_LOG(LOG_ERROR, "Failed to read the packet params include width/height/vps/sps/pps!\n");
            return ret;
          }
          if (bExtractor_)
          {
            this->setPacketParamsForExtractors(packet_params);
          }
          else
          {
            this->setPacketParams(packet_params);
          }
        }

        // cache packets
        // std::shared_ptr<MediaPacket> packet = make_unique_vcd<MediaPacket>;
        MediaPacket *packet = new MediaPacket();
        if (packet == nullptr) {
          OMAF_LOG(LOG_ERROR, "Failed to create the packet!\n");
          return ERROR_INVALID;
        }
        uint32_t packet_size = ((packet_params->width_ * packet_params->height_ * 3) >> 1) >> 1;
        // FIXME, we need refine the packet buffer. we may include the vps/pps/sps here
        packet->ReAllocatePacket(packet_size);

        if (mode_ == OmafDashMode::EXTRACTOR) {
          ret = reader->getExtractorTrackSampleData(reader_track_id, sample, static_cast<char *>(packet->Payload()),
                                                    packet_size);
        } else {
          ret = reader->getTrackSampleData(reader_track_id, sample, static_cast<char *>(packet->Payload()), packet_size);
        }
        if (ret != ERROR_NONE) {
          OMAF_LOG(LOG_ERROR, "Failed to read sample data from reader, code= %d\n", ret);
          SAFE_DELETE(packet);
          return ret;
        }

        std::unique_ptr<RegionWisePacking> pRwpk = make_unique_vcd<RegionWisePacking>();
        ret = reader->getPropertyRegionWisePacking(reader_track_id, sample, pRwpk.get());
        if (ret != ERROR_NONE) {
          OMAF_LOG(LOG_ERROR, "Failed to read region wise packing data from reader, code= %d\n", ret);
          SAFE_DELETE(packet);
          return ret;
        }
        packet->SetRwpk(std::move(pRwpk));
        if (reader->GetSegSampleSize() == 0) {
          reader->SetSegSampleSize(track_info->sampleProperties.size);
        }
        packet->SetPTS(reader->GetSegSampleSize() * (segment_->GetSegID() - 1) + sample_begin + sample);
        if ((sample + 1) == sample_end)
        {
            packet->SetSegmentEnded(true);
        }

        // for later binding
        packet->SetQualityRanking(segment_->GetQualityRanking());

        if (mode_ == OmafDashMode::EXTRACTOR) {
          packet->SetQualityNum(MAX_QUALITY_NUM);
          vector<uint32_t> boundLeft(1);  // num of quality is limited to 2.
          vector<uint32_t> boundTop(1);
          const RegionWisePacking &rwpk = packet->GetRwpk();
          packet->SetVideoWidth(rwpk.packedPicWidth);
          packet->SetVideoHeight(rwpk.packedPicHeight);
          if (projection_fmt_ == ProjectionFormat::PF_ERP)
          {
              for (int j = rwpk.numRegions - 1; j >= 0; j--) {
                if (rwpk.rectRegionPacking[j].projRegLeft == 0 && rwpk.rectRegionPacking[j].projRegTop == 0 &&
                    (rwpk.rectRegionPacking[j].packedRegLeft != 0) &&
                    (rwpk.rectRegionPacking[j].packedRegTop == 0)) {
                  boundLeft.push_back(rwpk.rectRegionPacking[j].packedRegLeft);
                  boundTop.push_back(rwpk.rectRegionPacking[j].packedRegTop);
                  break;
                }
              }
          }
          else if (projection_fmt_ == ProjectionFormat::PF_CUBEMAP)
          {
              uint32_t lowTileW = 0;
              uint32_t lowTileH = 0;
              int j = 0;
              for (j = rwpk.numRegions - 1; j >= 0; j--) {
                if (rwpk.rectRegionPacking[j].projRegLeft == 0 && rwpk.rectRegionPacking[j].projRegTop == 0)
                {
                    lowTileW = rwpk.rectRegionPacking[j].projRegWidth;
                    lowTileH = rwpk.rectRegionPacking[j].projRegHeight;
                    break;
                }
              }

              int lowResStartIdx = 0;
              for ( ; j >= 0; j--) {
                if ((rwpk.rectRegionPacking[j].projRegWidth == lowTileW) &&
                    (rwpk.rectRegionPacking[j].projRegHeight == lowTileH) &&
                    (rwpk.rectRegionPacking[j].packedRegLeft != 0) &&
                    (rwpk.rectRegionPacking[j].packedRegTop == 0))
                {
                    lowResStartIdx = j;
                }
              }
              boundLeft.push_back(rwpk.rectRegionPacking[lowResStartIdx].packedRegLeft);
              boundTop.push_back(rwpk.rectRegionPacking[lowResStartIdx].packedRegTop);
          }

          for (int idx = 0; idx < packet->GetQualityNum(); idx++) {
            SourceResolution srcRes;
            srcRes.qualityRanking = static_cast<QualityRank>(idx + 1);
            srcRes.top = boundTop[idx];
            srcRes.left = boundLeft[idx];
            srcRes.height = rwpk.packedPicHeight;
            if (idx == 0) {
              srcRes.width = boundLeft[idx + 1] - boundLeft[idx];
            } else {
              srcRes.width = rwpk.packedPicWidth - boundLeft[idx];
            }
            packet->SetSourceResolution(idx, srcRes);
          }
        } else {
          packet->SetSRDInfo(segment_->GetSRDInfo());
        }

        packet->SetRealSize(packet_size);
        packet->SetSegID(track_info->sampleProperties[sample].segmentId);
        packet->SetCatchupFlag(bCatchup_);
        if (bCatchup_)
          OMAF_LOG(LOG_INFO, "[FrameSequences][CatchUp][Parse]: Generate a parsed catchup packet with PTS %lld, for track id %d\n", packet->GetPTS(), segment_->GetTrackId());
        // ANDROID_LOGD("Generate a packet with PTS %lld, for track id %d, is catch up %d", packet->GetPTS(), segment_->GetTrackId(), bCatchup_);
        std::lock_guard<std::mutex> lock(packet_mutex_);
        media_packets_.push(packet);
        OMAF_LOG(LOG_INFO, "Push packet with PTS %ld for track %d\n", packet->GetPTS(), segment_->GetTrackId());
      }
    }
    else if (segment_->GetMediaType() == MediaType_Audio) {
      auto packet_params = getPacketParamsForAudio();
      for (size_t sample = sample_begin; sample < sample_end; sample++) {
        uint32_t reader_track_id = buildReaderTrackId(segment_->GetTrackId(), segment_->GetInitSegID());

        if (packet_params.get() == nullptr) {
          packet_params = std::make_shared<OmafAudioPacketParams>();
        }
        if (!packet_params->binit_) {
          ret = packet_params->init(reader, reader_track_id, sample);
          if (ret != ERROR_NONE) {
            OMAF_LOG(LOG_ERROR, "Failed to read the packet params for audio track!\n");
            return ret;
          }

          this->setPacketParamsForAudio(packet_params);
        }

        MediaPacket *packet = new MediaPacket();
        if (packet == nullptr) {
          OMAF_LOG(LOG_ERROR, "Failed to create the packet!\n");
          return ERROR_INVALID;
        }

        uint32_t chlNum = segment_->GetAudioChlNum();
        uint32_t packet_size = 1024 * chlNum;

        packet->ReAllocatePacket(packet_size);

        ret = reader->getTrackSampleData(reader_track_id, sample, static_cast<char *>(packet->Payload()), packet_size);

        if (ret != ERROR_NONE) {
          OMAF_LOG(LOG_ERROR, "Failed to read sample data from reader for audio track, code= %d\n", ret);
          SAFE_DELETE(packet);
          return ret;
        }

        if (reader->GetSegSampleSize() == 0) {
          reader->SetSegSampleSize(track_info->sampleProperties.size);
        }
        packet->SetPTS(reader->GetSegSampleSize() * (segment_->GetSegID() - 1) + sample_begin + sample);
        if ((sample + 1) == sample_end)
        {
            packet->SetSegmentEnded(true);
        }

        packet->SetMediaType(MediaType_Audio);

        packet->SetRealSize(packet_size);

        packet_params->writeADTSHdr(packet_size);
        packet->SetADTSHdr(packet_params->params_);

        packet->SetSegID(track_info->sampleProperties[sample].segmentId);

        std::lock_guard<std::mutex> lock(packet_mutex_);
        media_packets_.push(packet);
        OMAF_LOG(LOG_INFO, "Push packet with PTS %ld for audio track %d\n", packet->GetPTS(), segment_->GetTrackId());
        OMAF_LOG(LOG_INFO, "Add packet size %d\n", packet_size);
      }
    }

    return ERROR_NONE;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when read packets! ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

std::shared_ptr<TrackInformation> OmafSegmentNode::findTrackInformation(std::shared_ptr<OmafReader> reader) noexcept {
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
        if (buildDashTrackId(track->trackId) == segment_->GetTrackId()) {
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
    OMAF_LOG(LOG_ERROR, "Exception when find the track information! ex: %s\n", ex.what());
    return nullptr;
  }
}

bool OmafSegmentNode::findSampleIndexRange(std::shared_ptr<TrackInformation> track_info, size_t &begin,
                                           size_t &end) noexcept {
  try {
    if (track_info.get() == nullptr) {
      return false;
    }
    bool found = false;
    for (size_t index = 0; index < track_info->sampleProperties.size; index++) {
      if (segment_->GetSegID() == track_info->sampleProperties[index].segmentId) {
        end++;
        if (!found) {
          found = true;
          begin = track_info->sampleProperties[index].sampleId;
        }
      }
    }
    return found;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when find the start index! ex: %s\n", ex.what());
    return false;
  }
}

bool OmafSegmentNode::isReady() const noexcept {
  try {
    if (segment_.get() == nullptr) {
      OMAF_LOG(LOG_ERROR, "The segment is empty!\n");
      return false;
    }

    if (segment_->GetState() != OmafSegment::State::OPEN_SUCCES) {
      OMAF_LOG(LOG_WARNING, "The segment is not in open success. state=%d\n", static_cast<int>(segment_->GetState()));
      return false;
    }

    if (bExtractor_) {
      if (depends_.size() != depends_size_) {
        return false;
      }
      for (auto &node : depends_) {
        if (!node->isReady()) {
          return false;
        }
      }
    }

    return true;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when check segement state. ex: %s\n", ex.what());
    return false;
  }
}

VCD_OMAF_END
