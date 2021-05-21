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
//! \file:   testVideoStream.cpp
//! \brief:  Video stream class unit test
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include <dlfcn.h>
#include "gtest/gtest.h"
#include "VideoStreamPluginAPI.h"
#include "error.h"
#include "../../utils/safe_mem.h"


class VideoStreamTest : public testing::Test
{
public:
    virtual void SetUp()
    {
        const char *highResFileName = "3840x1920_5frames.265";
        m_highResFile = fopen(highResFileName, "r");
        if (!m_highResFile)
            return;

        const char *lowResFileName = "1920x960_5frames.265";
        m_lowResFile = fopen(lowResFileName, "r");
        if (!m_lowResFile)
        {
            fclose(m_highResFile);
            m_highResFile = NULL;
            return;
        }

        m_lowResStreamIdx = 0;
        m_highResStreamIdx = 1;

        int32_t highResHeaderSize = 98;
        int32_t lowResHeaderSize = 97;
        m_highResHeader = new uint8_t[highResHeaderSize];
        if (!m_highResHeader)
        {
            fclose(m_highResFile);
            m_highResFile = NULL;
            fclose(m_lowResFile);
            m_lowResFile = NULL;
            return;
        }

        m_lowResHeader = new uint8_t[lowResHeaderSize];
        if (!m_lowResHeader)
        {
            fclose(m_highResFile);
            m_highResFile = NULL;
            fclose(m_lowResFile);
            m_lowResFile = NULL;
            DELETE_ARRAY(m_highResHeader);
            return;
        }

        fread(m_highResHeader, 1, highResHeaderSize, m_highResFile);
        fread(m_lowResHeader, 1, lowResHeaderSize, m_lowResFile);

        uint64_t totalSizeLow = 80062;
        m_totalDataLow = new uint8_t[totalSizeLow];
        if (!m_totalDataLow)
        {
            fclose(m_highResFile);
            m_highResFile = NULL;
            fclose(m_lowResFile);
            m_lowResFile = NULL;
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            return;
        }

        uint64_t totalSizeHigh = 98741;
        m_totalDataHigh = new uint8_t[totalSizeHigh];
        if (!m_totalDataHigh)
        {
            fclose(m_highResFile);
            m_highResFile = NULL;
            fclose(m_lowResFile);
            m_lowResFile = NULL;
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            return;
        }

        fread(m_totalDataLow, 1, totalSizeLow, m_lowResFile);
        fread(m_totalDataHigh, 1, totalSizeHigh, m_highResFile);

        FILE *fp = fopen("test_vps_lowRes.bin", "wb+");
        if (!fp)
        {
            fclose(m_highResFile);
            m_highResFile = NULL;
            fclose(m_lowResFile);
            m_lowResFile = NULL;
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            return;
        }
        fwrite(m_lowResHeader, 1, 30, fp);
        fclose(fp);
        fp = NULL;

        fp = fopen("test_sps_lowRes.bin", "wb+");
        if (!fp)
        {
            fclose(m_highResFile);
            m_highResFile = NULL;
            fclose(m_lowResFile);
            m_lowResFile = NULL;
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            return;
        }
        fwrite(m_lowResHeader + 30, 1, 47, fp);
        fclose(fp);
        fp = NULL;

        fp = fopen("test_pps_lowRes.bin", "wb+");
        if (!fp)
        {
            fclose(m_highResFile);
            m_highResFile = NULL;
            fclose(m_lowResFile);
            m_lowResFile = NULL;
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            return;
        }
        fwrite(m_lowResHeader + 77, 1, 11, fp);
        fclose(fp);
        fp = NULL;

        fp = fopen("test_vps_highRes.bin", "wb+");
        if (!fp)
        {
            fclose(m_highResFile);
            m_highResFile = NULL;
            fclose(m_lowResFile);
            m_lowResFile = NULL;
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            return;
        }
        fwrite(m_highResHeader, 1, 30, fp);
        fclose(fp);
        fp = NULL;

        fp = fopen("test_sps_highRes.bin", "wb+");
        if (!fp)
        {
            fclose(m_highResFile);
            m_highResFile = NULL;
            fclose(m_lowResFile);
            m_lowResFile = NULL;
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            return;
        }
        fwrite(m_highResHeader + 30, 1, 48, fp);
        fclose(fp);
        fp = NULL;

        fp = fopen("test_pps_highRes.bin", "wb+");
        if (!fp)
        {
            fclose(m_highResFile);
            m_highResFile = NULL;
            fclose(m_lowResFile);
            m_lowResFile = NULL;
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            return;
        }
        fwrite(m_highResHeader + 78, 1, 11, fp);
        fclose(fp);
        fp = NULL;

        m_initInfo = new InitialInfo;
        if (!m_initInfo)
        {
            fclose(m_highResFile);
            m_highResFile = NULL;
            fclose(m_lowResFile);
            m_lowResFile = NULL;
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
            fclose(m_highResFile);
            m_highResFile = NULL;
            fclose(m_lowResFile);
            m_lowResFile = NULL;
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
        m_initInfo->bsBuffers[0].bitRate = 3225140; //bps
        m_initInfo->bsBuffers[0].frameRate.num = 25;
        m_initInfo->bsBuffers[0].frameRate.den = 1;

        m_initInfo->bsBuffers[1].data = m_highResHeader;
        m_initInfo->bsBuffers[1].dataSize = highResHeaderSize;
        m_initInfo->bsBuffers[1].mediaType = MediaType::VIDEOTYPE;
        m_initInfo->bsBuffers[1].codecId = CodecId::CODEC_ID_H265;
        m_initInfo->bsBuffers[1].bitRate = 3273220;
        m_initInfo->bsBuffers[1].frameRate.num = 25;
        m_initInfo->bsBuffers[1].frameRate.den = 1;

        m_initInfo->segmentationInfo = new SegmentationInfo;
        if (!m_initInfo->segmentationInfo)
        {
            fclose(m_highResFile);
            m_highResFile = NULL;
            fclose(m_lowResFile);
            m_lowResFile = NULL;
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

        m_initInfo->projType = E_SVIDEO_EQUIRECT;

        m_vsPlugin = NULL;
        m_vsPlugin = dlopen("/usr/local/lib/libHevcVideoStreamProcess.so", RTLD_LAZY);
        if (!m_vsPlugin)
        {
            fclose(m_highResFile);
            m_highResFile = NULL;
            fclose(m_lowResFile);
            m_lowResFile = NULL;
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            DELETE_ARRAY(m_initInfo->bsBuffers);
            DELETE_MEMORY(m_initInfo->segmentationInfo);
            DELETE_MEMORY(m_initInfo);
            return;
        }

        CreateVideoStream* createVS = NULL;
        createVS = (CreateVideoStream*)dlsym(m_vsPlugin, "Create");
        const char* dlsymErr1 = dlerror();
        if (dlsymErr1)
        {
            fclose(m_highResFile);
            m_highResFile = NULL;
            fclose(m_lowResFile);
            m_lowResFile = NULL;
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            DELETE_ARRAY(m_initInfo->bsBuffers);
            DELETE_MEMORY(m_initInfo->segmentationInfo);
            DELETE_MEMORY(m_initInfo);
            dlclose(m_vsPlugin);
            m_vsPlugin = NULL;
            return;
        }

        if (!createVS)
        {
            fclose(m_highResFile);
            m_highResFile = NULL;
            fclose(m_lowResFile);
            m_lowResFile = NULL;
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            DELETE_ARRAY(m_initInfo->bsBuffers);
            DELETE_MEMORY(m_initInfo->segmentationInfo);
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
            fclose(m_highResFile);
            m_highResFile = NULL;
            fclose(m_lowResFile);
            m_lowResFile = NULL;
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            DELETE_ARRAY(m_initInfo->bsBuffers);
            DELETE_MEMORY(m_initInfo->segmentationInfo);
            DELETE_MEMORY(m_initInfo);
            dlclose(m_vsPlugin);
            m_vsPlugin = NULL;
            return;
        }

        m_vsLow = createVS();
        if (!m_vsLow)
        {
            fclose(m_highResFile);
            m_highResFile = NULL;
            fclose(m_lowResFile);
            m_lowResFile = NULL;
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            DELETE_ARRAY(m_initInfo->bsBuffers);
            DELETE_MEMORY(m_initInfo->segmentationInfo);
            DELETE_MEMORY(m_initInfo);
            dlclose(m_vsPlugin);
            m_vsPlugin = NULL;
            return;
        }

        ((MediaStream*)m_vsLow)->SetMediaType(VIDEOTYPE);
        ((MediaStream*)m_vsLow)->SetCodecId(CODEC_ID_H265);

        int32_t ret = m_vsLow->Initialize(m_lowResStreamIdx, &(m_initInfo->bsBuffers[0]), m_initInfo);
        if (ret)
        {
            fclose(m_highResFile);
            m_highResFile = NULL;
            fclose(m_lowResFile);
            m_lowResFile = NULL;
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            DELETE_ARRAY(m_initInfo->bsBuffers);
            DELETE_MEMORY(m_initInfo->segmentationInfo);
            DELETE_MEMORY(m_initInfo);
            destroyVS((VideoStream*)(m_vsLow));
            dlclose(m_vsPlugin);
            m_vsPlugin = NULL;
            return;
        }

        //Create and Initialize high resolution video stream
        m_vsHigh = createVS();
        if (!m_vsHigh)
        {
            fclose(m_highResFile);
            m_highResFile = NULL;
            fclose(m_lowResFile);
            m_lowResFile = NULL;
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            DELETE_ARRAY(m_initInfo->bsBuffers);
            DELETE_MEMORY(m_initInfo->segmentationInfo);
            DELETE_MEMORY(m_initInfo);
            destroyVS((VideoStream*)(m_vsLow));
            dlclose(m_vsPlugin);
            m_vsPlugin = NULL;
            return;
        }

        ((MediaStream*)m_vsHigh)->SetMediaType(VIDEOTYPE);
        ((MediaStream*)m_vsHigh)->SetCodecId(CODEC_ID_H265);

        ret = m_vsHigh->Initialize(m_highResStreamIdx, &(m_initInfo->bsBuffers[1]), m_initInfo);
        if (ret)
        {
            fclose(m_highResFile);
            m_highResFile = NULL;
            fclose(m_lowResFile);
            m_lowResFile = NULL;
            DELETE_ARRAY(m_highResHeader);
            DELETE_ARRAY(m_lowResHeader);
            DELETE_ARRAY(m_totalDataLow);
            DELETE_ARRAY(m_totalDataHigh);
            DELETE_ARRAY(m_initInfo->bsBuffers);
            DELETE_MEMORY(m_initInfo->segmentationInfo);
            DELETE_MEMORY(m_initInfo);
            destroyVS((VideoStream*)(m_vsLow));
            destroyVS((VideoStream*)(m_vsHigh));
            dlclose(m_vsPlugin);
            m_vsPlugin = NULL;
            return;
        }
    }
    virtual void TearDown()
    {
        if (m_highResFile)
        {
            fclose(m_highResFile);
            m_highResFile = NULL;
        }

        if (m_lowResFile)
        {
            fclose(m_lowResFile);
            m_lowResFile = NULL;
        }

        DELETE_ARRAY(m_highResHeader);
        DELETE_ARRAY(m_lowResHeader);
        DELETE_ARRAY(m_totalDataLow);
        DELETE_ARRAY(m_totalDataHigh);

        if (m_initInfo)
        {
            DELETE_ARRAY(m_initInfo->bsBuffers);
            DELETE_MEMORY(m_initInfo->segmentationInfo);

            delete m_initInfo;
            m_initInfo = NULL;
        }

        remove("test_vps_lowRes.bin");
        remove("test_sps_lowRes.bin");
        remove("test_pps_lowRes.bin");

        remove("test_vps_highRes.bin");
        remove("test_sps_highRes.bin");
        remove("test_pps_highRes.bin");

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
            if (m_vsLow)
            {
                destroyVS((VideoStream*)(m_vsLow));
            }
            if (m_vsHigh)
            {
                destroyVS((VideoStream*)(m_vsHigh));
            }

            dlclose(m_vsPlugin);
            m_vsPlugin = NULL;
        }
    }

    FILE *m_highResFile;
    FILE *m_lowResFile;
    uint8_t m_lowResStreamIdx;
    uint8_t m_highResStreamIdx;
    uint8_t *m_highResHeader;
    uint8_t *m_lowResHeader;
    uint8_t *m_totalDataLow;
    uint8_t *m_totalDataHigh;
    InitialInfo *m_initInfo;
    void        *m_vsPlugin;
    VideoStream *m_vsLow;
    VideoStream *m_vsHigh;
};

TEST_F(VideoStreamTest, AllProcess)
{
    uint16_t lowResWidth = 1920;
    uint16_t lowResHeight = 960;
    uint8_t  lowTileInRow = 2;
    uint8_t  lowTileInCol = 1;

    uint16_t projType = 0;

    RegionWisePacking rwpk;
    rwpk.constituentPicMatching = 0;
    rwpk.numRegions = lowTileInRow * lowTileInCol;
    rwpk.projPicWidth = lowResWidth;
    rwpk.projPicHeight = lowResHeight;
    rwpk.packedPicWidth = lowResWidth;
    rwpk.packedPicHeight = lowResHeight;
    rwpk.rectRegionPacking = new RectangularRegionWisePacking[rwpk.numRegions];
    EXPECT_TRUE(rwpk.rectRegionPacking != NULL);
    if (!(rwpk.rectRegionPacking))
        return;

    uint16_t tileWidth = lowResWidth / lowTileInRow;
    uint16_t tileHeight = lowResHeight / lowTileInCol;
    for (uint16_t idx = 0; idx < rwpk.numRegions; idx++)
    {
        memset_s(&(rwpk.rectRegionPacking[idx]), sizeof(RectangularRegionWisePacking), 0);
        rwpk.rectRegionPacking[idx].transformType = 0;
        rwpk.rectRegionPacking[idx].guardBandFlag = 0;
        rwpk.rectRegionPacking[idx].projRegWidth  = tileWidth;
        rwpk.rectRegionPacking[idx].projRegHeight = tileHeight;
        rwpk.rectRegionPacking[idx].projRegTop    = (idx / lowTileInRow) * tileHeight;
        rwpk.rectRegionPacking[idx].projRegLeft   = (idx % lowTileInRow) * tileWidth;


        rwpk.rectRegionPacking[idx].packedRegWidth  = tileWidth;
        rwpk.rectRegionPacking[idx].packedRegHeight = tileHeight;
        rwpk.rectRegionPacking[idx].packedRegTop    = (idx / lowTileInRow) * tileHeight;
        rwpk.rectRegionPacking[idx].packedRegLeft   = (idx % lowTileInRow) * tileWidth;

        rwpk.rectRegionPacking[idx].gbNotUsedForPredFlag = true;
    }

    ContentCoverage covi;
    covi.coverageShapeType = 1;
    covi.numRegions        = lowTileInRow * lowTileInCol;
    covi.viewIdcPresenceFlag = false;
    covi.defaultViewIdc      = 0;
    covi.sphereRegions       = new SphereRegion[covi.numRegions];
    EXPECT_TRUE(covi.sphereRegions != NULL);
    if (!(covi.sphereRegions))
    {
        DELETE_ARRAY(rwpk.rectRegionPacking);
        return;
    }
    for (uint16_t idx = 0; idx < covi.numRegions; idx++)
    {
        memset_s(&(covi.sphereRegions[idx]), sizeof(SphereRegion), 0);
        covi.sphereRegions[idx].viewIdc = 0;
        covi.sphereRegions[idx].centreAzimuth   = (int32_t)((((lowResWidth / 2) - (float)((idx % lowTileInRow) * tileWidth + tileWidth / 2)) * 360 * 65536) / lowResWidth);
        covi.sphereRegions[idx].centreElevation = (int32_t)((((lowResHeight / 2) - (float)((idx / lowTileInRow) * tileHeight + tileHeight / 2)) * 180 * 65536) / lowResHeight);
        covi.sphereRegions[idx].centreTilt      = 0;
        covi.sphereRegions[idx].azimuthRange    = (uint32_t)((tileWidth * 360.f * 65536) / lowResWidth);
        covi.sphereRegions[idx].elevationRange  = (uint32_t)((tileHeight * 180.f * 65536) / lowResHeight);
        covi.sphereRegions[idx].interpolate     = 0;
    }

    EXPECT_TRUE(m_vsLow->GetSrcWidth()  == lowResWidth);
    EXPECT_TRUE(m_vsLow->GetSrcHeight() == lowResHeight);
    EXPECT_TRUE(m_vsLow->GetTileInRow() == lowTileInRow);
    EXPECT_TRUE(m_vsLow->GetTileInCol() == lowTileInCol);
    EXPECT_TRUE(m_vsLow->GetProjType()  == projType);

    RegionWisePacking *origRwpk = m_vsLow->GetSrcRwpk();
    //EXPECT_TRUE(*origRwpk == rwpk);
    EXPECT_TRUE(origRwpk->constituentPicMatching == rwpk.constituentPicMatching);
    EXPECT_TRUE(origRwpk->numRegions == rwpk.numRegions);
    EXPECT_TRUE(origRwpk->projPicWidth == rwpk.projPicWidth);
    EXPECT_TRUE(origRwpk->projPicHeight == rwpk.projPicHeight);
    EXPECT_TRUE(origRwpk->packedPicWidth == rwpk.packedPicWidth);
    EXPECT_TRUE(origRwpk->packedPicHeight == rwpk.packedPicHeight);
    int32_t compRet = 0;
    for (uint16_t idx = 0; idx < rwpk.numRegions; idx++)
    {
        int result = memcmp_s(&(origRwpk->rectRegionPacking[idx]), sizeof(RectangularRegionWisePacking), &(rwpk.rectRegionPacking[idx]), sizeof(RectangularRegionWisePacking), &compRet);
        EXPECT_TRUE(result == 0);
        EXPECT_TRUE(compRet == 0);
    }

    ContentCoverage *origCovi = m_vsLow->GetSrcCovi();
    //EXPECT_TRUE(*origCovi == covi);
    EXPECT_TRUE(origCovi->coverageShapeType == covi.coverageShapeType);
    EXPECT_TRUE(origCovi->numRegions == covi.numRegions);
    EXPECT_TRUE(origCovi->viewIdcPresenceFlag == covi.viewIdcPresenceFlag);
    EXPECT_TRUE(origCovi->defaultViewIdc == covi.defaultViewIdc);
    for (uint16_t idx = 0; idx < origCovi->numRegions; idx++)
    {
        int result = memcmp_s(&(origCovi->sphereRegions[idx]), sizeof(SphereRegion), &(covi.sphereRegions[idx]), sizeof(SphereRegion), &compRet);
        EXPECT_TRUE(result == 0);
        EXPECT_TRUE(compRet == 0);
    }

    Rational frameRate = { 25, 1 };
    Rational gottenFrameRate = m_vsLow->GetFrameRate();
    EXPECT_TRUE(gottenFrameRate.num == frameRate.num);
    EXPECT_TRUE(gottenFrameRate.den == frameRate.den);
    EXPECT_TRUE(m_vsLow->GetBitRate()   == 3225140);

    Nalu *vpsNalu = m_vsLow->GetVPSNalu();
    EXPECT_TRUE(vpsNalu != NULL);
    FILE *fp = fopen("test_vps_lowRes.bin", "r");
    if (!fp)
    {
        DELETE_ARRAY(rwpk.rectRegionPacking);
        DELETE_ARRAY(covi.sphereRegions);
        return;
    }
    fseek(fp, 0L, SEEK_END);
    uint32_t vpsLen = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    uint8_t *vpsData = new uint8_t[vpsLen];
    if (!vpsData)
    {
        DELETE_ARRAY(rwpk.rectRegionPacking);
        DELETE_ARRAY(covi.sphereRegions);
        fclose(fp);
        fp = NULL;
        return;
    }
    fread(vpsData, 1, vpsLen, fp);

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

    Nalu *spsNalu = m_vsLow->GetSPSNalu();
    EXPECT_TRUE(spsNalu != NULL);
    fp = fopen("test_sps_lowRes.bin", "r");
    if (!fp)
    {
        DELETE_ARRAY(rwpk.rectRegionPacking);
        DELETE_ARRAY(covi.sphereRegions);
        return;
    }
    fseek(fp, 0L, SEEK_END);
    uint32_t spsLen = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    uint8_t *spsData = new uint8_t[spsLen];
    if (!spsData)
    {
        DELETE_ARRAY(rwpk.rectRegionPacking);
        DELETE_ARRAY(covi.sphereRegions);
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

    Nalu *ppsNalu = m_vsLow->GetPPSNalu();
    EXPECT_TRUE(ppsNalu != NULL);
    fp = fopen("test_pps_lowRes.bin", "r");
    if (!fp)
    {
        DELETE_ARRAY(rwpk.rectRegionPacking);
        DELETE_ARRAY(covi.sphereRegions);
        return;
    }
    fseek(fp, 0L, SEEK_END);
    uint32_t ppsLen = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    uint8_t *ppsData = new uint8_t[ppsLen];
    if (!ppsData)
    {
        DELETE_ARRAY(rwpk.rectRegionPacking);
        DELETE_ARRAY(covi.sphereRegions);
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

    uint16_t highResWidth = 3840;
    uint16_t highResHeight = 1920;
    uint8_t  highTileInRow = 2;
    uint8_t  highTileInCol = 1;

    rwpk.constituentPicMatching = 0;
    rwpk.numRegions = highTileInRow * highTileInCol;
    rwpk.projPicWidth = highResWidth;
    rwpk.projPicHeight = highResHeight;
    rwpk.packedPicWidth = highResWidth;
    rwpk.packedPicHeight = highResHeight;
    DELETE_ARRAY(rwpk.rectRegionPacking);
    rwpk.rectRegionPacking = new RectangularRegionWisePacking[rwpk.numRegions];
    EXPECT_TRUE(rwpk.rectRegionPacking != NULL);
    if (!(rwpk.rectRegionPacking))
    {
        return;
    }
    tileWidth = highResWidth / highTileInRow;
    tileHeight = highResHeight / highTileInCol;
    for (uint16_t idx = 0; idx < rwpk.numRegions; idx++)
    {
        memset_s(&(rwpk.rectRegionPacking[idx]), sizeof(RectangularRegionWisePacking), 0);
        rwpk.rectRegionPacking[idx].transformType = 0;
        rwpk.rectRegionPacking[idx].guardBandFlag = 0;
        rwpk.rectRegionPacking[idx].projRegWidth  = tileWidth;
        rwpk.rectRegionPacking[idx].projRegHeight = tileHeight;
        rwpk.rectRegionPacking[idx].projRegTop    = (idx / highTileInRow) * tileHeight;
        rwpk.rectRegionPacking[idx].projRegLeft   = (idx % highTileInRow) * tileWidth;


        rwpk.rectRegionPacking[idx].packedRegWidth  = tileWidth;
        rwpk.rectRegionPacking[idx].packedRegHeight = tileHeight;
        rwpk.rectRegionPacking[idx].packedRegTop    = (idx / highTileInRow) * tileHeight;
        rwpk.rectRegionPacking[idx].packedRegLeft   = (idx % highTileInRow) * tileWidth;

        rwpk.rectRegionPacking[idx].gbNotUsedForPredFlag = true;
    }

    //ContentCoverage covi;
    covi.coverageShapeType = 1;
    covi.numRegions        = highTileInRow * highTileInCol;
    covi.viewIdcPresenceFlag = false;
    covi.defaultViewIdc      = 0;
    DELETE_ARRAY(covi.sphereRegions);
    covi.sphereRegions       = new SphereRegion[covi.numRegions];
    EXPECT_TRUE(covi.sphereRegions != NULL);
    if (!(covi.sphereRegions))
    {
        DELETE_ARRAY(rwpk.rectRegionPacking);
        return;
    }
    for (uint16_t idx = 0; idx < covi.numRegions; idx++)
    {
        memset_s(&(covi.sphereRegions[idx]), sizeof(SphereRegion), 0);
        covi.sphereRegions[idx].viewIdc = 0;
        covi.sphereRegions[idx].centreAzimuth   = (int32_t)((((highResWidth / 2) - (float)((idx % highTileInRow) * tileWidth + tileWidth / 2)) * 360 * 65536) / highResWidth);
        covi.sphereRegions[idx].centreElevation = (int32_t)((((highResHeight / 2) - (float)((idx / highTileInRow) * tileHeight + tileHeight / 2)) * 180 * 65536) / highResHeight);
        covi.sphereRegions[idx].centreTilt      = 0;
        covi.sphereRegions[idx].azimuthRange    = (uint32_t)((tileWidth * 360.f * 65536) / highResWidth);
        covi.sphereRegions[idx].elevationRange  = (uint32_t)((tileHeight * 180.f * 65536) / highResHeight);
        covi.sphereRegions[idx].interpolate     = 0;
    }


    EXPECT_TRUE(m_vsHigh->GetSrcWidth()  == highResWidth);
    EXPECT_TRUE(m_vsHigh->GetSrcHeight() == highResHeight);
    EXPECT_TRUE(m_vsHigh->GetTileInRow() == highTileInRow);
    EXPECT_TRUE(m_vsHigh->GetTileInCol() == highTileInCol);
    EXPECT_TRUE(m_vsHigh->GetProjType()  == projType);
    origRwpk = m_vsHigh->GetSrcRwpk();

    EXPECT_TRUE(origRwpk->constituentPicMatching == rwpk.constituentPicMatching);
    EXPECT_TRUE(origRwpk->numRegions == rwpk.numRegions);
    EXPECT_TRUE(origRwpk->projPicWidth == rwpk.projPicWidth);
    EXPECT_TRUE(origRwpk->projPicHeight == rwpk.projPicHeight);
    EXPECT_TRUE(origRwpk->packedPicWidth == rwpk.packedPicWidth);
    EXPECT_TRUE(origRwpk->packedPicHeight == rwpk.packedPicHeight);
    compRet = 0;
    for (uint16_t idx = 0; idx < rwpk.numRegions; idx++)
    {
        result = memcmp_s(&(origRwpk->rectRegionPacking[idx]), sizeof(RectangularRegionWisePacking), &(rwpk.rectRegionPacking[idx]), sizeof(RectangularRegionWisePacking), &compRet);
        EXPECT_TRUE(result == 0);
        EXPECT_TRUE(compRet == 0);
    }

    origCovi = m_vsHigh->GetSrcCovi();

    EXPECT_TRUE(origCovi->coverageShapeType == covi.coverageShapeType);
    EXPECT_TRUE(origCovi->numRegions == covi.numRegions);
    EXPECT_TRUE(origCovi->viewIdcPresenceFlag == covi.viewIdcPresenceFlag);
    EXPECT_TRUE(origCovi->defaultViewIdc == covi.defaultViewIdc);
    for (uint16_t idx = 0; idx < origCovi->numRegions; idx++)
    {
        result = memcmp_s(&(origCovi->sphereRegions[idx]), sizeof(SphereRegion), &(covi.sphereRegions[idx]), sizeof(SphereRegion), &compRet);
        EXPECT_TRUE(result == 0);
        EXPECT_TRUE(compRet == 0);
    }

    Rational gottenFrameRate2 = m_vsHigh->GetFrameRate();
    EXPECT_TRUE(gottenFrameRate2.num == frameRate.num);
    EXPECT_TRUE(gottenFrameRate2.den == frameRate.den);
    EXPECT_TRUE(m_vsHigh->GetBitRate()   == 3273220);

    DELETE_ARRAY(rwpk.rectRegionPacking);
    DELETE_ARRAY(covi.sphereRegions);

    vpsNalu = m_vsHigh->GetVPSNalu();
    EXPECT_TRUE(vpsNalu != NULL);
    fp = fopen("test_vps_highRes.bin", "r");
    if (!fp)
    {
        return;
    }
    fseek(fp, 0L, SEEK_END);
    vpsLen = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    vpsData = new uint8_t[vpsLen];
    if (!vpsData)
    {
        fclose(fp);
        fp = NULL;
        return;
    }
    fread(vpsData, 1, vpsLen, fp);

    result = memcmp_s(vpsNalu->data, vpsLen, vpsData, vpsLen, &compRet);
    EXPECT_TRUE(result == 0);
    EXPECT_TRUE(vpsNalu->dataSize == vpsLen);
    EXPECT_TRUE(compRet == 0);
    EXPECT_TRUE(vpsNalu->startCodesSize == 4);
    EXPECT_TRUE(vpsNalu->naluType == 32);

    delete[] vpsData;
    vpsData = NULL;
    fclose(fp);
    fp = NULL;

    spsNalu = m_vsHigh->GetSPSNalu();
    EXPECT_TRUE(spsNalu != NULL);
    fp = fopen("test_sps_highRes.bin", "r");
    if (!fp)
        return;

    fseek(fp, 0L, SEEK_END);
    spsLen = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    spsData = new uint8_t[spsLen];
    if (!spsData)
    {
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

    ppsNalu = m_vsHigh->GetPPSNalu();
    EXPECT_TRUE(ppsNalu != NULL);
    fp = fopen("test_pps_highRes.bin", "r");
    if (!fp)
        return;

    fseek(fp, 0L, SEEK_END);
    ppsLen = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    ppsData = new uint8_t[ppsLen];
    if (!ppsData)
    {
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

    uint64_t frameSize[5] = { 79306, 39, 85, 39, 593 };

    int32_t ret = ERROR_NONE;
    uint8_t tileInRow = 2;
    uint8_t tileInCol = 1;
    uint16_t tilesNum = tileInRow * tileInCol;
    fp = fopen("lowResFrameSliceSize.bin", "r");
    EXPECT_TRUE(fp != NULL);
    if (!fp)
        return;

    uint32_t sliceSize = 0;
    uint32_t sliceHrdLen = 0;
    uint32_t naluType = 0;
    uint64_t offset = 0;
    for (uint8_t idx = 0; idx < 5; idx++)
    {
        uint8_t *frameData = new uint8_t[frameSize[idx]];
        EXPECT_TRUE(frameData != NULL);
        if (!frameData)
        {
            fclose(fp);
            fp = NULL;
            return;
        }
        memset_s(frameData, frameSize[idx], 0);
        memcpy_s(frameData, frameSize[idx], m_totalDataLow+offset, frameSize[idx]);
        offset += frameSize[idx];

        FrameBSInfo *frameInfo = new FrameBSInfo;
        EXPECT_TRUE(frameInfo != NULL);
        if (!frameInfo)
        {
            DELETE_ARRAY(frameData);
            fclose(fp);
            fp = NULL;
            return;
        }
        frameInfo->data = frameData;
        frameInfo->dataSize = frameSize[idx];
        frameInfo->pts = idx;
        if (idx == 0)
        {
            frameInfo->isKeyFrame = true;
        }
        else
        {
            frameInfo->isKeyFrame = false;
        }

        ret = m_vsLow->AddFrameInfo(frameInfo);
        EXPECT_TRUE(ret == ERROR_NONE);

        m_vsLow->SetCurrFrameInfo();
        EXPECT_TRUE(m_vsLow->GetCurrFrameInfo() != NULL);

        ret = m_vsLow->UpdateTilesNalu();
        EXPECT_TRUE(ret == ERROR_NONE);

        TileInfo *tilesInfo = m_vsLow->GetAllTilesInfo();
        for (uint16_t i = 0; i < tilesNum; i++)
        {
            fscanf(fp, "%u,%u,%u", &sliceSize, &sliceHrdLen, &naluType);

            EXPECT_TRUE(tilesInfo[i].tileNalu->dataSize == sliceSize);
            EXPECT_TRUE(tilesInfo[i].tileNalu->startCodesSize == 4);
            EXPECT_TRUE(tilesInfo[i].tileNalu->naluType == naluType);
            EXPECT_TRUE(tilesInfo[i].tileNalu->sliceHeaderLen == sliceHrdLen);
        }

        m_vsLow->DestroyCurrFrameInfo();

        DELETE_ARRAY(frameData);
        DELETE_MEMORY(frameInfo);
    }

    fclose(fp);
    fp = NULL;

    uint64_t frameSize1[5] = { 96996, 63, 390, 67, 1225 };

    fp = fopen("highResFrameSliceSize.bin", "r");
    EXPECT_TRUE(fp != NULL);
    if (!fp)
        return;

    sliceSize = 0;
    sliceHrdLen = 0;
    naluType = 0;
    offset = 0;
    for (uint8_t idx = 0; idx < 5; idx++)
    {
        uint8_t *frameData = new uint8_t[frameSize1[idx]];
        EXPECT_TRUE(frameData != NULL);
        if (!frameData)
        {
            fclose(fp);
            fp = NULL;
            return;
        }

        memset_s(frameData, frameSize1[idx], 0);
        memcpy_s(frameData, frameSize1[idx], m_totalDataHigh+offset, frameSize1[idx]);
        offset += frameSize1[idx];

        FrameBSInfo *frameInfo = new FrameBSInfo;
        EXPECT_TRUE(frameInfo != NULL);
        if (!frameInfo)
        {
            DELETE_ARRAY(frameData);
            fclose(fp);
            fp = NULL;
            return;
        }

        frameInfo->data = frameData;
        frameInfo->dataSize = frameSize1[idx];
        frameInfo->pts = idx;
        if (idx == 0)
        {
            frameInfo->isKeyFrame = true;
        }
        else
        {
            frameInfo->isKeyFrame = false;
        }

        ret = m_vsHigh->AddFrameInfo(frameInfo);
        EXPECT_TRUE(ret == ERROR_NONE);

        m_vsHigh->SetCurrFrameInfo();
        EXPECT_TRUE(m_vsHigh->GetCurrFrameInfo() != NULL);

        ret = m_vsHigh->UpdateTilesNalu();
        EXPECT_TRUE(ret == ERROR_NONE);

        TileInfo *tilesInfo = m_vsHigh->GetAllTilesInfo();
        for (uint16_t i = 0; i < tilesNum; i++)
        {
            fscanf(fp, "%u,%u,%u", &sliceSize, &sliceHrdLen, &naluType);

            EXPECT_TRUE(tilesInfo[i].tileNalu->dataSize == sliceSize);
            EXPECT_TRUE(tilesInfo[i].tileNalu->startCodesSize == 4);
            EXPECT_TRUE(tilesInfo[i].tileNalu->naluType == naluType);
            EXPECT_TRUE(tilesInfo[i].tileNalu->sliceHeaderLen == sliceHrdLen);
        }

        m_vsHigh->DestroyCurrFrameInfo();

        DELETE_ARRAY(frameData);
        DELETE_MEMORY(frameInfo);
    }

    fclose(fp);
    fp = NULL;
}
