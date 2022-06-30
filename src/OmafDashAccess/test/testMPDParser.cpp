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
#include "../OmafMPDParser.h"

VCD_USE_VRVIDEO;

namespace{
class MPDParserTest : public testing::Test {
public:
    virtual void SetUp(){

        url_live = "http://10.67.115.92:8080/testOMAFlive/Test.mpd";
        url_static = "http://10.67.115.92:8080/testOMAFstatic/Test.mpd";
        url_cmaf_live = "http://10.67.115.92:8080/testCMAFlive/Test.mpd";
        url_static_free_view = "http://10.67.115.92:8080/testIINVstatic/Test.mpd";
    }

    virtual void TearDown(){
    }

    std::string url_live;
    std::string url_static;
    std::string url_static_free_view;
    std::string url_cmaf_live;
};

TEST_F(MPDParserTest, Create)
{
    OmafMPDParser* MPDParser = new OmafMPDParser();
    EXPECT_TRUE(MPDParser != NULL);
    delete MPDParser;
}

TEST_F(MPDParserTest, ParseMPD_static)
{
    OmafMPDParser* MPDParser = new OmafMPDParser();
    EXPECT_TRUE(MPDParser != NULL);

    OMAFSTREAMS listStream;
    int ret = MPDParser->ParseMPD(url_static, listStream);
    EXPECT_TRUE(ret == ERROR_NONE);

    EXPECT_TRUE(listStream.size() > 0);
    auto stream = listStream.front();
    DashStreamInfo* sInfo = stream->GetStreamInfo();
    EXPECT_TRUE(sInfo->height > 0 && sInfo->width > 0);
    EXPECT_TRUE(sInfo->stream_type == MediaType_Video);

    MPDInfo *mpdInfo = nullptr;
    mpdInfo = MPDParser->GetMPDInfo();
    EXPECT_TRUE(mpdInfo != nullptr);
    EXPECT_TRUE(mpdInfo->type == "static");

    delete MPDParser;
}

TEST_F(MPDParserTest, ParseMPD_static_freeview)
{
    OmafMPDParser* MPDParser = new OmafMPDParser();
    EXPECT_TRUE(MPDParser != NULL);

    OMAFSTREAMS listStream;
    int ret = MPDParser->ParseMPD(url_static_free_view, listStream);
    EXPECT_TRUE(ret == ERROR_NONE);

    EXPECT_TRUE(listStream.size() > 0);
    auto stream = listStream.front();
    DashStreamInfo* sInfo = stream->GetStreamInfo();
    EXPECT_TRUE(sInfo->height > 0 && sInfo->width > 0);
    EXPECT_TRUE(sInfo->stream_type == MediaType_Video);

    MPDInfo *mpdInfo = nullptr;
    mpdInfo = MPDParser->GetMPDInfo();
    EXPECT_TRUE(mpdInfo != nullptr);
    EXPECT_TRUE(mpdInfo->type == "static");
    for (auto stream : listStream)
    {
        if (stream->GetStreamMediaType() == MediaType::MediaType_Video)
        {
            EXPECT_TRUE(stream->GetDashMode() == OmafDashMode::MULTI_VIEW);
        }
    }

    delete MPDParser;
}

TEST_F(MPDParserTest, ParseMPD_live)
{
    OmafMPDParser* MPDParser = new OmafMPDParser();
    EXPECT_TRUE(MPDParser != NULL);

    OMAFSTREAMS listStream;
    int ret = MPDParser->ParseMPD(url_live, listStream);
    EXPECT_TRUE(ret == ERROR_NONE);

    EXPECT_TRUE(listStream.size() > 0);
    auto stream = listStream.front();
    DashStreamInfo* sInfo = stream->GetStreamInfo();
    EXPECT_TRUE(sInfo->height > 0 && sInfo->width > 0);
    EXPECT_TRUE(sInfo->stream_type == MediaType_Video);

    MPDInfo *mpdInfo = nullptr;
    mpdInfo = MPDParser->GetMPDInfo();
    EXPECT_TRUE(mpdInfo != nullptr);
    EXPECT_TRUE(mpdInfo->type == "dynamic");

    delete MPDParser;
}

TEST_F(MPDParserTest, ParseMPD_cmaf_live)
{
    OmafMPDParser* MPDParser = new OmafMPDParser();
    EXPECT_TRUE(MPDParser != NULL);

    OMAFSTREAMS listStream;
    int ret = MPDParser->ParseMPD(url_cmaf_live, listStream);
    EXPECT_TRUE(ret == ERROR_NONE);

    EXPECT_TRUE(listStream.size() > 0);
    auto stream = listStream.front();
    DashStreamInfo* sInfo = stream->GetStreamInfo();
    EXPECT_TRUE(sInfo->height > 0 && sInfo->width > 0);
    EXPECT_TRUE(sInfo->stream_type == MediaType_Video);

    MPDInfo *mpdInfo = nullptr;
    mpdInfo = MPDParser->GetMPDInfo();
    EXPECT_TRUE(mpdInfo != nullptr);
    EXPECT_TRUE(mpdInfo->type == "dynamic");
    EXPECT_TRUE(mpdInfo->availabilityStartTime != 0);
    EXPECT_TRUE(mpdInfo->target_latency != 0);

    delete MPDParser;
}

}
