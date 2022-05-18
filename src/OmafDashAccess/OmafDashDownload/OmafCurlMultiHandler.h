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
    CONTINUE = 6,
  };

  using TaskDoneCB = std::function<void(OmafDownloadTask::Ptr)>;
  using SourceParams = DashSegmentSourceParams;

 public:
  OmafDownloadTask(const SourceParams &params, OmafDashSegmentClient::OnData dcb, OmafDashSegmentClient::OnChunkData cdcb, OmafDashSegmentClient::OnState scb)
      : url_(params.dash_url_), dcb_(dcb), cdcb_(cdcb), scb_(scb), header_size_(params.header_size_), cloc_size_(params.cloc_size_),
        chunk_num_(params.chunk_num_), enable_byte_range_(params.enable_byte_range_), downloaded_chunk_id_(params.start_chunk_id_ - 1),
        stream_type_(params.stream_type_), chunk_info_type_(params.chunk_info_type_) {
    id_ = TASK_ID.fetch_add(1);
    parseTask();
  };

 public:
  virtual ~OmafDownloadTask() {
    dcb_ = nullptr;
    scb_ = nullptr;
    cdcb_ = nullptr;
    OMAF_LOG(LOG_INFO, "Release the %s\n", this->to_string().c_str());
  }

 public:
  static Ptr createTask(const SourceParams &params, OmafDashSegmentClient::OnData dcb, OmafDashSegmentClient::OnChunkData cdcb, OmafDashSegmentClient::OnState scb) {
    Ptr task = std::make_shared<OmafDownloadTask>(params, dcb, cdcb, scb);
    return task;
  }

 public:
  inline const std::string &url() const noexcept { return url_; }
  inline int64_t streamSize(void) const noexcept { return stream_size_; }
  inline int64_t lastStreamSize(void) const noexcept { return last_stream_size_; }
  inline size_t headerSize(void) const noexcept { return header_size_; }
  inline size_t id() const noexcept { return id_; }
  inline bool enableByteRange() const noexcept { return enable_byte_range_; }
  std::string to_string() const noexcept {
    std::stringstream ss;
    ss << "task, id=" << id_;
    ss << ", url=" << url_;
    ss << ", header_size=" << header_size_;
    ss << ", cloc_size=" << cloc_size_;
    ss << ", stream_size=" << stream_size_;
    ss << ", state=" << static_cast<int>(state_);
    return ss.str();
  }

  std::string get_info() const noexcept {
    std::stringstream ss;
    ss << "Task, segid=" << seg_id_;
    ss << ", track_id=" << track_id_;
    ss << ", chunk_id=" << downloaded_chunk_id_ + 1;
    ss << ", download_start_time=" << download_start_time_;
    ss << ", download_end_time=" << download_end_time_;
    ss << ", download_latency_ms=" << download_latency_;
    ss << ", segment/chunk size=" << (enable_byte_range_ ? last_stream_size_ : stream_size_);
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
          perf_counter_->downloadTime(easy_d_downloader_->downloadTime());
          perf_counter_->downloadSpeed(easy_d_downloader_->speed());
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
        } else if (state == OmafDownloadTask::State::CONTINUE) {
          scb_(OmafDashSegmentClient::State::CONTINUE);
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

 public:
  void setStartTime(int64_t time) { download_start_time_ = time; }
  void setEndTime(int64_t time) {
    download_end_time_ = time;
    download_latency_ = download_end_time_ - download_start_time_;
    LOG(WARNING) << "Task download data: " << get_info().c_str() << endl;
  }
 private:
 inline void parseTask() {
    string url = url_;
    string track_file;
    size_t pos = url.find_last_of('_');
    if (pos != string::npos) track_file = url.substr(pos);
    sscanf(track_file.c_str(), "_track%d.%d.mp4", &track_id_, &seg_id_);
 }

 private:
  std::string url_;
  uint32_t seg_id_ = 0;
  uint32_t track_id_ = 0;
  int64_t download_latency_ = 0;
  int64_t download_start_time_ = 0;
  int64_t download_end_time_ = 0;

  OmafDashSegmentClient::OnData dcb_;
  OmafDashSegmentClient::OnChunkData cdcb_;
  OmafDashSegmentClient::OnState scb_;
  State state_ = State::CREATE;
  OmafCurlEasyDownloader::Ptr easy_d_downloader_;
  OmafCurlEasyDownloader::Ptr easy_h_downloader_;
  int transfer_times_ = 0;
  size_t id_ = 0;
  size_t stream_size_ = 0;
  size_t last_stream_size_ = 0;
  OmafDownloadTaskPerfCounter::Ptr perf_counter_;
  size_t header_size_ = 0; // styp + (sidx) size
  size_t cloc_size_ = 0; // cloc size
  uint32_t chunk_num_ = 0;
  bool enable_byte_range_ = false;
  int32_t downloaded_chunk_id_ = -1;
  map<uint32_t, uint32_t> index_range_;
  DashStreamType stream_type_ = DASH_STREAM_STATIC;
  ChunkInfoType chunk_info_type_ = ChunkInfoType::NO_CHUNKINFO;

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
      OMAF_LOG(LOG_WARNING, "The task size is in invalid state!\n");
    }
    return static_cast<size_t>(size);
  }

 private:
  OMAF_STATUS removeReadyTask(OmafDownloadTask::Ptr task) noexcept;
  OMAF_STATUS removeRunningTask(OmafDownloadTask::Ptr task) noexcept;
  OMAF_STATUS moveDataTaskFromRun(OmafDownloadTask::Ptr task, OmafDownloadTask::State to_state) noexcept;
  OMAF_STATUS moveHeaderTaskFromRun(OmafDownloadTask::Ptr task, OmafDownloadTask::State to_state) noexcept;
  OMAF_STATUS markTaskFinish(OmafDownloadTask::Ptr task) noexcept;
  OMAF_STATUS markTaskTimeout(OmafDownloadTask::Ptr task) noexcept;
  OMAF_STATUS markTaskContinue(OmafDownloadTask::Ptr task) noexcept;

 private:
  void threadRunner(void) noexcept;
  OMAF_STATUS startTaskDownload(void) noexcept;
  size_t retriveDoneTask(int msgNum = -1) noexcept;
  OMAF_STATUS ProcessDataTasks() noexcept;
  OMAF_STATUS createTransferForTask(OmafDownloadTask::Ptr task) noexcept;
  OMAF_STATUS startTransferForTask(OmafDownloadTask::Ptr task) noexcept;
  OMAF_STATUS createTransfer(OmafDownloadTask::Ptr task, OmafCurlEasyDownloader::Ptr& downloader) noexcept;
  OMAF_STATUS startTransfer(OmafDownloadTask::Ptr task, OmafCurlEasyDownloader::Ptr downloader, int64_t offset, int64_t size) noexcept;
  OMAF_STATUS TriggerNextDataTransfer(OmafDownloadTask::Ptr task) noexcept;
  OMAF_STATUS removeTransfer(OmafDownloadTask::Ptr task, OmafCurlEasyDownloader::Ptr downloader) noexcept;
  void processTaskDone(OmafDownloadTask::Ptr task) noexcept;
  void processTaskContinue(OmafDownloadTask::Ptr task) noexcept;

 private:
  const long max_parallel_transfers_;
  CURLM *curl_multi_ = nullptr;
  CurlParams curl_params_;
  std::unique_ptr<OmafCurlEasyDownloaderPool> downloader_pool_;
  OmafDownloadTask::TaskDoneCB task_done_cb_;
  std::mutex ready_task_list_mutex_;
  std::list<OmafDownloadTask::Ptr> ready_task_list_;
  std::mutex run_task_map_mutex_;
  std::map<OmafCurlEasyDownloader::Ptr, OmafDownloadTask::Ptr> run_task_map_;
  std::thread worker_;
  std::atomic_int32_t task_size_{0};
  long max_parallel_ = DEFAULT_MAX_PARALLER_TRANSFERS;
  bool bworking_ = false;
  std::mutex pending_data_task_list_mutex;
  std::vector<OmafDownloadTask::Ptr> pending_data_tasks_;
};

}  // namespace OMAF
}  // namespace VCD
#endif  // !OMAFCURLMULTIWRAPPER_H
