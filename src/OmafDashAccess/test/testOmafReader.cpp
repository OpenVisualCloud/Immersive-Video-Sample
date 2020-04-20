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
//! \file:   testOmafReader.cpp
//! \brief:  Omaf reader class unit test
//!
//! Created on July 17, 2019, 6:04 AM
//!

#include "gtest/gtest.h"
#include "../OmafMPDParser.h"
#include "../OmafReader.h"
#include "../OmafMP4VRReader.h"

VCD_USE_VROMAF;
VCD_USE_VRVIDEO;

namespace {
class OmafReaderTest : public testing::Test
{
public:
    virtual void SetUp()
    {
        std::string mpdUrl = "./segs_for_readertest/Test.mpd";

        m_mpdParser = new OmafMPDParser();
        if (!m_mpdParser)
            return;

        m_mpdParser->ParseMPD(mpdUrl, m_listStream);

        m_mpdParser->GetMPDInfo();

        m_reader = new OmafMP4VRReader();
        if (!m_reader)
            return;

    }
    virtual void TearDown()
    {
        SAFE_DELETE(m_mpdParser);
        SAFE_DELETE(m_reader);
    }

    OmafMPDParser                 *m_mpdParser;
    OMAFSTREAMS                   m_listStream;
    OmafReader                    *m_reader;
};

TEST_F(OmafReaderTest, ParseInitialSegment)
{
    int ret = ERROR_NONE;
    uint32_t initSegID = 0;
    std::vector<TrackInformation*> trackInfos;
    std::map<int32_t, int32_t> mapSegCnt;
    std::string cacheFileName;
    char storedFileName[1024];

    for (auto it = m_listStream.begin(); it != m_listStream.end(); it++)
    {
        OmafMediaStream *stream = (OmafMediaStream*)(*it);
        EXPECT_TRUE(stream != NULL);

        std::map<int, OmafAdaptationSet*> normalAS = stream->GetMediaAdaptationSet();
        std::map<int, OmafExtractor*> extractorAS  = stream->GetExtractors();

        for (auto itAS = normalAS.begin(); itAS != normalAS.end(); itAS++)
        {
            OmafAdaptationSet *pAS = (OmafAdaptationSet*)(itAS->second);
            EXPECT_TRUE(pAS != NULL);

            ret = pAS->LoadLocalInitSegment();
            EXPECT_TRUE(ret == ERROR_NONE);

            OmafSegment *initSeg = pAS->GetInitSegment();
            EXPECT_TRUE(initSeg != NULL);

            memset(storedFileName, 0, 1024);
            std::string repId = pAS->GetRepresentationId();
            snprintf(storedFileName, 1024, "./segs_for_readertest/%s.init.mp4", repId.c_str());
            cacheFileName = storedFileName;

            initSeg->SetSegmentCacheFile(cacheFileName);
            initSeg->SetSegStored();
            ret = m_reader->parseInitializationSegment(initSeg, initSegID);
            EXPECT_TRUE(ret == ERROR_NONE);

            initSeg->SetInitSegID(initSegID);
            initSeg->SetSegID(initSegID);

            mapSegCnt[initSegID] = 0;

            initSegID++;
        }

        for (auto itAS = extractorAS.begin(); itAS != extractorAS.end(); itAS++)
        {
            OmafExtractor *extractor = (OmafExtractor*)(itAS->second);
            EXPECT_TRUE(extractor != NULL);

            ret = extractor->LoadLocalInitSegment();
            EXPECT_TRUE(ret == ERROR_NONE);

            OmafSegment *initSeg = extractor->GetInitSegment();
            EXPECT_TRUE(initSeg != NULL);

            memset(storedFileName, 0, 1024);
            std::string repId = extractor->GetRepresentationId();
            snprintf(storedFileName, 1024, "./segs_for_readertest/%s.init.mp4", repId.c_str());
            cacheFileName = storedFileName;

            initSeg->SetSegmentCacheFile(cacheFileName);
            initSeg->SetSegStored();
            ret = m_reader->parseInitializationSegment(initSeg, initSegID);
            EXPECT_TRUE(ret == ERROR_NONE);

            initSeg->SetInitSegID(initSegID);
            initSeg->SetSegID(initSegID);

            mapSegCnt[initSegID] = 0;

            initSegID++;
        }
    }

    EXPECT_TRUE(initSegID == 18);

    m_reader->getTrackInformations(trackInfos);

/*
    uint16_t idx = 0;
    for ( auto it = trackInfos.begin(); it != trackInfos.end(); it++)
    {
        TrackInformation trackInfo = *it;
        idx++;

        //EXPECT_TRUE(trackInfo.trackId == idx);
        //EXPECT_TRUE(trackInfo.initSegId == (idx - 1));
        //EXPECT_TRUE(trackInfo.alternateGroupID ==
        //printf("trackInfo.alternateGroupID %d \n", trackInfo.alternateGroupId);
        //printf("trackInfo.features %d \n", trackInfo.features);
        //printf("trackInfo.vrFeatures %d \n", trackInfo.vrFeatures);
        //printf("trackInfo.maxSampleSize %d \n", trackInfo.maxSampleSize);
        //printf("trackInfo.timeScale %d \n", trackInfo.timeScale);
        //printf("trackInfo.hasTypeInformation %d \n", trackInfo.hasTypeInformation);
        //EXPECT_TRUE(trackInfo.frameRate.den == 1);
        //EXPECT_TRUE(trackInfo.frameRate.num == 25);

        //EXPECT_TRUE(trackInfo.trackURI.size() != 0);
        for (auto it1 = trackInfo.trackURI.begin();
            it1 != trackInfo.trackURI.end(); it1++)
        {
            //printf("The character in trackURI is %c \n", (*it1));
        }

        //EXPECT_TRUE(trackInfo.alternateTrackIds.size() != 0);
        for (auto it1 = trackInfo.alternateTrackIds.begin();
            it1 != trackInfo.alternateTrackIds.end(); it1++)
        {
            //printf("The value in alternateTrackIds is %d \n", (*it1));
        }

        //EXPECT_TRUE(trackInfo.referenceTrackIds.size() != 0);
        for (auto it1 = trackInfo.referenceTrackIds.begin();
            it1 != trackInfo.referenceTrackIds.end(); it1++)
        {
            TypeToTrackIDs referenceTrackId = *it1;
            //printf("referenceTrackId.type is %s \n", referenceTrackId.type);
            //EXPECT_TRUE(referenceTrackId.trackIds.size() != 0);
            for (auto it2 = referenceTrackId.trackIds.begin();
                it2 != referenceTrackId.trackIds.end(); it2++)
            {
                //printf("reference track id is %d \n", (*it2));
            }
        }

        //EXPECT_TRUE(trackInfo.trackGroupIds.size() != 0);
        for (auto it1 = trackInfo.trackGroupIds.begin();
            it1 != trackInfo.trackGroupIds.end(); it1++)
        {
            TypeToTrackIDs trackGroupId = *it1;
            //printf("trackGroupId.type is %s \n", trackGroupId.type);
            //EXPECT_TRUE(trackGroupId.trackIds.size() != 0);
            for (auto it2 = trackGroupId.trackIds.begin();
                it2 != trackGroupId.trackIds.end(); it2++)
            {
                //printf("track id in trackGroupIds is %d \n", (*it2));
            }
        }

        //EXPECT_TRUE(trackInfo.sampleProperties.size() != 0);
        for (auto it1 = trackInfo.sampleProperties.begin();
            it1 != trackInfo.sampleProperties.end(); it1++)
        {
            SampleInformation info = *it1;
            //printf("sample info earliestTimestamp %ld \n", info.earliestTimestamp);
            //printf("sample info earliestTimestampTS %ld \n", info.earliestTimestampTS);
            //printf("sample info sampleDescriptionIndex %d \n", info.sampleDescriptionIndex);
            //printf("sample info initSegmentId %d \n", info.initSegmentId);
            //printf("sample info sampleDurationTS %ld \n", info.sampleDurationTS);
            //printf("sample info sampleEntryType %s \n", info.sampleEntryType);
            //printf("sample info sampleId %d \n", info.sampleId);
            //printf("sample info segmentId %d \n", info.segmentId);
            //printf("sample info sampleType %d \n", info.sampleType);
            //printf("sample info sampleFlags flagsAsUInt %d \n", info.sampleFlags.flagsAsUInt);
            //printf("sample info sampleFlags flags is_leading %d \n", info.sampleFlags.flags.is_leading);
            //printf("sample info sampleFlags flags reserved %d \n", info.sampleFlags.flags.reserved);
            //printf("sample info sampleFlags flags sample_degradation_priority %d \n", info.sampleFlags.flags.sample_degradation_priority);
            //printf("sample info sampleFlags flags sample_has_redundancy %d \n", info.sampleFlags.flags.sample_has_redundancy);
            //printf("sample info sampleFlags flags sample_depends_on %d \n", info.sampleFlags.flags.sample_depends_on);
            //printf("sample info sampleFlags flags sample_is_non_sync_sample %d \n", info.sampleFlags.flags.sample_is_non_sync_sample);
            //printf("sample info sampleFlags flags sample_padding_value %d \n", info.sampleFlags.flags.sample_padding_value);
        }
    }
*/
    for (auto it = m_listStream.begin(); it != m_listStream.end(); it++)
    {
        OmafMediaStream *stream = (OmafMediaStream*)(*it);
        EXPECT_TRUE(stream != NULL);

        std::map<int, OmafAdaptationSet*> normalAS = stream->GetMediaAdaptationSet();
        std::map<int, OmafExtractor*> extractorAS  = stream->GetExtractors();

        for (auto itAS = normalAS.begin(); itAS != normalAS.end(); itAS++)
        {
            OmafAdaptationSet *pAS = (OmafAdaptationSet*)(itAS->second);
            EXPECT_TRUE(pAS != NULL);

            pAS->Enable(true);

            ret = pAS->LoadLocalSegment();
            EXPECT_TRUE(ret == ERROR_NONE);

            OmafSegment *newSeg = pAS->GetLocalNextSegment();
            EXPECT_TRUE(newSeg != NULL);

            OmafSegment *initSeg = pAS->GetInitSegment();
            EXPECT_TRUE(initSeg != NULL);

            memset(storedFileName, 0, 1024);
            std::string repId = pAS->GetRepresentationId();
            snprintf(storedFileName, 1024, "./segs_for_readertest/%s.1.mp4", repId.c_str());
            cacheFileName = storedFileName;

            newSeg->SetSegmentCacheFile(cacheFileName);
            newSeg->SetSegStored();
            uint32_t initSegID = initSeg->GetInitSegID();
            uint32_t segID = ++(mapSegCnt[initSegID]);
            ret = m_reader->parseSegment(newSeg, initSegID, segID);
            EXPECT_TRUE(ret == ERROR_NONE);

            ret = m_reader->getTrackInformations(trackInfos);
            EXPECT_TRUE(ret == ERROR_NONE);
        }

        for (auto itAS = extractorAS.begin(); itAS != extractorAS.end(); itAS++)
        {
            OmafExtractor *extractor = (OmafExtractor*)(itAS->second);
            EXPECT_TRUE(extractor != NULL);

            extractor->Enable(true);

            ret = extractor->LoadLocalSegment();
            EXPECT_TRUE(ret == ERROR_NONE);

            OmafSegment *newSeg = extractor->GetLocalNextSegment();
            EXPECT_TRUE(newSeg != NULL);

            OmafSegment *initSeg = extractor->GetInitSegment();
            EXPECT_TRUE(initSeg != NULL);

            memset(storedFileName, 0, 1024);
            std::string repId = extractor->GetRepresentationId();
            snprintf(storedFileName, 1024, "./segs_for_readertest/%s.1.mp4", repId.c_str());
            cacheFileName = storedFileName;

            newSeg->SetSegmentCacheFile(cacheFileName);
            newSeg->SetSegStored();
            uint32_t initSegID = initSeg->GetInitSegID();
            uint32_t segID = ++(mapSegCnt[initSegID]);
            ret = m_reader->parseSegment(newSeg, initSegID, segID);
            EXPECT_TRUE(ret == ERROR_NONE);

            ret = m_reader->getTrackInformations(trackInfos);
            EXPECT_TRUE(ret == ERROR_NONE);
        }
    }

    FILE *fp = NULL;
    char fileName[256];
    memset(fileName, 0, 256);

    uint8_t vps[256] = { 0 };
    uint8_t sps[256] = { 0 };
    uint8_t pps[256] = { 0 };
    uint8_t vpsLen = 0;
    uint8_t spsLen = 0;
    uint8_t ppsLen = 0;
    for (uint32_t initSegIndex = 10; initSegIndex < 18; initSegIndex++)
    {
        uint32_t correspondTrackIdx = initSegIndex + 990;
        uint32_t trackIdx = ( initSegIndex << 16 ) | correspondTrackIdx;

        snprintf(fileName, 256, "Viewport%d.h265", correspondTrackIdx - 999);
        fp = fopen(fileName, "wb+");
        EXPECT_TRUE(fp != NULL);
        if(!fp) continue;

        uint32_t sampleIdx = 0;
        for ( ; sampleIdx < 25; sampleIdx++)
        {
            uint32_t packetSize = ((7680 * 3840 * 3) / 2) / 2;
            MediaPacket *packet = new MediaPacket();
            if (NULL == packet)
            {
                break;
            }
            packet->ReAllocatePacket(packetSize);

            if(!packet->Payload())
            {
                delete packet;
                break;
            }

            ret = m_reader->getExtractorTrackSampleData(trackIdx, sampleIdx, (char*)(packet->Payload()), packetSize);
            EXPECT_TRUE(ret == ERROR_NONE);

            if (sampleIdx == 0)
            {
                std::vector<VCD::OMAF::DecoderSpecificInfo> parameterSets;
                ret = m_reader->getDecoderConfiguration(trackIdx, sampleIdx, parameterSets);
                EXPECT_TRUE(ret == ERROR_NONE);

                for (auto const& parameter : parameterSets)
                {
                    if (parameter.codecSpecInfoType == VCD::MP4::HEVC_VPS)
                    {
                        vpsLen = parameter.codecSpecInfoBits.size;
                        for (uint32_t i = 0; i < parameter.codecSpecInfoBits.size; i++)
                        {
                            vps[i] = parameter.codecSpecInfoBits[i];
                        }
                    }

                    if (parameter.codecSpecInfoType == VCD::MP4::HEVC_SPS)
                    {
                        spsLen = parameter.codecSpecInfoBits.size;
                        for (uint32_t i = 0; i < parameter.codecSpecInfoBits.size; i++)
                        {
                            sps[i] = parameter.codecSpecInfoBits[i];
                        }
                    }

                    if (parameter.codecSpecInfoType == VCD::MP4::HEVC_PPS)
                    {
                        ppsLen = parameter.codecSpecInfoBits.size;
                        for (uint32_t i = 0; i < parameter.codecSpecInfoBits.size; i++)
                        {
                            pps[i] = parameter.codecSpecInfoBits[i];
                        }
                    }
                }

                fwrite(vps, 1, vpsLen, fp);
                fwrite(sps, 1, spsLen, fp);
                fwrite(pps, 1, ppsLen, fp);
            }

            fwrite((uint8_t*)(packet->Payload()), 1, packetSize, fp);

            delete packet;
            packet = NULL;
        }

        fclose(fp);
        fp = NULL;
    }
}
}
