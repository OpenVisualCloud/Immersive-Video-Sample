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

// FIXME use better class name?
class OmafSegmentNode : public VCD::NonCopyable {
 public:
  using Ptr = std::shared_ptr<OmafSegmentNode>;
  using WPtr = std::weak_ptr<OmafSegmentNode>;

 public:
  OmafSegmentNode(std::shared_ptr<OmafReaderManager> mgr, OmafDashMode mode, std::shared_ptr<OmafReader> reader,
                  std::shared_ptr<OmafSegment> segment, size_t depends_size = 0, bool isExtractor = false)
      : omaf_reader_mgr_(mgr),
        mode_(mode),
        reader_(reader),
        segment_(segment),
        depends_size_(depends_size),
        bExtractor_(isExtractor) {}

  virtual ~OmafSegmentNode() {
    VLOG(VLOG_TRACE) << "Release the segment node " << to_string() << " packet size=" << media_packets_.size()
                     << std::endl;
    // relase packet
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
    if (media_packets_.size()) {
      return media_packets_.front()->GetPTS();
    }
    return 0;
  }
  void clearPacketByPTS(uint64_t pts) {
    while (media_packets_.size()) {
      auto &packet = media_packets_.front();
      if (packet->GetPTS() >= pts) {
        break;
      }
      media_packets_.pop();
    }
  }

  const std::chrono::steady_clock::time_point &startTime() const noexcept { return start_time_; };
  bool checkTimeout(int32_t ms) const noexcept {
    auto now = std::chrono::steady_clock::now();
    std::chrono::milliseconds time_span = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_);
    return time_span.count() >= ms ? true : false;
  }
  size_t dependsSize() const noexcept { return depends_size_; }
  bool isExtractor() const noexcept { return bExtractor_; }
  bool isReady() const noexcept;

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

 private:
  std::weak_ptr<OmafReaderManager> omaf_reader_mgr_;

  // dash mode
  OmafDashMode mode_ = OmafDashMode::EXTRACTOR;

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

  // OmafPacketParams::Ptr packet_params;

  const size_t depends_size_ = 0;

  const bool bExtractor_ = false;
};

uint32_t buildDashTrackId(uint32_t id) noexcept { return id & static_cast<uint32_t>(0xffff); }

uint32_t buildReaderTrackId(uint32_t trackId, uint32_t initSegId) noexcept { return (initSegId << 16) | trackId; }

OMAF_STATUS OmafReaderManager::Initialize(OmafMediaSource *pSource) noexcept {
  try {
    if (pSource == nullptr) {
      LOG(ERROR) << "Invalid media source!" << std::endl;
      return ERROR_INVALID;
    }

    media_source_ = pSource;

    reader_ = std::make_shared<OmafMP4VRReader>();
    if (reader_.get() == nullptr) {
      LOG(ERROR) << "Failed to create the omaf mp4 vr reader!" << std::endl;
      return ERROR_INVALID;
    }
    breader_working_ = true;
    segment_reader_worker_ = std::thread(&OmafReaderManager::threadRunner, this);

    return ERROR_NONE;

  } catch (const std::exception &ex) {
    LOG(ERROR) << "Failed to init the client reader, ex: " << ex.what() << std::endl;
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafReaderManager::Close() noexcept {
  try {
    LOG(INFO) << "Close the omaf reader manager!" << std::endl;
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
    LOG(ERROR) << "Failed to close the client reader, ex: " << ex.what() << std::endl;
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafReaderManager::OpenInitSegment(std::shared_ptr<OmafSegment> pInitSeg) noexcept {
  try {
    if (pInitSeg.get() == nullptr) {
      LOG(ERROR) << "The init segment is empty!" << std::endl;
      return ERROR_INVALID;
    }

    pInitSeg->RegisterStateChange([this](std::shared_ptr<OmafSegment> pInitSeg, OmafSegment::State state) {
      this->initSegmentStateChange(std::move(pInitSeg), state);
    });
    OMAF_STATUS ret = pInitSeg->Open(dash_client_);
    if (ret != ERROR_NONE) {
      LOG(ERROR) << "Failed to open the init segment, code= " << ret << std::endl;
      return ret;
    }

    return ERROR_NONE;
  } catch (const std::exception &ex) {
    LOG(ERROR) << "Failed to add the init segment, ex: " << ex.what() << std::endl;
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafReaderManager::OpenLocalInitSegment(std::shared_ptr<OmafSegment> pInitSeg) noexcept {
  try {
    if (pInitSeg.get() == nullptr) {
      LOG(ERROR) << "The init segment is empty!" << std::endl;
      return ERROR_INVALID;
    }
    pInitSeg->SetState(OmafSegment::State::OPEN_SUCCES);
    this->initSegmentStateChange(std::move(pInitSeg), OmafSegment::State::OPEN_SUCCES);

    return ERROR_NONE;
  } catch (const std::exception &ex) {
    LOG(ERROR) << "Failed to add the local init segment, ex: " << ex.what() << std::endl;
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafReaderManager::OpenSegment(std::shared_ptr<OmafSegment> pSeg, bool isExtracotr) noexcept {
  try {
    size_t depends_size = 0;
    auto d_it = initSegId_depends_map_.find(pSeg->GetInitSegID());
    if (d_it != initSegId_depends_map_.end()) {
      depends_size = d_it->second.size();
    }

    pSeg->RegisterStateChange([this](std::shared_ptr<OmafSegment> segment, OmafSegment::State state) {
      this->normalSegmentStateChange(std::move(segment), state);
    });

    OmafSegmentNode::Ptr new_node = std::make_shared<OmafSegmentNode>(shared_from_this(), work_params_.mode_, reader_,
                                                                      std::move(pSeg), depends_size, isExtracotr);
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
      LOG(ERROR) << "Failed to open the segment, code= " << ret << std::endl;
      return ret;
    }
    return ERROR_NONE;
  } catch (const std::exception &ex) {
    LOG(ERROR) << "Failed to add segment, ex: " << ex.what() << std::endl;
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

    OmafSegmentNode::Ptr new_node = std::make_shared<OmafSegmentNode>(shared_from_this(), work_params_.mode_, reader_,
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
    LOG(ERROR) << "Failed to add segment, ex: " << ex.what() << std::endl;
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafReaderManager::GetNextPacket(uint32_t trackID, MediaPacket *&pPacket, bool requireParams) noexcept {
  try {
    OMAF_STATUS ret = ERROR_NONE;

    bool bpacket_readed = false;

    // while (!bpacket_readed) {
    {
      std::unique_lock<std::mutex> lock(segment_parsed_mutex_);

      // 1. read the required packet
      for (auto &nodeset : segment_parsed_list_) {
        std::list<OmafSegmentNode::Ptr>::iterator it = nodeset.segment_nodes_.begin();
        while (it != nodeset.segment_nodes_.end()) {
          auto &node = *it;
          VLOG(VLOG_TRACE) << "Require trackid=" << trackID << ", node trackid=" << node->getTrackId() << std::endl;
          if (node->getTrackId() == trackID) {
            ret = node->getPacket(pPacket, requireParams);
            if (ret == ERROR_NONE) {
              bpacket_readed = true;
            }
            timeline_point_ = node->getTimelinePoint();
            if (0 == node->packetQueueSize()) {
              VLOG(VLOG_TRACE) << "Node count=" << node.use_count() << "." << node->to_string() << std::endl;
              nodeset.segment_nodes_.erase(it);
            }

            break;
          }
          it++;
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
    //  if (!bpacket_readed) {
    //    LOG(INFO) << "Wait dash segment parsed!" << std::endl;
    //    segment_parsed_cv_.wait(lock);
    //   }
    //  }  // end while

    // 2. sync timeline point for outside reading
    // drop all dashset whose timeline point is less than timeline point
    if (timeline_point_ != -1) {
      VLOG(VLOG_TRACE) << "To clear the timeline point <" << timeline_point_ << std::endl;
      std::lock_guard<std::mutex> lock(segment_parsed_mutex_);
      std::list<OmafSegmentNodeTimedSet>::iterator it = segment_parsed_list_.begin();
      while (it != segment_parsed_list_.end() && it->timeline_point_ < timeline_point_) {
        it = segment_parsed_list_.erase(it);  // 'it' will move to next when calling erase
      }
    }
    return ret;
  } catch (const std::exception &ex) {
    LOG(ERROR) << "Failed to read frame for trackid=" << trackID << ", ex: " << ex.what() << std::endl;
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
    LOG(ERROR) << "Failed to check the empty"
               << ", ex: " << ex.what() << std::endl;
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
        LOG(INFO) << "segment opening list is not empty!" << std::endl;
        eos = false;
      }
    }
    if (eos) {
      if (!isEmpty(segment_opened_mutex_, segment_opened_list_)) {
        LOG(INFO) << "segment opened list is not empty!" << std::endl;
        eos = false;
      }
    }
    if (eos) {
      if (!isEmpty(segment_parsed_mutex_, segment_parsed_list_)) {
        LOG(INFO) << "segment parsed list is not empty!" << std::endl;
        eos = false;
      }
    }
    if (eos) {
      LOG(INFO) << "Meet EOS" << std::endl;
    }
    return eos;
  } catch (const std::exception &ex) {
    LOG(ERROR) << "Failed to check the eos"
               << ", ex: " << ex.what() << std::endl;
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
    LOG(ERROR) << "Failed to read packet size for trackid=" << trackID << ", ex: " << ex.what() << std::endl;
    return ERROR_INVALID;
  }
}
uint64_t OmafReaderManager::GetOldestPacketPTSForTrack(int trackId) {
  try {
    std::unique_lock<std::mutex> lock(segment_parsed_mutex_);
    for (auto &nodeset : segment_parsed_list_) {
      for (auto &node : nodeset.segment_nodes_) {
        if (node->getTrackId() == static_cast<uint32_t>(trackId)) {
          return node->getPTS();
        }
      }
    }

    return 0;
  } catch (const std::exception &ex) {
    LOG(ERROR) << "Failed to read packet size for trackid=" << trackId << ", ex: " << ex.what() << std::endl;
    return 0;
  }
}
void OmafReaderManager::RemoveOutdatedPacketForTrack(int trackId, uint64_t currPTS) {
  try {
    std::unique_lock<std::mutex> lock(segment_parsed_mutex_);
    for (auto &nodeset : segment_parsed_list_) {
      for (auto &node : nodeset.segment_nodes_) {
        if (node->getTrackId() == static_cast<uint32_t>(trackId)) {
          node->clearPacketByPTS(currPTS);
        }
      }
    }
  } catch (const std::exception &ex) {
    LOG(ERROR) << "Failed to read packet size for trackid=" << trackId << ", ex: " << ex.what() << std::endl;
  }
}

void OmafReaderManager::initSegmentStateChange(std::shared_ptr<OmafSegment> pInitSeg,
                                               OmafSegment::State state) noexcept {
  try {
    if (state != OmafSegment::State::OPEN_SUCCES) {
      LOG(FATAL) << "Failed to open the init segment, state= " << static_cast<int>(state) << std::endl;
      return;
    }

    // 1. parse the segment
    OMAF_STATUS ret = reader_->parseInitializationSegment(pInitSeg.get(), pInitSeg->GetInitSegID());
    if (ret != ERROR_NONE) {
      LOG(FATAL) << "parse initialization segment failed! code= " << ret << std::endl;
      return;
    }

    initSeg_ready_count_++;

    // 2 get the track information when all init segment parsed
    if (initSegParsedCount() == media_source_->GetTrackCount() && !IsInitSegmentsParsed()) {
      buildInitSegmentInfo();
    }
  } catch (const std::exception &ex) {
    LOG(ERROR) << "Failed to parse the init segment, ex: " << ex.what() << std::endl;
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
        LOG(ERROR) << "Meet empty track!" << std::endl;
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
            LOG(INFO) << "Initse id=" << track->initSegmentId << ",trackid=" << dash_track_id << std::endl;
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
            LOG(INFO) << "Initse id=" << track->initSegmentId << ",trackid=" << dash_track_id << std::endl;
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
    LOG(ERROR) << "Failed to parse the init segment, ex: " << ex.what() << std::endl;
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
    LOG(ERROR) << "Failed to set up track map, ex: " << ex.what() << std::endl;
  }
}

void OmafReaderManager::normalSegmentStateChange(std::shared_ptr<OmafSegment> segment,
                                                 OmafSegment::State state) noexcept {
  try {
    // 0. check params
    if (segment.get() == nullptr) {
      LOG(ERROR) << "Empty segment!" << std::endl;
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
      LOG(WARNING) << "Can't find the dash node for coming segment!" << std::endl;
      return;
    }

    // 2. append to the dash opened list
    {
      std::lock_guard<std::mutex> lock(segment_opened_mutex_);
      bool new_timeline_point = true;
      if (work_params_.mode_ == OmafDashMode::EXTRACTOR) {
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
            LOG(WARNING) << "Try to insert the timeline:" << opened_dash_node->getTimelinePoint()
                         << ", which <=" << tail_nodeset.timeline_point_ << std::endl;
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
    LOG(ERROR) << "Exception when set up track map, ex: " << ex.what() << std::endl;
  }
}

void OmafReaderManager::threadRunner() noexcept {
  try {
    LOG(INFO) << "Start the reader runner!" << std::endl;

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
      LOG(INFO) << "Get ready segment! timeline=" << timeline_point << std::endl;
      OMAF_STATUS ret = ready_dash_node->parse();

      // 3. move the parsed segment/dash_node to parsed list
      if (ret == ERROR_NONE) {
        LOG(INFO) << "Success to parsed dash segment! timeline=" << timeline_point << std::endl;
        std::unique_lock<std::mutex> lock(segment_parsed_mutex_);
        bool new_timeline_point = true;
        for (auto &nodeset : segment_parsed_list_) {
          if (nodeset.timeline_point_ == timeline_point) {
            nodeset.segment_nodes_.push_back(std::move(ready_dash_node));
            new_timeline_point = false;
            break;
          }
        }
        if (new_timeline_point) {
          OmafSegmentNodeTimedSet nodeset;
          nodeset.timeline_point_ = timeline_point;
          nodeset.create_time_ = std::chrono::steady_clock::now();
          nodeset.segment_nodes_.push_back(std::move(ready_dash_node));
          segment_parsed_list_.emplace_back(nodeset);
        }
        segment_parsed_cv_.notify_all();
      } else {
        LOG(ERROR) << "Failed to parse " << ready_dash_node->to_string() << std::endl;
      }

      // 4. clear dash set whose timeline point older than current ready segment/dash_node
      // we use simple logic to main the dash node sets
      // we will remove older dash nodes
      clearOlderSegmentSet(timeline_point);
    }
  } catch (const std::exception &ex) {
    LOG(ERROR) << "Exception in reader runner, ex: " << ex.what() << std::endl;
  }

  LOG(INFO) << "Exit from the reader runner!" << std::endl;
}

OmafSegmentNode::Ptr OmafReaderManager::findReadySegmentNode() noexcept {
  try {
    OmafSegmentNode::Ptr ready_dash_node;
    std::unique_lock<std::mutex> lock(segment_opened_mutex_);
    for (auto &nodeset : segment_opened_list_) {
      VLOG(VLOG_TRACE) << "To find the ready node set timeline=" << nodeset.timeline_point_ << std::endl;

      // 1.1.1 try to find the ready node
      std::list<OmafSegmentNode::Ptr>::iterator it = nodeset.segment_nodes_.begin();
      while (it != nodeset.segment_nodes_.end()) {
        auto &node = *it;
        if (node->isReady()) {
          if (work_params_.mode_ == OmafDashMode::EXTRACTOR) {
            if (node->isExtractor()) {
              ready_dash_node = std::move(node);
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
        break;
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
    LOG(ERROR) << "Exception when find the ready dash node, ex: " << ex.what() << std::endl;
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
        LOG(INFO) << "Removing older dash opening list, timeline=" << it->timeline_point_ << std::endl;
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
        LOG(INFO) << "Removing older dash opened list, timeline=" << it->timeline_point_ << std::endl;
        it = segment_opened_list_.erase(it);
      }
    }
  } catch (const std::exception &ex) {
    LOG(ERROR) << "Exception when clear the older dash set, whose timeline is older than" << timeline_point
               << ", ex: " << ex.what() << std::endl;
  }
}

// FIXME, dims and vps/sps/pps may not in the same sample
int OmafPacketParams::init(std::shared_ptr<OmafReader> reader, uint32_t reader_trackId, uint32_t sampleId) noexcept {
  try {
    // 1. read width and heigh
    OMAF_STATUS ret = reader->getDims(reader_trackId, sampleId, width_, height_);
    if (ret) {
      LOG(ERROR) << "Failed to get sample dims !" << endl;
      return ret;
    }
    if (!width_ || !height_) {
      LOG(ERROR) << "Failed to get the dims!" << std::endl;
      return OMAF_ERROR_INVALID_DATA;
    }

    // 2. read vps/sps/pps params
    std::vector<VCD::OMAF::DecoderSpecificInfo> parameterSets;
    ret = reader->getDecoderConfiguration(reader_trackId, sampleId, parameterSets);
    if (ret) {
      LOG(ERROR) << "Failed to get VPS/SPS/PPS ! " << endl;
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
    LOG(ERROR) << "Failed to init the packet params, ex: " << ex.what() << std::endl;
    binit_ = false;
    return ERROR_INVALID;
  }
}

int OmafSegmentNode::parse() noexcept {
  try {
    auto reader = reader_.lock();
    if (reader.get() == nullptr) {
      LOG(ERROR) << "The omaf reader is empty!" << std::endl;
      return ERROR_NULL_PTR;
    }

    OMAF_STATUS ret = ERROR_NONE;
    // 1.1 calling depends to parse the segment
    for (auto &node : depends_) {
      ret = node->parseSegmentStream(reader);
      if (ERROR_NONE != ret) {
        LOG(ERROR) << "Failed to parse the dependent node " << node->to_string() << ". Error code=" << ret << std::endl;
        return ret;
      }
    }

    // 1.2 parse self
    ret = parseSegmentStream(reader);
    if (ERROR_NONE != ret) {
      LOG(ERROR) << "Failed to parse " << this->to_string() << ". Error code=" << ret << std::endl;
      return ret;
    }

    // 2 cache packets from the reader
    ret = cachePackets(reader);
    if (ERROR_NONE != ret) {
      LOG(ERROR) << "Failed to read packet from " << this->to_string() << ". Error code=" << ret << std::endl;
      return ERROR_INVALID;
    }

    // 3.1 remove self segment from reader
    ret = removeSegmentStream(reader);
    if (ERROR_NONE != ret) {
      LOG(ERROR) << "Failed to remove segment from reader. " << this->to_string() << ". Error code=" << ret
                 << std::endl;
      return ERROR_INVALID;
    }

    // 3.2 remove segment of depends from reader
    for (auto &node : depends_) {
      ret = node->removeSegmentStream(reader);
      if (ERROR_NONE != ret) {
        LOG(ERROR) << "Failed to remove dependent segment form reader. " << node->to_string() << ". Rrror code=" << ret
                   << std::endl;
        return ret;
      }
    }
    return ERROR_NONE;
  } catch (const std::exception &ex) {
    LOG(ERROR) << "Exception when parse the segment! ex: " << ex.what() << std::endl;
    return ERROR_INVALID;
  }
}

int OmafSegmentNode::start(void) noexcept {
  try {
    OMAF_STATUS ret = ERROR_NONE;
    if (segment_.get() == nullptr) {
      LOG(ERROR) << "Try to open the empty segment!" << std::endl;
      return ERROR_INVALID;
    }

    auto reader_mgr = omaf_reader_mgr_.lock();
    if (reader_mgr.get() == nullptr) {
      LOG(ERROR) << "The reader manager is empty!" << std::endl;
      return ERROR_NULL_PTR;
    }

    start_time_ = std::chrono::steady_clock::now();
    ret = segment_->Open(reader_mgr->dash_client_);
    if (ret != ERROR_NONE) {
      LOG(ERROR) << "Failed to open the segment!" << std::endl;
      return ret;
    }

    return ERROR_NONE;
  } catch (const std::exception &ex) {
    LOG(ERROR) << "Exception when open the segment, ex: " << ex.what() << std::endl;
    return ERROR_INVALID;
  }
}

int OmafSegmentNode::stop(void) noexcept {
  try {
    OMAF_STATUS ret = ERROR_NONE;
    if (segment_.get() == nullptr) {
      LOG(ERROR) << "Try to stop the empty segment!" << std::endl;
      return ERROR_INVALID;
    }
    ret = this->segment_->Stop();
    if (ret != ERROR_NONE) {
      LOG(WARNING) << "Failed to stop the segment!" << std::endl;
    }

    for (auto &node : depends_) {
      node->stop();
    }
    return ret;
  } catch (const std::exception &ex) {
    LOG(ERROR) << "Exception when stop the segment, ex: " << ex.what() << std::endl;
    return ERROR_INVALID;
  }
}

int OmafSegmentNode::parseSegmentStream(std::shared_ptr<OmafReader> reader) noexcept {
  try {
    return reader->parseSegment(segment_.get(), segment_->GetInitSegID(), segment_->GetSegID());
  } catch (const std::exception &ex) {
    LOG(ERROR) << "Exception when parse the segment! ex: " << ex.what() << std::endl;
    return ERROR_INVALID;
  }
}

int OmafSegmentNode::removeSegmentStream(std::shared_ptr<OmafReader> reader) noexcept {
  try {
    return reader->invalidateSegment(segment_->GetInitSegID(), segment_->GetSegID());

  } catch (const std::exception &ex) {
    LOG(ERROR) << "Exception when remove the segment from reader! ex: " << ex.what() << std::endl;
    return ERROR_INVALID;
  }
}

// int OmafSegmentNode::getPacket(std::unique_ptr<MediaPacket> &pPacket, bool needParams) {
int OmafSegmentNode::getPacket(MediaPacket *&pPacket, bool requireParams) noexcept {
  try {
    if (media_packets_.size() <= 0) {
      LOG(ERROR) << "There is no packets" << std::endl;
      return ERROR_NULL_PACKET;
    }

    pPacket = media_packets_.front();
    media_packets_.pop();
    if (requireParams) {
      auto packet_params = getPacketParams();
      if (packet_params.get() == nullptr || !packet_params->binit_) {
        LOG(ERROR) << "Invalid VPS/SPS/PPS in getting packet ! " << std::endl;
        return OMAF_ERROR_INVALID_DATA;
      }
      pPacket->InsertParams(packet_params->params_);
      pPacket->SetVPSLen(packet_params->vps_.size());
      pPacket->SetSPSLen(packet_params->sps_.size());
      pPacket->SetPPSLen(packet_params->pps_.size());
      pPacket->SetVideoHeaderSize(packet_params->params_.size());
    }
    return ERROR_NONE;
  } catch (const std::exception &ex) {
    LOG(ERROR) << "Exception when read the frame! ex: " << ex.what() << std::endl;
    return ERROR_INVALID;
  }
}

int OmafSegmentNode::cachePackets(std::shared_ptr<OmafReader> reader) noexcept {
  try {
    OMAF_STATUS ret = ERROR_NONE;
    std::shared_ptr<TrackInformation> track_info = findTrackInformation(reader);
    if (track_info.get() == nullptr) {
      LOG(FATAL) << "Failed to find the sepcial track information." << this->to_string() << std::endl;
      return ERROR_INVALID;
    }

    size_t sample_begin = 0;
    size_t sample_end = 0;
    if (!findSampleIndexRange(track_info, sample_begin, sample_end)) {
      LOG(ERROR) << "Failed to find the sample range for segment. " << this->to_string() << std::endl;
      return ERROR_INVALID;
    }
#if 0
    if (sample_begin < 1) {
      LOG(FATAL) << "The begin sample id is less than 1, whose value =" << sample_begin << "." << this->to_string()
                 << std::endl;
      return ERROR_INVALID;
    }
#endif
    auto packet_params = getPacketParams();
    for (size_t sample = sample_begin; sample < sample_end; sample++) {
      uint32_t reader_track_id = buildReaderTrackId(segment_->GetTrackId(), segment_->GetInitSegID());

      if (packet_params.get() == nullptr) {
        packet_params = std::make_shared<OmafPacketParams>();
      }
      if (!packet_params->binit_) {
        ret = packet_params->init(reader, reader_track_id, sample);
        if (ret != ERROR_NONE) {
          LOG(ERROR) << "Failed to read the packet params include width/height/vps/sps/pps!" << std::endl;
          return ret;
        }
        this->setPacketParams(packet_params);
      }

      // cache packets
      // std::shared_ptr<MediaPacket> packet = make_unique_vcd<MediaPacket>;
      MediaPacket *packet = new MediaPacket();
      if (packet == nullptr) {
        LOG(ERROR) << "Failed to create the packet!" << std::endl;
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
        LOG(ERROR) << "Failed to read sample data from reader, code= " << ret << std::endl;
        SAFE_DELETE(packet);
        return ret;
      }

      std::unique_ptr<RegionWisePacking> pRwpk = make_unique_vcd<RegionWisePacking>();
      ret = reader->getPropertyRegionWisePacking(reader_track_id, sample, pRwpk.get());
      if (ret != ERROR_NONE) {
        LOG(ERROR) << "Failed to read region wise packing data from reader, code= " << ret << std::endl;
        return ret;
      }
      packet->SetRwpk(std::move(pRwpk));
      packet->SetPTS(this->getTimelinePoint());  // FIXME, to compute pts

      // for later binding
      packet->SetQualityRanking(segment_->GetQualityRanking());

      if (mode_ == OmafDashMode::EXTRACTOR) {
        packet->SetQualityNum(MAX_QUALITY_NUM);
        vector<uint32_t> boundLeft(1);  // num of quality is limited to 2.
        vector<uint32_t> boundTop(1);
        const RegionWisePacking &rwpk = packet->GetRwpk();
        for (int j = rwpk.numRegions - 1; j >= 0; j--) {
          if (rwpk.rectRegionPacking[j].projRegLeft == 0 && rwpk.rectRegionPacking[j].projRegTop == 0 &&
              !(rwpk.rectRegionPacking[j].packedRegLeft == 0 && rwpk.rectRegionPacking[j].packedRegTop == 0)) {
            boundLeft.push_back(rwpk.rectRegionPacking[j].packedRegLeft);
            boundTop.push_back(rwpk.rectRegionPacking[j].packedRegTop);
            break;
          }
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
          // cout<<"sourceRes:"<<srcRes.qualityRanking<<" "<<srcRes.top<<" "<<srcRes.left<<" "<<srcRes.width<<"
          // "<<srcRes.height<<endl;
        }
      } else {
        packet->SetSRDInfo(segment_->GetSRDInfo());
      }

      packet->SetRealSize(packet_size);
      VLOG(VLOG_TRACE) << "Sample id=" << sample << std::endl;
      // packet->SetSegID(track_info->sampleProperties[sample - 1].segmentId);
      packet->SetSegID(track_info->sampleProperties[sample].segmentId);
      // media_packets_.push_back(std::move(packet));
      media_packets_.push(packet);
    }
    return ERROR_NONE;
  } catch (const std::exception &ex) {
    LOG(ERROR) << "Exception when read packets! ex: " << ex.what() << std::endl;
    return ERROR_INVALID;
  }
}

std::shared_ptr<TrackInformation> OmafSegmentNode::findTrackInformation(std::shared_ptr<OmafReader> reader) noexcept {
  try {
    std::vector<TrackInformation *> track_infos;

    OMAF_STATUS ret = reader->getTrackInformations(track_infos);
    if (ERROR_NONE != ret) {
      LOG(ERROR) << "Failed to get the trackinformation list from reader, code=" << ret << std::endl;
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
    LOG(ERROR) << "Exception when find the track information! ex: " << ex.what() << std::endl;
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
    LOG(ERROR) << "Exception when find the start index! ex: " << ex.what() << std::endl;
    return false;
  }
}

bool OmafSegmentNode::isReady() const noexcept {
  try {
    if (segment_.get() == nullptr) {
      LOG(ERROR) << "The segment is empty!" << std::endl;
      return false;
    }

    if (segment_->GetState() != OmafSegment::State::OPEN_SUCCES) {
      LOG(WARNING) << "The segment is not in open success. state=" << static_cast<int>(segment_->GetState())
                   << std::endl;
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
    LOG(ERROR) << "Exception when check segement state. ex: " << ex.what() << std::endl;
    return false;
  }
}

VCD_OMAF_END