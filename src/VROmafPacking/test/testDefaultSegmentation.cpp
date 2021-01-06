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
//! \file:   testDefaultSegmentation.cpp
//! \brief:  Default segmentation class unit test
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include "gtest/gtest.h"
#include "../OmafPackage.h"

VCD_USE_VRVIDEO;

namespace {
class DefaultSegmentationTest : public testing::Test
{
public:
    virtual void SetUp()
    {
        const char *highResFileName = "3840x1920_10frames.h265";
        FILE *highResFile = fopen(highResFileName, "r");
        if (!highResFile)
            return;

        const char *lowResFileName = "1920x960_10frames.h265";
        FILE *lowResFile = fopen(lowResFileName, "r");
        if (!lowResFile)
        {
            fclose(highResFile);
            highResFile = NULL;
            return;
        }

        int32_t highResHeaderSize = 99;
        int32_t lowResHeaderSize = 97;
        m_highResHeader = new uint8_t[highResHeaderSize];
        if (!m_highResHeader)
        {
            fclose(highResFile);
            highResFile = NULL;
            fclose(lowResFile);
            lowResFile = NULL;
            return;
        }

        m_lowResHeader = new uint8_t[lowResHeaderSize];
        if (!m_lowResHeader)
        {
            fclose(highResFile);
            highResFile = NULL;
            fclose(lowResFile);
            lowResFile = NULL;
            DELETE_ARRAY(m_highResHeader);
            return;
        }

        fread(m_highResHeader, 1, highResHeaderSize, highResFile);
        fread(m_lowResHeader, 1, lowResHeaderSize, lowResFile);

        uint64_t totalSizeLow = 99768;
        m_totalDataLow = new uint8_t[totalSizeLow];
        if (!m_totalDataLow)
        {
            fclose(highResFile);
            highResFile = NULL;
            fclose(lowResFile);
            lowResFile = NULL;
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            return;
        }

        uint64_t totalSizeHigh = 104157;
        m_totalDataHigh = new uint8_t[totalSizeHigh];
        if (!m_totalDataHigh)
        {
            fclose(highResFile);
            highResFile = NULL;
            fclose(lowResFile);
            lowResFile = NULL;
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            return;
        }

        fread(m_totalDataLow, 1, totalSizeLow, lowResFile);
        fread(m_totalDataHigh, 1, totalSizeHigh, highResFile);

        fclose(lowResFile);
        lowResFile = NULL;
        fclose(highResFile);
        highResFile = NULL;


        m_initInfo = new InitialInfo;
        if (!m_initInfo)
        {
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            return;
        }

        memset_s(m_initInfo, sizeof(InitialInfo), 0);
        m_initInfo->bsNumVideo = 2;
        m_initInfo->bsNumAudio = 0;
        m_initInfo->packingPluginPath = "/usr/local/lib";
        m_initInfo->packingPluginName = "HighResPlusFullLowResPacking";
        m_initInfo->videoProcessPluginPath = "/usr/local/lib";
        m_initInfo->videoProcessPluginName = "HevcVideoStreamProcess";
        m_initInfo->bsBuffers = new BSBuffer[2];
        if (!m_initInfo->bsBuffers)
        {
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            DELETE_MEMORY(m_initInfo);
            return;
        }

        m_initInfo->bsBuffers[0].data = m_lowResHeader;
        m_initInfo->bsBuffers[0].dataSize = lowResHeaderSize;
        m_initInfo->bsBuffers[0].mediaType = MediaType::VIDEOTYPE;
        m_initInfo->bsBuffers[0].codecId = CodecId::CODEC_ID_H265;
        m_initInfo->bsBuffers[0].bitRate = 3990720; //bps
        m_initInfo->bsBuffers[0].frameRate.num = 25;
        m_initInfo->bsBuffers[0].frameRate.den = 1;

        m_initInfo->bsBuffers[1].data = m_highResHeader;
        m_initInfo->bsBuffers[1].dataSize = highResHeaderSize;
        m_initInfo->bsBuffers[1].mediaType = MediaType::VIDEOTYPE;
        m_initInfo->bsBuffers[1].codecId = CodecId::CODEC_ID_H265;
        m_initInfo->bsBuffers[1].bitRate = 4166280;
        m_initInfo->bsBuffers[1].frameRate.num = 25;
        m_initInfo->bsBuffers[1].frameRate.den = 1;

        m_initInfo->segmentationInfo = new SegmentationInfo;
        if (!m_initInfo->segmentationInfo)
        {
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            DELETE_ARRAY(m_initInfo->bsBuffers);
            DELETE_MEMORY(m_initInfo);
            return;
        }

        memset_s(m_initInfo->segmentationInfo, sizeof(SegmentationInfo), 0);
        m_initInfo->segmentationInfo->needBufedFrames = 0;
        m_initInfo->segmentationInfo->segDuration = 2;
        m_initInfo->segmentationInfo->dirName = "./test/";
        m_initInfo->segmentationInfo->outName = "Test";
        m_initInfo->segmentationInfo->baseUrl = NULL;
        m_initInfo->segmentationInfo->utcTimingUrl = NULL;
        m_initInfo->segmentationInfo->isLive = true;

        m_initInfo->viewportInfo = new ViewportInformation;
        if (!m_initInfo->viewportInfo)
        {
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            DELETE_ARRAY(m_initInfo->bsBuffers);
            DELETE_MEMORY(m_initInfo->segmentationInfo);
            DELETE_MEMORY(m_initInfo);
            return;
        }

        memset_s(m_initInfo->viewportInfo, sizeof(ViewportInformation), 0);
        m_initInfo->viewportInfo->viewportWidth      = 1024;
        m_initInfo->viewportInfo->viewportHeight     = 1024;
        m_initInfo->viewportInfo->viewportPitch      = 0;
        m_initInfo->viewportInfo->viewportYaw        = 90;
        m_initInfo->viewportInfo->horizontalFOVAngle = 80;
        m_initInfo->viewportInfo->verticalFOVAngle   = 90;
        m_initInfo->viewportInfo->outGeoType         = E_SVIDEO_VIEWPORT;
        m_initInfo->viewportInfo->inGeoType          = E_SVIDEO_EQUIRECT;

        m_initInfo->projType = E_SVIDEO_EQUIRECT;

        m_omafPackage = new OmafPackage();
        if (!m_omafPackage)
        {
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            DELETE_ARRAY(m_initInfo->bsBuffers);
            DELETE_MEMORY(m_initInfo->segmentationInfo);
            DELETE_MEMORY(m_initInfo->viewportInfo);
            DELETE_MEMORY(m_initInfo);
            return;
        }

        int32_t ret = m_omafPackage->InitOmafPackage(m_initInfo);
        if (ret)
        {
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            DELETE_ARRAY(m_initInfo->bsBuffers);
            DELETE_MEMORY(m_initInfo->segmentationInfo);
            DELETE_MEMORY(m_initInfo->viewportInfo);
            DELETE_MEMORY(m_initInfo);
            DELETE_MEMORY(m_omafPackage);
            return;
        }

    }
    virtual void TearDown()
    {

        DELETE_ARRAY(m_highResHeader);
        DELETE_ARRAY(m_lowResHeader);
        DELETE_ARRAY(m_totalDataLow);
        DELETE_ARRAY(m_totalDataHigh);

        if (m_initInfo)
        {
            DELETE_ARRAY(m_initInfo->bsBuffers);
            DELETE_MEMORY(m_initInfo->segmentationInfo);
            DELETE_MEMORY(m_initInfo->viewportInfo);

            delete m_initInfo;
            m_initInfo = NULL;
        }

        DELETE_MEMORY(m_omafPackage);
    }

    InitialInfo                     *m_initInfo;
    uint8_t                         *m_highResHeader;
    uint8_t                         *m_lowResHeader;
    uint8_t                         *m_totalDataLow;
    uint8_t                         *m_totalDataHigh;
    OmafPackage                     *m_omafPackage;
};

TEST_F(DefaultSegmentationTest, AllProcess)
{
    uint64_t frameSizeLow[5] = { 97161, 39, 544, 44, 1980 };
    uint64_t frameSizeHigh[5] = { 101531, 159, 613, 170, 1684 };
    uint64_t offsetLow = 0;
    uint64_t offsetHigh = 0;

    int32_t ret = 0;
    for (uint8_t frameIdx = 0; frameIdx < 5; frameIdx++)
    {
        FrameBSInfo *frameLowRes = new FrameBSInfo;
        EXPECT_TRUE(frameLowRes != NULL);
        frameLowRes->data = m_totalDataLow + offsetLow;
        frameLowRes->dataSize = frameSizeLow[frameIdx];
        frameLowRes->pts = frameIdx;
        if (frameIdx == 0)
        {
            frameLowRes->isKeyFrame = true;
        }
        else
        {
            frameLowRes->isKeyFrame = false;
        }
        offsetLow += frameSizeLow[frameIdx];

        FrameBSInfo *frameHighRes = new FrameBSInfo;
        EXPECT_TRUE(frameHighRes != NULL);
        frameHighRes->data = m_totalDataHigh + offsetHigh;
        frameHighRes->dataSize = frameSizeHigh[frameIdx];
        frameHighRes->pts = frameIdx;
        if (frameIdx == 0)
        {
            frameHighRes->isKeyFrame = true;
        }
        else
        {
            frameHighRes->isKeyFrame = false;
        }
        offsetHigh += frameSizeHigh[frameIdx];

        ret = m_omafPackage->OmafPacketStream(0, frameLowRes);
        EXPECT_TRUE(ret == ERROR_NONE);
        ret = m_omafPackage->OmafPacketStream(1, frameHighRes);
        EXPECT_TRUE(ret == ERROR_NONE);

        DELETE_MEMORY(frameLowRes);
        DELETE_MEMORY(frameHighRes);
    }
    usleep(500000);
    ret = m_omafPackage->OmafEndStreams();
    EXPECT_TRUE(ret == ERROR_NONE);

    EXPECT_TRUE(access(m_initInfo->segmentationInfo->dirName, 0) == 0);
    char initSegName1[1024];
    char initSegName2[1024];
    for (uint8_t i = 0; i < 10; i++)
    {
        snprintf(initSegName1, 1024, "./test/Test_track%d.init.mp4", i + 1);
        EXPECT_TRUE(access(initSegName1, 0) == 0);
        struct stat buf;
        EXPECT_TRUE(stat(initSegName1, &buf) == 0);
        EXPECT_TRUE(buf.st_size != 0);
    }

    for (uint8_t i = 0; i < 7; i++)
    {
        snprintf(initSegName2, 1024, "./test/Test_track%d.init.mp4", 1000+i);
        EXPECT_TRUE(access(initSegName2, 0) == 0);
        struct stat buf;
        EXPECT_TRUE(stat(initSegName2, &buf) == 0);
        EXPECT_TRUE(buf.st_size != 0);
    }

    usleep(1000000);
    char segName[1024];
    for (uint8_t i = 0; i < 10; i++)
    {
        snprintf(segName, 1024, "./test/Test_track%d.1.mp4", i + 1);
        EXPECT_TRUE(access(segName, 0) == 0);
        struct stat buf;
        EXPECT_TRUE(stat(segName, &buf) == 0);
        EXPECT_TRUE(buf.st_size != 0);
    }

    for (uint8_t i = 0; i < 7; i++)
    {
        snprintf(segName, 1024, "./test/Test_track%d.1.mp4", i + 1000);
        EXPECT_TRUE(access(segName, 0) == 0);
        struct stat buf;
        EXPECT_TRUE(stat(segName, &buf) == 0);
        EXPECT_TRUE(buf.st_size != 0);
    }
}
}
