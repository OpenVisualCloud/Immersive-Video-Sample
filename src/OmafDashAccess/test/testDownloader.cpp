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

#include "../OmafDashDownload/OmafDownloader.h"

using namespace VCD::OMAF;

namespace {

class DownloaderTest : public testing::Test {
 public:
  virtual void SetUp() {
    OMAF_LOG(LOG_INFO, "Calling setup\n");

    outsite_url = "https://www.baidu.com";
    valid_url = "http://10.67.112.194:8080/testOMAFlive/Test.mpd";

    invalid_url = invalid_url + "invalid";
    no_proxy = "127.0.0.1,*.intel.com,10.67.112.194";
    proxy_url = "http://child-prc.intel.com:913";
    invalid_proxy_url = "http://chil-prc.intel.com:913";

    dash_client_ = OmafDashSegmentHttpClient::create(10);

    client_params.bssl_verify_host_ = false;
    client_params.bssl_verify_peer_ = false;
    client_params.conn_timeout_ = 5000;    // 5s
    client_params.total_timeout_ = 30000;  // 30s

    client_params.retry_times_ = 3;

    dash_client_->setParams(client_params);
  }

  virtual void TearDown() {
    OMAF_LOG(LOG_INFO, "Calling TearDown\n");

    OMAF_STATUS ret = dash_client_->stop();
    EXPECT_TRUE(ret == ERROR_NONE);
  }

  OmafDashSegmentHttpClient::Ptr dash_client_ = nullptr;

  std::string valid_url;
  std::string invalid_url;
  std::string outsite_url;
  std::string proxy_url;
  std::string invalid_proxy_url;
  std::string no_proxy;
  OmafDashHttpParams client_params;
};

TEST_F(DownloaderTest, Create) {
  OmafDashSegmentHttpClient::Ptr dash_client = OmafDashSegmentHttpClient::create(10);
  EXPECT_TRUE(dash_client != nullptr);
}

TEST_F(DownloaderTest, downloadSuccess) {
  OMAF_STATUS ret = dash_client_->start();
  EXPECT_TRUE(ret == ERROR_NONE);

  DashSegmentSourceParams ds;
  ds.dash_url_ = valid_url;
  ds.timeline_point_ = 1;

  bool isState = false;
  dash_client_->open(
      ds,
      [](std::unique_ptr<VCD::OMAF::StreamBlock> sb) {
        EXPECT_TRUE(sb != nullptr);
        EXPECT_TRUE(sb->size() > 0);
      },
      [&isState](OmafDashSegmentClient::State state) {
        OMAF_LOG(LOG_INFO, "Receive the state: %d\n", static_cast<int>(state));
        EXPECT_TRUE(state == OmafDashSegmentClient::State::SUCCESS);
        isState = true;
      });
  while (!isState) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}
TEST_F(DownloaderTest, downloadFailure) {
  OMAF_STATUS ret = dash_client_->start();
  EXPECT_TRUE(ret == ERROR_NONE);
  DashSegmentSourceParams ds;
  ds.dash_url_ = invalid_url;
  ds.timeline_point_ = 1;
  bool isState = false;
  dash_client_->open(
      ds,
      [](std::unique_ptr<VCD::OMAF::StreamBlock> sb) {

      },
      [&isState](OmafDashSegmentClient::State state) {
        OMAF_LOG(LOG_INFO, "Receive the state: %d\n", static_cast<int>(state));
        EXPECT_TRUE(state == OmafDashSegmentClient::State::TIMEOUT);
        isState = true;
      });
  while (!isState) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}
TEST_F(DownloaderTest, proxy_success) {
  DashSegmentSourceParams ds;
  ds.dash_url_ = outsite_url;
  ds.timeline_point_ = 1;

  OmafDashHttpProxy proxy;
  proxy.http_proxy_ = proxy_url;
  // proxy.https_proxy_ = proxy_url;

  dash_client_->setProxy(proxy);

  OMAF_STATUS ret = dash_client_->start();
  EXPECT_TRUE(ret == ERROR_NONE);

  bool isState = false;
  dash_client_->open(
      ds,
      [](std::unique_ptr<VCD::OMAF::StreamBlock> sb) {
        EXPECT_TRUE(sb != nullptr);
        EXPECT_TRUE(sb->size() > 0);
      },
      [&isState](OmafDashSegmentClient::State state) {
        OMAF_LOG(LOG_INFO, "Receive the state: %d\n", static_cast<int>(state));
        EXPECT_TRUE(state == OmafDashSegmentClient::State::SUCCESS);
        isState = true;
      });
  while (!isState) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

TEST_F(DownloaderTest, invalid_proxy) {
  DashSegmentSourceParams ds;
  ds.dash_url_ = outsite_url;
  ds.timeline_point_ = 1;

  OmafDashHttpProxy proxy;

  proxy.https_proxy_ = invalid_proxy_url;

  dash_client_->setProxy(proxy);

  OMAF_STATUS ret = dash_client_->start();
  EXPECT_TRUE(ret == ERROR_NONE);

  bool isState = false;
  dash_client_->open(
      ds,
      [](std::unique_ptr<VCD::OMAF::StreamBlock> sb) {

      },
      [&isState](OmafDashSegmentClient::State state) {
        OMAF_LOG(LOG_INFO, "Receive the state: %d\n", static_cast<int>(state));
        EXPECT_TRUE(state == OmafDashSegmentClient::State::TIMEOUT);
        isState = true;
      });
  while (!isState) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

TEST_F(DownloaderTest, no_proxy_success) {
  DashSegmentSourceParams ds;
  ds.dash_url_ = valid_url;
  ds.timeline_point_ = 1;

  OmafDashHttpProxy proxy;
  proxy.http_proxy_ = proxy_url;
  proxy.no_proxy_ = no_proxy;
  dash_client_->setProxy(proxy);
  OMAF_STATUS ret = dash_client_->start();
  EXPECT_TRUE(ret == ERROR_NONE);
  bool isState = false;
  dash_client_->open(
      ds,
      [](std::unique_ptr<VCD::OMAF::StreamBlock> sb) {
        EXPECT_TRUE(sb != nullptr);
        EXPECT_TRUE(sb->size() > 0);
      },
      [&isState](OmafDashSegmentClient::State state) {
        OMAF_LOG(LOG_INFO, "Receive the state: %d\n", static_cast<int>(state));
        EXPECT_TRUE(state == OmafDashSegmentClient::State::SUCCESS);
        isState = true;
      });
  while (!isState) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

}  // namespace
