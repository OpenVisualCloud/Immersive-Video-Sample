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

/*
 * File:   OmafSegment.cpp
 * Author: media
 *
 * Created on May 24, 2019, 11:07 AM
 */

#include "DownloadManager.h"
#include "OmafSegment.h"

#include <fstream>
#include <sstream>

VCD_OMAF_BEGIN

std::atomic_uint32_t OmafSegment::INITSEG_ID(0);

OmafSegment::OmafSegment(DashSegmentSourceParams ds_params, int segCnt, bool bInitSegment)
    : ds_params_(ds_params), seg_count_(segCnt), bInit_segment_(bInitSegment) {
  if (bInit_segment_) {
    initSeg_id_ = INITSEG_ID.fetch_add(1);
    seg_id_ = initSeg_id_;
  }
  mQualityRanking = INVALID_QUALITY_RANKING;
  mMediaType = MediaType_Video;
  mChlsNum = 0;
  mSampleRate = 0;
}

OmafSegment::~OmafSegment() {
  if (buse_stored_file_ && !cache_file_.empty()) {
    DOWNLOADMANAGER::GetInstance()->DeleteCacheFile(cache_file_);
  }
}

int OmafSegment::Open(std::shared_ptr<OmafDashSegmentClient> dash_client) noexcept {
  try {
    if (dash_client.get() == nullptr) {
      return ERROR_NULL_PTR;
    }

    dash_client_ = std::move(dash_client);

    state_ = State::CREATE;

    // mSegElement->StartDownloadSegment((OmafDownloaderObserver *)this);
    dash_client_->open(
        ds_params_, [this](std::unique_ptr<VCD::OMAF::StreamBlock> sb) { this->dash_stream_.push_back(std::move(sb)); },
        [this](OmafDashSegmentClient::State s) {
          switch (s) {
            case OmafDashSegmentClient::State::SUCCESS:
              this->state_ = State::OPEN_SUCCES;
              break;
            case OmafDashSegmentClient::State::STOPPED:
              this->state_ = State::OPEN_STOPPED;
              break;
            case OmafDashSegmentClient::State::TIMEOUT:
              this->state_ = State::OPEN_TIMEOUT;
              break;
            case OmafDashSegmentClient::State::FAILURE:
              this->state_ = State::OPEN_FAILED;
              break;
            default:
              break;
          }
          if (this->state_change_cb_) {
            this->state_change_cb_(this->shared_from_this(), this->state_);
          }
        });
    return ERROR_NONE;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when start downloading the file: %s, ex: %s\n", ds_params_.dash_url_.c_str(), ex.what());
    return ERROR_INVALID;
  }
}

int OmafSegment::Stop() noexcept {
  try {
    if (dash_client_.get() == nullptr) {
      return ERROR_NULL_PTR;
    }
    dash_client_->remove(ds_params_);
    return ERROR_NONE;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when start downloading the file: %s, ex: %s\n", ds_params_.dash_url_.c_str(), ex.what());
    return ERROR_INVALID;
  }
}

#if 0
int OmafSegment::Read(uint8_t *data, size_t len) {
  // if (NULL == mSegElement) return ERROR_NULL_PTR;

  // if (mStatus != SegDownloaded) WaitComplete();

  // return mSegElement->Read(data, len);
}

int OmafSegment::Peek(uint8_t *data, size_t len) {
  // if (NULL == mSegElement) return ERROR_NULL_PTR;

  // if (mStatus != SegDownloaded) WaitComplete();

  // return mSegElement->Peek(data, len);
}

int OmafSegment::Peek(uint8_t *data, size_t len, size_t offset) {
  // if (NULL == mSegElement) return ERROR_NULL_PTR;

  // if (mStatus != SegReady) WaitComplete();

  // return mSegElement->Peek(data, len, offset);
}


int OmafSegment::Close() {
  // if (NULL == mSegElement) return ERROR_NULL_PTR;

  // if (mStatus != SegDownloading)
  //   mSegElement->StopDownloadSegment((OmafDownloaderObserver *)this);

  // SAFE_DELETE( mSegElement );

  return ERROR_NONE;
}
#endif
int OmafSegment::CacheToFile() noexcept {
  try {
    std::string fileName =
        ds_params_.dash_url_.substr(ds_params_.dash_url_.find_last_of('/') + 1,
                                    ds_params_.dash_url_.length() - ds_params_.dash_url_.find_last_of('/') - 1);
    cache_file_ = DOWNLOADMANAGER::GetInstance()->GetCacheFolder() + "/" +
                  DOWNLOADMANAGER::GetInstance()->AssignCacheFileName() + fileName;
    if (!dash_stream_.cacheToFile(cache_file_)) {
      OMAF_LOG(LOG_ERROR, "Failed to cache the dash to file: %s\n", cache_file_.c_str());
      return OMAF_ERROR_FILE_WRITE;
    }

    OMAF_LOG(LOG_INFO, "Success to cache dash to file: %s. url=%s\n", cache_file_.c_str(), ds_params_.dash_url_.c_str());
    OMAF_LOG(LOG_INFO, "And file size=%ld\n", dash_stream_.GetStreamSize());
    return ERROR_NONE;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when cache dash to file: %s\n", cache_file_.c_str());
    return ERROR_INVALID;
  }
}

std::string OmafSegment::to_string() const noexcept {
  std::stringstream ss;
  ss << "segment initsegId=" << initSeg_id_;
  ss << ", segId=" << seg_id_;
  ss << ", url=" << ds_params_.dash_url_;
  ss << ", file=" << cache_file_;
  return ss.str();
}
VCD_OMAF_END
