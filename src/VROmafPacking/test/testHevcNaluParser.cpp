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
//! \file:   testHevcNaluParser.cpp
//! \brief:  Hevc nalu parser class unit test
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include "gtest/gtest.h"
//#include "../../plugins/StreamProcess_Plugin/VideoStream_Plugin/HevcVideoStream//HevcNaluParser.h"
#include "HevcNaluParser.h"
#include "OmafPackingLog.h"

#include "../../utils/safe_mem.h"

class HevcNaluParserTest : public testing::Test
{
public:
    virtual void SetUp()
    {
        const char *fileName = "test.265";
        m_testFile = fopen(fileName, "r");
        if (!m_testFile)
            return;

        m_headerSize = 98; //changed later
        m_headerData = new uint8_t[m_headerSize];
        if (!m_headerData)
        {
            fclose(m_testFile);
            m_testFile = NULL;
            return;
        }

        fread(m_headerData, 1, m_headerSize, m_testFile);

        FILE *vpsFile = fopen("test_vps.bin", "wb+");
        if (!vpsFile)
        {
            fclose(m_testFile);
            m_testFile = NULL;
            DELETE_ARRAY(m_headerData);
            return;
        }
        fwrite(m_headerData, 1, 30, vpsFile);
        fclose(vpsFile);
        vpsFile = NULL;

        FILE *spsFile = fopen("test_sps.bin", "wb+");
        if (!spsFile)
        {
            fclose(m_testFile);
            m_testFile = NULL;
            DELETE_ARRAY(m_headerData);
            return;
        }
        fwrite(m_headerData + 30, 1, 48, spsFile);
        fclose(spsFile);
        spsFile = NULL;

        FILE *ppsFile = fopen("test_pps.bin", "wb+");
        if (!ppsFile)
        {
            fclose(m_testFile);
            m_testFile = NULL;
            DELETE_ARRAY(m_headerData);
            return;
        }
        fwrite(m_headerData + 78, 1, 11, ppsFile);
        fclose(ppsFile);
        ppsFile = NULL;

        m_360scvpParam = new param_360SCVP;
        if (!m_360scvpParam)
        {
            fclose(m_testFile);
            m_testFile = NULL;
            DELETE_ARRAY(m_headerData);
            return;
        }

        memset_s(m_360scvpParam, sizeof(param_360SCVP), 0);
        m_360scvpParam->usedType = E_PARSER_ONENAL;
        m_360scvpParam->pInputBitstream = m_headerData;
        m_360scvpParam->inputBitstreamLen = m_headerSize;
        m_360scvpParam->logFunction = (void*)logCallBack;

        m_360scvpHandle = I360SCVP_Init(m_360scvpParam);
        if (!m_360scvpHandle)
        {
            fclose(m_testFile);
            m_testFile = NULL;
            DELETE_ARRAY(m_headerData);
            DELETE_MEMORY(m_360scvpParam);
            return;
        }

        m_picWidth   = 3840;
        m_picHeight  = 1920;
        m_tilesInRow = 2;
        m_tilesInCol = 1;
        m_tileWidth  = 1920;
        m_tileHeight = 1920;
    }
    virtual void TearDown()
    {
        if (m_testFile)
        {
            fclose(m_testFile);
            m_testFile = NULL;
        }

        if (m_headerData)
        {
            delete[] m_headerData;
            m_headerData = NULL;
        }

        if (m_360scvpHandle)
        {
            I360SCVP_unInit(m_360scvpHandle);
            m_360scvpHandle = NULL;
        }

        if (m_360scvpParam)
        {
            delete m_360scvpParam;
            m_360scvpParam = NULL;
        }
        remove("test_vps.bin");
        remove("test_sps.bin");
        remove("test_pps.bin");
    }

    FILE          *m_testFile;
    void          *m_360scvpHandle;
    param_360SCVP *m_360scvpParam;
    uint8_t       *m_headerData;
    uint32_t      m_headerSize;
    uint16_t      m_picWidth;
    uint16_t      m_picHeight;
    uint8_t       m_tilesInRow;
    uint8_t       m_tilesInCol;
    uint16_t      m_tileWidth;
    uint16_t      m_tileHeight;
};

TEST_F(HevcNaluParserTest, ParseHevcHeader)
{
    HevcNaluParser *parser = new HevcNaluParser(m_360scvpHandle, m_360scvpParam);
    EXPECT_TRUE(parser != NULL);
    if (!parser)
        return;

    int32_t ret = parser->ParseHeaderData();
    EXPECT_TRUE(ret == 0);

    uint16_t frameWidth = parser->GetSrcWidth();
    EXPECT_TRUE(frameWidth == m_picWidth);

    uint16_t frameHeight = parser->GetSrcHeight();
    EXPECT_TRUE(frameHeight == m_picHeight);

    uint8_t tilesInRow = parser->GetTileInRow();
    EXPECT_TRUE(tilesInRow == m_tilesInRow);

    uint8_t tilesInCol = parser->GetTileInCol();
    EXPECT_TRUE(tilesInCol == m_tilesInCol);

    uint16_t projType = parser->GetProjectionType();
    EXPECT_TRUE(projType == 0);

    uint16_t widthOfTile = m_tileWidth;
    uint16_t heightOfTile = m_tileHeight;
    uint16_t tilesNum = tilesInRow * tilesInCol;
    TileInfo *tilesInfo = new TileInfo[tilesNum];
    EXPECT_TRUE(tilesInfo != NULL);
    if (!tilesInfo)
    {
        DELETE_MEMORY(parser);
        return;
    }
    for (uint16_t i = 0; i < tilesNum; i++)
    {
        parser->GetTileInfo(i, &(tilesInfo[i]));

        EXPECT_TRUE(tilesInfo[i].tileWidth == widthOfTile);
        EXPECT_TRUE(tilesInfo[i].tileHeight == heightOfTile);
        EXPECT_TRUE(tilesInfo[i].horizontalPos == (widthOfTile * (i % tilesInRow)));
        EXPECT_TRUE(tilesInfo[i].verticalPos == (heightOfTile * (i / tilesInRow)));

    }

    Nalu *vpsNalu = parser->GetVPSNalu();
    EXPECT_TRUE(vpsNalu != NULL);
    FILE *fp = fopen("test_vps.bin", "r");
    if (!fp)
    {
        DELETE_ARRAY(tilesInfo);
        DELETE_MEMORY(parser);
        return;
    }
    fseek(fp, 0L, SEEK_END);
    uint32_t vpsLen = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    uint8_t *vpsData = new uint8_t[vpsLen];
    if (!vpsData)
    {
        DELETE_ARRAY(tilesInfo);
        DELETE_MEMORY(parser);
        fclose(fp);
        fp = NULL;
        return;
    }
    fread(vpsData, 1, vpsLen, fp);

    int32_t diff = 0;
    int compRet = 0;
    compRet = memcmp_s(vpsNalu->data, vpsLen, vpsData, vpsLen, &diff);
    EXPECT_TRUE(compRet == 0);
    EXPECT_TRUE(vpsNalu->dataSize == vpsLen);
    EXPECT_TRUE(diff == 0);
    EXPECT_TRUE(vpsNalu->startCodesSize == 4);
    EXPECT_TRUE(vpsNalu->naluType == 32);

    delete[] vpsData;
    vpsData = NULL;
    fclose(fp);
    fp = NULL;

    Nalu *spsNalu = parser->GetSPSNalu();
    EXPECT_TRUE(spsNalu != NULL);
    fp = fopen("test_sps.bin", "r");
    if (!fp)
    {
        DELETE_ARRAY(tilesInfo);
        DELETE_MEMORY(parser);
        return;
    }
    fseek(fp, 0L, SEEK_END);
    uint32_t spsLen = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    uint8_t *spsData = new uint8_t[spsLen];
    if (!spsData)
    {
        DELETE_ARRAY(tilesInfo);
        DELETE_MEMORY(parser);
        fclose(fp);
        fp = NULL;
        return;
    }
    fread(spsData, 1, spsLen, fp);

    compRet = memcmp_s(spsNalu->data, spsLen, spsData, spsLen, &diff);
    EXPECT_TRUE(compRet == 0);
    EXPECT_TRUE(spsNalu->dataSize == spsLen); //includes start codes
    EXPECT_TRUE(diff == 0);
    EXPECT_TRUE(spsNalu->startCodesSize == 4);
    EXPECT_TRUE(spsNalu->naluType == 33);

    delete[] spsData;
    spsData = NULL;
    fclose(fp);
    fp = NULL;

    Nalu *ppsNalu = parser->GetPPSNalu();
    EXPECT_TRUE(ppsNalu != NULL);
    fp = fopen("test_pps.bin", "r");
    if (!fp)
    {
        DELETE_ARRAY(tilesInfo);
        DELETE_MEMORY(parser);
        return;
    }
    fseek(fp, 0L, SEEK_END);
    uint32_t ppsLen = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    uint8_t *ppsData = new uint8_t[ppsLen];
    if (!ppsData)
    {
        DELETE_ARRAY(tilesInfo);
        DELETE_MEMORY(parser);
        fclose(fp);
        fp = NULL;
        return;
    }
    fread(ppsData, 1, ppsLen, fp);

    compRet = memcmp_s(ppsNalu->data, ppsLen, ppsData, ppsLen, &diff);
    EXPECT_TRUE(compRet == 0);
    EXPECT_TRUE(ppsNalu->dataSize == ppsLen); //includes start codes
    EXPECT_TRUE(diff == 0);
    EXPECT_TRUE(ppsNalu->startCodesSize == 4);
    EXPECT_TRUE(ppsNalu->naluType == 34);

    delete[] ppsData;
    ppsData = NULL;
    fclose(fp);
    fp = NULL;

    delete parser;
    parser = NULL;
    delete[] tilesInfo;
    tilesInfo = NULL;
}

TEST_F(HevcNaluParserTest, ParseSliceNalu)
{
    HevcNaluParser *parser = new HevcNaluParser(m_360scvpHandle, m_360scvpParam);
    EXPECT_TRUE(parser != NULL);
    if (!parser)
        return;

    int32_t ret = parser->ParseHeaderData();
    EXPECT_TRUE(ret == 0);

    uint32_t firstFrameSize = 370771; //changed later
    uint8_t  *firstFrameData = new uint8_t[firstFrameSize];
    EXPECT_TRUE(firstFrameData != NULL);
    if (!firstFrameData)
    {
        DELETE_MEMORY(parser);
        return;
    }

    fread(firstFrameData, 1, firstFrameSize, m_testFile);

    uint16_t tilesNum = m_tilesInRow * m_tilesInCol;
    TileInfo *tilesInfo = new TileInfo[tilesNum];
    EXPECT_TRUE(tilesInfo != NULL);
    if (!tilesInfo)
    {
        DELETE_MEMORY(parser);
        DELETE_ARRAY(firstFrameData);
        return;
    }
    for (uint16_t i = 0; i < tilesNum; i++)
    {
        tilesInfo[i].tileNalu = new Nalu;
        EXPECT_TRUE(tilesInfo[i].tileNalu != NULL);
        if (!(tilesInfo[i].tileNalu))
        {
            DELETE_MEMORY(parser);
            DELETE_ARRAY(firstFrameData);
            for (uint16_t j = 0; j < i; j++)
            {
                DELETE_MEMORY(tilesInfo[j].tileNalu);
            }
            DELETE_ARRAY(tilesInfo);
            return;
        }
    }

    ret = parser->ParseSliceNalu(
                    firstFrameData,
                    firstFrameSize,
                    tilesNum,
                    tilesInfo);
    EXPECT_TRUE(ret == 0);

    FILE *fp1 = fopen("firstFrame.bin", "r");
    EXPECT_TRUE(fp1 != NULL);
    if (!fp1)
    {
        DELETE_MEMORY(parser);
        DELETE_ARRAY(firstFrameData);
        for (uint16_t i = 0; i < tilesNum; i++)
        {
            DELETE_MEMORY(tilesInfo[i].tileNalu);
        }
        DELETE_ARRAY(tilesInfo);
        return;
    }
    uint32_t sliceSize = 0;
    uint32_t sliceHrdLen = 0;
    for (uint16_t i = 0; i < tilesNum; i++)
    {
        fscanf(fp1, "%u,%u", &sliceSize, &sliceHrdLen);

        if (i == 0)
        {
            EXPECT_TRUE(tilesInfo[i].tileNalu->data == firstFrameData);
        }
        else
        {
            Nalu *prevNalu = tilesInfo[i-1].tileNalu;
            EXPECT_TRUE(tilesInfo[i].tileNalu->data == (prevNalu->data + prevNalu->dataSize));
        }
        EXPECT_TRUE(tilesInfo[i].tileNalu->dataSize == sliceSize);
        EXPECT_TRUE(tilesInfo[i].tileNalu->startCodesSize == 4);
        EXPECT_TRUE(tilesInfo[i].tileNalu->naluType == 19); //IDR_W_RADL
        EXPECT_TRUE(tilesInfo[i].tileNalu->sliceHeaderLen == sliceHrdLen);
    }

    delete[] firstFrameData;
    firstFrameData = NULL;
    fclose(fp1);
    fp1 = NULL;

    for (uint16_t i = 0; i < tilesNum; i++)
    {
        delete (tilesInfo[i].tileNalu);
        tilesInfo[i].tileNalu = NULL;
    }
    delete[] tilesInfo;
    tilesInfo = NULL;

    delete parser;
    parser = NULL;
}
