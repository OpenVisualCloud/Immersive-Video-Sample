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
//! \file:   OmafSegment.h
//! \brief:
//! \detail:
//! Created on May 24, 2019, 11:07 AM
//!

#ifndef OMAFSEGMENT_H
#define OMAFSEGMENT_H

#include "OmafDashParser/Common.h"
#include "OmafDashDownload/Stream.h"
#include "OmafDashDownload/OmafDownloader.h"
#include "../isolib/dash_parser/Mp4StreamIO.h"
#include "general.h"
#include "iso_structure.h"

#include <memory>
#include <atomic>
#include <fstream>

VCD_OMAF_BEGIN

class OmafSegment : public VCD::NonCopyable, public VCD::MP4::StreamIO, public enable_shared_from_this<OmafSegment> {
 public:
  using Ptr = std::shared_ptr<OmafSegment>;

  //
  // @enum State
  // @brief segment state
  //
  enum class State {
    CREATE = 0,
    OPEN = 1,
    OPEN_SUCCES = 2,
    OPEN_STOPPED = 3,

    OPEN_TIMEOUT = 4,
    OPEN_FAILED = 5,
    PARSE_OK = 6,
    PARSE_FAILED = 7,
    DESTROYED = 8
  };

  //
  // @method
  // @brief segment state change callback
  // @brief segment state
  //
  using OnStateChange = std::function<void(OmafSegment::Ptr, State)>;

 public:
  //
  // @brief constructor with dash source
  //
  OmafSegment(DashSegmentSourceParams ds_params, int segCnt, bool bInitSegment = false);

  //!
  //! \brief  de-construct
  //!
  virtual ~OmafSegment();

 public:
  offset_t ReadStream(char* buffer, offset_t size) override {
    if (!buse_stored_file_) {
      return dash_stream_.ReadStream(buffer, size);
    } else {
      if (!mFileStream.is_open()) {
        mFileStream.open(this->GetSegmentCacheFile().c_str(), ios_base::binary | ios_base::in);
      }
      mFileStream.read(buffer, size);
      std::streamsize readCnt = mFileStream.gcount();
      return (offset_t)readCnt;
    }
  };

  bool SeekAbsoluteOffset(offset_t offset) override {
    if (!buse_stored_file_) {
      return dash_stream_.SeekAbsoluteOffset(offset);
    } else {
      if (!mFileStream.is_open()) {
        mFileStream.open(this->GetSegmentCacheFile().c_str(), ios_base::binary | ios_base::in);
      }
      if (mFileStream.tellg() == -1) {
        mFileStream.clear();
        mFileStream.seekg(0, ios_base::beg);
      }

      mFileStream.seekg(offset);
      return true;
    }
  }

  offset_t TellOffset() override {
    if (!buse_stored_file_) {
      return dash_stream_.TellOffset();
    } else {
      if (!mFileStream.is_open()) {
        mFileStream.open(this->GetSegmentCacheFile().c_str(), ios_base::binary | ios_base::in);
      }
      return mFileStream.tellg();
    }
  };

  offset_t GetStreamSize() override {
    if (!buse_stored_file_) {
      return dash_stream_.GetStreamSize();
    } else {
      if (!mFileStream.is_open()) {
        mFileStream.open(this->GetSegmentCacheFile().c_str(), ios_base::binary | ios_base::in);
      }
      mFileStream.seekg(0, ios_base::end);
      int64_t size = mFileStream.tellg();
      mFileStream.seekg(0, ios_base::beg);
      return size;
    }
  };

 public:
  //
  // @brief register state change callback
  //
  // @param[in] cb
  // @brief state change callback
  //
  // @return void
  // @brief
  inline void RegisterStateChange(OnStateChange cb) noexcept { state_change_cb_ = cb; }

  //
  // @brief get dash segment cache file path
  //
  // @return std::string
  // @brief segment cache file path
  inline std::string GetSegmentCacheFile() const noexcept { return cache_file_; };

  //
  // @brief set segment cache file path
  //
  // @param[in] cacheFileName
  // @brief cache file path
  //
  // @return void
  // @brief
  inline void SetSegmentCacheFile(std::string cacheFileName) noexcept { cache_file_ = cacheFileName; };

  //
  // @brief get segment state
  //
  // @return State
  // @brief segment state
  inline State GetState() const noexcept { return state_; }
  inline void SetState(State s) noexcept { state_ = s; }

  //
  // @brief check is init segment
  //
  // @return bool
  // @brief yes or no
  inline bool IsInitSegment() const noexcept { return bInit_segment_; }

  //
  // @brief set init segement or not
  //
  // @param[in] bInit
  // @brief yes or no
  //
  // @return void
  // @brief
  inline void SetInitSegment(bool bInit) noexcept { bInit_segment_ = bInit; };

  //
  // @brief open this segment, which will trigger the dash download
  //
  // @param[in] ds
  // @brief dash source params
  //
  // @param[in] cb
  // @brief dash open state change callback
  //
  // @return int
  // @brief calling success or not
  int Open(std::shared_ptr<OmafDashSegmentClient> dash_client) noexcept;
  int Stop() noexcept;
  // int Read(uint8_t* data, size_t len);
  // int Peek(uint8_t* data, size_t len);
  // int Peek(uint8_t* data, size_t len, size_t offset);

  // int Close();

  void SetSegID(uint32_t id) noexcept { seg_id_ = id; };
  uint32_t GetSegID() noexcept { return seg_id_; };
  void SetInitSegID(uint32_t id) noexcept { initSeg_id_ = id; };
  uint32_t GetInitSegID() const noexcept { return initSeg_id_; };
  void SetTrackId(uint32_t id) noexcept { track_id_ = id; };
  uint32_t GetTrackId() const noexcept { return track_id_; };
  void SetSegStored() noexcept { buse_stored_file_ = true; };
  int GetSegCount() const noexcept { return seg_count_; };
  void SetSegSize(uint64_t segSize) noexcept { seg_size_ = segSize; };
  uint64_t GetSegSize() const noexcept { return seg_size_; };

  int64_t GetTimelinePoint(void) { return ds_params_.timeline_point_; }
  void SetQualityRanking(QualityRank qualityRanking) { mQualityRanking = qualityRanking; };
  QualityRank GetQualityRanking() { return mQualityRanking; };

  void SetSRDInfo(const SRDInfo& srdInfo) { mSRDInfo = srdInfo; }

  SRDInfo GetSRDInfo() { return mSRDInfo; };

  std::string to_string() const noexcept;

  void SetMediaType(MediaType type) { mMediaType = type; };

  MediaType GetMediaType() { return mMediaType; };

  void SetAudioChlNum(uint32_t chlNum) { mChlsNum = chlNum; };

  uint32_t GetAudioChlNum() { return mChlsNum; };

  void SetAudioSampleRate(uint32_t sampleRate) { mSampleRate = sampleRate; };

  uint32_t GetAudioSampleRate() { return mSampleRate; };

 private:
  //!
  //!  \brief save the memory data to file.
  //!
  int CacheToFile() noexcept;

 private:
  std::shared_ptr<OmafDashSegmentClient> dash_client_;
  DashSegmentSourceParams ds_params_;

  StreamBlocks dash_stream_;

  // SegmentElement* mSegElement;  //<! SegmentElement
  //<! flag to indicate whether the segment should be stored
  // in disk
  bool buse_stored_file_ = false;
  //<! the file name for downloaded segment file
  std::string cache_file_;
  //<! status of the segment

  OnStateChange state_change_cb_;
  State state_ = State::CREATE;

  //<! the total size of data downloaded for this segment
  uint64_t seg_size_ = 0;
  //<! the Segment ID used for segment reading
  uint32_t seg_id_ = 0;
  //<! the init Segement ID relative to this segment
  uint32_t initSeg_id_ = 0;
  uint32_t track_id_ = 0;

  //<! the count for this segment
  int seg_count_ = 0;
  //<! flag to indicate whether this segment is initialize
  // MP4
  bool bInit_segment_ = false;
  QualityRank mQualityRanking;  //<! quality ranking of the segment
  SRDInfo mSRDInfo;             //<! top/left/width/height info for the tile track segment

  std::ifstream mFileStream;

  MediaType mMediaType;

  uint32_t mChlsNum;
  uint32_t mSampleRate;

 private:
  static std::atomic_uint32_t INITSEG_ID;
};

VCD_OMAF_END

#endif /* MEDIASEGMENT_H */
