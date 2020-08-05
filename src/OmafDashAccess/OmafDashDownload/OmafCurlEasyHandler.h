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
//! \file:   OmafCurlEasyDownloader.h
//! \brief:  curl easy handlers class
//!
#ifndef OMAFCURLEASYWRAPPER_H
#define OMAFCURLEASYWRAPPER_H
#include <curl/curl.h>

#include "../common.h"           // VCD::NonCopyable
#include "../../utils/ns_def.h"  // namespace VCD::OMAF
#include "../OmafTypes.h"
#include "OmafDownloader.h"  // VCD::OMAF::State, VCD::OMAF::DownloadPolicy
#include "Stream.h"          // VCD::OMAF::StreamBlock

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <queue>

// namespace
namespace VCD {
namespace OMAF {

struct _curlParams {
  OmafDashHttpProxy http_proxy_;
  OmafDashHttpParams http_params_;
  std::string to_string() {
    std::stringstream ss;
    ss << "curl params: {" << std::endl;
    ss << "\t" << http_proxy_.to_string() << std::endl;
    ss << "\t" << http_params_.to_string() << std::endl;
    ss << "}";
    return ss.str();
  }
};

using CurlParams = struct _curlParams;

struct _httpHeader {
  long http_status_code_ = -1;
  int64_t content_length_ = -1;
};

using HttpHeader = struct _httpHeader;

struct _curlTimes {
  long nameloopup_ = 0;
  long connect_ = 0;
  long app_connect_ = 0;
  long pre_transfer_ = 0;
  long start_transfer_ = 0;
  long total_ = 0;
  long redirect_ = 0;
};
using CurlTimes = struct _curlTimes;

class OmafCurlEasyDownloaderPool;

class OmafCurlEasyHelper : public NonCopyable {
 protected:
  OmafCurlEasyHelper() = default;

 public:
  virtual ~OmafCurlEasyHelper(){};

 public:
  static HttpHeader header(CURL *easy_curl) noexcept;
  static double speed(CURL *easy_curl) noexcept;
  static OMAF_STATUS setParams(CURL *easy_curl, CurlParams parmas) noexcept;
  static bool success(const long http_status_code_) noexcept {
    return http_status_code_ >= 200 && http_status_code_ < 300;
  }
  static long namelookupTime(CURL *easy_curl) noexcept;
  static long connectTime(CURL *easy_curl) noexcept;
  static long appConnectTime(CURL *easy_curl) noexcept;
  static long preTransferTime(CURL *easy_curl) noexcept;
  static long startTransferTime(CURL *easy_curl) noexcept;

  static long redirectTime(CURL *easy_curl) noexcept;
  static void curlTime(CURL *easy_curl, CurlTimes &) noexcept;
  static long totalTime(CURL *easy_curl) noexcept {
    curl_off_t timev = 0;
    curl_easy_getinfo(easy_curl, CURLINFO_TOTAL_TIME_T, &timev);

    return static_cast<long>(timev);
  };
};

class OmafCurlEasyDownloader : public VCD::NonCopyable {
  friend OmafCurlEasyDownloaderPool;

 public:
  enum class CurlWorkMode {
    MULTI_MODE = 0,
    EASY_MODE = 1,
  };

  enum class State {
    DOWNLOADING = 0,
    SUCCESS = 1,
    STOPPED = 2,
    FAILED = 3,
  };

 public:
  using onData = std::function<void(std::unique_ptr<StreamBlock>)>;
  using onState = std::function<void(State)>;
  using Ptr = std::shared_ptr<OmafCurlEasyDownloader>;

 public:
  OmafCurlEasyDownloader(CurlWorkMode work_mode = CurlWorkMode::MULTI_MODE) : work_mode_(work_mode){};

  virtual ~OmafCurlEasyDownloader();

 public:
  OMAF_STATUS init(const CurlParams &params) noexcept;
  OMAF_STATUS open(const std::string &url) noexcept;
  OMAF_STATUS start(int64_t offset, int64_t size, onData scb, onState fcb) noexcept;
  OMAF_STATUS stop() noexcept;
  OMAF_STATUS close() noexcept;
  HttpHeader header() noexcept;
  double speed() noexcept;

 public:
  static size_t curlBodyCallback(char *ptr, size_t size, size_t nmemb, void *userdata) noexcept;

 public:
  inline CURL *handler() noexcept {
    std::lock_guard<std::mutex> lock(easy_curl_mutex_);
    return easy_curl_;
  };
  inline CurlTimes curlTimes() {
    CurlTimes curl_times;
    if (easy_curl_) {
      OmafCurlEasyHelper::curlTime(easy_curl_, curl_times);
    }
    return curl_times;
  }
  inline long downloadTime() {
    if (easy_curl_) {
      return OmafCurlEasyHelper::totalTime(easy_curl_);
    }
    return 0;
  }

 private:
  void receiveSB(std::unique_ptr<StreamBlock>) noexcept;
  void params(const CurlParams &params) noexcept { curl_params_ = params; }

 private:
  CurlWorkMode work_mode_ = CurlWorkMode::MULTI_MODE;
  CurlParams curl_params_;
  std::mutex easy_curl_mutex_;
  CURL *easy_curl_ = nullptr;
  std::string url_;
  std::mutex cb_mutex_;
  onData dcb_ = nullptr;
  onState scb_ = nullptr;
  State state_ = State::DOWNLOADING;
};

class OmafCurlEasyDownloaderPool : public VCD::NonCopyable {
 public:
  OmafCurlEasyDownloaderPool(int32_t max_downloader) : max_downloader_(max_downloader){};
  virtual ~OmafCurlEasyDownloaderPool();

 public:
  OmafCurlEasyDownloader::Ptr pop() noexcept;
  void push(OmafCurlEasyDownloader::Ptr) noexcept;
  void params(const CurlParams &params) noexcept { curl_params_ = params; }

 private:
  int32_t max_downloader_;
  int32_t downloader_count_ = 0;
  CurlParams curl_params_;
  std::mutex easy_downloader_pool_mutex_;
  std::queue<OmafCurlEasyDownloader::Ptr> easy_downloader_pool_;
};

class OmafCurlChecker : public VCD::NonCopyable {
 public:
  using Ptr = std::shared_ptr<OmafCurlChecker>;

 public:
  OmafCurlChecker(){};
  ~OmafCurlChecker();

 public:
  OMAF_STATUS init(const CurlParams &params) noexcept;
  OMAF_STATUS check(const std::string &url) noexcept;
  OMAF_STATUS close() noexcept;

 private:
  CURL *easy_curl_ = nullptr;
};

}  // namespace OMAF
}  // namespace VCD

#endif  // OMAFCURLEASYWRAPPER_H