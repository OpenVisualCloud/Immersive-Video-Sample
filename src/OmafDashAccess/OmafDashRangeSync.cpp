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
//! \file:   OmafDashRangeSync.cpp
//! \brief:
//! \detail:
//! Created on May 22, 2019, 3:18 PM
//!

#include "OmafDashRangeSync.h"

#include <chrono>

#include "OmafAdaptationSet.h"

VCD_OMAF_BEGIN

class OmafDashRangeSyncImpl : public OmafDashRangeSync {
 public:
  OmafDashRangeSyncImpl(const OmafAdaptationSet& oas, SegmentSyncNodeCB cb) : adaptation_set_(oas), sync_cb_(cb){};
  virtual ~OmafDashRangeSyncImpl() {}

 public:
  virtual std::string getUrl(const SegmentSyncNode& value) const override;
  virtual SegmentSyncNode getSegmentNode() override {
    SegmentSyncNode node;
    node.segment_value.number_ = adaptation_set_.GetSegmentNumber();
    return node;
  };
  virtual int64_t getStartSegment() override;
  virtual void notifyRangeChange(SyncRange range) override;

 private:
  const OmafAdaptationSet& adaptation_set_;
  SegmentSyncNodeCB sync_cb_;
};

OmafDashRangeSync::Ptr make_omaf_syncer(const OmafAdaptationSet& oas, SegmentSyncNodeCB cb) {
  return std::make_shared<OmafDashRangeSyncImpl>(oas, cb);
}

std::string OmafDashRangeSyncImpl::getUrl(const SegmentSyncNode& value) const { return adaptation_set_.GetUrl(value); };

int64_t OmafDashRangeSyncImpl::getStartSegment() {
  return 0;  // return adaptation_set_.GetSegmentNumber();
};

void OmafDashRangeSyncImpl::notifyRangeChange(SyncRange range) {
  int64_t number = adaptation_set_.GetSegmentNumber();
  if (number < range.left_) {
    OMAF_LOG(LOG_INFO, "slower than server, reset segment number to left range [%ld, %ld], segment number=%ld\n", range.left_, range.right_,  number);
    if (sync_cb_) {
      SegmentSyncNode node;
      node.segment_value.number_ = range.left_;
      sync_cb_(node);
    }
  } else if (number > range.right_) {
    OMAF_LOG(LOG_INFO, "faster than server, reset segment number to right range [%ld, %ld], segment number=%ld\n", range.left_, range.right_, number);
    if (sync_cb_) {
      SegmentSyncNode node;
      node.segment_value.number_ = range.right_;
      sync_cb_(node);
    }
  }
};

int OmafDashSourceSyncHelper::start(CurlParams params) noexcept {
  try {
    checker_.reset(new OmafCurlChecker());
    int ret = ERROR_NONE;

    ret = checker_->init(params);
    if (ERROR_NONE != ret) {
      OMAF_LOG(LOG_ERROR, "Failed to init the curl checker with error: %d\n", ret);
      return ret;
    }
    bsyncing_ = true;
    sync_worker_ = std::thread(&OmafDashSourceSyncHelper::threadRunner, this);

    return ERROR_NONE;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when start the dash source sync, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

int OmafDashSourceSyncHelper::stop() noexcept {
  try {
    bsyncing_ = false;
    if (sync_worker_.joinable()) {
      bsyncing_ = false;
      sync_worker_.join();
    }
    return ERROR_NONE;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when stop the dash source sync, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

void OmafDashSourceSyncHelper::addSyncer(OmafDashRangeSync::Ptr syncer) noexcept {
  syncers_range_.emplace_back(nullptr);
  syncers_.push_back(std::move(syncer));
}

void OmafDashSourceSyncHelper::threadRunner() noexcept {
  try {
    while (bsyncing_) {
      int syncer_index = 0;
      auto start = std::chrono::high_resolution_clock::now();

      for (auto syncer : syncers_) {
        if (!bsyncing_) {
          break;
        }

        auto& range = syncers_range_[syncer_index++];

        if (range.get() == nullptr) {
          range.reset(new SyncRange());
          if (!initRange(syncer, range)) {
            OMAF_LOG(LOG_ERROR, "Failed to initialize the sync range!\n" );
            range.reset();
          }
        } else {
          if (!updateRange(syncer, range)) {
            OMAF_LOG(LOG_ERROR, "Failed to update the sync range!\n");
            range.reset();
          }
        }
        if (range.get() != nullptr) {
          syncer->notifyRangeChange(*range.get());
        }

      }  // loop all syncer

      // 2. sync frequncy logic
      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> elapsed = end - start;
      int64_t sleep_time = sync_frequency_ - static_cast<int64_t>(elapsed.count() * 1000);
      while (bsyncing_ && sleep_time > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        sleep_time--;
      }
    }  // end thread while(bsyncing_)
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception in the dash source sync runner, ex: %s\n", ex.what());
  }
}

bool OmafDashSourceSyncHelper::initRange(OmafDashRangeSync::Ptr syncer, std::shared_ptr<SyncRange> range) noexcept {
  try {
    SegmentSyncNode syncnode = syncer->getSegmentNode();
    OMAF_LOG(LOG_INFO, "Calling initRange from start point: %ld\n", syncnode.segment_value.number_);
    int64_t left_check_start = syncnode.segment_value.number_;
    int64_t right_check_start = syncnode.segment_value.number_;
    int32_t check_times = 0;
    Direction direction = Direction::RIGHT;
    bool meetleft = false;
    int64_t point = 0;
    bool bfind = true;
    while (bsyncing_ && check_times < check_range_times_) {
      if (direction == Direction::RIGHT) {
        bfind = findRange(syncer, right_check_start, direction, point);
        if (bfind) {
          bfind = findRangeEdge(syncer, point, range);
          break;
        } else {
          if (!meetleft) {
            direction = Direction::LEFT;
          }
          right_check_start += range_size_ * check_range_strides_;
        }
      } else {
        bfind = findRange(syncer, left_check_start, direction, point);
        if (bfind) {
          bfind = findRangeEdge(syncer, point, range);
          break;
        } else if (point == MEET_LEFT) {
          meetleft = true;
        }
        direction = Direction::RIGHT;
        left_check_start -= range_size_ * check_range_strides_;
      }
    }

    return bfind;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception create the range, ex: %s\n", ex.what());
    return false;
  }
}
bool OmafDashSourceSyncHelper::findRange(OmafDashRangeSync::Ptr syncer, int64_t check_start, Direction direction,
                                         int64_t& point) noexcept {
  try {
    SegmentSyncNode syncnode = syncer->getSegmentNode();
    int64_t index = 0;
    while (bsyncing_ && (index < check_range_strides_)) {
      if (direction == Direction::RIGHT) {
        syncnode.segment_value.number_ = check_start + index * range_size_;
      } else {
        syncnode.segment_value.number_ = check_start - index * range_size_;
        if (syncnode.segment_value.number_ <= syncer->getStartSegment()) {
          point = MEET_LEFT;
          return false;
        }
      }
      std::string url = syncer->getUrl(syncnode);
      OMAF_LOG(LOG_INFO, "To check the url: %s\n", url.c_str());
      if (checker_->check(url)) {
        point = syncnode.segment_value.number_;
        return true;
      }
      index++;
    }
    return false;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception create the range, ex: %s\n", ex.what());
    return false;
  }
}

bool OmafDashSourceSyncHelper::findRangeEdge(OmafDashRangeSync::Ptr syncer, int64_t point,
                                             std::shared_ptr<SyncRange> range) noexcept {
  try {
    SegmentSyncNode syncnode = syncer->getSegmentNode();

    // 1. find right
    int64_t index = 0;
    bool pre_valid = false;
    while (bsyncing_ && (index < range_size_)) {
      syncnode.segment_value.number_ = point + index;
      std::string url = syncer->getUrl(syncnode);
      bool bvalid = checker_->check(url);
      if (!bvalid && pre_valid) {
        break;
      }
      pre_valid = bvalid;
      index++;
    }
    if (!pre_valid) {
      OMAF_LOG(LOG_ERROR, "Failed to find the range right edge!\n");
      return false;
    }

    range->right_ = point + index - 1;

    // 2. find the left,
    // FIXME optimize the search direction in the future
    index = 0;
    point = range->right_ - range_size_;
    while (bsyncing_ && (index < range_size_)) {
      syncnode.segment_value.number_ = point + index;
      std::string url = syncer->getUrl(syncnode);
      if (checker_->check(url)) {
        break;
      }
      index++;
    }
    range->left_ = point + index;
    return true;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception create the range, ex: %s\n", ex.what());
    return false;
  }
}
bool OmafDashSourceSyncHelper::updateRange(OmafDashRangeSync::Ptr syncer, std::shared_ptr<SyncRange> range) noexcept {
  try {
    SegmentSyncNode syncnode = syncer->getSegmentNode();

    // 1. update the left,
    int64_t index = 0;
    while (bsyncing_ && (index < range_size_)) {
      syncnode.segment_value.number_ = range->left_ + index;
      std::string url = syncer->getUrl(syncnode);
      if (checker_->check(url)) {
        break;
      }
      index++;
    }
    range->left_ = range->left_ + index;

    // 2. update right
    index = 0;
    range->right_ = range->left_ + range_size_;
    while (bsyncing_ && (index < range_size_)) {
      syncnode.segment_value.number_ = range->right_ + index;
      std::string url = syncer->getUrl(syncnode);
      if (!checker_->check(url)) {
        break;
      }
      index++;
    }

    range->right_ = range->right_ + index - 1;
    return true;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception create the range, ex: %s\n", ex.what());
    return false;
  }
}

VCD_OMAF_END
