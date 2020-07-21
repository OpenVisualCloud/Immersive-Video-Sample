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
//! \file:   OmafDashRangeSync.h
//! \brief:
//! \detail:
//! Created on Jun 4, 2020, 3:18 PM
//!

#ifndef OMAFDASHSOURCESYNC_H
#define OMAFDASHSOURCESYNC_H

#include "OmafDashParser/Common.h"
#include "general.h"
#include "OmafDashDownload/OmafCurlEasyHandler.h"

#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace VCD {
namespace OMAF {

enum class SegmentTemplateType { SEGTEMPLATE_NUMBER = 0, SEGTEMPLATE_TIME = 1 };
struct _segmentSyncNode {
  SegmentTemplateType type_ = SegmentTemplateType::SEGTEMPLATE_NUMBER;
  int32_t bandwith_ = 0;
  union _segment_value {
    int64_t number_;
    int64_t time_;
  } segment_value;
};

struct _syncRange {
  int64_t left_;
  int64_t right_;
};

using SegmentSyncNode = struct _segmentSyncNode;
using SyncRange = struct _syncRange;
using SegmentSyncNodeCB = std::function<void(SegmentSyncNode)>;

class OmafDashRangeSync : public std::enable_shared_from_this<OmafDashRangeSync> {
 public:
  using Ptr = std::shared_ptr<OmafDashRangeSync>;

 public:
  OmafDashRangeSync(){};
  OmafDashRangeSync(OmafDashRangeSync &&) = default;
  OmafDashRangeSync(const OmafDashRangeSync &) = default;
  OmafDashRangeSync &operator=(OmafDashRangeSync &&) = default;
  OmafDashRangeSync &operator=(const OmafDashRangeSync &) = default;
  virtual ~OmafDashRangeSync(){};

 public:
  virtual std::string getUrl(const SegmentSyncNode &value) const = 0;
  virtual SegmentSyncNode getSegmentNode() = 0;
  virtual int64_t getStartSegment() = 0;
  virtual void notifyRangeChange(SyncRange range) = 0;
};

class OmafAdaptationSet;

OmafDashRangeSync::Ptr make_omaf_syncer(const OmafAdaptationSet &, SegmentSyncNodeCB);

class OmafDashSourceSyncHelper : public VCD::NonCopyable {
 public:
  OmafDashSourceSyncHelper(){};

  virtual ~OmafDashSourceSyncHelper() { stop(); };

 private:
  enum class Direction {
    LEFT = 0,
    RIGHT = 1,
  };

 public:
  int start(CurlParams params) noexcept;
  int stop() noexcept;

 public:
  void addSyncer(OmafDashRangeSync::Ptr) noexcept;
  void setWindowSize(int s) noexcept {
    if (s > 0) range_size_ = s - 1;
  }
  void setSyncFrequency(int ms) noexcept { sync_frequency_ = ms; };

 private:
  void threadRunner() noexcept;
  bool initRange(OmafDashRangeSync::Ptr, std::shared_ptr<SyncRange>) noexcept;
  bool findRange(OmafDashRangeSync::Ptr, int64_t check_start, Direction direction, int64_t &point) noexcept;
  bool findRangeEdge(OmafDashRangeSync::Ptr, int64_t point, std::shared_ptr<SyncRange> range) noexcept;
  bool updateRange(OmafDashRangeSync::Ptr, std::shared_ptr<SyncRange> range) noexcept;

 private:
  std::shared_ptr<OmafCurlChecker> checker_;
  int64_t sync_frequency_ = 1000;
  int32_t range_size_ = 19;
  int32_t check_range_strides_ = 10;
  int32_t check_range_times_ = 1000;
  // std::mutex syncers_mutex_;
  // not use weak_ptr to simple the logic
  std::vector<OmafDashRangeSync::Ptr> syncers_;
  std::vector<std::shared_ptr<SyncRange>> syncers_range_;

  std::thread sync_worker_;
  bool bsyncing_ = false;

  const int64_t NOT_FOUND = -1;
  const int64_t MEET_LEFT = -2;
  const int64_t EXCEPTION = -3;
};
}  // namespace OMAF
}  // namespace VCD

#endif  // OMAFDASHSOURCESYNC_H