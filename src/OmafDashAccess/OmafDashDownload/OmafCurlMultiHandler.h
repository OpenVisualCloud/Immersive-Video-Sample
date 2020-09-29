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
//! \file:   OmafCurlMultiDownloader.h
//! \brief:  downloader class with libcurl multi handler
//!

#ifndef OMAFCURLMULTIWRAPPER_H
#define OMAFCURLMULTIWRAPPER_H

#include "../common.h"  // VCD::NonCopyable
#include "OmafCurlEasyHandler.h"
#include "OmafDownloader.h"
#include "performance.h"

#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <atomic>
#include <queue>

namespace VCD {
namespace OMAF {

class OmafCurlMultiDownloader;
class OmafDownloadTask;

const int DEFAULT_MAX_PARALLER_TRANSFERS = 50;

class OmafDownloadTaskPerfCounter : public VCD::NonCopyable {
 public:
  using UPtr = std::unique_ptr<OmafDownloadTaskPerfCounter>;
  using Ptr = std::shared_ptr<OmafDownloadTaskPerfCounter>;

 public:
  OmafDownloadTaskPerfCounter() { create_time_ = std::chrono::steady_clock::now(); };
  virtual ~OmafDownloadTaskPerfCounter(){};
  inline void markStart() { start_transfer_ = std::chrono::steady_clock::now(); };
  inline void markStop() { stop_transfer_ = std::chrono::steady_clock::now(); };
  inline std::chrono::milliseconds transferDuration() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(stop_transfer_ - start_transfer_);
  };
  inline std::chrono::milliseconds duration() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(stop_transfer_ - create_time_);
  };
  inline long downloadTime() const { return download_time_; }
  inline void downloadTime(long t) { download_time_ = t; }
  inline double downloadSpeed() const { return download_speed_; }
  inline void downloadSpeed(double s) { download_speed_ = s; }

 private:
  std::chrono::steady_clock::time_point create_time_;
  std::chrono::steady_clock::time_point start_transfer_;
  std::chrono::steady_clock::time_point stop_transfer_;
  long download_time_ = 0;
  double download_speed_ = 0.0;
};

class OmafDownloadTask : public VCD::NonCopyable {
  friend OmafCurlMultiDownloader;

 public:
  using Ptr = std::shared_ptr<OmafDownloadTask>;

  enum class State {
    CREATE = 0,
    READY = 1,
    RUNNING = 2,
    STOPPED = 3,
    TIMEOUT = 4,
    FINISH = 5,
  };

  using TaskDoneCB = std::function<void(OmafDownloadTask::Ptr)>;

 public:
  OmafDownloadTask(const std::string &url, OmafDashSegmentClient::OnData dcb, OmafDashSegmentClient::OnState scb)
      : url_(url), dcb_(dcb), scb_(scb) {
    id_ = TASK_ID.fetch_add(1);
  };

 public:
  virtual ~OmafDownloadTask() {
    dcb_ = nullptr;
    scb_ = nullptr;
    OMAF_LOG(LOG_INFO, "Release the %s\n", this->to_string().c_str());
  }

 public:
  static Ptr createTask(const std::string &url, OmafDashSegmentClient::OnData dcb, OmafDashSegmentClient::OnState scb) {
    Ptr task = std::make_shared<OmafDownloadTask>(url, dcb, scb);
    return task;
  }

 public:
  inline const std::string &url() const noexcept { return url_; }
  inline int64_t streamSize(void) const noexcept { return stream_size_; }
  inline size_t id() const noexcept { return id_; }
  std::string to_string() const noexcept {
    std::stringstream ss;
    ss << "task, id=" << id_;
    ss << ", url=" << url_;
    ss << ", stream_size=" << stream_size_;
    ss << ", state=" << static_cast<int>(state_);
    return ss.str();
  }

 public:
  inline void state(State s) noexcept {
    switch (s) {
      case State::CREATE:
      case State::READY:
        break;
      case State::RUNNING:
        if (perf_counter_) perf_counter_->markStart();
        break;
      case State::STOPPED:
      case State::TIMEOUT:
      case State::FINISH:
        if (perf_counter_) {
          perf_counter_->downloadTime(easy_downloader_->downloadTime());
          perf_counter_->downloadSpeed(easy_downloader_->speed());
        }
        break;
      default:
        break;
    }
    state_ = s;
  };
  inline State state() noexcept { return state_; }

  inline void taskDoneCallback(State state) noexcept {
    if (perf_counter_) perf_counter_->markStop();
    if (scb_) {
      if (state == OmafDownloadTask::State::FINISH) {
        scb_(OmafDashSegmentClient::State::SUCCESS);
      } else {
        if (state == OmafDownloadTask::State::STOPPED) {
          scb_(OmafDashSegmentClient::State::STOPPED);
        } else if (state == OmafDownloadTask::State::TIMEOUT) {
          scb_(OmafDashSegmentClient::State::TIMEOUT);
        } else {
          scb_(OmafDashSegmentClient::State::FAILURE);
        }
      }
    }
  }
  std::chrono::milliseconds transferDuration() const {
    if (perf_counter_) return perf_counter_->transferDuration();
    return std::chrono::milliseconds(0);
  }
  std::chrono::milliseconds duration() const {
    if (perf_counter_) return perf_counter_->duration();
    return std::chrono::milliseconds(0);
  }
  long downloadTime() const {
    if (perf_counter_) return perf_counter_->downloadTime();
    return 0;
  }
  double downloadSpeed() const {
    if (perf_counter_) return perf_counter_->downloadSpeed();
    return 0.0f;
  }

 public:
  void perfCounter(OmafDownloadTaskPerfCounter::Ptr s) { perf_counter_ = s; }
  OmafDownloadTaskPerfCounter::Ptr perfCounter() { return perf_counter_; };

 private:
  std::string url_;
  OmafDashSegmentClient::OnData dcb_;
  OmafDashSegmentClient::OnState scb_;
  State state_ = State::CREATE;
  OmafCurlEasyDownloader::Ptr easy_downloader_;
  int transfer_times_ = 0;
  size_t id_ = 0;
  size_t stream_size_ = 0;
  OmafDownloadTaskPerfCounter::Ptr perf_counter_;

 private:
  static std::atomic_size_t TASK_ID;
};  // namespace OMAF

class OmafCurlMultiDownloader : public VCD::NonCopyable {
 public:
  using Ptr = std::shared_ptr<OmafCurlMultiDownloader>;

 public:
  OmafCurlMultiDownloader(long max_parallel_transfers = DEFAULT_MAX_PARALLEL_TRANSFERS);
  virtual ~OmafCurlMultiDownloader();

 public:
  OMAF_STATUS init(const CurlParams &p, OmafDownloadTask::TaskDoneCB) noexcept;
  OMAF_STATUS close() noexcept;
  void setParams(const CurlParams &p) noexcept {
    curl_params_ = p;
    if (downloader_pool_) {
      downloader_pool_->params(p);
    }
  };

 public:
  OMAF_STATUS addTask(OmafDownloadTask::Ptr task) noexcept;
  OMAF_STATUS removeTask(OmafDownloadTask::Ptr task) noexcept;

  inline size_t size() const noexcept {  // return ready_task_list_.size() + run_task_map_.size();
    int size = task_size_.load();
    if (size < 0) {
      OMAF_LOG(LOG_FATAL, "The task size is in invalid state!\n");
    }
    return static_cast<size_t>(size);
  }

 private:
  OMAF_STATUS removeReadyTask(OmafDownloadTask::Ptr task) noexcept;
  OMAF_STATUS removeRunningTask(OmafDownloadTask::Ptr task) noexcept;
  OMAF_STATUS moveTaskFromRun(OmafDownloadTask::Ptr task, OmafDownloadTask::State to_state) noexcept;
  OMAF_STATUS markTaskFinish(OmafDownloadTask::Ptr task) noexcept;
  OMAF_STATUS markTaskTimeout(OmafDownloadTask::Ptr task) noexcept;

 private:
  void threadRunner(void) noexcept;
  OMAF_STATUS startTaskDownload(void) noexcept;
  size_t retriveDoneTask(int msgNum = -1) noexcept;
  OMAF_STATUS createTransfer(OmafDownloadTask::Ptr task) noexcept;
  OMAF_STATUS startTransfer(OmafDownloadTask::Ptr task) noexcept;
  OMAF_STATUS removeTransfer(OmafDownloadTask::Ptr task) noexcept;
  void processTaskDone(OmafDownloadTask::Ptr task) noexcept;

 private:
  const long max_parallel_transfers_;
  CURLM *curl_multi_ = nullptr;
  CurlParams curl_params_;
  std::unique_ptr<OmafCurlEasyDownloaderPool> downloader_pool_;
  OmafDownloadTask::TaskDoneCB task_done_cb_;
  std::mutex ready_task_list_mutex_;
  std::list<OmafDownloadTask::Ptr> ready_task_list_;
  std::mutex run_task_map_mutex_;
  std::map<void *, OmafDownloadTask::Ptr> run_task_map_;
  std::thread worker_;
  std::atomic_int32_t task_size_{0};
  long max_parallel_ = DEFAULT_MAX_PARALLER_TRANSFERS;
  bool bworking_ = false;
};

}  // namespace OMAF
}  // namespace VCD
#endif  // !OMAFCURLMULTIWRAPPER_H
