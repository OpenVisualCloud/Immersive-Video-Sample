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

    return ERROR_NONE;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when remove task, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafCurlMultiDownloader::createTransferForTask(OmafDownloadTask::Ptr task) noexcept {
  int ret = ERROR_NONE;
  // 1. create header handler if in byte range mode
  if (task->header_size_ != 0 && task->easy_h_downloader_.get() == nullptr) {
    OMAF_LOG(LOG_INFO, "Create transfer for header downloader\n");
    ret = createTransfer(task, task->easy_h_downloader_);

    if (ret != ERROR_NONE || task->easy_h_downloader_.get() == nullptr)
      return ERROR_NULL_PTR;

    task->easy_h_downloader_->setType(OmafCurlEasyDownloader::Type::HEADER);
  }
  // 2. create data handler
  if (task->easy_d_downloader_.get() == nullptr) {
    OMAF_LOG(LOG_INFO, "Create transfer for data downloader\n");
    ret = createTransfer(task, task->easy_d_downloader_);

    if (ret != ERROR_NONE || task->easy_d_downloader_.get() == nullptr)
      return ERROR_NULL_PTR;

    task->easy_d_downloader_->setType(OmafCurlEasyDownloader::Type::DATA);
  }
  return ret;
}

OMAF_STATUS OmafCurlMultiDownloader::startTransferForTask(OmafDownloadTask::Ptr task) noexcept {
  int ret = ERROR_NONE;
  // 1. start header transfer
  if (task->easy_h_downloader_.get() != nullptr) {
    OMAF_LOG(LOG_INFO, "Start transfer for header downloader %s\n", task->to_string().c_str());
    ret = startTransfer(task, task->easy_h_downloader_, 0, task->header_size_);

    if (ret != ERROR_NONE) {
      OMAF_LOG(LOG_ERROR, "Failed to start the transfer!\n");
      removeRunningTask(task);
    }
  }
  // 2. start data transfer in full range
  else {
    OMAF_LOG(LOG_INFO, "Start transfer for data downloader for stream size %ld\n", task->streamSize());
    ret = startTransfer(task, task->easy_d_downloader_, task->streamSize(), -1);
    if (ret != ERROR_NONE) {
      OMAF_LOG(LOG_ERROR, "Failed to start the transfer for full range data!\n");
      removeRunningTask(task);
    }
  }
  return ret;
}

OMAF_STATUS OmafCurlMultiDownloader::createTransfer(OmafDownloadTask::Ptr task, OmafCurlEasyDownloader::Ptr& downloader) noexcept {
  try {
    if (task.get() == nullptr || downloader_pool_.get() == nullptr) {
      return ERROR_INVALID;
    }
    // create data downloader
    OmafCurlEasyDownloader::Ptr pDownloader = std::move(downloader_pool_->pop());
    if (pDownloader.get() == nullptr) {
      OMAF_LOG(LOG_ERROR, "Failed to create the curl easy downloader!\n");
      return ERROR_NULL_PTR;
    }

    OMAF_STATUS ret = pDownloader->open(task->url());
    if (ERROR_NONE != ret) {
      OMAF_LOG(LOG_ERROR, "Failed to open the curl easy downloader, err=%d\n", ret);
      // return the downloader to pool
      downloader_pool_->push(std::move(pDownloader));
      return ret;
    }

    downloader = std::move(pDownloader);

    return ERROR_NONE;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when create curl transfer, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafCurlMultiDownloader::startTransfer(OmafDownloadTask::Ptr task, OmafCurlEasyDownloader::Ptr downloader, int64_t offset, int64_t size) noexcept {
  try {
    if (task.get() == nullptr) {
      return ERROR_INVALID;
    }

    OMAF_LOG(LOG_INFO, "To start transfer for url: %s\n", task->url_.c_str());

    if (downloader == nullptr) {
      OMAF_LOG(LOG_ERROR, "The curl easy downloader is empty!\n");
      return ERROR_NULL_PTR;
    }

    OMAF_STATUS ret = ERROR_NONE;
    // multi hanlder will manager the life cycle of curl easy hanlder,
    // so, we won't send the state callback to downloader.
    if (downloader->getType() == OmafCurlEasyDownloader::Type::DATA) {
      ret = downloader->start(
          offset, size,
          [task](std::unique_ptr<StreamBlock> sb) {
            if (task->dcb_) {
              task->stream_size_ += sb->size();
              task->last_stream_size_ += sb->size();
              task->dcb_(std::move(sb));
            }
          },
          nullptr,
          nullptr);
    } else if (downloader->getType() == OmafCurlEasyDownloader::Type::HEADER) {
      ret = downloader->start(
          offset, size,
          nullptr,
          [task](std::unique_ptr<StreamBlock> sb, map<uint32_t, uint32_t>& index_range) {
            if (task->cdcb_) {
              task->cdcb_(std::move(sb), index_range);
            }
          },
          nullptr);
    }

    if (ret != ERROR_NONE) {
      OMAF_LOG(LOG_ERROR, "Failed to start the curl easy downloader!\n");
      return ERROR_INVALID;
    }

    {
      std::lock_guard<std::mutex> lock(run_task_map_mutex_);
      auto handler = downloader->handler();
      if (handler) {
        OMAF_LOG(LOG_INFO, "Add to multi handler transfer for url: %s, handler: %ld\n", task->url_.c_str(), reinterpret_cast<int64_t>(downloader->handler()));
        curl_multi_add_handle(curl_multi_, handler);

        task->state(OmafDownloadTask::State::RUNNING);
        run_task_map_[downloader] = task;
        downloader->setState(OmafCurlEasyDownloader::State::DOWNLOADING);
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

OMAF_STATUS OmafCurlMultiDownloader::removeTransfer(OmafDownloadTask::Ptr task, OmafCurlEasyDownloader::Ptr downloader) noexcept {
  try {
    if (task.get() == nullptr || downloader.get() == nullptr) {
      return ERROR_INVALID;
    }
    OMAF_LOG(LOG_INFO, "Remove transfer for url: %s, handler: %ld\n", task->url_.c_str(), reinterpret_cast<int64_t>(downloader->handler()));
    CURLMcode code = curl_multi_remove_handle(curl_multi_, downloader->handler());
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
    moveDataTaskFromRun(task, OmafDownloadTask::State::STOPPED);
    moveHeaderTaskFromRun(task, OmafDownloadTask::State::STOPPED);

    removeTransfer(task, task->easy_d_downloader_);
    removeTransfer(task, task->easy_h_downloader_);

    return ERROR_NONE;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when remove running task, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafCurlMultiDownloader::markTaskFinish(OmafDownloadTask::Ptr task) noexcept {
  try {
    OMAF_LOG(LOG_INFO, "Task finish, url=%s\n", task->url().c_str());

    moveDataTaskFromRun(task, OmafDownloadTask::State::FINISH);
    moveHeaderTaskFromRun(task, OmafDownloadTask::State::FINISH);
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
    moveDataTaskFromRun(task, OmafDownloadTask::State::TIMEOUT);
    // other logic
    // restart the trasfer when timeout
    if (task->transfer_times_ < curl_params_.http_params_.retry_times_) {
      if (task->enable_byte_range_) {
        int64_t offset = task->header_size_, range_size = 0;
        map<uint32_t, uint32_t> index_range = task->easy_h_downloader_->getIndexRange();
        for (auto index : index_range) {
          if ((int32_t)index.first < task->downloaded_chunk_id_) offset += index.second;
          range_size = index_range[task->downloaded_chunk_id_];
        }
        startTransfer(task, task->easy_d_downloader_, offset, range_size);
      }
      else {
        startTransfer(task, task->easy_d_downloader_, task->streamSize(), -1);
      }
    } else {
      processTaskDone(std::move(task));
    }
    return ERROR_NONE;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when process timeout task, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafCurlMultiDownloader::markTaskContinue(OmafDownloadTask::Ptr task) noexcept {
  try {
    int ret = ERROR_NONE;
    OMAF_LOG(LOG_INFO, "Task continue, url=%s\n", task->url().c_str());
    // remove task from run task and set state to ready
    moveHeaderTaskFromRun(task, OmafDownloadTask::State::RUNNING);
    // trigger data transfer for the next chunk
    TriggerNextDataTransfer(task);
    // restart header downloader transfer
    processTaskContinue(task);

    return ret;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when process finished task, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafCurlMultiDownloader::TriggerNextDataTransfer(OmafDownloadTask::Ptr task) noexcept {
  int ret = ERROR_NONE;
  int32_t avail_chunk_id = -1;
  int64_t offset = task->header_size_, range_size = 0;
  if (task->easy_h_downloader_.get() == nullptr) return ERROR_NULL_PTR;
  map<uint32_t, uint32_t> index_range = task->easy_h_downloader_->getIndexRange();
  for (auto index : index_range) {
    if ((int32_t)index.first <= task->downloaded_chunk_id_) offset += index.second;
    if (index.second == 0)
      break;
    avail_chunk_id = index.first;
  }
  // LOG(INFO) << "Trigger data task " << task->to_string().c_str() << endl;
  OMAF_LOG(LOG_INFO, "avail_chunk_id %d, downloaded_chunk_id_ %d\n", avail_chunk_id, task->downloaded_chunk_id_);
  // LOG(INFO) << "task->easy_d_downloader_->getState() " << (int)task->easy_d_downloader_->getState() << endl;
  OMAF_LOG(LOG_INFO, "task->easy_d_downloader_->handler()%ld\n", reinterpret_cast<int64_t>(task->easy_d_downloader_->handler()));
  if (task->downloaded_chunk_id_ < avail_chunk_id && task->easy_d_downloader_->getState() == OmafCurlEasyDownloader::State::IDLE) {
    OMAF_LOG(LOG_INFO, "Start transfer for data downloader for chunk id %d, task %s\n", task->downloaded_chunk_id_+1, task->to_string().c_str());
    range_size = index_range[++task->downloaded_chunk_id_];
    OMAF_LOG(LOG_INFO, "Start transfer data downloader offset %ld, range size %ld\n", offset, range_size);
    ret = startTransfer(task, task->easy_d_downloader_, offset, range_size);

    if (ret != ERROR_NONE) {
      OMAF_LOG(LOG_ERROR, "Failed to start the transfer for index range data!\n");
      removeRunningTask(task);
    }
  }
  return ret;
}

void OmafCurlMultiDownloader::processTaskDone(OmafDownloadTask::Ptr task) noexcept {
  try {
    if (task.get() == nullptr) {
      OMAF_LOG(LOG_ERROR, "Try to process empty task!\n");
    } else {
      task_size_.fetch_sub(1);

      if (task->easy_d_downloader_) {
        OMAF_LOG(LOG_INFO, "Process data downloader task done\n");
        // return the downloader to pool
        if (downloader_pool_) {
          task->easy_d_downloader_->stop();
          downloader_pool_->push(std::move(task->easy_d_downloader_));
        } else {
          task->easy_d_downloader_->close();
          task->easy_d_downloader_.reset();
        }
      }
      if (task->easy_h_downloader_) {
        OMAF_LOG(LOG_INFO, "Process header downloader task done\n");
        // return the downloader to pool
        if (downloader_pool_) {
          task->easy_h_downloader_->stop();
          downloader_pool_->push(std::move(task->easy_h_downloader_));
        } else {
          task->easy_h_downloader_->close();
          task->easy_h_downloader_.reset();
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

void OmafCurlMultiDownloader::processTaskContinue(OmafDownloadTask::Ptr task) noexcept {
  try {
    if (task.get() == nullptr) {
      OMAF_LOG(LOG_ERROR, "Try to process empty task!\n");
    } else {
      startTransfer(task, task->easy_h_downloader_, 0, task->header_size_);
    }
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when process task done, ex: %s\n", ex.what());
  }
}

OMAF_STATUS OmafCurlMultiDownloader::moveDataTaskFromRun(OmafDownloadTask::Ptr task,
                                                     OmafDownloadTask::State to_state) noexcept {
  try {
    std::lock_guard<std::mutex> lock(run_task_map_mutex_);
    auto downloader_d = task->easy_d_downloader_;
    auto find_d = run_task_map_.find(downloader_d);
    if (find_d != run_task_map_.end()) {
      run_task_map_.erase(find_d);
      task->state(to_state);
    }

    return ERROR_NONE;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when task state from run to %d, ex: %s\n", static_cast<OMAF_STATUS>(to_state), ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafCurlMultiDownloader::moveHeaderTaskFromRun(OmafDownloadTask::Ptr task,
                                                     OmafDownloadTask::State to_state) noexcept {
  try {
    std::lock_guard<std::mutex> lock(run_task_map_mutex_);
    auto downloader_h = task->easy_h_downloader_;
    auto find_h = run_task_map_.find(downloader_h);
    if (find_h != run_task_map_.end()) {
      run_task_map_.erase(find_h);
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

      ProcessDataTasks();
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
        ret = createTransferForTask(task);
        if (ret == ERROR_NONE) {
          ret = startTransferForTask(task);
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
        auto run_task_iterator = run_task_map_.begin();
        OmafDownloadTask::Ptr task;
        //1. find corresponding task and downloaders according to msg->easy_handle
        {
          std::lock_guard<std::mutex> lock(run_task_map_mutex_);
          for ( ;run_task_iterator != run_task_map_.end(); run_task_iterator++) {
            if (run_task_iterator->first->handler() == handler) {
              task = run_task_iterator->second;
              break;
            }
          }
        }

        if (task.get() != nullptr && run_task_iterator != run_task_map_.end()) {
          OMAF_LOG(LOG_INFO, "3-task id %lld, task count=%d\n", task->id(), task.use_count());
          OmafCurlEasyDownloader::Ptr downloader = run_task_iterator->first;
          // 2. remove transfer from task
          removeTransfer(task, downloader);
          // 3. mark task finish/continue/timeout
          auto header = downloader->header();
          OMAF_LOG(LOG_INFO, "Header content length=%lld\n", header.content_length_);
          // 3.1 disable byte range mode, to mark task finish or timeout
          if (!task->enable_byte_range_) {
            if (OmafCurlEasyHelper::success(header.http_status_code_) && (header.content_length_ == task->streamSize())) {
              markTaskFinish(std::move(task));
            } else {
              markTaskTimeout(std::move(task));
            }
          }
          // 3.2 enable byte range mode, to mark task finish, continue, timeout
          else {
            // data downloader to determine whether the task is finished
            if (task->easy_d_downloader_ == downloader) {
              OMAF_LOG(LOG_INFO, "Task last stream size %ld\n", task->lastStreamSize());
              if (OmafCurlEasyHelper::success(header.http_status_code_) && header.content_length_ == task->lastStreamSize()) {
                OMAF_LOG(LOG_INFO, "Downloaded chunk id %d, chunk_num %d\n", task->downloaded_chunk_id_, task->chunk_num_);
                if (task->downloaded_chunk_id_ == (int32_t)task->chunk_num_ - 1) markTaskFinish(std::move(task));
                else {
                  task->last_stream_size_ = 0;
                  task->easy_d_downloader_->setState(OmafCurlEasyDownloader::State::IDLE);
                  moveDataTaskFromRun(task, OmafDownloadTask::State::RUNNING);
                }
              }
              else markTaskTimeout(std::move(task));
            }
            // header downloader to continue header handler or push back to pending_data_tasks_ list
            else if (task->easy_h_downloader_ == downloader) {
              map<uint32_t, uint32_t> index_range = task->easy_h_downloader_->getIndexRange();
              // index information is completed
              if (index_range.find(task->chunk_num_-1) != index_range.end() && index_range[task->chunk_num_-1] != 0) {
                std::lock_guard<std::mutex> lock(pending_data_task_list_mutex);
                OMAF_LOG(LOG_INFO, "Add task %s to pending data tasks\n", task->to_string().c_str());
                pending_data_tasks_.push_back(std::move(task));
              }
              else {
                markTaskContinue(std::move(task));
              }
            }
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
}
OMAF_STATUS OmafCurlMultiDownloader::ProcessDataTasks() noexcept {
  std::lock_guard<std::mutex> lock(pending_data_task_list_mutex);
  for (auto it = pending_data_tasks_.begin(); it != pending_data_tasks_.end();) {
    OmafDownloadTask::Ptr task = *it;
    TriggerNextDataTransfer(task);
    // LOG(INFO) << "Task->chunk num " << task->chunk_num_ << endl;
    if (task->downloaded_chunk_id_ == (int32_t)task->chunk_num_ - 1) {
      OMAF_LOG(LOG_INFO, "Erase task %s from pending data tasks\n", task->to_string().c_str());
      it = pending_data_tasks_.erase(it);
    }
    else ++it;
  }
  return ERROR_NONE;
}

}  // namespace OMAF
}  // namespace VCD
