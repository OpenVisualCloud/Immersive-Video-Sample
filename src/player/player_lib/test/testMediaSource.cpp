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
//! \file     testMediaSource.cpp
//! \brief    unit test for FFmpegMediaSource.
//!

#include "gtest/gtest.h"
#include "../FFmpegMediaSource.h"

VCD_NS_BEGIN

namespace
{
class FFmpegMediaSourceTest : public testing::Test
{
public:
    virtual void SetUp()
    {
    }
    virtual void TearDown()
    {
    }
};

TEST_F(FFmpegMediaSourceTest, FFmpegMediaSourceConstruct)
{
    MediaSource *ffmpegMediaSource = new FFmpegMediaSource();
    ASSERT_TRUE(ffmpegMediaSource != NULL);
}

TEST_F(FFmpegMediaSourceTest, LoadMediaSourceTest)
{
    MediaSource *ffmpegMediaSource = new FFmpegMediaSource();
    struct RenderConfig renderConfig;
    renderConfig.url = "./packet_MultiBS.265";
    ffmpegMediaSource->Initialize(renderConfig);
    ASSERT_EQ(ffmpegMediaSource->GetMediaSourceInfo().width, 5760);
    ASSERT_EQ(ffmpegMediaSource->GetMediaSourceInfo().height, 3840);
}

TEST_F(FFmpegMediaSourceTest, GetFrameTest)
{
    MediaSource *ffmpegMediaSource = new FFmpegMediaSource();
    uint8_t *buffer[4];
    struct RenderConfig renderConfig;
    struct RegionInfo regionInfo;
    renderConfig.url = "./packet_MultiBS.265";
    ffmpegMediaSource->Initialize(renderConfig);
    RenderStatus renderStatus = RENDER_STATUS_OK;
    do
    {
        renderStatus = ffmpegMediaSource->GetFrame(&buffer[0], &regionInfo);
    } while (renderStatus == RENDER_ERROR);
    //1.check the buffer data
    FILE *fout1;
    fout1 = fopen("1.raw", "w");
    uint32_t width = ffmpegMediaSource->GetMediaSourceInfo().width;
    uint32_t height = ffmpegMediaSource->GetMediaSourceInfo().height;
    for (uint32_t i = 0; i < height; i++)
    {
        // can display a jpg file. ffmpeg -f rawvideo -s 5760x3840 -pix_fmt rgb24 -i 1.raw 1.jpg
        fwrite(buffer[0] + i * ((struct SourceData*)ffmpegMediaSource->GetSourceMetaData())->av_frame->linesize[0], 1, ((struct SourceData*)ffmpegMediaSource->GetSourceMetaData())->av_frame->linesize[0], fout1);
    }
    FILE *fout2;
    fout2 = fopen("2.raw", "w");
    FILE *fout3;
    fout3 = fopen("3.raw", "w");
    uint32_t width2 = ffmpegMediaSource->GetMediaSourceInfo().width / 2;
    uint32_t height2 = ffmpegMediaSource->GetMediaSourceInfo().height / 2;
    for (uint32_t i = 0; i < height2; i++)
    {
        // can display a jpg file. ffmpeg -f rawvideo -s 5760x3840 -pix_fmt rgb24 -i 1.raw 1.jpg
        fwrite(buffer[1] + i * ((struct SourceData *)ffmpegMediaSource->GetSourceMetaData())->av_frame->linesize[1], 1, ((struct SourceData *)ffmpegMediaSource->GetSourceMetaData())->av_frame->linesize[1], fout2);
        fwrite(buffer[2] + i * ((struct SourceData *)ffmpegMediaSource->GetSourceMetaData())->av_frame->linesize[2], 1, ((struct SourceData *)ffmpegMediaSource->GetSourceMetaData())->av_frame->linesize[2], fout3);
    }

    ASSERT_FALSE(buffer == NULL);
}
} // namespace

VCD_NS_END
