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
//! \file:   testExtractorTrack.cpp
//! \brief:  Extractor track class unit test
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include <dlfcn.h>
#include "gtest/gtest.h"
#include "VideoStreamPluginAPI.h"
#include "../ExtractorTrackManager.h"

VCD_USE_VRVIDEO;

namespace {
class ExtractorTrackTest : public testing::Test
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

        uint8_t lowResStreamIdx = 0;
        uint8_t highResStreamIdx = 1;

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

        m_initInfo->segmentationInfo->needBufedFrames = 15;
        m_initInfo->segmentationInfo->segDuration = 2;
        m_initInfo->segmentationInfo->dirName = "./test/";
        m_initInfo->segmentationInfo->outName = "Test";
        m_initInfo->segmentationInfo->baseUrl = NULL;
        m_initInfo->segmentationInfo->utcTimingUrl = NULL;

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

        m_initInfo->viewportInfo->viewportWidth      = 1024;
        m_initInfo->viewportInfo->viewportHeight     = 1024;
        m_initInfo->viewportInfo->viewportPitch      = 0;
        m_initInfo->viewportInfo->viewportYaw        = 90;
        m_initInfo->viewportInfo->horizontalFOVAngle = 80;
        m_initInfo->viewportInfo->verticalFOVAngle   = 90;
        m_initInfo->viewportInfo->outGeoType         = E_SVIDEO_VIEWPORT;
        m_initInfo->viewportInfo->inGeoType          = E_SVIDEO_EQUIRECT;

        m_initInfo->projType = E_SVIDEO_EQUIRECT;

        m_vsPlugin = NULL;
        m_vsPlugin = dlopen("/usr/local/lib/libHevcVideoStreamProcess.so", RTLD_LAZY);
        if (!m_vsPlugin)
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

        CreateVideoStream* createVS = NULL;
        createVS = (CreateVideoStream*)dlsym(m_vsPlugin, "Create");
        const char* dlsymErr1 = dlerror();
        if (dlsymErr1)
        {
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            DELETE_ARRAY(m_initInfo->bsBuffers);
            DELETE_MEMORY(m_initInfo->segmentationInfo);
            DELETE_MEMORY(m_initInfo->viewportInfo);
            DELETE_MEMORY(m_initInfo);
            dlclose(m_vsPlugin);
            m_vsPlugin = NULL;
            return;
        }
        if (!createVS)
        {
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            DELETE_ARRAY(m_initInfo->bsBuffers);
            DELETE_MEMORY(m_initInfo->segmentationInfo);
            DELETE_MEMORY(m_initInfo->viewportInfo);
            DELETE_MEMORY(m_initInfo);
            dlclose(m_vsPlugin);
            m_vsPlugin = NULL;
            return;
        }

        DestroyVideoStream* destroyVS = NULL;
        destroyVS = (DestroyVideoStream*)dlsym(m_vsPlugin, "Destroy");
        const char *dlsymErr = dlerror();
        if (dlsymErr || !destroyVS)
        {
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            DELETE_ARRAY(m_initInfo->bsBuffers);
            DELETE_MEMORY(m_initInfo->segmentationInfo);
            DELETE_MEMORY(m_initInfo->viewportInfo);
            DELETE_MEMORY(m_initInfo);
            dlclose(m_vsPlugin);
            m_vsPlugin = NULL;
            return;
        }

        VideoStream *vsLow = createVS();
        if (!vsLow)
        {
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            DELETE_ARRAY(m_initInfo->bsBuffers);
            DELETE_MEMORY(m_initInfo->segmentationInfo);
            DELETE_MEMORY(m_initInfo->viewportInfo);
            DELETE_MEMORY(m_initInfo);
            dlclose(m_vsPlugin);
            m_vsPlugin = NULL;
            return;
        }

        ((MediaStream*)vsLow)->SetMediaType(VIDEOTYPE);
        ((MediaStream*)vsLow)->SetCodecId(CODEC_ID_H265);

        int32_t ret = vsLow->Initialize(lowResStreamIdx, &(m_initInfo->bsBuffers[0]), m_initInfo);
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
            destroyVS((VideoStream*)(vsLow));
            dlclose(m_vsPlugin);
            m_vsPlugin = NULL;
            return;
        }

        m_streams.insert(std::make_pair(lowResStreamIdx, (MediaStream*)vsLow));

        //Create and Initialize high resolution video stream
        VideoStream *vsHigh = createVS();
        if (!vsHigh)
        {
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            DELETE_ARRAY(m_initInfo->bsBuffers);
            DELETE_MEMORY(m_initInfo->segmentationInfo);
            DELETE_MEMORY(m_initInfo->viewportInfo);
            DELETE_MEMORY(m_initInfo);
            destroyVS((VideoStream*)(vsLow));
            dlclose(m_vsPlugin);
            m_vsPlugin = NULL;
            return;
        }

        ((MediaStream*)vsHigh)->SetMediaType(VIDEOTYPE);
        ((MediaStream*)vsHigh)->SetCodecId(CODEC_ID_H265);

        ret = vsHigh->Initialize(highResStreamIdx, &(m_initInfo->bsBuffers[1]), m_initInfo);
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
            destroyVS((VideoStream*)(vsLow));
            destroyVS((VideoStream*)(vsHigh));
            dlclose(m_vsPlugin);
            m_vsPlugin = NULL;
            return;
        }

        m_streams.insert(std::make_pair(highResStreamIdx, (MediaStream*)vsHigh));

        m_extractorTrackMan = new ExtractorTrackManager(m_initInfo);
        if (!m_extractorTrackMan)
        {
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            DELETE_ARRAY(m_initInfo->bsBuffers);
            DELETE_MEMORY(m_initInfo->segmentationInfo);
            DELETE_MEMORY(m_initInfo->viewportInfo);
            DELETE_MEMORY(m_initInfo);
            destroyVS((VideoStream*)(vsLow));
            destroyVS((VideoStream*)(vsHigh));
            dlclose(m_vsPlugin);
            m_vsPlugin = NULL;
            return;
        }

        ret = m_extractorTrackMan->Initialize(&m_streams);
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
            destroyVS((VideoStream*)(vsLow));
            destroyVS((VideoStream*)(vsHigh));
            dlclose(m_vsPlugin);
            m_vsPlugin = NULL;
            DELETE_MEMORY(m_extractorTrackMan);
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

        DELETE_MEMORY(m_extractorTrackMan);

        if (m_vsPlugin)
        {
            DestroyVideoStream* destroyVS = NULL;
            destroyVS = (DestroyVideoStream*)dlsym(m_vsPlugin, "Destroy");
            const char *dlsymErr = dlerror();
            if (dlsymErr)
            {
                return;
            }
            if (!destroyVS)
            {
                return;
            }

            std::map<uint8_t, MediaStream*>::iterator it;
            for (it = m_streams.begin(); it != m_streams.end();)
            {
                MediaStream *stream = it->second;
                if (stream && (stream->GetCodecId() == CODEC_ID_H265))
                {
                    destroyVS((VideoStream*)(stream));
                }
                m_streams.erase(it++);
            }
            m_streams.clear();

            dlclose(m_vsPlugin);
            m_vsPlugin = NULL;
        }
    }

    std::map<uint8_t, MediaStream*> m_streams;
    ExtractorTrackManager           *m_extractorTrackMan;
    InitialInfo                     *m_initInfo;
    uint8_t                         *m_highResHeader;
    uint8_t                         *m_lowResHeader;
    uint8_t                         *m_totalDataLow;
    uint8_t                         *m_totalDataHigh;
    void                            *m_vsPlugin;
};

TEST_F(ExtractorTrackTest, AllProcess)
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
        if (!frameLowRes)
        {
            return;
        }
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
        if (!frameHighRes)
        {
            DELETE_MEMORY(frameLowRes);
            return;
        }
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

        std::map<uint8_t, MediaStream*>::iterator itStream;
        itStream = m_streams.begin();
        EXPECT_TRUE(itStream->first == 0);
        VideoStream *vsLow = (VideoStream*)(itStream->second);
        ret = vsLow->AddFrameInfo(frameLowRes);
        EXPECT_TRUE(ret == ERROR_NONE);
        vsLow->SetCurrFrameInfo();
        EXPECT_TRUE(vsLow->GetCurrFrameInfo() != NULL);
        ret = vsLow->UpdateTilesNalu();
        EXPECT_TRUE(ret == ERROR_NONE);
        itStream++;
        EXPECT_TRUE(itStream->first == 1);
        VideoStream *vsHigh = (VideoStream*)(itStream->second);
        ret = vsHigh->AddFrameInfo(frameHighRes);
        EXPECT_TRUE(ret == ERROR_NONE);
        vsHigh->SetCurrFrameInfo();
        EXPECT_TRUE(vsHigh->GetCurrFrameInfo() != NULL);
        ret = vsHigh->UpdateTilesNalu();
        EXPECT_TRUE(ret == ERROR_NONE);

        std::map<uint16_t, ExtractorTrack*> *extractorTracks = m_extractorTrackMan->GetAllExtractorTracks();
        EXPECT_TRUE(extractorTracks != NULL);
        std::map<uint16_t, ExtractorTrack*>::iterator it;
        for (it = extractorTracks->begin(); it != extractorTracks->end(); it++)
        {
            uint16_t viewportIdx = it->first;

            ExtractorTrack *extractorTrack = it->second;
            Nalu *vpsNalu = extractorTrack->GetVPS();
            Nalu *spsNalu = extractorTrack->GetSPS();
            Nalu *ppsNalu = extractorTrack->GetPPS();

            RegionWisePacking *dstRwpk = extractorTrack->GetRwpk();
            ContentCoverage   *dstCovi = extractorTrack->GetCovi();
            std::list<PicResolution> *picResList = extractorTrack->GetPicRes();
            Nalu *projSEI = extractorTrack->GetProjectionSEI();
            Nalu *rwpkSEI = extractorTrack->GetRwpkSEI();
            TilesMergeDirectionInCol *tilesMerDir = extractorTrack->GetTilesMergeDir();

            FILE *fp = fopen("extractorTrack_vps.bin", "r");
            if (!fp)
            {
                DELETE_MEMORY(frameLowRes);
                DELETE_MEMORY(frameHighRes);
                return;
            }
            fseek(fp, 0L, SEEK_END);
            uint64_t vpsLen = ftell(fp);
            fseek(fp, 0L, SEEK_SET);
            uint8_t *vpsData = new uint8_t[vpsLen];
            if (!vpsData)
            {
                DELETE_MEMORY(frameLowRes);
                DELETE_MEMORY(frameHighRes);
                fclose(fp);
                fp = NULL;
                return;
            }
            fread(vpsData, 1, vpsLen, fp);

            int32_t compRet = 0;
            int result = memcmp_s(vpsNalu->data, vpsLen, vpsData, vpsLen, &compRet);
            EXPECT_TRUE(result == 0);
            EXPECT_TRUE(vpsNalu->dataSize == vpsLen);
            EXPECT_TRUE(compRet == 0);
            EXPECT_TRUE(vpsNalu->startCodesSize == 4);
            EXPECT_TRUE(vpsNalu->naluType == 32);

            delete[] vpsData;
            vpsData = NULL;
            fclose(fp);
            fp = NULL;
            char spsFile[256] = { 0 };
            snprintf(spsFile, 256, "extractorTrack%d_sps.bin", viewportIdx);
            fp = fopen(spsFile, "r");
            if (!fp)
            {
                DELETE_MEMORY(frameLowRes);
                DELETE_MEMORY(frameHighRes);
                return;
            }
            fseek(fp, 0L, SEEK_END);
            uint32_t spsLen = ftell(fp);
            fseek(fp, 0L, SEEK_SET);
            uint8_t *spsData = new uint8_t[spsLen];
            if (!spsData)
            {
                DELETE_MEMORY(frameLowRes);
                DELETE_MEMORY(frameHighRes);
                fclose(fp);
                fp = NULL;
                return;
            }
            fread(spsData, 1, spsLen, fp);
            result = memcmp_s(spsNalu->data, spsLen, spsData, spsLen, &compRet);
            EXPECT_TRUE(result == 0);
            EXPECT_TRUE(spsNalu->dataSize == spsLen); //includes start codes
            EXPECT_TRUE(compRet == 0);
            EXPECT_TRUE(spsNalu->startCodesSize == 4);
            EXPECT_TRUE(spsNalu->naluType == 33);

            delete[] spsData;
            spsData = NULL;
            fclose(fp);
            fp = NULL;

            char ppsFile[256] = { 0 };
            snprintf(ppsFile, 256, "extractorTrack%d_pps.bin", viewportIdx);
            fp = fopen(ppsFile, "r");

            if (!fp)
            {
                DELETE_MEMORY(frameLowRes);
                DELETE_MEMORY(frameHighRes);
                return;
            }
            fseek(fp, 0L, SEEK_END);
            uint32_t ppsLen = ftell(fp);
            fseek(fp, 0L, SEEK_SET);
            uint8_t *ppsData = new uint8_t[ppsLen];
            if (!ppsData)
            {
                DELETE_MEMORY(frameLowRes);
                DELETE_MEMORY(frameHighRes);
                fclose(fp);
                fp = NULL;
                return;
            }
            fread(ppsData, 1, ppsLen, fp);
            result = memcmp_s(ppsNalu->data, ppsLen, ppsData, ppsLen, &compRet);
            EXPECT_TRUE(result == 0);
            EXPECT_TRUE(ppsNalu->dataSize == ppsLen); //includes start codes
            EXPECT_TRUE(compRet == 0);
            EXPECT_TRUE(ppsNalu->startCodesSize == 4);
            EXPECT_TRUE(ppsNalu->naluType == 34);

            delete[] ppsData;
            ppsData = NULL;
            fclose(fp);
            fp = NULL;
            switch (viewportIdx) {
                case 0:
                case 2:
                case 4:
                    EXPECT_TRUE(dstRwpk->constituentPicMatching == 0);
                    EXPECT_TRUE(dstRwpk->numRegions == 6);
                    EXPECT_TRUE(dstRwpk->projPicWidth == 3840);
                    EXPECT_TRUE(dstRwpk->projPicHeight == 1920);
                    EXPECT_TRUE(dstRwpk->packedPicWidth == 2880);
                    EXPECT_TRUE(dstRwpk->packedPicHeight == 1920);
                    break;
                case 1:
                case 3:
                case 6:
                    EXPECT_TRUE(dstRwpk->constituentPicMatching == 0);
                    EXPECT_TRUE(dstRwpk->numRegions == 8);
                    EXPECT_TRUE(dstRwpk->projPicWidth == 3840);
                    EXPECT_TRUE(dstRwpk->projPicHeight == 1920);
                    EXPECT_TRUE(dstRwpk->packedPicWidth == 3840);
                    EXPECT_TRUE(dstRwpk->packedPicHeight == 1920);
                    break;
                case 5:
                    EXPECT_TRUE(dstRwpk->constituentPicMatching == 0);
                    EXPECT_TRUE(dstRwpk->numRegions == 10);
                    EXPECT_TRUE(dstRwpk->projPicWidth == 3840);
                    EXPECT_TRUE(dstRwpk->projPicHeight == 1920);
                    EXPECT_TRUE(dstRwpk->packedPicWidth == 4800);
                    EXPECT_TRUE(dstRwpk->packedPicHeight == 1920);
                    break;
                default:
                    EXPECT_TRUE(dstRwpk->constituentPicMatching == 0);
                    EXPECT_TRUE(dstRwpk->numRegions == 10);
                    EXPECT_TRUE(dstRwpk->projPicWidth == 3840);
                    EXPECT_TRUE(dstRwpk->projPicHeight == 1920);
                    EXPECT_TRUE(dstRwpk->packedPicWidth == 2880);
                    EXPECT_TRUE(dstRwpk->packedPicHeight == 1920);
                    break;
            }
            EXPECT_TRUE(dstCovi->coverageShapeType == 1);
            EXPECT_TRUE(dstCovi->numRegions == 1);
            EXPECT_TRUE(dstCovi->viewIdcPresenceFlag == false);
            EXPECT_TRUE(dstCovi->defaultViewIdc == 0);
            uint16_t idx = 0;

            switch (viewportIdx) {
                case 0:
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].centreAzimuth == -11796480);
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].centreElevation == -5898240);
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].azimuthRange == 8192000);
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].elevationRange == 8847360);
                    break;
                case 1:
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].centreAzimuth == -11796480);
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].centreElevation == -2949120);
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].azimuthRange == 8192000);
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].elevationRange == 8847360);
                    break;
                case 2:
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].centreAzimuth == -11796480);
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].centreElevation == 0);
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].azimuthRange == 8192000);
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].elevationRange == 8847360);
                    break;
                case 3:
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].centreAzimuth == -11796480);
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].centreElevation == 2949120);
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].azimuthRange == 8192000);
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].elevationRange == 8847360);
                    break;
                case 4:
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].centreAzimuth == -11796480);
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].centreElevation == 5898240);
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].azimuthRange == 8192000);
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].elevationRange == 8847360);
                    break;
                case 5:
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].centreAzimuth == -8847360);
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].centreElevation == -2949120);
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].azimuthRange == 8192000);
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].elevationRange == 8847360);
                    break;
                case 6:
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].centreAzimuth == -8847360);
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].centreElevation == 0);
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].azimuthRange == 8192000);
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].elevationRange == 8847360);
                    break;
                default:
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].centreAzimuth == 5898240);
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].centreElevation == 0);
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].azimuthRange == 11796480);
                    EXPECT_TRUE(dstCovi->sphereRegions[idx].elevationRange == 11796480);
                    break;
            }

            EXPECT_TRUE(picResList->size() == 2);
            std::list<PicResolution>::iterator itRes;
            itRes = picResList->begin();
            EXPECT_TRUE(itRes->width == 3840);
            EXPECT_TRUE(itRes->height == 1920);
            itRes++;
            EXPECT_TRUE(itRes->width == 1920);
            EXPECT_TRUE(itRes->height == 960);

            fp = fopen("extractorTrack_projSEI.bin", "r");
            if (!fp)
            {
                DELETE_MEMORY(frameLowRes);
                DELETE_MEMORY(frameHighRes);
                return;
            }
            fseek(fp, 0L, SEEK_END);
            uint64_t projSEILen = ftell(fp);
            fseek(fp, 0L, SEEK_SET);
            uint8_t *projSEIData = new uint8_t[projSEILen];
            if (!projSEIData)
            {
                DELETE_MEMORY(frameLowRes);
                DELETE_MEMORY(frameHighRes);
                fclose(fp);
                fp = NULL;
                return;
            }
            fread(projSEIData, 1, projSEILen, fp);

            result = memcmp_s(projSEI->data, projSEILen, projSEIData, projSEILen, &compRet);
            EXPECT_TRUE(result == 0);
            EXPECT_TRUE(projSEI->dataSize == projSEILen);
            EXPECT_TRUE(compRet == 0);

            delete[] projSEIData;
            projSEIData = NULL;
            fclose(fp);
            fp = NULL;

            EXPECT_TRUE(rwpkSEI != NULL);
            EXPECT_TRUE(rwpkSEI->data != NULL);
            EXPECT_TRUE(rwpkSEI->dataSize != 4);

            EXPECT_TRUE(tilesMerDir != NULL);
            int32_t tilesArrangeInColSize = tilesMerDir->tilesArrangeInCol.size();
            if ((viewportIdx == 0) || (viewportIdx == 2) || (viewportIdx == 4))
            {
                EXPECT_TRUE(tilesMerDir->tilesArrangeInCol.size() == 3);
            }
            else if ((viewportIdx == 1) || (viewportIdx == 3) || (viewportIdx == 6))
            {
                EXPECT_TRUE(tilesMerDir->tilesArrangeInCol.size() == 4);
            }
            else if (viewportIdx == 5)
            {
                EXPECT_TRUE(tilesMerDir->tilesArrangeInCol.size() == 5);
            }

            std::list<TilesInCol*>::iterator itCol;
            itCol = tilesMerDir->tilesArrangeInCol.begin();
            tilesArrangeInColSize--;
            TilesInCol *tileCol = *itCol;
            std::list<SingleTile*>::iterator itTile;
            SingleTile *tile;

            EXPECT_TRUE(tileCol->size() == 2);
            itTile = tileCol->begin();
            tile = *itTile;
            EXPECT_TRUE(tile->streamIdxInMedia == 1);
            EXPECT_TRUE(tile->dstCTUIndex == 0);
            itTile++;
            tile = *itTile;
            EXPECT_TRUE(tile->streamIdxInMedia == 1);

            if ((viewportIdx == 0) || (viewportIdx == 2) || (viewportIdx == 4))
            {
                EXPECT_TRUE(tile->dstCTUIndex == 675);
            }
            else if ((viewportIdx == 1) || (viewportIdx == 3) || (viewportIdx == 6))
            {
                EXPECT_TRUE(tile->dstCTUIndex == 900);
            }
            else if (viewportIdx == 5)
            {
                EXPECT_TRUE(tile->dstCTUIndex == 1125);
            }

            itCol++;
            tilesArrangeInColSize--;
            tileCol = *itCol;
            EXPECT_TRUE(tileCol->size() == 2);
            itTile = tileCol->begin();
            tile = *itTile;
            EXPECT_TRUE(tile->streamIdxInMedia == 1);
            EXPECT_TRUE(tile->dstCTUIndex == 15);

            itTile++;
            tile = *itTile;
            EXPECT_TRUE(tile->streamIdxInMedia == 1);
            if ((viewportIdx == 0) || (viewportIdx == 2) || (viewportIdx == 4))
            {
                EXPECT_TRUE(tile->dstCTUIndex == 690);
            }
            else if ((viewportIdx == 1) || (viewportIdx == 3) || (viewportIdx == 6))
            {
                EXPECT_TRUE(tile->dstCTUIndex == 915);
            }
            else if (viewportIdx == 5)
            {
                EXPECT_TRUE(tile->dstCTUIndex == 1140);
            }

            itCol++;
            tilesArrangeInColSize--;
            tileCol = *itCol;
            EXPECT_TRUE(tileCol->size() == 2);
            itTile = tileCol->begin();
            tile = *itTile;
            switch (viewportIdx) {
                case 0:
                case 2:
                case 4:
                    EXPECT_TRUE(tile->streamIdxInMedia == 0);
                    break;
                case 1:
                case 3:
                case 5:
                case 6:
                    EXPECT_TRUE(tile->streamIdxInMedia == 1);
                    break;
                default:
                    EXPECT_TRUE(tile->streamIdxInMedia == 1);
                    break;
            }
            EXPECT_TRUE(tile->dstCTUIndex == 30);

            itTile++;
            tile = *itTile;
            if  ((viewportIdx == 0) || (viewportIdx == 2) || (viewportIdx == 4))
            {
                EXPECT_TRUE(tile->streamIdxInMedia == 0);
                EXPECT_TRUE(tile->dstCTUIndex == 705);
            }
            else if ((viewportIdx == 1) || (viewportIdx == 3) || (viewportIdx == 6))
            {
                EXPECT_TRUE(tile->streamIdxInMedia == 1);
                EXPECT_TRUE(tile->dstCTUIndex == 930);
            }
            else if (viewportIdx == 5)
            {
                EXPECT_TRUE(tile->streamIdxInMedia == 1);
                EXPECT_TRUE(tile->dstCTUIndex == 1155);
            }
            if (tilesArrangeInColSize != 0) {
                if ((viewportIdx == 1) || (viewportIdx == 3) || (viewportIdx == 6))
                {
                    itCol++;
                    tileCol = *itCol;
                    EXPECT_TRUE(tileCol->size() == 2);
                    itTile = tileCol->begin();
                    tile = *itTile;
                    EXPECT_TRUE(tile->streamIdxInMedia == 0);
                    EXPECT_TRUE(tile->dstCTUIndex == 45);
                    itTile++;
                    tile = *itTile;
                    EXPECT_TRUE(tile->streamIdxInMedia == 0);
                    EXPECT_TRUE(tile->dstCTUIndex == 945);
                }
                else if (viewportIdx == 5)
                {
                    itCol++;
                    tileCol = *itCol;
                    EXPECT_TRUE(tileCol->size() == 2);
                    itTile = tileCol->begin();
                    tile = *itTile;
                    EXPECT_TRUE(tile->streamIdxInMedia == 1);
                    EXPECT_TRUE(tile->dstCTUIndex == 45);
                    itTile++;
                    tile = *itTile;
                    EXPECT_TRUE(tile->streamIdxInMedia == 1);
                    EXPECT_TRUE(tile->dstCTUIndex == 1170);
                }
                tilesArrangeInColSize--;
            }
            if (tilesArrangeInColSize != 0) {
                if (viewportIdx == 5)
                {
                    itCol++;
                    tileCol = *itCol;
                    EXPECT_TRUE(tileCol->size() == 2);
                    itTile = tileCol->begin();
                    tile = *itTile;
                    EXPECT_TRUE(tile->streamIdxInMedia == 0);
                    EXPECT_TRUE(tile->dstCTUIndex == 60);
                    itTile++;
                    tile = *itTile;
                    EXPECT_TRUE(tile->streamIdxInMedia == 0);
                    EXPECT_TRUE(tile->dstCTUIndex == 1185);
                }
            }

            extractorTrack->ConstructExtractors();
            std::map<uint8_t, Extractor*> *extractors = extractorTrack->GetAllExtractors();
            if  ((viewportIdx == 0) || (viewportIdx == 2) || (viewportIdx == 4))
            {
                EXPECT_TRUE(extractors->size() == 6);
            }
            else if ((viewportIdx == 1) || (viewportIdx == 3) || (viewportIdx == 6))
            {
                EXPECT_TRUE(extractors->size() == 8);
            }
            else if (viewportIdx == 5)
            {
                EXPECT_TRUE(extractors->size() == 10);
            }

            std::map<uint8_t, Extractor*>::iterator itExtractor;
            for (itExtractor = extractors->begin();
                itExtractor != extractors->end();
                itExtractor++)
            {
                Extractor *extractor = itExtractor->second;
                EXPECT_TRUE(extractor->sampleConstructor.size() == 1);
                EXPECT_TRUE(extractor->inlineConstructor.size() == 1);
                std::list<InlineConstructor*>::iterator itInlineCtor;
                itInlineCtor = extractor->inlineConstructor.begin();
                InlineConstructor *inlineCtor = *itInlineCtor;
                EXPECT_TRUE(inlineCtor->length != 0);
                EXPECT_TRUE(inlineCtor->inlineData != NULL);
            }

            ret = extractorTrack->DestroyExtractors();
            EXPECT_TRUE(ret == ERROR_NONE);
        }

        DELETE_MEMORY(frameLowRes);
        DELETE_MEMORY(frameHighRes);
    }

}
}
