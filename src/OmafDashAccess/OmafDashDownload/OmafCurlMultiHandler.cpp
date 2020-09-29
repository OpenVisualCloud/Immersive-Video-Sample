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
//! \file:   OmafCurlMultiHandler.cpp
//! \brief:  downloader class with libcurl multi handler
//!

#include "OmafCurlMultiHandler.h"

namespace VCD {
namespace OMAF {

std::atomic_size_t OmafDownloadTask::TASK_ID(0);

OmafCurlMultiDownloader::OmafCurlMultiDownloader(long max_parallel_transfers)
    : max_parallel_transfers_(max_parallel_transfers) {}
OmafCurlMultiDownloader::~OmafCurlMultiDownloader() { close(); }

OMAF_STATUS OmafCurlMultiDownloader::init(const CurlParams& p, OmafDownloadTask::TaskDoneCB taskcb) noexcept {
  try {
    // 1. params
    curl_params_ = p;
    task_done_cb_ = taskcb;

    // 2. create the multi handle
    curl_multi_ = curl_multi_init();
    max_parallel_ = (max_parallel_transfers_ > 0) ? max_parallel_transfers_ : DEFAULT_MAX_PARALLER_TRANSFERS;
    OMAF_LOG(LOG_INFO, "Set max transfer to %ld\n", max_parallel_);
    curl_multi_setopt(curl_multi_, CURLMOPT_MAXCONNECTS, max_parallel_ << 1);

    // 3. create the easy downloader pool
    downloader_pool_ = std::move(make_unique_vcd<OmafCurlEasyDownloaderPool>(max_parallel_ << 1));
    if (downloader_pool_ == NULL) {
      OMAF_LOG(LOG_ERROR, "Failed to create the downloader pool!\n");
      return ERROR_NULL_PTR;
    }
    downloader_pool_->params(curl_params_);

    // 4. create thread for multi runner
    bworking_ = true;
    worker_ = std::thread(&OmafCurlMultiDownloader::threadRunner, this);
    return ERROR_NONE;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when init curl multi handler, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafCurlMultiDownloader::close() noexcept {
  try {
    OMAF_LOG(LOG_INFO, "To close the curl multi downloader!\n");
    // 1. mark Stop
    bworking_ = false;

    // 2. detach the thread
    if (worker_.joinable()) {
      bworking_ = false;
      worker_.join();
    }
    // 3. clean up
    if (curl_multi_) {
      curl_multi_cleanup(curl_multi_);
      curl_multi_ = nullptr;
    }
    return ERROR_NONE;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when close curl multi handler, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafCurlMultiDownloader::addTask(OmafDownloadTask::Ptr task) noexcept {
  try {
    if (task.get() == nullptr) {
      OMAF_LOG(LOG_ERROR, "Try to add empty task!\n");
      return ERROR_INVALID;
    }
    OMAF_LOG(LOG_INFO, "01-task id %lld, task count=%d\n", task->id(), task.use_count());
    task->state(OmafDownloadTask::State::READY);
    {
      std::lock_guard<std::mutex> lock(ready_task_list_mutex_);
      ready_task_list_.push_back(task);
    }
    task_size_.fetch_add(1);
    OMAF_LOG(LOG_INFO, "02-task id %lld,  task count=%d\n", task->id(), task.use_count());
    return ERROR_NONE;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when add task, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}
OMAF_STATUS OmafCurlMultiDownloader::removeTask(OmafDownloadTask::Ptr task) noexcept {
  try {
    if (task.get() == nullptr) {
      OMAF_LOG(LOG_ERROR, "Try to remove empty task!\n");
      return ERROR_INVALID;
    }

    // FIXME, the task state may change when processing, skip this issue now
    if (task->state() == OmafDownloadTask::State::READY) {
      removeReadyTask(task);
    }

    if (task->state() == OmafDownloadTask::State::RUNNING) {
      removeRunningTask(task);
    }
    task_size_.fetch_sub(1);
    return ERROR_NONE;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when remove task, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafCurlMultiDownloader::createTransfer(OmafDownloadTask::Ptr task) noexcept {
  try {
    if (task.get() == nullptr || downloader_pool_.get() == nullptr) {
      return ERROR_INVALID;
    }

    OmafCurlEasyDownloader::Ptr downloader = std::move(downloader_pool_->pop());
    if (downloader.get() == nullptr) {
      OMAF_LOG(LOG_ERROR, "Failed to create the curl easy downloader!\n");
      return ERROR_NULL_PTR;
    }

    OMAF_STATUS ret = downloader->open(task->url());
    if (ERROR_NONE != ret) {
      OMAF_LOG(LOG_ERROR, "Failed to open the curl easy downloader, err=%d\n", ret);
      // return the downloader to pool
      downloader_pool_->push(std::move(downloader));
      return ret;
    }

    task->easy_downloader_ = std::move(downloader);
    return ERROR_NONE;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when create curl transfer, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafCurlMultiDownloader::startTransfer(OmafDownloadTask::Ptr task) noexcept {
  try {
    if (task.get() == nullptr) {
      return ERROR_INVALID;
    }

    OMAF_LOG(LOG_INFO, "To start transfer for url: %s\n", task->url_.c_str());
    auto downloader = task->easy_downloader_;
    if (downloader == nullptr) {
      OMAF_LOG(LOG_ERROR, "The curl easy downloader is empty!\n");
      return ERROR_NULL_PTR;
    }

    OMAF_STATUS ret = ERROR_NONE;
    auto offset = task->streamSize();
    // multi hanlder will manager the life cycle of curl easy hanlder,
    // so, we won't send the state callback to downloader.
    ret = downloader->start(
        offset, -1,
        [task](std::unique_ptr<StreamBlock> sb) {
          if (task->dcb_) {
            task->stream_size_ += sb->size();
            task->dcb_(std::move(sb));
          }
        },
        nullptr);
    if (ret != ERROR_NONE) {
      OMAF_LOG(LOG_ERROR, "Failed to start the curl easy downloader!\n");
      return ERROR_INVALID;
    }

    {
      std::lock_guard<std::mutex> lock(run_task_map_mutex_);
      auto handler = downloader->handler();
      if (handler) {
        OMAF_LOG(LOG_INFO, "Add to multi handler transfer for url: %s, handler: %ld\n", task->url_.c_str(), reinterpret_cast<int64_t>(task->easy_downloader_->handler()));
        curl_multi_add_handle(curl_multi_, handler);

        task->state(OmafDownloadTask::State::RUNNING);
        run_task_map_[handler] = task;
        task->transfer_times_ += 1;
      }
    }

    int still_alive = 0;
    curl_multi_perform(curl_multi_, &still_alive);
    return ERROR_NONE;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when start curl transfer task, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafCurlMultiDownloader::removeTransfer(OmafDownloadTask::Ptr task) noexcept {
  try {
    if (task.get() == nullptr) {
      return ERROR_INVALID;
    }
    OMAF_LOG(LOG_INFO, "Remove transfer for url: %s, handler: %ld\n", task->url_.c_str(), reinterpret_cast<int64_t>(task->easy_downloader_->handler()));
    CURLMcode code = curl_multi_remove_handle(curl_multi_, task->easy_downloader_->handler());
    if (code != CURLM_OK) {
      OMAF_LOG(LOG_ERROR, "Failed to remove curl easy handle from multi handler!, code= %d\n", code);
      return ERROR_INVALID;
    }
    return ERROR_NONE;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when remove curl transfer task, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafCurlMultiDownloader::removeReadyTask(OmafDownloadTask::Ptr task) noexcept {
  try {
    std::lock_guard<std::mutex> lock(ready_task_list_mutex_);
    std::list<OmafDownloadTask::Ptr>::iterator it = ready_task_list_.begin();
    for (; it != ready_task_list_.end(); ++it) {
      if ((*it)->url() == task->url()) {
        break;
      }
    }

    if (it != ready_task_list_.end()) {
      ready_task_list_.erase(it);
      task->state(OmafDownloadTask::State::STOPPED);
    }
    return ERROR_NONE;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when remove waitting task, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafCurlMultiDownloader::removeRunningTask(OmafDownloadTask::Ptr task) noexcept {
  try {
    moveTaskFromRun(task, OmafDownloadTask::State::STOPPED);

    removeTransfer(task);

    return ERROR_NONE;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when remove running task, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafCurlMultiDownloader::markTaskFinish(OmafDownloadTask::Ptr task) noexcept {
  try {
    OMAF_LOG(LOG_INFO, "Task finish, url=%s\n", task->url().c_str());

    moveTaskFromRun(task, OmafDownloadTask::State::FINISH);
    // other logic
    processTaskDone(std::move(task));

    return ERROR_NONE;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when process finished task, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafCurlMultiDownloader::markTaskTimeout(OmafDownloadTask::Ptr task) noexcept {
  try {
    OMAF_LOG(LOG_INFO, "Task timeout, url=%s\n", task->url().c_str());
    moveTaskFromRun(task, OmafDownloadTask::State::TIMEOUT);
    // other logic
    // restart the trasfer when timeout
    if (task->transfer_times_ < curl_params_.http_params_.retry_times_) {
      startTransfer(task);
    } else {
      processTaskDone(std::move(task));
    }
    return ERROR_NONE;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when process timeout task, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

void OmafCurlMultiDownloader::processTaskDone(OmafDownloadTask::Ptr task) noexcept {
  try {
    if (task.get() == nullptr) {
      OMAF_LOG(LOG_ERROR, "Try to process empty task!\n");
    } else {
      task_size_.fetch_sub(1);

      if (task->easy_downloader_) {
        // return the downloader to pool
        if (downloader_pool_) {
          task->easy_downloader_->stop();
          downloader_pool_->push(std::move(task->easy_downloader_));
        } else {
          task->easy_downloader_->close();
          task->easy_downloader_.reset();
        }
      }
      if (task_done_cb_) {
        task_done_cb_(std::move(task));
      }
    }
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when process task done, ex: %s\n", ex.what());
  }
}

OMAF_STATUS OmafCurlMultiDownloader::moveTaskFromRun(OmafDownloadTask::Ptr task,
                                                     OmafDownloadTask::State to_state) noexcept {
  try {
    std::lock_guard<std::mutex> lock(run_task_map_mutex_);
    auto handler = task->easy_downloader_->handler();
    auto find = run_task_map_.find(handler);
    if (find != run_task_map_.end()) {
      run_task_map_.erase(find);
      task->state(to_state);
    }

    return ERROR_NONE;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when task state from run to %d, ex: %s\n", static_cast<OMAF_STATUS>(to_state), ex.what());
    return ERROR_INVALID;
  }
}

void OmafCurlMultiDownloader::threadRunner(void) noexcept {
  try {
    while (bworking_) {
      startTaskDownload();

      int still_alive = 0;
      curl_multi_perform(curl_multi_, &still_alive);

      if (still_alive == max_parallel_) {
        int numfds;
        curl_multi_wait(curl_multi_, nullptr, 0, 100, &numfds);
      }

      retriveDoneTask();
    }
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception in the multi thread worker, ex: %s\n", ex.what());
  }
}

OMAF_STATUS OmafCurlMultiDownloader::startTaskDownload(void) noexcept {
  try {
    // start a new task
    if ((run_task_map_.size() < static_cast<size_t>(max_parallel_transfers_)) && (ready_task_list_.size() > 0)) {
      OmafDownloadTask::Ptr task = NULL;
      {
        std::lock_guard<std::mutex> lock(ready_task_list_mutex_);
        task = std::move(ready_task_list_.front());
        OMAF_LOG(LOG_INFO, "1-task id %lld, task count=%d\n", task->id(),  task.use_count());
        ready_task_list_.pop_front();
      }
      OMAF_STATUS ret = ERROR_NONE;
      if (task != NULL) {
        ret = createTransfer(task);
        if (ret == ERROR_NONE) {
          ret = startTransfer(task);
          if (ret != ERROR_NONE) {
            OMAF_LOG(LOG_ERROR, "Failed to start the transfer!\n");
            removeRunningTask(task);
          }
        } else {
          OMAF_LOG(LOG_ERROR, "Failed to create the transfer!\n");
        }
      }
      else
      {
        OMAF_LOG(LOG_ERROR, "Download task failed to create!\n");
        return ERROR_NULL_PTR;
      }
      OMAF_LOG(LOG_INFO, "2-task id %lld, task count=%d\n", task->id(), task.use_count());
    }
    return ERROR_NONE;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when start a new task, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

size_t OmafCurlMultiDownloader::retriveDoneTask(int msgNum) noexcept {
  try {
    UNUSED(msgNum);

    struct CURLMsg* msg;
    size_t num = 0;
    do {
      int msgq = 0;
      msg = curl_multi_info_read(curl_multi_, &msgq);
      if (msg && (msg->msg == CURLMSG_DONE)) {
        auto handler = msg->easy_handle;
        OmafDownloadTask::Ptr task;
        {
          std::lock_guard<std::mutex> lock(run_task_map_mutex_);
          auto find = run_task_map_.find(handler);
          if (find != run_task_map_.end()) {
            task = find->second;
          }
        }

        if (task.get() != nullptr) {
          OMAF_LOG(LOG_INFO, "3-task id %lld, task count=%d\n", task->id(), task.use_count());
          removeTransfer(task);
          auto header = task->easy_downloader_->header();
          OMAF_LOG(LOG_INFO, "Header content length=%lld\n", header.content_length_);
          if (OmafCurlEasyHelper::success(header.http_status_code_) && (header.content_length_ == task->streamSize())) {
            markTaskFinish(std::move(task));
          } else {
            // FIXME how to check timeout
            markTaskTimeout(std::move(task));
          }
        }
      }

      num++;
    } while (msg);

    return num;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception in the process multi handler information, ex: %s\n", ex.what());
    return 0;
  }
}  // namespace OMAF
}  // namespace OMAF
}  // namespace VCD
