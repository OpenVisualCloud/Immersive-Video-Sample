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

#include "gtest/gtest.h"
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
        m_initInfo->pluginPath = "/usr/local/lib";
        m_initInfo->pluginName = "HighResPlusFullLowResPacking";
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

        VideoStream *vsLow = new VideoStream();
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
            return;
        }

        ((MediaStream*)vsLow)->SetMediaType(VIDEOTYPE);

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
            DELETE_MEMORY(vsLow);
            return;
        }

        m_streams.insert(std::make_pair(lowResStreamIdx, (MediaStream*)vsLow));

        //Create and Initialize high resolution video stream
        VideoStream *vsHigh = new VideoStream();
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
            DELETE_MEMORY(vsLow);
            return;
        }

        ((MediaStream*)vsHigh)->SetMediaType(VIDEOTYPE);

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
            DELETE_MEMORY(vsLow);
            DELETE_MEMORY(vsHigh);
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
            DELETE_MEMORY(vsLow);
            DELETE_MEMORY(vsHigh);
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
            DELETE_MEMORY(vsLow);
            DELETE_MEMORY(vsHigh);
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

        std::map<uint8_t, MediaStream*>::iterator it;
        for (it = m_streams.begin(); it != m_streams.end();)
        {
            DELETE_MEMORY(it->second);

            m_streams.erase(it++);
        }
        m_streams.clear();
    }

    std::map<uint8_t, MediaStream*> m_streams;
    ExtractorTrackManager           *m_extractorTrackMan;
    InitialInfo                     *m_initInfo;
    uint8_t                         *m_highResHeader;
    uint8_t                         *m_lowResHeader;
    uint8_t                         *m_totalDataLow;
    uint8_t                         *m_totalDataHigh;
};

TEST_F(ExtractorTrackTest, AllProcess)
{
    uint64_t frameSizeLow[5] = { 97161, 39, 544, 44, 1980 };
    uint64_t frameSizeHigh[5] = { 101531, 159, 613, 170, 1684 };
    uint64_t offsetLow = 0;
    uint64_t offsetHigh = 0;

    FILE *fpDataOffset = fopen("extractorsDataOffset.bin", "r");
    EXPECT_TRUE(fpDataOffset != NULL);
    if (!fpDataOffset)
        return;

    uint32_t sliceHrdLen[8];
    memset_s(sliceHrdLen, 8 * sizeof(uint32_t), 0);
    int32_t ret = 0;
    for (uint8_t frameIdx = 0; frameIdx < 5; frameIdx++)
    {
        FrameBSInfo *frameLowRes = new FrameBSInfo;
        EXPECT_TRUE(frameLowRes != NULL);
        if (!frameLowRes)
        {
            fclose(fpDataOffset);
            fpDataOffset = NULL;
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
            fclose(fpDataOffset);
            fpDataOffset = NULL;
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

        std::map<uint8_t, ExtractorTrack*> *extractorTracks = m_extractorTrackMan->GetAllExtractorTracks();
        EXPECT_TRUE(extractorTracks != NULL);

        std::map<uint8_t, ExtractorTrack*>::iterator it;
        for (it = extractorTracks->begin(); it != extractorTracks->end(); it++)
        {
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

                fclose(fpDataOffset);
                fpDataOffset = NULL;
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
                fclose(fpDataOffset);
                fpDataOffset = NULL;
                DELETE_MEMORY(frameLowRes);
                DELETE_MEMORY(frameHighRes);
                fclose(fp);
                fp = NULL;
                return;
            }
            fread(vpsData, 1, vpsLen, fp);

            int32_t compRet = 0;
            errno_t result = memcmp_s(vpsNalu->data, vpsLen, vpsData, vpsLen, &compRet);
            EXPECT_TRUE(result == EOK);
            EXPECT_TRUE(vpsNalu->dataSize == vpsLen);
            EXPECT_TRUE(compRet == 0);
            EXPECT_TRUE(vpsNalu->startCodesSize == 4);
            EXPECT_TRUE(vpsNalu->naluType == 32);

            delete[] vpsData;
            vpsData = NULL;
            fclose(fp);
            fp = NULL;

            fp = fopen("extractorTrack_sps.bin", "r");
            if (!fp)
            {
                fclose(fpDataOffset);
                fpDataOffset = NULL;
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
                fclose(fpDataOffset);
                fpDataOffset = NULL;
                DELETE_MEMORY(frameLowRes);
                DELETE_MEMORY(frameHighRes);
                fclose(fp);
                fp = NULL;
                return;
            }
            fread(spsData, 1, spsLen, fp);

            result = memcmp_s(spsNalu->data, spsLen, spsData, spsLen, &compRet);
            EXPECT_TRUE(result == EOK);
            EXPECT_TRUE(spsNalu->dataSize == spsLen); //includes start codes
            EXPECT_TRUE(compRet == 0);
            EXPECT_TRUE(spsNalu->startCodesSize == 4);
            EXPECT_TRUE(spsNalu->naluType == 33);

            delete[] spsData;
            spsData = NULL;
            fclose(fp);
            fp = NULL;

            fp = fopen("extractorTrack_pps.bin", "r");
            if (!fp)
            {
                fclose(fpDataOffset);
                fpDataOffset = NULL;
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
                fclose(fpDataOffset);
                fpDataOffset = NULL;
                DELETE_MEMORY(frameLowRes);
                DELETE_MEMORY(frameHighRes);
                fclose(fp);
                fp = NULL;
                return;
            }
            fread(ppsData, 1, ppsLen, fp);

            result = memcmp_s(ppsNalu->data, ppsLen, ppsData, ppsLen, &compRet);
            EXPECT_TRUE(result == EOK);
            EXPECT_TRUE(ppsNalu->dataSize == ppsLen); //includes start codes
            EXPECT_TRUE(compRet == 0);
            EXPECT_TRUE(ppsNalu->startCodesSize == 4);
            EXPECT_TRUE(ppsNalu->naluType == 34);

            delete[] ppsData;
            ppsData = NULL;
            fclose(fp);
            fp = NULL;

            RegionWisePacking rwpk;
            rwpk.constituentPicMatching = 0;
            rwpk.numRegions = 6;
            rwpk.projPicWidth = 3840;
            rwpk.projPicHeight = 1920;
            rwpk.packedPicWidth = 2880;
            rwpk.packedPicHeight = 1920;
            rwpk.rectRegionPacking = new RectangularRegionWisePacking[6];
            EXPECT_TRUE(rwpk.rectRegionPacking != NULL);
            if (!(rwpk.rectRegionPacking))
            {
                fclose(fpDataOffset);
                fpDataOffset = NULL;
                DELETE_MEMORY(frameLowRes);
                DELETE_MEMORY(frameHighRes);
                return;
            }

            for (int32_t i = 0; i < 4; i++)
            {
                memset_s(&(rwpk.rectRegionPacking[i]), sizeof(RectangularRegionWisePacking), 0);
                rwpk.rectRegionPacking[i].transformType = 0;
                rwpk.rectRegionPacking[i].guardBandFlag = false;
                rwpk.rectRegionPacking[i].projRegWidth = 960;
                rwpk.rectRegionPacking[i].projRegHeight = 960;
                if (it->first < 4)
                {
                    if (i % 2 == 0)
                    {
                        rwpk.rectRegionPacking[i].projRegTop = 0;
                    }
                    else
                    {
                        rwpk.rectRegionPacking[i].projRegTop = 960;
                    }
                }
                else
                {
                    if (i % 2 == 0)
                    {
                        rwpk.rectRegionPacking[i].projRegTop = 960;
                    }
                    else
                    {
                        rwpk.rectRegionPacking[i].projRegTop = 0;
                    }
                }
                if ((it->first) != 3 && (it->first) != 7)
                {
                    if (i == 0 || i == 1)
                    {
                        rwpk.rectRegionPacking[i].projRegLeft = ((it->first) % 4) * 960;
                    }
                    else
                    {
                        rwpk.rectRegionPacking[i].projRegLeft = ((it->first) % 4) * 960 + 960;
                    }
                }
                else
                {
                    if (i == 0 || i == 1)
                    {
                        rwpk.rectRegionPacking[i].projRegLeft = 2880;
                    }
                    else
                    {
                        rwpk.rectRegionPacking[i].projRegLeft = 0;
                    }
                }
                rwpk.rectRegionPacking[i].packedRegWidth = 960;
                rwpk.rectRegionPacking[i].packedRegHeight = 960;
                if (i == 0 || i == 2)
                {
                    rwpk.rectRegionPacking[i].packedRegTop = 0;
                }
                else
                {
                    rwpk.rectRegionPacking[i].packedRegTop = 960;
                }
                if (i == 0 || i == 1)
                {
                    rwpk.rectRegionPacking[i].packedRegLeft = 0;
                }
                else
                {
                    rwpk.rectRegionPacking[i].packedRegLeft = 960;
                }
                rwpk.rectRegionPacking[i].gbNotUsedForPredFlag = true;
            }
            memset_s(&(rwpk.rectRegionPacking[4]), sizeof(RectangularRegionWisePacking), 0);
            rwpk.rectRegionPacking[4].transformType = 0;
            rwpk.rectRegionPacking[4].guardBandFlag = false;
            rwpk.rectRegionPacking[4].projRegWidth = 1920;
            rwpk.rectRegionPacking[4].projRegHeight = 1920;
            rwpk.rectRegionPacking[4].projRegTop = 0;
            rwpk.rectRegionPacking[4].projRegLeft = 0;
            rwpk.rectRegionPacking[4].packedRegWidth = 960;
            rwpk.rectRegionPacking[4].packedRegHeight = 960;
            rwpk.rectRegionPacking[4].packedRegTop = 0;
            rwpk.rectRegionPacking[4].packedRegLeft = 1920;
            rwpk.rectRegionPacking[4].gbNotUsedForPredFlag = true;

            memset_s(&(rwpk.rectRegionPacking[5]), sizeof(RectangularRegionWisePacking), 0);
            rwpk.rectRegionPacking[5].transformType = 0;
            rwpk.rectRegionPacking[5].guardBandFlag = false;
            rwpk.rectRegionPacking[5].projRegWidth = 1920;
            rwpk.rectRegionPacking[5].projRegHeight = 1920;
            rwpk.rectRegionPacking[5].projRegTop = 0;
            rwpk.rectRegionPacking[5].projRegLeft = 1920;
            rwpk.rectRegionPacking[5].packedRegWidth = 960;
            rwpk.rectRegionPacking[5].packedRegHeight = 960;
            rwpk.rectRegionPacking[5].packedRegTop = 960;
            rwpk.rectRegionPacking[5].packedRegLeft = 1920;
            rwpk.rectRegionPacking[5].gbNotUsedForPredFlag = true;

            EXPECT_TRUE(dstRwpk->constituentPicMatching == rwpk.constituentPicMatching);
            EXPECT_TRUE(dstRwpk->numRegions == rwpk.numRegions);
            EXPECT_TRUE(dstRwpk->projPicWidth == rwpk.projPicWidth);
            EXPECT_TRUE(dstRwpk->projPicHeight == rwpk.projPicHeight);
            EXPECT_TRUE(dstRwpk->packedPicWidth == rwpk.packedPicWidth);
            EXPECT_TRUE(dstRwpk->packedPicHeight == rwpk.packedPicHeight);

            compRet = 0;
            for (uint16_t idx = 0; idx < rwpk.numRegions; idx++)
            {
                result = memcmp_s(&(dstRwpk->rectRegionPacking[idx]), sizeof(RectangularRegionWisePacking), &(rwpk.rectRegionPacking[idx]), sizeof(RectangularRegionWisePacking), &compRet);
                EXPECT_TRUE(result == EOK);
                EXPECT_TRUE(compRet == 0);
            }

            ContentCoverage covi;
            covi.coverageShapeType = 1;
            covi.numRegions        = 1;
            covi.viewIdcPresenceFlag = false;
            covi.defaultViewIdc      = 0;
            covi.sphereRegions       = new SphereRegion[covi.numRegions];
            EXPECT_TRUE(covi.sphereRegions != NULL);
            if (!(covi.sphereRegions))
            {
                fclose(fpDataOffset);
                fpDataOffset = NULL;
                DELETE_MEMORY(frameLowRes);
                DELETE_MEMORY(frameHighRes);
                DELETE_ARRAY(rwpk.rectRegionPacking);
                return;
            }

            for (uint16_t idx = 0; idx < covi.numRegions; idx++)
            {
                memset_s(&(covi.sphereRegions[idx]), sizeof(SphereRegion), 0);
                covi.sphereRegions[idx].viewIdc = 0;
                covi.sphereRegions[idx].centreAzimuth   = (int32_t)((((3840 / 2) - (float)(((it->first) % 4) * 960 + 1920 / 2)) * 360 * 65536) / 3840);
                covi.sphereRegions[idx].centreElevation = (int32_t)((((1920 / 2) - (float)(((it->first) / 4) * 960 + 1920 / 2)) * 180 * 65536) / 1920);
                covi.sphereRegions[idx].centreTilt      = 0;
                covi.sphereRegions[idx].azimuthRange    = (uint32_t)((1920 * 360.f * 65536) / 3840);
                covi.sphereRegions[idx].elevationRange  = (uint32_t)((1920 * 180.f * 65536) / 1920);
                covi.sphereRegions[idx].interpolate     = 0;
            }

            EXPECT_TRUE(dstCovi->coverageShapeType == covi.coverageShapeType);
            EXPECT_TRUE(dstCovi->numRegions == covi.numRegions);
            EXPECT_TRUE(dstCovi->viewIdcPresenceFlag == covi.viewIdcPresenceFlag);
            EXPECT_TRUE(dstCovi->defaultViewIdc == covi.defaultViewIdc);
            compRet = 0;
            for (uint16_t idx = 0; idx < dstCovi->numRegions; idx++)
            {
                result = memcmp_s(&(dstCovi->sphereRegions[idx]), sizeof(SphereRegion), &(covi.sphereRegions[idx]), sizeof(SphereRegion), &compRet);
                EXPECT_TRUE(result == EOK);
                EXPECT_TRUE(compRet == 0);
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
                fclose(fpDataOffset);
                fpDataOffset = NULL;
                DELETE_MEMORY(frameLowRes);
                DELETE_MEMORY(frameHighRes);
                DELETE_ARRAY(rwpk.rectRegionPacking);
                DELETE_ARRAY(covi.sphereRegions);
                return;
            }
            fseek(fp, 0L, SEEK_END);
            uint64_t projSEILen = ftell(fp);
            fseek(fp, 0L, SEEK_SET);
            uint8_t *projSEIData = new uint8_t[projSEILen];
            if (!projSEIData)
            {
                fclose(fpDataOffset);
                fpDataOffset = NULL;
                DELETE_MEMORY(frameLowRes);
                DELETE_MEMORY(frameHighRes);
                DELETE_ARRAY(rwpk.rectRegionPacking);
                DELETE_ARRAY(covi.sphereRegions);
                fclose(fp);
                fp = NULL;
                return;
            }
            fread(projSEIData, 1, projSEILen, fp);

            result = memcmp_s(projSEI->data, projSEILen, projSEIData, projSEILen, &compRet);
            EXPECT_TRUE(result == EOK);
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
            EXPECT_TRUE(tilesMerDir->tilesArrangeInCol.size() == 3);
            std::list<TilesInCol*>::iterator itCol;
            itCol = tilesMerDir->tilesArrangeInCol.begin();
            TilesInCol *tileCol = *itCol;
            EXPECT_TRUE(tileCol->size() == 2);
            std::list<SingleTile*>::iterator itTile;
            itTile = tileCol->begin();
            SingleTile *tile = *itTile;
            EXPECT_TRUE(tile->streamIdxInMedia == 1);
            EXPECT_TRUE(tile->origTileIdx == (it->first));
            EXPECT_TRUE(tile->dstCTUIndex == 0);
            itTile++;
            tile = *itTile;
            EXPECT_TRUE(tile->streamIdxInMedia == 1);
            EXPECT_TRUE(tile->origTileIdx == ((it->first) < 4 ? (it->first) + 4 : (it->first) - 4));
            EXPECT_TRUE(tile->dstCTUIndex == 675);
            itCol++;
            tileCol = *itCol;
            EXPECT_TRUE(tileCol->size() == 2);
            itTile = tileCol->begin();
            tile = *itTile;
            EXPECT_TRUE(tile->streamIdxInMedia == 1);
            if (it->first != 3 && it->first != 7)
            {
                EXPECT_TRUE(tile->origTileIdx == ((it->first) + 1));
            }
            else
            {
                EXPECT_TRUE(tile->origTileIdx == ((it->first) - 3));
            }
            EXPECT_TRUE(tile->dstCTUIndex == 15);
            itTile++;
            tile = *itTile;
            EXPECT_TRUE(tile->streamIdxInMedia == 1);
            if (it->first != 3 && it->first != 7)
            {
                if (it->first < 4)
                {
                    EXPECT_TRUE(tile->origTileIdx == ((it->first) + 5));
                }
                else
                {
                    EXPECT_TRUE(tile->origTileIdx == ((it->first) - 3));
                }
            }
            else if ((it->first) == 3)
            {
                EXPECT_TRUE(tile->origTileIdx == 4);
            }
            else
            {
                EXPECT_TRUE(tile->origTileIdx == 0);
            }
            EXPECT_TRUE(tile->dstCTUIndex == 690);
            itCol++;
            tileCol = *itCol;
            EXPECT_TRUE(tileCol->size() == 2);
            itTile = tileCol->begin();
            tile = *itTile;
            EXPECT_TRUE(tile->streamIdxInMedia == 0);
            EXPECT_TRUE(tile->origTileIdx == 0);
            EXPECT_TRUE(tile->dstCTUIndex == 30);
            itTile++;
            tile = *itTile;
            EXPECT_TRUE(tile->streamIdxInMedia == 0);
            EXPECT_TRUE(tile->origTileIdx == 1);
            EXPECT_TRUE(tile->dstCTUIndex == 705);

            fscanf(fpDataOffset, "%u,%u,%u,%u,%u,%u", &sliceHrdLen[0], &sliceHrdLen[1], &sliceHrdLen[2], &sliceHrdLen[3], &sliceHrdLen[4], &sliceHrdLen[5]);

            extractorTrack->ConstructExtractors();
            std::map<uint8_t, Extractor*> *extractors = extractorTrack->GetAllExtractors();
            EXPECT_TRUE(extractors->size() == 6);
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
                std::list<SampleConstructor*>::iterator itSmpCtor;
                itSmpCtor = extractor->sampleConstructor.begin();
                SampleConstructor *sampleCtor = *itSmpCtor;
                EXPECT_TRUE(sampleCtor->dataOffset == (DASH_SAMPLELENFIELD_SIZE + HEVC_NALUHEADER_LEN + sliceHrdLen[itExtractor->first]));
            }

            ret = extractorTrack->DestroyExtractors();
            EXPECT_TRUE(ret == ERROR_NONE);

            DELETE_ARRAY(rwpk.rectRegionPacking);
            DELETE_ARRAY(covi.sphereRegions);
        }

        DELETE_MEMORY(frameLowRes);
        DELETE_MEMORY(frameHighRes);
    }

    fclose(fpDataOffset);
    fpDataOffset = NULL;
}
}
