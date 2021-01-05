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
//! \file:   OmafCurlEasyHandler.cpp
//! \brief:  curl easy handlers class implementation
//!
#include "OmafCurlEasyHandler.h"

#include <sstream>

//#include "../../utils/GlogWrapper.h"  // GLOG

namespace VCD {
namespace OMAF {

HttpHeader OmafCurlEasyHelper::header(CURL *easy_curl) noexcept {
  try {
    HttpHeader header;
    if (easy_curl == nullptr) {
      return header;
    }

    long ldata = -1;
    CURLcode res;
    res = curl_easy_getinfo(easy_curl, CURLINFO_RESPONSE_CODE, &ldata);
    if (CURLE_OK == res) {
      header.http_status_code_ = ldata;
    }
    curl_off_t cl;
    res = curl_easy_getinfo(easy_curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &cl);
    if (CURLE_OK == res) {
      header.content_length_ = static_cast<int64_t>(cl);
    }
    return header;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when set params for curl easy handler, ex: %s\n", ex.what());
    return HttpHeader();
  }
}

double OmafCurlEasyHelper::speed(CURL *easy_curl) noexcept {
  try {
    if (easy_curl == nullptr) {
      return 0.0f;
    }
    curl_off_t speed;
    CURLcode res = curl_easy_getinfo(easy_curl, CURLINFO_SPEED_DOWNLOAD_T, &speed);
    if (CURLE_OK == res) {
      return static_cast<double>(speed) * 8;
    } else {
      return 0.0f;
    }

  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when get speed for curl easy handler, ex: %s\n", ex.what());
    return 0.0f;
  }
}

OMAF_STATUS OmafCurlEasyHelper::setParams(CURL *easy_curl, CurlParams params) noexcept {
  try {
    if (easy_curl == nullptr) {
      return ERROR_NULL_PTR;
    }

    if (!params.http_params_.bssl_verify_peer_) {
      curl_easy_setopt(easy_curl, CURLOPT_SSL_VERIFYPEER, 0L);
    }
    if (!params.http_params_.bssl_verify_host_) {
      curl_easy_setopt(easy_curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }

    if (params.http_params_.conn_timeout_ > 0) {
      curl_easy_setopt(easy_curl, CURLOPT_CONNECTTIMEOUT_MS, params.http_params_.conn_timeout_);
    }
    if (params.http_params_.total_timeout_ > 0) {
      curl_easy_setopt(easy_curl, CURLOPT_TIMEOUT_MS, params.http_params_.total_timeout_);
    }

    if (!params.http_proxy_.http_proxy_.empty()) {
      curl_easy_setopt(easy_curl, CURLOPT_PROXY, params.http_proxy_.http_proxy_.c_str());
      curl_easy_setopt(easy_curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
    }
    if (!params.http_proxy_.https_proxy_.empty()) {
      curl_easy_setopt(easy_curl, CURLOPT_PROXY, params.http_proxy_.https_proxy_.c_str());
      curl_easy_setopt(easy_curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTPS);
    }
    if (!params.http_proxy_.no_proxy_.empty()) {
      curl_easy_setopt(easy_curl, CURLOPT_NOPROXY, params.http_proxy_.no_proxy_.c_str());
    }
    if (!params.http_proxy_.proxy_user_.empty()) {
      curl_easy_setopt(easy_curl, CURLOPT_USERNAME, params.http_proxy_.proxy_user_.c_str());
    }
    if (!params.http_proxy_.proxy_passwd_.empty()) {
      curl_easy_setopt(easy_curl, CURLOPT_PASSWORD, params.http_proxy_.proxy_passwd_.c_str());
    }
    return ERROR_NONE;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when set params for curl easy handler, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

inline long OmafCurlEasyHelper::namelookupTime(CURL *easy_curl) noexcept {
  curl_off_t timev = 0;
  curl_easy_getinfo(easy_curl, CURLINFO_NAMELOOKUP_TIME_T, &timev);

  return static_cast<long>(timev);
}
inline long OmafCurlEasyHelper::connectTime(CURL *easy_curl) noexcept {
  curl_off_t timev = 0;
  curl_easy_getinfo(easy_curl, CURLINFO_CONNECT_TIME_T, &timev);

  return static_cast<long>(timev);
}
inline long OmafCurlEasyHelper::appConnectTime(CURL *easy_curl) noexcept {
  curl_off_t timev = 0;
  curl_easy_getinfo(easy_curl, CURLINFO_APPCONNECT_TIME_T, &timev);

  return static_cast<long>(timev);
}
inline long OmafCurlEasyHelper::preTransferTime(CURL *easy_curl) noexcept {
  curl_off_t timev = 0;
  curl_easy_getinfo(easy_curl, CURLINFO_PRETRANSFER_TIME_T, &timev);

  return static_cast<long>(timev);
}
inline long OmafCurlEasyHelper::startTransferTime(CURL *easy_curl) noexcept {
  curl_off_t timev = 0;
  curl_easy_getinfo(easy_curl, CURLINFO_STARTTRANSFER_TIME_T, &timev);

  return static_cast<long>(timev);
}

inline long OmafCurlEasyHelper::redirectTime(CURL *easy_curl) noexcept {
  curl_off_t timev = 0;
  curl_easy_getinfo(easy_curl, CURLINFO_REDIRECT_TIME_T, &timev);

  return static_cast<long>(timev);
}
void OmafCurlEasyHelper::curlTime(CURL *easy_curl, CurlTimes &curl_time) noexcept {
  curl_time.nameloopup_ = namelookupTime(easy_curl);
  curl_time.connect_ = connectTime(easy_curl);
  curl_time.app_connect_ = appConnectTime(easy_curl);
  curl_time.pre_transfer_ = preTransferTime(easy_curl);
  curl_time.start_transfer_ = startTransferTime(easy_curl);
  curl_time.total_ = totalTime(easy_curl);
  curl_time.redirect_ = redirectTime(easy_curl);
}

OmafCurlEasyDownloader::~OmafCurlEasyDownloader() { close(); }

OMAF_STATUS OmafCurlEasyDownloader::init(const CurlParams &params) noexcept {
  try {
    OMAF_STATUS ret = ERROR_NONE;
    std::lock_guard<std::mutex> lock(easy_curl_mutex_);
    if (easy_curl_) {
      ret = close();
      if (ret != ERROR_NONE) {
        OMAF_LOG(LOG_ERROR, "Failed to clean older easy handle! err=%d\n", ret);
        return ret;
      }
    }

    easy_curl_ = curl_easy_init();
    if (easy_curl_ == nullptr) {
      OMAF_LOG(LOG_ERROR, "Failed to create the curl easy handler!\n");
      return ERROR_NULL_PTR;
    }
    curl_params_ = params;
    return ERROR_NONE;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when create the curl easy hanlder, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafCurlEasyDownloader::open(const std::string &url) noexcept {
  try {
    std::lock_guard<std::mutex> lock(easy_curl_mutex_);
    url_ = url;
    OMAF_LOG(LOG_INFO, "To open the url: %s\n", url.c_str());
    if (easy_curl_ == nullptr) {
      OMAF_LOG(LOG_ERROR, "curl easy handler is invalid!\n");
      return ERROR_NULL_PTR;
    }

    curl_easy_reset(easy_curl_);
    OMAF_STATUS ret = OmafCurlEasyHelper::setParams(easy_curl_, curl_params_);
    if (ERROR_NONE != ret) {
      OMAF_LOG(LOG_ERROR, "Failed to set params for easy curl handler!\n");
      return ret;
    }

    curl_easy_setopt(easy_curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(easy_curl_, CURLOPT_PRIVATE, url.c_str());
    curl_easy_setopt(easy_curl_, CURLOPT_WRITEFUNCTION, curlBodyCallback);
    curl_easy_setopt(easy_curl_, CURLOPT_WRITEDATA, (void *)this);

    return ERROR_NONE;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when set options for curl easy hanlder, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafCurlEasyDownloader::start(int64_t offset, int64_t size, onData dcb, onState scb) noexcept {
  try {
    std::lock_guard<std::mutex> lock(easy_curl_mutex_);
    if (offset > 0 || size > 0) {
      std::stringstream ss;
      if (offset > 0) {
        ss << offset;
      }
      ss << "-";
      if (size > 0) {
        ss << size;
      }
      // OMAF_LOG(LOG_INFO, "To download the range: %s\n", ss.str());
      curl_easy_setopt(easy_curl_, CURLOPT_RANGE, ss.str().c_str());
    }
    dcb_ = dcb;
    scb_ = scb;

    // TODO, easy mode
    if (work_mode_ == CurlWorkMode::EASY_MODE) {
      CURLcode res = curl_easy_perform(easy_curl_);

      if (res != CURLE_OK) {
        OMAF_LOG(LOG_ERROR, "Failed to download the url: %s\n", url_.c_str());
        if (scb_) {
          scb_(OmafCurlEasyDownloader::State::FAILED);
        }
        return ERROR_DOWNLOAD_FAIL;
      } else {
        if (scb_) {
          scb_(OmafCurlEasyDownloader::State::SUCCESS);
        }
        return ERROR_NONE;
      }
    }

    return ERROR_NONE;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when start curl easy hanlder, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

OMAF_STATUS OmafCurlEasyDownloader::stop() noexcept {
  {
    std::lock_guard<std::mutex> lock(cb_mutex_);
    dcb_ = nullptr;
    scb_ = nullptr;
  }

  return ERROR_NONE;
}

OMAF_STATUS OmafCurlEasyDownloader::close() noexcept {
  try {
    stop();

    std::lock_guard<std::mutex> lock(easy_curl_mutex_);

    if (easy_curl_) {
      curl_easy_cleanup(easy_curl_);
      easy_curl_ = nullptr;
    }
    return ERROR_NONE;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when close curl easy hanlder, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}

HttpHeader OmafCurlEasyDownloader::header() noexcept {
  try {
    std::lock_guard<std::mutex> lock(easy_curl_mutex_);
    return OmafCurlEasyHelper::header(this->easy_curl_);
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when read header, ex: %s\n", ex.what());
    return HttpHeader();
  }
}

double OmafCurlEasyDownloader::speed() noexcept {
  try {
    std::lock_guard<std::mutex> lock(easy_curl_mutex_);
    return OmafCurlEasyHelper::speed(this->easy_curl_);
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when read average speed, ex: %s\n", ex.what());
    return 0.0f;
  }
}

void OmafCurlEasyDownloader::receiveSB(std::unique_ptr<StreamBlock> sb) noexcept {
  try {
    {
      std::lock_guard<std::mutex> lock(cb_mutex_);
      if (dcb_ != nullptr) {
        dcb_(std::move(sb));
      }
    }

  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when process stream block! ex: %s\n", ex.what());
  }
}

size_t OmafCurlEasyDownloader::curlBodyCallback(char *ptr, size_t size, size_t nmemb, void *userdata) noexcept {
  size_t bsize = size * nmemb;

  try {
    //OMAF_LOG(LOG_INFO, "Receive bytes size= %lld\n", bsize);
    if (ptr == nullptr || bsize <= 0) {
      OMAF_LOG(LOG_ERROR, "The buffer from curl handler is empty!\n");
      return bsize;
    }
    OmafCurlEasyDownloader *phandler = reinterpret_cast<OmafCurlEasyDownloader *>(userdata);
    if (phandler == nullptr) {
      OMAF_LOG(LOG_ERROR, "The OmafCurlEasyDownloader invalid handler!\n");
      return bsize;
    }
    std::unique_ptr<StreamBlock> sb = make_unique_vcd<StreamBlock>();
    if (!sb->resize(bsize)) {
      OMAF_LOG(LOG_ERROR, "Failed to allocate the target buffer for curl download data!\n");
      return bsize;
    }
    // FIXME, use security memcpy_s
    memcpy_s(sb->buf(), sb->capacity(), ptr, bsize);
    sb->size(bsize);

    phandler->receiveSB(std::move(sb));
    return bsize;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when receive data from curl easy hanlder, ex: %s\n", ex.what());
    return bsize;
  }
}
OmafCurlEasyDownloaderPool::~OmafCurlEasyDownloaderPool() {
  try {
    std::lock_guard<std::mutex> lock(easy_downloader_pool_mutex_);
    while (easy_downloader_pool_.size()) {
      auto downloader = std::move(easy_downloader_pool_.front());
      downloader->close();
      easy_downloader_pool_.pop();
    }
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when release all easy downloader! ex=%s\n", ex.what());
  }
}

OmafCurlEasyDownloader::Ptr OmafCurlEasyDownloaderPool::pop() noexcept {
  try {
    // 1. check and get the downloader from the pool
    {
      std::lock_guard<std::mutex> lock(easy_downloader_pool_mutex_);
      if (easy_downloader_pool_.size()) {
        OmafCurlEasyDownloader::Ptr downloader = std::move(easy_downloader_pool_.front());
        downloader->params(curl_params_);
        easy_downloader_pool_.pop();
        return std::move(downloader);
      }
      if (downloader_count_ >= max_downloader_) {
        return nullptr;
      }
    }
    // 2. create a new downloader
    OmafCurlEasyDownloader::Ptr downloader = std::make_shared<OmafCurlEasyDownloader>();
    if (ERROR_NONE == downloader->init(curl_params_)) {
      return std::move(downloader);
    }
    return nullptr;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when request one easy downloader! ex=%s\n", ex.what());
    return nullptr;
  }
}
void OmafCurlEasyDownloaderPool::push(OmafCurlEasyDownloader::Ptr downloader) noexcept {
  try {
    std::lock_guard<std::mutex> lock(easy_downloader_pool_mutex_);
    downloader->stop();
    easy_downloader_pool_.push(downloader);
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when return one easy downloader! ex=%s\n", ex.what());
  }
}

OmafCurlChecker::~OmafCurlChecker() { close(); }

OMAF_STATUS OmafCurlChecker::init(const CurlParams &params) noexcept {
  try {
    easy_curl_ = curl_easy_init();
    if (easy_curl_ == nullptr) {
      OMAF_LOG(LOG_ERROR, "Failed to create the curl easy handler!\n");
      return ERROR_NULL_PTR;
    }
    OMAF_STATUS ret = OmafCurlEasyHelper::setParams(easy_curl_, params);
    if (ERROR_NONE != ret) {
      OMAF_LOG(LOG_ERROR, "Failed to set params for easy curl handler!\n");
      return ret;
    }
    curl_easy_setopt(easy_curl_, CURLOPT_NOBODY, 1L);
    return ERROR_NONE;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when create the curl easy hanlder, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}
OMAF_STATUS OmafCurlChecker::check(const std::string &url) noexcept {
  try {
    if (easy_curl_ == nullptr) {
      OMAF_LOG(LOG_ERROR, "curl easy handler is invalid!\n");
      return ERROR_INVALID;
    }

    curl_easy_setopt(easy_curl_, CURLOPT_URL, url.c_str());

    CURLcode res = curl_easy_perform(easy_curl_);
    if (CURLE_OK == res) {
      long response_code = 0;
      curl_easy_getinfo(easy_curl_, CURLINFO_RESPONSE_CODE, &response_code);
      return OmafCurlEasyHelper::success(response_code);
    }
    return ERROR_INVALID;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when call curl easy handler, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}
OMAF_STATUS OmafCurlChecker::close() noexcept {
  try {
    OMAF_LOG(LOG_INFO, "To close the curl checker!\n");
    if (easy_curl_) {
      curl_easy_cleanup(easy_curl_);
      easy_curl_ = nullptr;
    }
    return ERROR_NONE;
  } catch (const std::exception &ex) {
    OMAF_LOG(LOG_ERROR, "Exception when close curl easy hanlder, ex: %s\n", ex.what());
    return ERROR_INVALID;
  }
}
}  // namespace OMAF
}  // namespace VCD
