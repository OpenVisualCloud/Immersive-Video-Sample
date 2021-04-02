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
//! \file:   OmafDownloader.cpp
//! \brief:  downloader class
//!

#include "OmafDownloader.h"
#include "OmafCurlMultiHandler.h"
#include "performance.h"

#include <chrono>
#include <list>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

namespace VCD {
namespace OMAF {

#define PRIORITYTASKSIZE static_cast<int>(TaskPriority::END)

struct _taskList {
  using Ptr = std::shared_ptr<struct _taskList>;

  int64_t timeline_point_ = -1;
  std::list<OmafDownloadTask::Ptr> tasks_[PRIORITYTASKSIZE];
};
using TaskList = struct _taskList;

class OmafDashSegmentHttpClientPerf;

/******************************************************************************
 *
 * class define
 *
 * ****************************************************************************/
class OmafDashSegmentHttpClientImpl : public OmafDashSegmentHttpClient {
 public:
  OmafDashSegmentHttpClientImpl(long max_parallel_transfers = DEFAULT_MAX_PARALLEL_TRANSFERS)
      : max_parallel_transfers_(max_parallel_transfers)
       {
         tmpMultiDownloader_ = nullptr;
       };

  virtual ~OmafDashSegmentHttpClientImpl() { stop(); };

 public:
  OMAF_STATUS start() noexcept override;
  OMAF_STATUS stop() noexcept override;

 public:
  OMAF_STATUS open(const SourceParams &ds_params, OnData dcb, OnState scb) noexcept override;
  OMAF_STATUS remove(const SourceParams &ds_params) noexcept override;
  OMAF_STATUS check(const SourceParams &ds_params) noexcept override;
  inline void setStatisticsWindows(int32_t time_window) noexcept override;
  inline std::unique_ptr<PerfStatistics> statistics(void) noexcept override;

 public:
  // set policy based on the priority
  void setProxy(OmafDashHttpProxy proxy) noexcept override {
    curl_params_.http_proxy_ = proxy;
    if (segment_downloader_) {
      segment_downloader_->setParams(curl_params_);
    }
  };
  void setParams(OmafDashHttpParams params) noexcept override {
    curl_params_.http_params_ = params;
    if (segment_downloader_) {
      segment_downloader_->setParams(curl_params_);
    }
  };

 private:
  void threadRunner(void) noexcept;
  OmafDownloadTask::Ptr fetchReadyTask(void) noexcept;
  void processDoneTask(OmafDownloadTask::Ptr task) noexcept;

 private:
  const long max_parallel_transfers_;
  OmafCurlMultiDownloader::Ptr segment_downloader_;
  CurlParams curl_params_;
  OmafCurlChecker::Ptr url_checker_;
  std::mutex task_queue_mutex_;
  std::condition_variable task_queue_cv_;
  std::list<TaskList::Ptr> task_queue_;
  std::mutex downloading_task_mutex_;
  std::map<std::string, OmafDownloadTask::Ptr> downloading_tasks_;
  std::thread download_worker_;
  bool bworking_ = false;

  std::unique_ptr<OmafDashSegmentHttpClientPerf> perf_stats_;
  OmafCurlMultiDownloader *tmpMultiDownloader_;
};

class OmafDashSegmentHttpClientPerf : public VCD::NonCopyable {
 public:
  OmafDashSegmentHttpClientPerf() = default;
  virtual ~OmafDashSegmentHttpClientPerf() {}

 public:
  void setStatisticsWindows(int32_t time_window) noexcept;
  std::unique_ptr<OmafDashSegmentClient::PerfStatistics> statistics(void) noexcept;

  void addTime(OmafDownloadTask::State, const std::chrono::milliseconds &) noexcept;
  void addTransfer(OmafDownloadTask::State, size_t) noexcept;
  void addDownloadTime(OmafDownloadTask::State, long) noexcept;
  void add(OmafDownloadTask::State state, const std::chrono::milliseconds &duration, size_t transfer_size,
           long download, double network_spped);

 private:
  void copyPerf(WindowCounter<size_t> &time_counter, WindowCounter<size_t> &transfer_counter,
                WindowCounter<long> &download_counter, OmafDashSegmentClient::PerfNode &to_node) noexcept;

 private:
  WindowCounter<size_t> success_task_time_counter_;
  WindowCounter<size_t> timeout_task_time_counter_;
  WindowCounter<size_t> failure_task_time_counter_;
  WindowCounter<size_t> success_task_transfer_counter_;
  WindowCounter<size_t> timeout_task_transfer_counter_;
  WindowCounter<size_t> failure_task_transfer_counter_;
  WindowCounter<long> success_task_download_counter_;
  WindowCounter<long> timeout_task_download_counter_;
  WindowCounter<long> failure_task_download_counter_;
  WindowCounter<double> network_speed_counter_;
};

/******************************************************************************
 *
 * class implementation
 *
 * ****************************************************************************/
OmafDashSegmentHttpClient::Ptr OmafDashSegmentHttpClient::create(long max_parallel_transfers) noexcept {
  try {
    return std::make_shared<OmafDashSegmentHttpClientImpl>(max_parallel_transfers);
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when create the dash segment http source, ex:%s\n", ex.what());
    return nullptr;
  }
}

OMAF_STATUS OmafDashSegmentHttpClientImpl::start() noexcept {
  try {
    OMAF_LOG(LOG_INFO, "Start the dash source http client!\n");
    curl_global_init(CURL_GLOBAL_ALL);

    // 1. create the multi downloader
    tmpMultiDownloader_ = new OmafCurlMultiDownloader;
    if (tmpMultiDownloader_ == NULL) return ERROR_INVALID;
    segment_downloader_.reset(tmpMultiDownloader_);
    if (segment_downloader_.get() == nullptr) {
      OMAF_LOG(LOG_ERROR, "Failed to create the curl multi downloader!\n");
      return ERROR_INVALID;
    }
    OMAF_STATUS ret = segment_downloader_->init(
        curl_params_, [this](OmafDownloadTask::Ptr task) { this->processDoneTask(std::move(task)); });
    if (ERROR_NONE != ret) {
      OMAF_LOG(LOG_ERROR, "Failed to init the multi downloader!\n");
      return ERROR_INVALID;
    }

    // 2. create the checker
    url_checker_ = std::make_shared<OmafCurlChecker>();
    if (url_checker_ == nullptr) {
      OMAF_LOG(LOG_ERROR, "Failed to create the curl checker downloader!\n");
      return ERROR_NULL_PTR;
    }
    ret = url_checker_->init(curl_params_);
    if (ERROR_NONE != ret) {
      OMAF_LOG(LOG_ERROR, "Failed to init the curl easy downloader!\n");
      return ERROR_INVALID;
    }

    // 3. start the worker
    bworking_ = true;
    download_worker_ = std::thread(&OmafDashSegmentHttpClientImpl::threadRunner, this);
    return ERROR_NONE;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when start the dash source, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafDashSegmentHttpClientImpl::stop() noexcept {
  try {
    OMAF_LOG(LOG_INFO, "Stop the dash source http client!\n");
    bworking_ = false;
    if (segment_downloader_.get() != nullptr) {
      segment_downloader_->close();
    }

    if (url_checker_.get() != nullptr) {
      url_checker_->close();
    }

    {
      std::lock_guard<std::mutex> lock(task_queue_mutex_);
      task_queue_.clear();
      task_queue_cv_.notify_all();
    }

    {
      std::lock_guard<std::mutex> lock(downloading_task_mutex_);
      downloading_tasks_.clear();
    }

    if (download_worker_.joinable()) {
      bworking_ = false;
      download_worker_.join();
    }

    curl_global_cleanup();
    tmpMultiDownloader_ = nullptr;
    OMAF_LOG(LOG_INFO, "Success to stop the dash client!\n");
    return ERROR_NONE;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when Stop the dash source, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafDashSegmentHttpClientImpl::open(const SourceParams &ds_params, OnData dcb, OnState scb) noexcept {
  try {
    OmafDownloadTask::Ptr task = OmafDownloadTask::createTask(ds_params.dash_url_, dcb, scb);
    if (task.get() == nullptr) {
      OMAF_LOG(LOG_ERROR, "Failed to create the task\n");
      return ERROR_INVALID;
    }
    if (perf_stats_.get() != nullptr) {
      OmafDownloadTaskPerfCounter::Ptr t_perf = std::make_shared<OmafDownloadTaskPerfCounter>();
      task->perfCounter(std::move(t_perf));
    }

    OMAF_LOG(LOG_INFO, "Open the task count=%d. %s\n", task.use_count(), task->to_string().c_str());
    bool new_timeline = true;

    std::lock_guard<std::mutex> lock(task_queue_mutex_);
    for (auto &tl : task_queue_) {
      if (ds_params.timeline_point_ == tl->timeline_point_) {
        new_timeline = false;
        auto &tasks = tl->tasks_[static_cast<int>(ds_params.priority_)];
        tasks.push_back(task);
        break;
      }
    }

    // this is a download request with new timeline
    if (new_timeline) {
      TaskList::Ptr tl = std::make_shared<TaskList>();
      if (tl.get() == nullptr) {
        OMAF_LOG(LOG_ERROR, "Task list create failed!\n");
        return ERROR_NULL_PTR;
      }
      tl->timeline_point_ = ds_params.timeline_point_;
      int priority = static_cast<int>(ds_params.priority_);
      if (priority < PRIORITYTASKSIZE){
        tl->tasks_[priority].push_back(task);
      }else
      {
        OMAF_LOG(LOG_ERROR, "Priority %d is invalid, the max task size is %d\n", priority, PRIORITYTASKSIZE);
        return ERROR_INVALID;
      }

      // Since there exists catch up download tasks, remove this condition
      // if (!task_queue_.empty()) {
      //   auto &tail_tasks = task_queue_.back();
      //   if (tail_tasks->timeline_point_ >= ds_params.timeline_point_) {
      //     OMAF_LOG(LOG_FATAL, "Invalid timeline point happen! < %ld, %ld>\n", tail_tasks->timeline_point_, ds_params.timeline_point_);
      //   }
      // }

      task_queue_.push_back(tl);
    }
    task_queue_cv_.notify_all();
    return ERROR_NONE;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when open the dash source, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}
OMAF_STATUS OmafDashSegmentHttpClientImpl::remove(const SourceParams &ds_params) noexcept {
  try {
    OmafDownloadTask::Ptr to_remove_task;

    // 1. find it from create state list
    {
      std::lock_guard<std::mutex> lock(task_queue_mutex_);
      for (auto &tl : task_queue_) {
        if (tl->timeline_point_ == ds_params.timeline_point_) {
          int priority = static_cast<int>(ds_params.priority_);
          auto &tasks = tl->tasks_[priority];

          std::list<OmafDownloadTask::Ptr>::iterator it = tasks.begin();
          while (it != tasks.end()) {
            auto &task = *it;
            if (task->url() == ds_params.dash_url_) {
              if (task->state() == OmafDownloadTask::State::CREATE) {
                to_remove_task = *it;
                tasks.erase(it);
                break;
              }
            }
          }
          break;
        }
      }
    }

    // 2. remove it from downloading task list
    if (to_remove_task.get() == nullptr) {
      {
        std::lock_guard<std::mutex> lock(downloading_task_mutex_);
        auto it = downloading_tasks_.find(ds_params.dash_url_);
        if (it != downloading_tasks_.end()) {
          to_remove_task = std::move(it->second);
          downloading_tasks_.erase(it);
        }
      }
      if (to_remove_task.get() != nullptr) {
        segment_downloader_->removeTask(to_remove_task);
      }
    }

    if (to_remove_task.get() == nullptr) {
      OMAF_LOG(LOG_ERROR, "Failed to remove the task for the dash: %s\n", ds_params.dash_url_.c_str());
      return ERROR_INVALID;
    }

    return ERROR_NONE;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when Stop the dash source, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}
OMAF_STATUS OmafDashSegmentHttpClientImpl::check(const SourceParams &ds_params) noexcept {
  try {
    return url_checker_->check(ds_params.dash_url_);
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when check the dash source, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

inline void OmafDashSegmentHttpClientImpl::setStatisticsWindows(int32_t time_window) noexcept {
  if (perf_stats_ == nullptr) {
    perf_stats_.reset(new OmafDashSegmentHttpClientPerf());
  }
  perf_stats_->setStatisticsWindows(time_window);
};
inline std::unique_ptr<OmafDashSegmentClient::PerfStatistics> OmafDashSegmentHttpClientImpl::statistics(void) noexcept {
  if (perf_stats_ != nullptr) {
    return std::move(perf_stats_->statistics());
  }
  return nullptr;
};

void OmafDashSegmentHttpClientImpl::threadRunner(void) noexcept {
  try {
    const size_t max_queue_size = static_cast<size_t>(max_parallel_transfers_ << 1);
    while (bworking_) {
      // 1.1. check
      // too many task in dash downloader
      if (segment_downloader_->size() > max_queue_size) {
        OMAF_LOG(LOG_INFO, "The size of downloading is: %lld\n", segment_downloader_->size());
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        continue;
      }
      // 2. fetch ready task

      OmafDownloadTask::Ptr task = fetchReadyTask();
      if (task.get() != nullptr) {
        // 2.1 add to downloader
        OMAF_LOG(LOG_INFO, "downloader-0-task id %lld, task count=%d\n", task->id(), task.use_count());
        segment_downloader_->addTask(task);
        {
          // 2.1.2 cache in the downloading list to support remove

          std::lock_guard<std::mutex> lock(downloading_task_mutex_);
          downloading_tasks_[task->url()] = task;
        }
        OMAF_LOG(LOG_INFO, "Start download for task count=%d. %s\n", task.use_count(), task->to_string().c_str());
      }
    }
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception in the dash source thread runner, ex: %s\n", ex.what());
  }
}

OmafDownloadTask::Ptr OmafDashSegmentHttpClientImpl::fetchReadyTask(void) noexcept {
  try {
    std::unique_lock<std::mutex> lock(task_queue_mutex_);

    while (task_queue_.size()) {
      auto &tl = task_queue_.front();
      for (int i = 0; i < PRIORITYTASKSIZE; i++) {
        if (tl->tasks_[i].size()) {
          auto task = std::move(tl->tasks_[i].front());
          tl->tasks_[i].pop_front();
          return std::move(task);
        }
      }
      // no new task list with new timeline ready, then wait
      if (task_queue_.size() <= 1) {
        task_queue_cv_.wait(lock);
      } else {
        // drop the oldest task list of queue and move next
        task_queue_.pop_front();
      }
    }

    return nullptr;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when fetch the avaiable task, ex: %s\n", ex.what());
    return nullptr;
  }
}

void OmafDashSegmentHttpClientImpl::processDoneTask(OmafDownloadTask::Ptr task) noexcept {
  try {
    if (task.get() == nullptr) {
      return;
    }
    OmafDownloadTask::State state = task->state();
    task->taskDoneCallback(state);

    // remove from downloading list
    {
      std::lock_guard<std::mutex> lock(downloading_task_mutex_);
      auto it = downloading_tasks_.find(task->url());
      if (it != downloading_tasks_.end()) {
        downloading_tasks_.erase(it);
        OMAF_LOG(LOG_INFO, "Done the task count=%d. %s\n", task.use_count(), task->to_string().c_str());
      }
    }

    if (perf_stats_) {
      auto duration = task->transferDuration();
      auto transfer_size = task->streamSize();
      auto download_time = task->downloadTime();
      auto network_speed = task->downloadSpeed();
      perf_stats_->add(state, duration, transfer_size, download_time, network_speed);
    }
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when process the done task, ex: %s\n", ex.what());
  }
}

void OmafDashSegmentHttpClientPerf::addTime(OmafDownloadTask::State state,
                                            const std::chrono::milliseconds &duration) noexcept {
  switch (state) {
    case OmafDownloadTask::State::STOPPED:
      failure_task_time_counter_.add(duration.count());

      break;
    case OmafDownloadTask::State::TIMEOUT:
      timeout_task_time_counter_.add(duration.count());

      break;
    case OmafDownloadTask::State::FINISH:
      success_task_time_counter_.add(duration.count());

      break;
    default:
      break;
  }
}
void OmafDashSegmentHttpClientPerf::addTransfer(OmafDownloadTask::State state, size_t transfer_size) noexcept {
  switch (state) {
    case OmafDownloadTask::State::STOPPED:

      failure_task_transfer_counter_.add(transfer_size);
      break;
    case OmafDownloadTask::State::TIMEOUT:

      timeout_task_transfer_counter_.add(transfer_size);
      break;
    case OmafDownloadTask::State::FINISH:

      success_task_transfer_counter_.add(transfer_size);
      break;
    default:
      break;
  }
}
void OmafDashSegmentHttpClientPerf::addDownloadTime(OmafDownloadTask::State state, long download) noexcept {
  switch (state) {
    case OmafDownloadTask::State::STOPPED:
      failure_task_download_counter_.add(download);
      break;
    case OmafDownloadTask::State::TIMEOUT:
      timeout_task_download_counter_.add(download);
      break;
    case OmafDownloadTask::State::FINISH:
      success_task_download_counter_.add(download);
      break;
    default:
      break;
  }
}
void OmafDashSegmentHttpClientPerf::add(OmafDownloadTask::State state, const std::chrono::milliseconds &duration,
                                        size_t transfer_size, long download_time, double network_speed) {
  switch (state) {
    case OmafDownloadTask::State::STOPPED:
      failure_task_time_counter_.add(duration.count());
      failure_task_transfer_counter_.add(transfer_size);
      failure_task_download_counter_.add(download_time);
      break;
    case OmafDownloadTask::State::TIMEOUT:
      timeout_task_time_counter_.add(duration.count());
      timeout_task_transfer_counter_.add(transfer_size);
      timeout_task_download_counter_.add(download_time);
      break;
    case OmafDownloadTask::State::FINISH:
      success_task_time_counter_.add(duration.count());
      success_task_transfer_counter_.add(transfer_size);
      success_task_download_counter_.add(download_time);
      break;
    default:
      break;
  }
  network_speed_counter_.add(network_speed);
}

void OmafDashSegmentHttpClientPerf::setStatisticsWindows(int32_t time_window) noexcept {
  try {
    success_task_time_counter_.setWindow(time_window);
    timeout_task_time_counter_.setWindow(time_window);
    failure_task_time_counter_.setWindow(time_window);
    success_task_transfer_counter_.setWindow(time_window);
    timeout_task_transfer_counter_.setWindow(time_window);
    failure_task_transfer_counter_.setWindow(time_window);
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when set time window, ex: %s\n", ex.what());
  }
}

std::unique_ptr<OmafDashSegmentClient::PerfStatistics> OmafDashSegmentHttpClientPerf::statistics(void) noexcept {
  std::unique_ptr<OmafDashSegmentClient::PerfStatistics> perf =
      make_unique_vcd<OmafDashSegmentClient::PerfStatistics>();
  perf->check_time_ = std::chrono::system_clock::now();
  copyPerf(success_task_time_counter_, success_task_transfer_counter_, success_task_download_counter_, perf->success_);
  copyPerf(timeout_task_time_counter_, timeout_task_transfer_counter_, timeout_task_download_counter_, perf->timeout_);
  copyPerf(failure_task_time_counter_, failure_task_transfer_counter_, failure_task_download_counter_, perf->failure_);

  perf->download_speed_bps_ = network_speed_counter_.count().avr_value_window_;
  return perf;
}

inline void OmafDashSegmentHttpClientPerf::copyPerf(WindowCounter<size_t> &time_counter,
                                                    WindowCounter<size_t> &transfer_counter,
                                                    WindowCounter<long> &download_counter,
                                                    OmafDashSegmentClient::PerfNode &to_node) noexcept {
  auto time_v = time_counter.count();
  auto transfer_v = transfer_counter.count();
  auto download_v = download_counter.count();
  to_node.count_total_ = time_v.points_size_total_;
  to_node.transfer_bytes_total_ = transfer_v.sum_value_total_;
  to_node.count_ = time_v.points_size_window_;
  to_node.total_transfer_time_ms_ = time_v.sum_value_total_;
  to_node.avr_transfer_time_ms_ = time_v.avr_value_window_;
  to_node.transfer_bytes_ = transfer_v.sum_value_total_;
  to_node.download_time_ms_ = download_v.avr_value_window_ / 1000.0;  // to ms
}

}  // namespace OMAF
}  // namespace VCD
