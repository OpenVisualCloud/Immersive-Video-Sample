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
#include "../OmafTracksSelector.h"
#include "../OmafViewTracksSelector.h"
#include "../OmafMPDParser.h"

VCD_USE_VRVIDEO;

namespace{
class TracksSelectorTest : public testing::Test {
public:
    virtual void SetUp(){
        url_static_free_view = "http://10.67.115.92:8080/testIINVstatic/Test.mpd";
    }

    virtual void TearDown(){
    }

    std::string url_static_free_view;
};

TEST_F(TracksSelectorTest, Create)
{
    OmafViewTracksSelector* tracksSelector = new OmafViewTracksSelector();
    EXPECT_TRUE(tracksSelector != NULL);
    delete tracksSelector;
}

TEST_F(TracksSelectorTest, SetInitialViewport)
{
    OmafViewTracksSelector* tracksSelector = new OmafViewTracksSelector();
    EXPECT_TRUE(tracksSelector != NULL);

    std::vector<Viewport*> views;
    HeadSetInfo info;
    info.pose = new HeadPose;
    if (!info.pose) return;
    info.pose->hViewId = 5;
    info.pose->vViewId = 0;
    int ret = tracksSelector->SetInitialViewport(views, &info, nullptr);
    EXPECT_TRUE(ret == ERROR_NONE);

    SAFE_DELETE(tracksSelector);
}

TEST_F(TracksSelectorTest, TracksSelector_AssignedManual)
{
    OmafMPDParser *MPDParser = new OmafMPDParser();
    EXPECT_FALSE(nullptr == MPDParser);

    OMAFSTREAMS listStream;
    MPDParser->SetCacheDir("./");
    int ret = MPDParser->ParseMPD(url_static_free_view, listStream);
    EXPECT_TRUE(ret == ERROR_NONE);

    OmafMediaStream* stream = nullptr;
    for (auto st : listStream)
    {
        MediaType type = st->GetStreamMediaType();
        if (type == MediaType::MediaType_Video)
        {
            stream = st;
            break;
        }
    }
    EXPECT_TRUE(stream != nullptr);

    OmafViewTracksSelector* tracksSelector = new OmafViewTracksSelector();
    EXPECT_TRUE(tracksSelector != NULL);

    tracksSelector->SetProjectionFmt(ProjectionFormat::PF_PLANAR);

    tracksSelector->SetSegmentDuration(1);

    std::vector<Viewport*> views;
    HeadSetInfo info;
    info.pose = new HeadPose;
    if (!info.pose) return;
    info.pose->hViewId = 5;
    info.pose->vViewId = 0;
    int ret1 = tracksSelector->SetInitialViewport(views, &info, nullptr);
    EXPECT_TRUE(ret1 == ERROR_NONE);

    TracksMap tracks = tracksSelector->GetViewTracksInManualMode(stream, info.pose);
    EXPECT_TRUE(!tracks.empty());

    int expectedTracksId[5] = {6, 7, 8, 9, 10};
    int i = 0;
    for (auto tk : tracks)
    {
        EXPECT_TRUE(tk.first == expectedTracksId[i++]);
    }

    SAFE_DELETE(tracksSelector);
}

TEST_F(TracksSelectorTest, TracksSelector_AssignedAuto)
{
    OmafMPDParser *MPDParser = new OmafMPDParser();
    EXPECT_FALSE(nullptr == MPDParser);

    OMAFSTREAMS listStream;
    MPDParser->SetCacheDir("./");
    int ret = MPDParser->ParseMPD(url_static_free_view, listStream);
    EXPECT_TRUE(ret == ERROR_NONE);

    OmafMediaStream* stream = nullptr;
    for (auto st : listStream)
    {
        MediaType type = st->GetStreamMediaType();
        if (type == MediaType::MediaType_Video)
        {
            stream = st;
            break;
        }
    }
    EXPECT_TRUE(stream != nullptr);

    OmafViewTracksSelector* tracksSelector = new OmafViewTracksSelector();
    EXPECT_TRUE(tracksSelector != NULL);

    tracksSelector->SetProjectionFmt(ProjectionFormat::PF_PLANAR);

    tracksSelector->SetSegmentDuration(1);

    std::vector<Viewport*> views;
    HeadSetInfo info;
    info.pose = new HeadPose;
    if (!info.pose) return;
    info.pose->hViewId = 5;
    info.pose->vViewId = 0;
    int ret1 = tracksSelector->SetInitialViewport(views, &info, nullptr);
    EXPECT_TRUE(ret1 == ERROR_NONE);

    TracksMap tracks = tracksSelector->GetViewTracksInAutoMode(stream, info.pose);
    EXPECT_TRUE(!tracks.empty());

    int expectedTracksId[5] = {1, 2, 3, 4, 5};
    int i = 0;
    for (auto tk : tracks)
    {
        EXPECT_TRUE(tk.first == expectedTracksId[i++]);
    }

    SAFE_DELETE(tracksSelector);
}
}