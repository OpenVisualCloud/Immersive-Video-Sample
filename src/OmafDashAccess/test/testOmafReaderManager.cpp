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
//! \file:   testOmafReaderManager.cpp
//! \brief:  Omaf reader manager class unit test
//!
//! Created on July 17, 2019, 6:04 AM
//!

#include "gtest/gtest.h"
#include "../OmafReader.h"
#include "../OmafReaderManager.h"

VCD_USE_VROMAF;
VCD_USE_VRVIDEO;

namespace {
class OmafReaderManagerTest : public testing::Test
{
public:
    virtual void SetUp()
    {
        m_clientInfo = new HeadSetInfo;
        m_clientInfo->input_geoType = 0;
        m_clientInfo->output_geoType = E_SVIDEO_VIEWPORT;
        m_clientInfo->pose = new HeadPose;
        m_clientInfo->pose->yaw = -90;
        m_clientInfo->pose->pitch = 0;
        m_clientInfo->viewPort_hFOV = 80;
        m_clientInfo->viewPort_vFOV = 90;
        m_clientInfo->viewPort_Width = 1024;
        m_clientInfo->viewPort_Height = 1024;

        m_pose = new HeadPose;
        m_pose->yaw = 90;
        m_pose->pitch = 0;

        m_source = new OmafDashSource();
        if (!m_source)
            return;

        int ret = m_source->SetupHeadSetInfo(m_clientInfo);
        if (ret)
            return;

        std::string mpdUrl = "./segs_for_readertest/Test.mpd";

        ret = m_source->OpenMedia(mpdUrl, "./cache", false);
        if (ret)
        {
            printf("Failed to open media \n");
            return;
        }
        READERMANAGER::GetInstance()->StartThread();
    }
    virtual void TearDown()
    {
        delete (m_clientInfo->pose);
        m_clientInfo->pose = NULL;

        delete m_clientInfo;
        m_clientInfo = NULL;

        delete m_pose;
        m_pose = NULL;

        m_source->CloseMedia();
        SAFE_DELETE(m_source);
    }

    HeadSetInfo                   *m_clientInfo;
    OmafMediaSource               *m_source;
    HeadPose                      *m_pose;
};

TEST_F(OmafReaderManagerTest, ReaderTrackSegments)
{
    int ret = ERROR_NONE;
    char storedFileName[1024];
    std::string cacheFileName;

    EXPECT_TRUE(m_source->GetStreamCount() == 1);
    OmafMediaStream *stream = m_source->GetStream(0);
    EXPECT_TRUE(stream != NULL);

    char genFileName[1024];
    memset(genFileName, 0, 1024);
    uint32_t extractorTrackID = 1000;
    for ( ; extractorTrackID < 1001; extractorTrackID++)
    {
        memset(genFileName, 0, 1024);
        snprintf(genFileName, 1024, "Viewport_Extractor%d.h265", (extractorTrackID - 999));
        FILE *fpGen = fopen(genFileName, "wb+");
        if(!fpGen) continue;

        ret = m_source->SelectSpecialSegments(extractorTrackID);
        EXPECT_TRUE(ret == ERROR_NONE);

        std::map<int, OmafAdaptationSet*> normalAS    = stream->GetMediaAdaptationSet();
        std::map<int, OmafExtractor*>     extractorAS = stream->GetExtractors();

        for (auto itAS = normalAS.begin(); itAS != normalAS.end(); itAS++)
        {
            OmafAdaptationSet *pAS = (OmafAdaptationSet*)(itAS->second);
            EXPECT_TRUE(pAS != NULL);

            memset(storedFileName, 0, 1024);
            std::string repId = pAS->GetRepresentationId();
            snprintf(storedFileName, 1024, "./segs_for_readertest/%s.init.mp4", repId.c_str());
            cacheFileName = storedFileName;

            FILE *fp = fopen(storedFileName, "rb");
            if (!fp)
            {
                fclose(fpGen);
                fpGen = NULL;
                return;
            }
            fseek(fp, 0L, SEEK_END);
            uint64_t segSize = ftell(fp);
            fseek(fp, 0L, SEEK_SET);
            fclose(fp);
            fp = NULL;

            ret = pAS->LoadAssignedInitSegment(cacheFileName);
            EXPECT_TRUE(ret == ERROR_NONE);

            OmafSegment *initSeg = pAS->GetInitSegment();
            EXPECT_TRUE(initSeg != NULL);

            initSeg->SetSegSize(segSize);

            uint32_t initSegID = 0;
            ret = READERMANAGER::GetInstance()->AddInitSegment(initSeg, initSegID);
            EXPECT_TRUE(ret == ERROR_NONE);
        }

        for (auto itAS = extractorAS.begin(); itAS != extractorAS.end(); itAS++)
        {
            OmafExtractor *extractor = (OmafExtractor*)(itAS->second);
            EXPECT_TRUE(extractor != NULL);

            memset(storedFileName, 0, 1024);
            std::string repId = extractor->GetRepresentationId();
            snprintf(storedFileName, 1024, "./segs_for_readertest/%s.init.mp4", repId.c_str());
            cacheFileName = storedFileName;

            FILE *fp = fopen(storedFileName, "rb");
            if (!fp)
            {
                fclose(fpGen);
                fpGen = NULL;
                return;
            }
            fseek(fp, 0L, SEEK_END);
            uint64_t segSize = ftell(fp);
            fseek(fp, 0L, SEEK_SET);
            fclose(fp);
            fp = NULL;

            ret = extractor->LoadAssignedInitSegment(cacheFileName);
            EXPECT_TRUE(ret == ERROR_NONE);

            OmafSegment *initSeg = extractor->GetInitSegment();
            EXPECT_TRUE(initSeg != NULL);

            initSeg->SetSegSize(segSize);

            uint32_t initSegID = 0;
            ret = READERMANAGER::GetInstance()->AddInitSegment(initSeg, initSegID);
            EXPECT_TRUE(ret == ERROR_NONE);
        }

        usleep(1000000);
        for (uint8_t segID = 1; segID < 5; segID++)
        {
            for (auto itAS = normalAS.begin(); itAS != normalAS.end(); itAS++)
            {
                OmafAdaptationSet *pAS = (OmafAdaptationSet*)(itAS->second);
                EXPECT_TRUE(pAS != NULL);

                if (extractorTrackID == 1000 || extractorTrackID == 1004)
                {
                    if ((itAS->first == 4) || (itAS->first == 8))
                    {
                        continue;
                    }
                }

                pAS->Enable(true);

                memset(storedFileName, 0, 1024);
                std::string repId = pAS->GetRepresentationId();
                snprintf(storedFileName, 1024, "./segs_for_readertest/%s.%d.mp4", repId.c_str(), segID);
                cacheFileName = storedFileName;

                FILE *fp = fopen(storedFileName, "rb");
                if (!fp)
                {
                    fclose(fpGen);
                    fpGen = NULL;
                    return;
                }
                fseek(fp, 0L, SEEK_END);
                uint64_t segSize = ftell(fp);
                fseek(fp, 0L, SEEK_SET);
                fclose(fp);
                fp = NULL;

                OmafSegment *newSeg = pAS->LoadAssignedSegment(cacheFileName);
                EXPECT_TRUE(newSeg != NULL);
                if(!newSeg) break;

                newSeg->SetSegSize(segSize);

                uint32_t newSegID = 0;
                ret = READERMANAGER::GetInstance()->AddSegment(newSeg, newSeg->GetInitSegID(), newSegID);
                EXPECT_TRUE(ret == ERROR_NONE);
            }

            for (auto itAS = extractorAS.begin(); itAS != extractorAS.end(); itAS++)
            {
                OmafExtractor *extractor = (OmafExtractor*)(itAS->second);
                EXPECT_TRUE(extractor != NULL);

                if ((itAS->first) != extractorTrackID)
                {
                    continue;
                }

                extractor->Enable(true);

                memset(storedFileName, 0, 1024);
                std::string repId = extractor->GetRepresentationId();
                snprintf(storedFileName, 1024, "./segs_for_readertest/%s.%d.mp4", repId.c_str(), segID);
                cacheFileName = storedFileName;

                FILE *fp = fopen(storedFileName, "rb");
                if (!fp)
                {
                    fclose(fpGen);
                    fpGen = NULL;
                    return;
                }
                fseek(fp, 0L, SEEK_END);
                uint64_t segSize = ftell(fp);
                fseek(fp, 0L, SEEK_SET);
                fclose(fp);
                fp = NULL;

                OmafSegment *newSeg = extractor->LoadAssignedSegment(cacheFileName);
                EXPECT_TRUE(newSeg != NULL);
                if(!newSeg) break;

                newSeg->SetSegSize(segSize);

                uint32_t newSegID = 0;
                ret = READERMANAGER::GetInstance()->AddSegment(newSeg, newSeg->GetInitSegID(), newSegID);
                EXPECT_TRUE(ret == ERROR_NONE);
            }
        }

        usleep(10000000);
        std::list<MediaPacket*> pkts;
        bool clearBuf = false;
        m_source->GetPacket(0, &pkts, true, clearBuf);

        for (uint8_t frameIdx = 1; frameIdx < 100; frameIdx++)
        {
            m_source->GetPacket(0, &pkts, false, clearBuf);
        }

        EXPECT_TRUE(pkts.size() == 100);

        for (auto itPacket = pkts.begin(); itPacket != pkts.end(); itPacket++)
        {
            uint32_t size = (*itPacket)->Size();
            char *data    = (*itPacket)->Payload();
            if(data) fwrite(data, 1, size, fpGen);

            delete (*itPacket);
        }

        pkts.clear();

        fclose(fpGen);
        fpGen = NULL;
    }
}
}
