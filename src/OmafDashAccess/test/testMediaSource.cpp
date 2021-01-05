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
#include <pwd.h>
#include "../OmafMediaSource.h"
#include "../OmafDashSource.h"
#include "360SCVPAPI.h"

VCD_USE_VRVIDEO;

namespace {
class MediaSourceTest : public testing::Test {
 public:
  virtual void SetUp() {
    url_live = "http://10.67.112.194:8080/testOMAFlive/Test.mpd";
    // url_static = "https://10.67.119.113:443/UT_testOMAFstatic/Test.mpd";
    // url_static = "http://10.67.119.113:8080/UT_testOMAFstatic/Test.mpd";
    url_static = "http://10.67.112.194:8080/testOMAFstatic/Test.mpd";
    cache = "./cache";  // getpwuid(getuid())->pw_dir + std::string("/cache");
    pluginName = "libViewportPredict_LR.so";
    libPath = "../../plugins/ViewportPredict_Plugin/predict_LR/";

    clientInfo = new HeadSetInfo;
    //clientInfo->input_geoType = 0;
    //clientInfo->output_geoType = E_SVIDEO_VIEWPORT;
    clientInfo->pose = new HeadPose;
    clientInfo->pose->yaw = -90;
    clientInfo->pose->pitch = 0;
    clientInfo->viewPort_hFOV = 80;
    clientInfo->viewPort_vFOV = 80;
    clientInfo->viewPort_Width = 960;
    clientInfo->viewPort_Height = 960;

    pose = new HeadPose;
    pose->yaw = 90;
    pose->pitch = 0;
  }

  virtual void TearDown() {
    free(clientInfo->pose);
    clientInfo->pose = nullptr;

    free(clientInfo);
    clientInfo = nullptr;

    free(pose);
    pose = nullptr;
  }

  int GetFileCntUnderCache() {
    // check the downloaded files number > 0
    string downloadedFileCnt = "ls -l " + cache + " | grep -v ^l | wc -l";
    std::array<char, 128> buffer;
    std::string result;
    FILE* pipe = popen(downloadedFileCnt.c_str(), "r");
    if (!pipe) cout << "failed to get downloaded files count!" << endl;

    while (pipe && fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
      result += buffer.data();
    }

    int32_t cnt = std::stoi(result);

    if (pipe) pclose(pipe);

    return cnt;
  }

  HeadSetInfo* clientInfo;
  HeadPose* pose;
  std::string url_live;
  std::string url_static;
  std::string cache;
  std::string pluginName;
  std::string libPath;
};

TEST_F(MediaSourceTest, Create) {
  OmafMediaSource* dashSource = new OmafDashSource();
  EXPECT_TRUE(dashSource != NULL);
  delete dashSource;
}

TEST_F(MediaSourceTest, OpenMedia_static) {
  const string command = "rm -rf " + cache + "/*";
  system(command.c_str());

  OmafMediaSource* dashSource = new OmafDashSource();
  EXPECT_TRUE(dashSource != NULL);

  int ret = dashSource->SetupHeadSetInfo(clientInfo);
  EXPECT_TRUE(ret == ERROR_NONE);

  PluginDef i360ScvpPlugin;
  i360ScvpPlugin.pluginLibPath = NULL;
  ret = dashSource->OpenMedia(url_static, cache, NULL, i360ScvpPlugin, true, false, "", "");
  EXPECT_TRUE(ret == ERROR_NONE);
  dashSource->StartStreaming();

  sleep(5);
  dashSource->CloseMedia();

  // check the downloaded files number > 0
  int32_t cnt = GetFileCntUnderCache();
  EXPECT_TRUE(cnt > 1);

  delete dashSource;
}

TEST_F(MediaSourceTest, OpenMedia_live) {
  const string command = "rm -rf " + cache + "/*";
  system(command.c_str());

  OmafMediaSource* dashSource = new OmafDashSource();
  EXPECT_TRUE(dashSource != NULL);

  int ret = dashSource->SetupHeadSetInfo(clientInfo);
  EXPECT_TRUE(ret == ERROR_NONE);

  PluginDef i360ScvpPlugin;
  i360ScvpPlugin.pluginLibPath = NULL;
  ret = dashSource->OpenMedia(url_live, cache, NULL, i360ScvpPlugin, true, false, "", "");
  EXPECT_TRUE(ret == ERROR_NONE);
  dashSource->StartStreaming();

  sleep(5);
  dashSource->CloseMedia();

  // check the downloaded files number > 0
  int32_t cnt = GetFileCntUnderCache();

  EXPECT_TRUE(cnt > 1);

  delete dashSource;
}

TEST_F(MediaSourceTest, OpenMedia_static_withPredictor) {
  const string command = "rm -rf " + cache + "/*";
  system(command.c_str());

  OmafMediaSource* dashSource = new OmafDashSource();
  EXPECT_TRUE(dashSource != NULL);

  int ret = dashSource->SetupHeadSetInfo(clientInfo);
  EXPECT_TRUE(ret == ERROR_NONE);

  PluginDef i360ScvpPlugin;
  i360ScvpPlugin.pluginLibPath = NULL;
  ret = dashSource->OpenMedia(url_static, cache, NULL, i360ScvpPlugin, true, true, pluginName, libPath);
  EXPECT_TRUE(ret == ERROR_NONE);
  dashSource->StartStreaming();

  sleep(15);
  dashSource->CloseMedia();

  // check the downloaded files number > 0
  int32_t cnt = GetFileCntUnderCache();

  EXPECT_TRUE(cnt > 1);

  delete dashSource;
}

TEST_F(MediaSourceTest, OpenMedia_live_withPredictor) {
  const string command = "rm -rf " + cache + "/*";
  system(command.c_str());

  OmafMediaSource* dashSource = new OmafDashSource();
  EXPECT_TRUE(dashSource != NULL);

  int ret = dashSource->SetupHeadSetInfo(clientInfo);
  EXPECT_TRUE(ret == ERROR_NONE);

  PluginDef i360ScvpPlugin;
  i360ScvpPlugin.pluginLibPath = NULL;
  ret = dashSource->OpenMedia(url_live, cache, NULL, i360ScvpPlugin, true, true, pluginName, libPath);
  EXPECT_TRUE(ret == ERROR_NONE);
  dashSource->StartStreaming();

  sleep(15);
  dashSource->CloseMedia();

  // check the downloaded files number > 0
  int32_t cnt = GetFileCntUnderCache();

  EXPECT_TRUE(cnt > 1);

  delete dashSource;
}

TEST_F(MediaSourceTest, OpenMedia_static_changeViewport) {
  string command = "rm -rf " + cache + "/*";
  system(command.c_str());

  OmafMediaSource* dashSource = new OmafDashSource();
  EXPECT_TRUE(dashSource != NULL);

  int ret = dashSource->SetupHeadSetInfo(clientInfo);
  EXPECT_TRUE(ret == ERROR_NONE);

  PluginDef i360ScvpPlugin;
  i360ScvpPlugin.pluginLibPath = NULL;
  ret = dashSource->OpenMedia(url_static, cache, NULL, i360ScvpPlugin, true, false, "", "");
  EXPECT_TRUE(ret == ERROR_NONE);
  dashSource->StartStreaming();

  int16_t vpcnt = 200;
  while (vpcnt > 0) {
    pose->yaw += 10;
    pose->yaw = pose->yaw > 180 ? pose->yaw - 360 : pose->yaw;
    ret = dashSource->ChangeViewport(pose);
    EXPECT_TRUE(ret == ERROR_NONE);
    usleep(10000);
    vpcnt--;
  }

  sleep(5);
  dashSource->CloseMedia();

  // check the downloaded files number > 0
  int32_t cnt = GetFileCntUnderCache();

  EXPECT_TRUE(cnt > 1);

  delete dashSource;
}

TEST_F(MediaSourceTest, OpenMedia_live_changeViewport) {
  const string command = "rm -rf " + cache + "/*";
  system(command.c_str());

  OmafMediaSource* dashSource = new OmafDashSource();
  EXPECT_TRUE(dashSource != NULL);

  int ret = dashSource->SetupHeadSetInfo(clientInfo);
  EXPECT_TRUE(ret == ERROR_NONE);

  PluginDef i360ScvpPlugin;
  i360ScvpPlugin.pluginLibPath = NULL;
  ret = dashSource->OpenMedia(url_live, cache, NULL, i360ScvpPlugin, true, false, "", "");
  EXPECT_TRUE(ret == ERROR_NONE);
  dashSource->StartStreaming();

  sleep(1);

  ret = dashSource->ChangeViewport(pose);
  EXPECT_TRUE(ret == ERROR_NONE);

  sleep(4);
  dashSource->CloseMedia();

  // check the downloaded files number > 0
  int32_t cnt = GetFileCntUnderCache();

  EXPECT_TRUE(cnt > 1);

  delete dashSource;
}

}  // namespace
