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
//! \file:   OmafDownloader.h
//! \brief:  downloader abstract class
//!

#ifndef OMAFDOWNLOADER_H
#define OMAFDOWNLOADER_H

#include "../OmafDashParser/Common.h"
#include "../OmafTypes.h"
#include "Stream.h"

#include <string>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace VCD {
namespace OMAF {
class OmafDashSegmentClient : public VCD::NonCopyable {
 public:
  enum class State {
    SUCCESS = 0,
    STOPPED = 1,
    TIMEOUT = 2,
    FAILURE = 3,
  };

  struct _perfNode {
    size_t count_total_ = 0;
    size_t transfer_bytes_total_ = 0;
    size_t count_ = 0;
    size_t transfer_bytes_ = 0;
    double download_time_ms_ = 0;
    size_t total_transfer_time_ms_ = 0;
    double avr_transfer_time_ms_ = 0;
    std::string to_string() {
      std::stringstream ss;
      ss << "{ total: { count=" << count_total_ << ", transfer=" << transfer_bytes_total_ << " bytes}, ";
      ss << " sliding window: { count=" << count_;
      ss << ", curl download time=" << download_time_ms_;
      ss << ", transfer=" << transfer_bytes_ << " bytes";
      ss << ", transfer time=" << total_transfer_time_ms_ << " ms";
      ss << ", average transfer time=" << avr_transfer_time_ms_ << " ms";
      ss << "}}";
      return ss.str();
    }
  };
  using PerfNode = struct _perfNode;

  struct _perfStatistics {
    std::chrono::system_clock::time_point check_time_;
    PerfNode success_;
    PerfNode timeout_;
    PerfNode failure_;
    float download_speed_bps_ = 0.0f;

    std::string serializeTimePoint(const std::chrono::system_clock::time_point &time) {
      auto t_sec = std::chrono::time_point_cast<std::chrono::seconds>(time);
      auto t_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(time);
      auto tt = std::chrono::system_clock::to_time_t(t_sec);
      auto ms = (t_ms - t_sec).count();
      auto tm = *std::gmtime(&tt);
      std::stringstream ss;
      ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << ":" << ms;
      return ss.str();
    }

    std::string to_string() {
      std::stringstream ss;
      ss << std::endl;
      ss << "time: " << serializeTimePoint(check_time_) << std::endl;
      ss << "per segment transfer speed: " << download_speed_bps_ / 1000.0 << " kbps" << std::endl;
      ss << "success segment transfer: " << success_.to_string() << std::endl;
      ss << "timeout segment transfer: " << timeout_.to_string() << std::endl;
      ss << "failure segment transfer: " << failure_.to_string() << std::endl;
      return ss.str();
    }
  };

  using SourceParams = DashSegmentSourceParams;
  using PerfStatistics = struct _perfStatistics;
  using OnData = std::function<void(std::unique_ptr<VCD::OMAF::StreamBlock>)>;
  using OnState = std::function<void(State)>;

 protected:
  OmafDashSegmentClient() = default;

 public:
  virtual ~OmafDashSegmentClient(){};

 public:
  virtual OMAF_STATUS start() noexcept = 0;
  virtual OMAF_STATUS stop() noexcept = 0;

 public:
  virtual OMAF_STATUS open(const SourceParams &dash_source, OnData scb, OnState fcb) noexcept = 0;
  virtual OMAF_STATUS remove(const SourceParams &dash_source) noexcept = 0;
  virtual OMAF_STATUS check(const SourceParams &dash_source) noexcept = 0;
  virtual void setStatisticsWindows(int32_t time_window) noexcept = 0;
  virtual std::unique_ptr<PerfStatistics> statistics(void) noexcept = 0;
};

class OmafDashSegmentHttpClient : public OmafDashSegmentClient {
 public:
  using Ptr = std::shared_ptr<OmafDashSegmentHttpClient>;

 protected:
  OmafDashSegmentHttpClient() : OmafDashSegmentClient(){};

 public:
  virtual ~OmafDashSegmentHttpClient(){};

 public:
  virtual void setProxy(OmafDashHttpProxy proxy) noexcept = 0;
  virtual void setParams(OmafDashHttpParams params) noexcept = 0;

 public:
  static OmafDashSegmentHttpClient::Ptr create(long max_parallel_transfers) noexcept;
};

}  // namespace OMAF
}  // namespace VCD

#endif  // OMAFDOWNLOADER_H