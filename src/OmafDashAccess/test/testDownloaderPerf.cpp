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

#include "gtest/gtest.h"
#include <string>
#include <thread>
#include <memory>
#include <pwd.h>
#include <atomic>

#include "../OmafDashDownload/OmafDownloader.h"

using namespace VCD::OMAF;

namespace {

class DownloaderPerfTest : public testing::Test {
 public:
  virtual void SetUp() {
    OMAF_LOG(LOG_INFO, "Calling setup\n");

    valid_url = "http://10.67.112.194:8080/testOMAFlive/Test.mpd";

    invalid_url = invalid_url + "invalid";

    dash_client_ = OmafDashSegmentHttpClient::create(10);
    dash_client_->setStatisticsWindows(10000);

    client_params.bssl_verify_host_ = false;
    client_params.bssl_verify_peer_ = false;
    client_params.conn_timeout_ = 3000;
    client_params.total_timeout_ = 5000;
    client_params.retry_times_ = 2;

    dash_client_->setParams(client_params);
    OMAF_STATUS ret = dash_client_->start();
    EXPECT_TRUE(ret == ERROR_NONE);
  }

  virtual void TearDown() {
    OMAF_LOG(LOG_INFO, "Calling TearDown\n");

    OMAF_STATUS ret = dash_client_->stop();
    EXPECT_TRUE(ret == ERROR_NONE);
  }

  OmafDashSegmentHttpClient::Ptr dash_client_ = nullptr;

  std::string valid_url;
  std::string invalid_url;
  OmafDashHttpParams client_params;
};

TEST_F(DownloaderPerfTest, Create) {
  OmafDashSegmentHttpClient::Ptr dash_client = OmafDashSegmentHttpClient::create(10);
  EXPECT_TRUE(dash_client != nullptr);
}

TEST_F(DownloaderPerfTest, perf_success) {
  DashSegmentSourceParams ds;
  ds.dash_url_ = valid_url;
  ds.timeline_point_ = 1;
  const size_t TEST_COUNT = 1;
  std::atomic_int32_t sync_num(0);
  dash_client_->open(
      ds,
      [](std::unique_ptr<VCD::OMAF::StreamBlock> sb) {
        EXPECT_TRUE(sb != nullptr);
        EXPECT_TRUE(sb->size() > 0);
      },
      [&sync_num](OmafDashSegmentClient::State state) {
        OMAF_LOG(LOG_INFO, "Receive the state: %d\n", static_cast<int>(state));
        EXPECT_TRUE(state == OmafDashSegmentClient::State::SUCCESS);
        sync_num++;
      });

  while (sync_num.load() != TEST_COUNT) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  std::unique_ptr<OmafDashSegmentHttpClient::PerfStatistics> stats = dash_client_->statistics();
  EXPECT_TRUE(stats != nullptr);

  EXPECT_TRUE(stats->success_.count_total_ == TEST_COUNT);
  EXPECT_TRUE(stats->timeout_.count_total_ == 0);
  EXPECT_TRUE(stats->failure_.count_total_ == 0);
}

TEST_F(DownloaderPerfTest, perf_timeout) {
  DashSegmentSourceParams ds;
  ds.dash_url_ = invalid_url;
  ds.timeline_point_ = 1;
  const size_t TEST_COUNT = 1;
  std::atomic_int32_t sync_num(0);
  dash_client_->open(
      ds,
      [](std::unique_ptr<VCD::OMAF::StreamBlock> sb) {
        EXPECT_TRUE(sb != nullptr);
        EXPECT_TRUE(sb->size() > 0);
      },
      [&sync_num](OmafDashSegmentClient::State state) {
        OMAF_LOG(LOG_INFO, "Receive the state:%d\n", static_cast<int>(state));
        EXPECT_TRUE(state == OmafDashSegmentClient::State::TIMEOUT);
        sync_num++;
      });
  while (sync_num.load() != TEST_COUNT) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  std::unique_ptr<OmafDashSegmentHttpClient::PerfStatistics> stats = dash_client_->statistics();
  EXPECT_TRUE(stats != nullptr);

  EXPECT_TRUE(stats->success_.count_total_ == 0);
  EXPECT_TRUE(stats->timeout_.count_total_ == TEST_COUNT);
  EXPECT_TRUE(stats->failure_.count_total_ == 0);
}

TEST_F(DownloaderPerfTest, perf_success_100) {
  DashSegmentSourceParams ds;
  ds.dash_url_ = valid_url;
  ds.timeline_point_ = 1;

  std::atomic_int32_t sync_num(0);
  const size_t TEST_COUNT = 100;
  for (size_t i = 0; i < TEST_COUNT; i++) {
    dash_client_->open(
        ds,
        [](std::unique_ptr<VCD::OMAF::StreamBlock> sb) {
          EXPECT_TRUE(sb != nullptr);
          EXPECT_TRUE(sb->size() > 0);
        },
        [&sync_num](OmafDashSegmentClient::State state) {
          OMAF_LOG(LOG_INFO, "Receive the state:%d\n", static_cast<int>(state));
          EXPECT_TRUE(state == OmafDashSegmentClient::State::SUCCESS);
          sync_num++;
        });
  }

  while (sync_num.load() != TEST_COUNT) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  std::unique_ptr<OmafDashSegmentHttpClient::PerfStatistics> stats = dash_client_->statistics();
  EXPECT_TRUE(stats != nullptr);

  EXPECT_TRUE(stats->success_.count_total_ == TEST_COUNT);
  EXPECT_TRUE(stats->timeout_.count_total_ == 0);
  EXPECT_TRUE(stats->failure_.count_total_ == 0);
}

}  // namespace
