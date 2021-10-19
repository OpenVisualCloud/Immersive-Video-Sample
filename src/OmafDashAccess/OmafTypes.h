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
//! \file:   OmafTypes.h
//! \brief:
//! \detail:
//! Created on May 20, 2020, 10:07 AM
//!
#ifndef OMAF_TYPES_H
#define OMAF_TYPES_H

#include <sstream>
#include <string>
#include "../utils/safe_mem.h"

namespace VCD {
namespace OMAF {

const long DEFAULT_MAX_PARALLEL_TRANSFERS = 20;
const int32_t DEFAULT_SEGMENT_OPEN_TIMEOUT = 3000;

class OmafDashHttpProxy {
 public:
  std::string http_proxy_;
  std::string https_proxy_;
  std::string no_proxy_;
  std::string proxy_user_;
  std::string proxy_passwd_;
  std::string to_string() {
    std::stringstream ss;
    ss << "http proxy: {" << std::endl;
    ss << "\thttp proxy: " << http_proxy_ << ", " << std::endl;
    ss << "\thttps proxy: " << https_proxy_ << ", " << std::endl;
    ss << "\tno proxy: " << no_proxy_ << ", " << std::endl;
    ss << "\tproxy user: " << proxy_user_ << "" << std::endl;
    ss << "}";
    return ss.str();
  }
};

class OmafDashHttpParams {
 public:
  long conn_timeout_ = -1;
  long total_timeout_ = -1;

  int32_t retry_times_ = -1;
  bool bssl_verify_peer_ = false;
  bool bssl_verify_host_ = false;
  std::string to_string() {
    std::stringstream ss;
    ss << "http params: {" << std::endl;
    ss << "\tconnection timeout: " << conn_timeout_ << ", " << std::endl;
    ss << "\ttotal timeout: " << total_timeout_ << ", " << std::endl;
    ss << "\tretry times: " << retry_times_ << "" << std::endl;
    ss << "\tssl verify peer state: " << bssl_verify_peer_ << "" << std::endl;
    ss << "\tssl verify host state: " << bssl_verify_host_ << "" << std::endl;
    ss << "}";
    return ss.str();
  }
};

class OmafDashStatisticsParams {
 public:
  int32_t window_size_ms_ = 10000;  // 10s
  bool enable_ = false;
  std::string to_string() {
    std::stringstream ss;
    ss << "dash statistics params: {" << std::endl;
    ss << "\tstate: " << enable_ << std::endl;
    ss << "\twindow size: " << window_size_ms_ << " ms," << std::endl;
    ss << "}" << std::endl;
    return ss.str();
  }
};

struct _omafDashSynchronizerParams {
  int32_t segment_range_size_ = 20;
  bool enable_ = false;
  std::string to_string() {
    std::stringstream ss;
    ss << "dash segment syncer params: {" << std::endl;
    ss << "\tstate: " << enable_ << std::endl;
    ss << "\tsegment window size: " << segment_range_size_ << std::endl;
    ss << "}" << std::endl;
    return ss.str();
  }
};
using OmafDashSynchronizerParams = struct _omafDashSynchronizerParams;

struct _omafDashPredictorParams {
  std::string name_;
  std::string libpath_;
  bool enable_ = false;
  std::string to_string() {
    std::stringstream ss;
    ss << "dash statistics params: {" << std::endl;
    ss << "\tstate: " << enable_ << std::endl;
    ss << "\tname: " << name_ << std::endl;
    ss << "\tlib path: " << libpath_ << std::endl;
    ss << "}" << std::endl;
    return ss.str();
  }
};
using OmafDashPredictorParams = struct _omafDashPredictorParams;

class OmafDashParams {
 public:
 public:
  // for download
  OmafDashHttpProxy http_proxy_;
  OmafDashHttpParams http_params_;
  OmafDashStatisticsParams stats_params_;
  OmafDashSynchronizerParams syncer_params_;
  OmafDashPredictorParams prediector_params_;
  long max_parallel_transfers_ = DEFAULT_MAX_PARALLEL_TRANSFERS;
  int32_t segment_open_timeout_ms_ = DEFAULT_SEGMENT_OPEN_TIMEOUT;
  // for stitch
  uint32_t max_decode_width_;
  uint32_t max_decode_height_;
  // for catch up
  bool enable_in_time_viewport_update;
  uint32_t max_response_times_in_seg;
  uint32_t max_catchup_width;
  uint32_t max_catchup_height;

  std::string to_string() {
    std::stringstream ss;
    ss << http_proxy_.to_string();
    ss << http_params_.to_string();
    ss << "\tmax parallel transfers: " << max_parallel_transfers_ << ", " << std::endl;
    ss << stats_params_.to_string();
    ss << syncer_params_.to_string();
    ss << prediector_params_.to_string();
    return ss.str();
  }
};

enum class TaskPriority {
  HIGH = 0,
  NORMAL = 1,
  LOW = 2,
  END = 3,
};

inline std::string priority(TaskPriority p) {
  switch (p) {
    case TaskPriority::HIGH:
      return "HIGH";
    case TaskPriority::NORMAL:
      return "NORMAL";
    case TaskPriority::LOW:
      return "LOW";
    default:
      return "unknown";
  }
}

class DashSegmentSourceParams {
 public:
  int64_t timeline_point_ = -1;
  std::string dash_url_;  // unique in the system
  TaskPriority priority_ = TaskPriority::LOW;
  std::string to_string() const noexcept {
    std::stringstream ss;
    ss << "url=" << dash_url_;
    ss << ", priority=" << priority(priority_);
    ss << ", timeline_point=" << timeline_point_;
    return ss.str();
  }
};

}  // namespace OMAF
}  // namespace VCD

#endif  // !OMAF_TYPES_H
