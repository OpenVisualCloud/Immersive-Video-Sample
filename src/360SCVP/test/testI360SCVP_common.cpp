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
#include <fstream>
#include "../360SCVPAPI.h"

#include "../../utils/safe_mem.h"

namespace{
class I360SCVPTest_common : public testing::Test {
public:
    virtual void SetUp()
    {
      pInputFile = fopen("./test.265", "rb");
      pInputFileLow = fopen("./test_low.265", "rb");
      if((pInputFile==NULL) ||(pInputFileLow==NULL))
        return;
      frameWidth = 3840;
      frameHeight = 2048;
      frameWidthlow = 1280;
      frameHeightlow = 768;
      bufferlen = frameWidth * frameHeight * 3 / 2;
      bufferlenlow = frameWidthlow * frameHeightlow * 3 / 2;
      memset_s((void*)&param, sizeof(param_360SCVP), 0);
      pInputBuffer = new unsigned char[bufferlen];
      pInputBufferlow = new unsigned char[bufferlenlow];
      pOutputBuffer = new unsigned char[bufferlen];
      pOutputSEI = new unsigned char[2000];
      if(!pInputBuffer || !pOutputBuffer || !pInputBufferlow || !pOutputSEI)
        return;
      bufferlen = fread(pInputBuffer, 1, bufferlen, pInputFile);
      bufferlenlow = fread(pInputBufferlow, 1, bufferlenlow, pInputFileLow);
      param.pInputBitstream = pInputBuffer;
      param.inputBitstreamLen = bufferlen;
      param.pOutputBitstream = pOutputBuffer;
      param.pInputLowBitstream = pInputBufferlow;
      param.inputLowBistreamLen = bufferlenlow;
      param.frameWidthLow = frameWidthlow;
      param.frameHeightLow = frameHeightlow;
      param.frameWidth = frameWidth;
      param.frameHeight = frameHeight;
      param.pOutputSEI = pOutputSEI;
      param.outputSEILen = 0;



    }
    virtual void TearDown()
    {
      if(pInputBuffer)
        delete pInputBuffer;
      pInputBuffer = NULL;
      if(pInputBufferlow)
        delete pInputBufferlow;
      pInputBufferlow = NULL;
      if(pOutputBuffer)
        delete pOutputBuffer;
      pOutputBuffer = NULL;
      if(pOutputSEI)
        delete pOutputSEI;
      pOutputSEI = NULL;
      fclose(pInputFile);
      fclose(pInputFileLow);
    }

    param_360SCVP           param;
    unsigned char*          pInputBuffer;
    unsigned char*          pInputBufferlow;
    unsigned char*          pOutputBuffer;
    unsigned char*          pOutputSEI;
    int                     frameWidth;
    int                     frameHeight;
    int                     frameWidthlow;
    int                     frameHeightlow;
    int                     bufferlen;
    int                     bufferlenlow;
    FILE*                   pInputFile;
    FILE*                   pInputFileLow;
	param_oneStream_info*   pTiledBitstreamTotal;

};

TEST_F(I360SCVPTest_common, I360SCVPCreate_type0)
{
    param.usedType = E_STREAM_STITCH_ONLY;
    void* pI360SCVP = I360SCVP_Init(&param);
    bool notnull = (pI360SCVP != NULL);
    I360SCVP_unInit(pI360SCVP);
    EXPECT_TRUE(notnull == true);
}

TEST_F(I360SCVPTest_common, I360SCVPCreate_type1)
{
    param.usedType = E_MERGE_AND_VIEWPORT;
    param.paramViewPort.faceWidth = frameWidth;
    param.paramViewPort.faceHeight = frameHeight;
    param.paramViewPort.geoTypeInput = EGeometryType(E_SVIDEO_EQUIRECT);
    param.paramViewPort.viewportHeight = 960;
    param.paramViewPort.viewportWidth = 960;
    param.paramViewPort.geoTypeOutput = E_SVIDEO_VIEWPORT;
    param.paramViewPort.viewPortYaw = -90;
    param.paramViewPort.viewPortPitch = 0;
    param.paramViewPort.viewPortFOVH = 80;
    param.paramViewPort.viewPortFOVV = 80;
    param.paramViewPort.tileNumCol = 6;
    param.paramViewPort.tileNumRow = 3;
    param.paramViewPort.paramVideoFP.faces[0][0].idFace = 0;
    param.paramViewPort.paramVideoFP.faces[0][0].rotFace = NO_TRANSFORM;
    void* pI360SCVP = I360SCVP_Init(&param);
    bool notnull = (pI360SCVP != NULL);
    I360SCVP_unInit(pI360SCVP);
    EXPECT_TRUE(notnull == true);
}

TEST_F(I360SCVPTest_common, I360SCVPCreate_type2)
{
    param.usedType = E_PARSER_ONENAL;
    void* pI360SCVP = I360SCVP_Init(&param);
    bool notnull = (pI360SCVP != NULL);
    I360SCVP_unInit(pI360SCVP);
    EXPECT_TRUE(notnull == true);
}

TEST_F(I360SCVPTest_common, ParseNAL_type0)
{
    int ret = 0;
    param.usedType = E_STREAM_STITCH_ONLY;
    void* pI360SCVP = I360SCVP_Init(&param);
    EXPECT_TRUE(pI360SCVP != NULL);
    Nalu nal;
    nal.data = pInputBuffer;
    nal.dataSize = bufferlen;
    ret = I360SCVP_ParseNAL(&nal, pI360SCVP);
    EXPECT_TRUE(ret == 0);
    I360SCVP_unInit(pI360SCVP);
}

TEST_F(I360SCVPTest_common, ParseNAL_type1)
{
    int ret = 0;
    param.paramViewPort.faceWidth = 7680;
    param.paramViewPort.faceHeight = 3840;
    param.paramViewPort.geoTypeInput = EGeometryType(E_SVIDEO_EQUIRECT);
    param.paramViewPort.viewportHeight = 960;
    param.paramViewPort.viewportWidth = 960;
    param.paramViewPort.geoTypeOutput = E_SVIDEO_VIEWPORT;
    param.paramViewPort.viewPortYaw = -90;
    param.paramViewPort.viewPortPitch = 0;
    param.paramViewPort.viewPortFOVH = 80;
    param.paramViewPort.viewPortFOVV = 80;
    param.paramViewPort.tileNumCol = 6;
    param.paramViewPort.tileNumRow = 3;
    param.paramViewPort.paramVideoFP.faces[0][0].idFace = 0;
    param.paramViewPort.paramVideoFP.faces[0][0].rotFace = NO_TRANSFORM;

    param.usedType = E_MERGE_AND_VIEWPORT;
    void* pI360SCVP = I360SCVP_Init(&param);
    EXPECT_TRUE(pI360SCVP != NULL);
    if (!pI360SCVP)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    Nalu nal;
    nal.data = pInputBuffer;
    nal.dataSize = bufferlen;
    ret = I360SCVP_ParseNAL(&nal, pI360SCVP);
    I360SCVP_unInit(pI360SCVP);
    EXPECT_TRUE(ret == 0);
}

TEST_F(I360SCVPTest_common, ParseNAL_type2)
{
    int ret = 0;
    param.usedType = E_PARSER_ONENAL;
    void* pI360SCVP = I360SCVP_Init(&param);
    EXPECT_TRUE(pI360SCVP != NULL);
    if (!pI360SCVP)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    Nalu nal;
    nal.data = pInputBuffer;
    nal.dataSize = bufferlen;
    ret = I360SCVP_ParseNAL(&nal, pI360SCVP);
    I360SCVP_unInit(pI360SCVP);
    EXPECT_TRUE(ret == 0);
}

TEST_F(I360SCVPTest_common, GenerateSPS_PPS_SliceHdr)
{
    param.usedType = E_PARSER_ONENAL;
    void* pI360SCVP = I360SCVP_Init(&param);
    EXPECT_TRUE(pI360SCVP != NULL);
    if (!pI360SCVP)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    Nalu nal;
    int ret = 0;
    int loop = 6;
    unsigned char*   pInputBufferTmp = pInputBuffer;

    while (loop && bufferlen)
    {
        nal.data = pInputBufferTmp;
        nal.dataSize = bufferlen;
        ret = I360SCVP_ParseNAL(&nal, pI360SCVP);
        if (nal.naluType == 33)
        {
            param.pInputBitstream = nal.data;
            param.inputBitstreamLen = nal.dataSize;
            param.destWidth = 640;
            param.destHeight = 320;
            ret = I360SCVP_GenerateSPS(&param, pI360SCVP);
            if (ret < 0)
                printf("generate sps error\n");
        }
        else if (nal.naluType == 34)
        {
            uint16_t width[2] = { 720, 720 };
            uint16_t height[2] = { 360, 360 };
            TileArrangement tileArr;
            tileArr.tileColsNum = 2;
            tileArr.tileRowsNum = 2;
            tileArr.tileColWidth = width;
            tileArr.tileRowHeight = height;
            param.pInputBitstream = pInputBuffer;
            param.inputBitstreamLen = bufferlen;
            ret = I360SCVP_GeneratePPS(&param, &tileArr, pI360SCVP);
            if (ret < 0)
                printf("generate pps error\n");
        }
        else if (nal.naluType < 22)
        {
            param.pInputBitstream = pInputBuffer;
            param.inputBitstreamLen = bufferlen;

            ret = I360SCVP_GenerateSliceHdr(&param, 0, pI360SCVP);
            if (ret < 0)
                printf("generate slice header error\n");
        }

        pInputBufferTmp += (nal.dataSize + nal.startCodesSize);
        bufferlen -= (nal.dataSize - nal.startCodesSize);
        loop--;
    }

    I360SCVP_unInit(pI360SCVP);
    EXPECT_TRUE(ret == 0);
}

TEST_F(I360SCVPTest_common, GetParameter_PicInfo_type0)
{
    int ret = 0;
    param.usedType = E_STREAM_STITCH_ONLY;
    void* pI360SCVP = I360SCVP_Init(&param);
    EXPECT_TRUE(pI360SCVP != NULL);
    if (!pI360SCVP)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    Nalu nal;
    nal.data = pInputBuffer;
    nal.dataSize = bufferlen;
    ret = I360SCVP_ParseNAL(&nal, pI360SCVP);
    EXPECT_TRUE(ret == 0);
    if (ret)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }
    Param_PicInfo* pPicInfo,pic;
    pPicInfo = &pic;
    ret = I360SCVP_GetParameter(pI360SCVP, ID_SCVP_PARAM_PICINFO, (void**)&pPicInfo);
    I360SCVP_unInit(pI360SCVP);
    EXPECT_TRUE(ret == 0);
    EXPECT_TRUE(pPicInfo->picWidth !=0);
}

TEST_F(I360SCVPTest_common, GetParameter_PicInfo_type1)
{
    int ret = 0;
    param.paramViewPort.faceWidth = 7680;
    param.paramViewPort.faceHeight = 3840;
    param.paramViewPort.geoTypeInput = EGeometryType(E_SVIDEO_EQUIRECT);
    param.paramViewPort.viewportHeight = 960;
    param.paramViewPort.viewportWidth = 960;
    param.paramViewPort.geoTypeOutput = E_SVIDEO_VIEWPORT;
    param.paramViewPort.viewPortYaw = -90;
    param.paramViewPort.viewPortPitch = 0;
    param.paramViewPort.viewPortFOVH = 80;
    param.paramViewPort.viewPortFOVV = 80;
    param.paramViewPort.tileNumCol = 6;
    param.paramViewPort.tileNumRow = 3;
    param.usedType = E_MERGE_AND_VIEWPORT;
    param.paramViewPort.paramVideoFP.cols = 1;
    param.paramViewPort.paramVideoFP.rows = 1;
    param.paramViewPort.paramVideoFP.faces[0][0].idFace = 0;
    param.paramViewPort.paramVideoFP.faces[0][0].rotFace = NO_TRANSFORM;
    void* pI360SCVP = I360SCVP_Init(&param);
    EXPECT_TRUE(pI360SCVP != NULL);
    if (!pI360SCVP)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    Nalu nal;
    nal.data = pInputBuffer;
    nal.dataSize = bufferlen;
    ret = I360SCVP_ParseNAL(&nal, pI360SCVP);
    EXPECT_TRUE(ret == 0);
    if (ret)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }
    Param_PicInfo* pPicInfo,pic;
    pPicInfo = &pic;
    ret = I360SCVP_GetParameter(pI360SCVP, ID_SCVP_PARAM_PICINFO, (void**)&pPicInfo);
    I360SCVP_unInit(pI360SCVP);
    EXPECT_TRUE(ret == 0);
    EXPECT_TRUE(pPicInfo->picWidth !=0);
}

TEST_F(I360SCVPTest_common, GetParameter_PicInfo_type2)
{
    int ret = 0;
    param.usedType = E_PARSER_ONENAL;
    void* pI360SCVP = I360SCVP_Init(&param);
    EXPECT_TRUE(pI360SCVP != NULL);
    if (!pI360SCVP)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }
    int loop = 5;
    Nalu nal;
    unsigned char* pInputBufferTemp = pInputBuffer;
    while (loop && bufferlen)
    {
        nal.data = pInputBufferTemp;
        nal.dataSize = bufferlen;
        ret = I360SCVP_ParseNAL(&nal, pI360SCVP);
        EXPECT_TRUE(ret == 0);
        if (ret)
        {
            I360SCVP_unInit(pI360SCVP);
            return;
        }
        pInputBufferTemp += (nal.dataSize);
        bufferlen -= (nal.dataSize);
        loop--;
    }

    Param_PicInfo* pPicInfo,pic;
    pPicInfo = &pic;
    ret = I360SCVP_GetParameter(pI360SCVP, ID_SCVP_PARAM_PICINFO, (void**)&pPicInfo);
    I360SCVP_unInit(pI360SCVP);
    EXPECT_TRUE(ret == 0);
    EXPECT_TRUE(pPicInfo->picWidth !=0);
}

TEST_F(I360SCVPTest_common, GenerateProj)
{
    int ret = 0;
    param.usedType = E_PARSER_ONENAL;
    void* pI360SCVP = I360SCVP_Init(&param);
    EXPECT_TRUE(pI360SCVP != NULL);
    if (!pI360SCVP)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    ret = I360SCVP_GenerateProj(pI360SCVP, E_EQUIRECT_PROJECTION, param.pOutputBitstream, (int32_t*)&param.outputBitstreamLen);
    EXPECT_TRUE(ret ==0);
    if (ret)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }
    EXPECT_TRUE(param.outputBitstreamLen > 0);
    if (param.outputBitstreamLen <= 0)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    ret = I360SCVP_GenerateProj(pI360SCVP, E_CUBEMAP_PROJECTION, param.pOutputBitstream, (int32_t*)&param.outputBitstreamLen);
    I360SCVP_unInit(pI360SCVP);
    EXPECT_TRUE(ret ==0);
    EXPECT_TRUE(param.outputBitstreamLen > 0);
}

TEST_F(I360SCVPTest_common, GenerateRWPK)
{
    int ret = 0;
    param.usedType = E_PARSER_ONENAL;
    void* pI360SCVP = I360SCVP_Init(&param);
    EXPECT_TRUE(pI360SCVP != NULL);
    if (!pI360SCVP)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    RegionWisePacking reginWisePack;
    reginWisePack.constituentPicMatching = 0;
    reginWisePack.numRegions = 2;
    reginWisePack.packedPicHeight = 100;
    reginWisePack.packedPicWidth = 200;
    reginWisePack.projPicHeight = 480;
    reginWisePack.projPicWidth = 640;
    reginWisePack.rectRegionPacking = new RectangularRegionWisePacking[reginWisePack.numRegions];
    EXPECT_TRUE(reginWisePack.rectRegionPacking != NULL);
    RectangularRegionWisePacking* pRectRegionPackTmp = reginWisePack.rectRegionPacking;
    int num = reginWisePack.numRegions;
    if (reginWisePack.rectRegionPacking)
    {
        while (num)
        {
            memset_s(pRectRegionPackTmp, sizeof(RectangularRegionWisePacking), 0);
            pRectRegionPackTmp++;
            num--;
        }
        ret = I360SCVP_GenerateRWPK(pI360SCVP, &reginWisePack, param.pOutputBitstream, (int32_t*)&param.outputBitstreamLen);
        EXPECT_TRUE(ret == 0);
    }
    if (reginWisePack.rectRegionPacking)
        delete[]reginWisePack.rectRegionPacking;
    reginWisePack.rectRegionPacking = NULL;
    I360SCVP_unInit(pI360SCVP);
    EXPECT_TRUE(param.outputBitstreamLen > 0);
}

TEST_F(I360SCVPTest_common, SetParameter_SetViewport)
{
    int ret = 0;
    param.usedType = E_PARSER_ONENAL;
    void* pI360SCVP = I360SCVP_Init(&param);
    EXPECT_TRUE(pI360SCVP != NULL);
    if (!pI360SCVP)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    Param_ViewPortInfo paramViewPortInfo;
    paramViewPortInfo.faceWidth = 7680;
    paramViewPortInfo.faceHeight = 3840;
    paramViewPortInfo.geoTypeInput = EGeometryType(E_SVIDEO_EQUIRECT);
    paramViewPortInfo.viewportHeight = 960;
    paramViewPortInfo.viewportWidth = 960;
    paramViewPortInfo.geoTypeOutput = E_SVIDEO_VIEWPORT;
    paramViewPortInfo.viewPortYaw = -90;
    paramViewPortInfo.viewPortPitch = 0;
    paramViewPortInfo.viewPortFOVH = 80;
    paramViewPortInfo.viewPortFOVV = 80;
    paramViewPortInfo.tileNumCol = 6;
    paramViewPortInfo.tileNumRow = 3;
    paramViewPortInfo.usageType = E_STREAM_STITCH_ONLY;
    paramViewPortInfo.paramVideoFP.cols = 1;
    paramViewPortInfo.paramVideoFP.rows = 1;
    paramViewPortInfo.paramVideoFP.faces[0][0].idFace = 0;
    paramViewPortInfo.paramVideoFP.faces[0][0].rotFace = NO_TRANSFORM;
    ret = I360SCVP_SetParameter(pI360SCVP, ID_SCVP_PARAM_VIEWPORT, &paramViewPortInfo);
    I360SCVP_unInit(pI360SCVP);
    EXPECT_TRUE(ret ==0);
}

TEST_F(I360SCVPTest_common, SetViewportSEI)
{
    int ret = 0;
    param.usedType = E_PARSER_ONENAL;
    param.paramPicInfo.tileHeightNum = 2;
    param.paramPicInfo.tileWidthNum = 2;
    void* pI360SCVP = I360SCVP_Init(&param);
    EXPECT_TRUE(pI360SCVP != NULL);
    if (!pI360SCVP)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }
    OMNIViewPort viewport;
    viewport.viewportsSize = 2;
    viewport.vpId = 64;
    viewport.pViewports = new oneViewport[viewport.viewportsSize];
    EXPECT_TRUE(viewport.pViewports != NULL);
    oneViewport* pTemp = viewport.pViewports;
    for (int i = 0; i < viewport.viewportsSize; i++)
    {
        pTemp->AzimuthCentre = i;
        pTemp->ElevationCentre = i;
        pTemp->HorzRange = i;
        pTemp->tiltCentre = i;
        pTemp->VertRange = i;
        pTemp++;
    }
    ret = I360SCVP_SetParameter(pI360SCVP, ID_SCVP_PARAM_SEI_VIEWPORT, &viewport);
    delete [] viewport.pViewports;
    viewport.pViewports = NULL;
    EXPECT_TRUE(ret == 0);
    if (ret)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    Param_ViewPortInfo paramViewPortInfo;
    paramViewPortInfo.faceWidth = 7680;
    paramViewPortInfo.faceHeight = 3840;
    paramViewPortInfo.geoTypeInput = EGeometryType(E_SVIDEO_EQUIRECT);
    paramViewPortInfo.viewportHeight = 960;
    paramViewPortInfo.viewportWidth = 960;
    paramViewPortInfo.geoTypeOutput = E_SVIDEO_VIEWPORT;
    paramViewPortInfo.viewPortYaw = -90;
    paramViewPortInfo.viewPortPitch = 0;
    paramViewPortInfo.viewPortFOVH = 80;
    paramViewPortInfo.viewPortFOVV = 80;
    paramViewPortInfo.tileNumCol = 6;
    paramViewPortInfo.tileNumRow = 3;
    paramViewPortInfo.usageType = E_STREAM_STITCH_ONLY;
    paramViewPortInfo.paramVideoFP.cols = 1;
    paramViewPortInfo.paramVideoFP.rows = 1;
    paramViewPortInfo.paramVideoFP.faces[0][0].idFace = 0;
    paramViewPortInfo.paramVideoFP.faces[0][0].rotFace = NO_TRANSFORM;
    ret = I360SCVP_SetParameter(pI360SCVP, ID_SCVP_PARAM_VIEWPORT, &paramViewPortInfo);    EXPECT_TRUE(ret == 0);
    EXPECT_TRUE(ret == 0);
    if (ret)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    pTiledBitstreamTotal = new param_oneStream_info[param.paramPicInfo.tileHeightNum*param.paramPicInfo.tileWidthNum];
    memset_s(pTiledBitstreamTotal, param.paramPicInfo.tileHeightNum*param.paramPicInfo.tileWidthNum * sizeof(param_oneStream_info), 0);
    EXPECT_TRUE(pTiledBitstreamTotal != NULL);

    param_oneStream_info *ptemp = pTiledBitstreamTotal;
    for (int i = 0; i < param.paramPicInfo.tileHeightNum; i++)
    {
        for (int j = 0; j < param.paramPicInfo.tileWidthNum; j++)
        {
            ptemp->pTiledBitstreamBuffer = new unsigned char[bufferlen];
            ptemp->tilesHeightCount = 1;
            ptemp->tilesWidthCount = 1;
            ptemp->inputBufferLen = 4096;
            memcpy_s(ptemp->pTiledBitstreamBuffer, 4096, pInputBuffer, 4096);
            ptemp++;
        }
    }

    param.paramStitchInfo.pTiledBitstream = &pTiledBitstreamTotal;
    ret = I360SCVP_process(&param, pI360SCVP);

    ptemp = pTiledBitstreamTotal;
    for (int i = 0; i < param.paramPicInfo.tileHeightNum; i++)
    {
        for (int j = 0; j < param.paramPicInfo.tileWidthNum; j++)
        {
            if (ptemp->pTiledBitstreamBuffer)
                delete[]ptemp->pTiledBitstreamBuffer;
            ptemp++;
        }
    }

    if (pTiledBitstreamTotal)
        delete[]pTiledBitstreamTotal;
    if (viewport.pViewports)
        delete[]viewport.pViewports;

    I360SCVP_unInit(pI360SCVP);
    EXPECT_TRUE(ret == 0);
}

TEST_F(I360SCVPTest_common, SetRWPKSEI)
{
    int ret = 0;
    param.usedType = E_PARSER_ONENAL;
    param.paramPicInfo.tileHeightNum = 2;
    param.paramPicInfo.tileWidthNum = 2;
    void* pI360SCVP = I360SCVP_Init(&param);
    EXPECT_TRUE(pI360SCVP != NULL);
    if (!pI360SCVP)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    RegionWisePacking reginWisePack;
    reginWisePack.constituentPicMatching = 0;
    reginWisePack.numRegions = 2;
    reginWisePack.packedPicHeight = 100;
    reginWisePack.packedPicWidth = 200;
    reginWisePack.projPicHeight = 480;
    reginWisePack.projPicWidth = 640;
    reginWisePack.rectRegionPacking = new RectangularRegionWisePacking[reginWisePack.numRegions];
    EXPECT_TRUE(reginWisePack.rectRegionPacking != NULL);
    if (!(reginWisePack.rectRegionPacking))
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    RectangularRegionWisePacking* pRectRegionPackTmp = reginWisePack.rectRegionPacking;
    int num = reginWisePack.numRegions;
    if (reginWisePack.rectRegionPacking)
    {
        while (num)
        {
            memset_s(pRectRegionPackTmp, sizeof(RectangularRegionWisePacking), 0);
            pRectRegionPackTmp++;
            num--;
        }
        ret = I360SCVP_SetParameter(pI360SCVP, ID_SCVP_PARAM_SEI_RWPK, &reginWisePack);
        EXPECT_TRUE(ret == 0);
    }
    delete[] reginWisePack.rectRegionPacking;
    reginWisePack.rectRegionPacking = NULL;

    Param_ViewPortInfo paramViewPortInfo;
    paramViewPortInfo.faceWidth = 7680;
    paramViewPortInfo.faceHeight = 3840;
    paramViewPortInfo.geoTypeInput = EGeometryType(E_SVIDEO_EQUIRECT);
    paramViewPortInfo.viewportHeight = 960;
    paramViewPortInfo.viewportWidth = 960;
    paramViewPortInfo.geoTypeOutput = E_SVIDEO_VIEWPORT;
    paramViewPortInfo.viewPortYaw = -90;
    paramViewPortInfo.viewPortPitch = 0;
    paramViewPortInfo.viewPortFOVH = 80;
    paramViewPortInfo.viewPortFOVV = 80;
    paramViewPortInfo.tileNumCol = 6;
    paramViewPortInfo.tileNumRow = 3;
    paramViewPortInfo.paramVideoFP.cols = 1;
    paramViewPortInfo.paramVideoFP.rows = 1;
    paramViewPortInfo.paramVideoFP.faces[0][0].idFace = 0;
    paramViewPortInfo.paramVideoFP.faces[0][0].rotFace = NO_TRANSFORM;

    paramViewPortInfo.usageType = E_STREAM_STITCH_ONLY;
    ret = I360SCVP_SetParameter(pI360SCVP, ID_SCVP_PARAM_VIEWPORT, &paramViewPortInfo);
    EXPECT_TRUE(ret == 0);
    if (ret)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    pTiledBitstreamTotal = new param_oneStream_info[param.paramPicInfo.tileHeightNum*param.paramPicInfo.tileWidthNum];
    memset_s(pTiledBitstreamTotal, param.paramPicInfo.tileHeightNum*param.paramPicInfo.tileWidthNum * sizeof(param_oneStream_info), 0);
    EXPECT_TRUE(pTiledBitstreamTotal != NULL);

    param_oneStream_info *ptemp = pTiledBitstreamTotal;
    for (int i = 0; i < param.paramPicInfo.tileHeightNum; i++)
    {
        for (int j = 0; j < param.paramPicInfo.tileWidthNum; j++)
        {
            ptemp->pTiledBitstreamBuffer = new unsigned char[bufferlen];
            ptemp->tilesHeightCount = 1;
            ptemp->tilesWidthCount = 1;
            ptemp->inputBufferLen = 4096;
            memcpy_s(ptemp->pTiledBitstreamBuffer, 4096, pInputBuffer, 4096);
            ptemp++;
        }
    }

    param.paramStitchInfo.pTiledBitstream = &pTiledBitstreamTotal;
    ret = I360SCVP_process(&param, pI360SCVP);

    ptemp = pTiledBitstreamTotal;
    for (int i = 0; i < param.paramPicInfo.tileHeightNum; i++)
    {
        for (int j = 0; j < param.paramPicInfo.tileWidthNum; j++)
        {
            if (ptemp->pTiledBitstreamBuffer)
                delete[]ptemp->pTiledBitstreamBuffer;
            ptemp++;
        }
    }

    if (pTiledBitstreamTotal)
        delete[]pTiledBitstreamTotal;
    I360SCVP_unInit(pI360SCVP);
    EXPECT_TRUE(ret == 0);
}

TEST_F(I360SCVPTest_common, SetRotationSEI)
{
    int ret = 0;
    param.usedType = E_PARSER_ONENAL;
    param.paramPicInfo.tileHeightNum = 2;
    param.paramPicInfo.tileWidthNum = 2;
    void* pI360SCVP = I360SCVP_Init(&param);
    EXPECT_TRUE(pI360SCVP != NULL);
    if (!pI360SCVP)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }
    SphereRotation sphereRot;
    sphereRot.yawRotation = 0;
    sphereRot.pitchRotation = 2;
    sphereRot.rollRotation = 100;
    ret = I360SCVP_SetParameter(pI360SCVP, ID_SCVP_PARAM_SEI_ROTATION, &sphereRot);
    EXPECT_TRUE(ret == 0);
    if (ret)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }
    Param_ViewPortInfo paramViewPortInfo;
    paramViewPortInfo.faceWidth = 7680;
    paramViewPortInfo.faceHeight = 3840;
    paramViewPortInfo.geoTypeInput = EGeometryType(E_SVIDEO_EQUIRECT);
    paramViewPortInfo.viewportHeight = 960;
    paramViewPortInfo.viewportWidth = 960;
    paramViewPortInfo.geoTypeOutput = E_SVIDEO_VIEWPORT;
    paramViewPortInfo.viewPortYaw = -90;
    paramViewPortInfo.viewPortPitch = 0;
    paramViewPortInfo.viewPortFOVH = 80;
    paramViewPortInfo.viewPortFOVV = 80;
    paramViewPortInfo.tileNumCol = 6;
    paramViewPortInfo.tileNumRow = 3;
    paramViewPortInfo.usageType = E_STREAM_STITCH_ONLY;
    paramViewPortInfo.paramVideoFP.cols = 1;
    paramViewPortInfo.paramVideoFP.rows = 1;
    paramViewPortInfo.paramVideoFP.faces[0][0].idFace = 0;
    paramViewPortInfo.paramVideoFP.faces[0][0].rotFace = NO_TRANSFORM;
    ret = I360SCVP_SetParameter(pI360SCVP, ID_SCVP_PARAM_VIEWPORT, &paramViewPortInfo);
    EXPECT_TRUE(ret == 0);
    if (ret)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    pTiledBitstreamTotal = new param_oneStream_info[param.paramPicInfo.tileHeightNum*param.paramPicInfo.tileWidthNum];
    memset_s(pTiledBitstreamTotal, param.paramPicInfo.tileHeightNum*param.paramPicInfo.tileWidthNum * sizeof(param_oneStream_info), 0);
    EXPECT_TRUE(pTiledBitstreamTotal != NULL);

    param_oneStream_info *ptemp = pTiledBitstreamTotal;
    for (int i = 0; i < param.paramPicInfo.tileHeightNum; i++)
    {
        for (int j = 0; j < param.paramPicInfo.tileWidthNum; j++)
        {
            ptemp->pTiledBitstreamBuffer = new unsigned char[bufferlen];
            ptemp->tilesHeightCount = 1;
            ptemp->tilesWidthCount = 1;
            ptemp->inputBufferLen = 4096;
            memcpy_s(ptemp->pTiledBitstreamBuffer, 4096, pInputBuffer, 4096);
            ptemp++;
        }
    }

    param.paramStitchInfo.pTiledBitstream = &pTiledBitstreamTotal;
    ret = I360SCVP_process(&param, pI360SCVP);

    ptemp = pTiledBitstreamTotal;
    for (int i = 0; i < param.paramPicInfo.tileHeightNum; i++)
    {
        for (int j = 0; j < param.paramPicInfo.tileWidthNum; j++)
        {
            if (ptemp->pTiledBitstreamBuffer)
                delete[]ptemp->pTiledBitstreamBuffer;
            ptemp++;
        }
    }

    if (pTiledBitstreamTotal)
        delete[]pTiledBitstreamTotal;
    I360SCVP_unInit(pI360SCVP);
    EXPECT_TRUE(ret == 0);
}

TEST_F(I360SCVPTest_common, SetFramePackingSEI)
{
    int ret = 0;
    param.usedType = E_PARSER_ONENAL;
    param.paramPicInfo.tileHeightNum = 2;
    param.paramPicInfo.tileWidthNum = 2;
    void* pI360SCVP = I360SCVP_Init(&param);
    EXPECT_TRUE(pI360SCVP != NULL);
    if (!pI360SCVP)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    FramePacking framepack;
    memset_s(&framepack, sizeof(FramePacking), 0);
    framepack.frame0GridX = 10;
    framepack.frame0GridY = 10;
    framepack.frame1GridX = 100;
    framepack.frame1GridY = 100;
    ret = I360SCVP_SetParameter(pI360SCVP, ID_SCVP_PARAM_SEI_FRAMEPACKING, &framepack);
    EXPECT_TRUE(ret == 0);
    if (ret)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }
    Param_ViewPortInfo paramViewPortInfo;
    paramViewPortInfo.faceWidth = 7680;
    paramViewPortInfo.faceHeight = 3840;
    paramViewPortInfo.geoTypeInput = EGeometryType(E_SVIDEO_EQUIRECT);
    paramViewPortInfo.viewportHeight = 960;
    paramViewPortInfo.viewportWidth = 960;
    paramViewPortInfo.geoTypeOutput = E_SVIDEO_VIEWPORT;
    paramViewPortInfo.viewPortYaw = -90;
    paramViewPortInfo.viewPortPitch = 0;
    paramViewPortInfo.viewPortFOVH = 80;
    paramViewPortInfo.viewPortFOVV = 80;
    paramViewPortInfo.tileNumCol = 6;
    paramViewPortInfo.tileNumRow = 3;
    paramViewPortInfo.usageType = E_STREAM_STITCH_ONLY;
    paramViewPortInfo.paramVideoFP.cols = 1;
    paramViewPortInfo.paramVideoFP.rows = 1;
    paramViewPortInfo.paramVideoFP.faces[0][0].idFace = 0;
    paramViewPortInfo.paramVideoFP.faces[0][0].rotFace = NO_TRANSFORM;
    ret = I360SCVP_SetParameter(pI360SCVP, ID_SCVP_PARAM_VIEWPORT, &paramViewPortInfo);
    EXPECT_TRUE(ret == 0);
    if (ret)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    pTiledBitstreamTotal = new param_oneStream_info[param.paramPicInfo.tileHeightNum*param.paramPicInfo.tileWidthNum];
    memset_s(pTiledBitstreamTotal, param.paramPicInfo.tileHeightNum*param.paramPicInfo.tileWidthNum * sizeof(param_oneStream_info), 0);
    EXPECT_TRUE(pTiledBitstreamTotal != NULL);

    param_oneStream_info *ptemp = pTiledBitstreamTotal;
    for (int i = 0; i < param.paramPicInfo.tileHeightNum; i++)
    {
        for (int j = 0; j < param.paramPicInfo.tileWidthNum; j++)
        {
            ptemp->pTiledBitstreamBuffer = new unsigned char[bufferlen];
            ptemp->tilesHeightCount = 1;
            ptemp->tilesWidthCount = 1;
            ptemp->inputBufferLen = 4096;
            memcpy_s(ptemp->pTiledBitstreamBuffer, 4096, pInputBuffer, 4096);
            ptemp++;
        }
    }

    param.paramStitchInfo.pTiledBitstream = &pTiledBitstreamTotal;
    ret = I360SCVP_process(&param, pI360SCVP);

    ptemp = pTiledBitstreamTotal;
    for (int i = 0; i < param.paramPicInfo.tileHeightNum; i++)
    {
        for (int j = 0; j < param.paramPicInfo.tileWidthNum; j++)
        {
            if (ptemp->pTiledBitstreamBuffer)
                delete[]ptemp->pTiledBitstreamBuffer;
            ptemp++;
        }
    }

    if (pTiledBitstreamTotal)
        delete[]pTiledBitstreamTotal;
    I360SCVP_unInit(pI360SCVP);
    EXPECT_TRUE(ret == 0);
}

TEST_F(I360SCVPTest_common, parseRWPK)
{
    RegionWisePacking RWPK;
    RWPK.rectRegionPacking = new RectangularRegionWisePacking[DEFAULT_REGION_NUM];
    EXPECT_TRUE( RWPK.rectRegionPacking != NULL);
    if (!(RWPK.rectRegionPacking))
    {
        return;
    }

    RegionWisePacking* pOriRWPK = NULL;
    pOriRWPK = new RegionWisePacking;
    EXPECT_TRUE( pOriRWPK != NULL);
    if (!pOriRWPK )
    {
        delete [] RWPK.rectRegionPacking;
        RWPK.rectRegionPacking = NULL;
        return;
    }

    int ret = 0;
    param.paramViewPort.faceWidth = 3840;
    param.paramViewPort.faceHeight = 2048;
    param.paramViewPort.geoTypeInput = EGeometryType(E_SVIDEO_EQUIRECT);
    param.paramViewPort.viewportHeight = 960;
    param.paramViewPort.viewportWidth = 960;
    param.paramViewPort.geoTypeOutput = E_SVIDEO_VIEWPORT;
    param.paramViewPort.viewPortYaw = -90;
    param.paramViewPort.viewPortPitch = 0;
    param.paramViewPort.viewPortFOVH = 80;
    param.paramViewPort.viewPortFOVV = 80;
    param.usedType = E_MERGE_AND_VIEWPORT;
    param.paramViewPort.paramVideoFP.cols = 1;
    param.paramViewPort.paramVideoFP.rows = 1;
    param.paramViewPort.paramVideoFP.faces[0][0].idFace = 0;
    param.paramViewPort.paramVideoFP.faces[0][0].rotFace = NO_TRANSFORM;
    void* pI360SCVP = I360SCVP_Init(&param);
    EXPECT_TRUE(pI360SCVP != NULL);
    if (!pI360SCVP)
    {
        delete [] RWPK.rectRegionPacking;
        RWPK.rectRegionPacking = NULL;
        delete pOriRWPK;
        pOriRWPK = NULL;
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    ret = I360SCVP_process(&param, pI360SCVP);
    EXPECT_TRUE(ret == 0);
    if (ret)
    {
        delete [] RWPK.rectRegionPacking;
        RWPK.rectRegionPacking = NULL;
        delete pOriRWPK;
        pOriRWPK = NULL;
        I360SCVP_unInit(pI360SCVP);
        return;
    }
    EXPECT_TRUE(param.outputBitstreamLen > 0);
    if (param.outputBitstreamLen <= 0)
    {
        delete [] RWPK.rectRegionPacking;
        RWPK.rectRegionPacking = NULL;
        delete pOriRWPK;
        pOriRWPK = NULL;
        I360SCVP_unInit(pI360SCVP);
        return;
    }
    ret = I360SCVP_ParseRWPK(pI360SCVP, &RWPK, param.pOutputSEI, param.outputSEILen);
    EXPECT_TRUE(ret == 0);
    if (ret)
    {
        delete [] RWPK.rectRegionPacking;
        RWPK.rectRegionPacking = NULL;
        delete pOriRWPK;
        pOriRWPK = NULL;
        I360SCVP_unInit(pI360SCVP);
        return;
    }
    ret = I360SCVP_GetParameter(pI360SCVP, ID_SCVP_RWPK_INFO, (void**)&pOriRWPK);
    EXPECT_TRUE(ret == 0);
    if (ret)
    {
        delete [] RWPK.rectRegionPacking;
        RWPK.rectRegionPacking = NULL;
        delete pOriRWPK;
        pOriRWPK = NULL;
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    if ((RWPK.numRegions != pOriRWPK->numRegions) ||
        (RWPK.packedPicHeight != pOriRWPK->packedPicHeight) ||
        (RWPK.packedPicWidth != pOriRWPK->packedPicWidth) ||
        (RWPK.projPicHeight != pOriRWPK->projPicHeight) ||
        (RWPK.projPicWidth != pOriRWPK->projPicWidth) ||
        (RWPK.rectRegionPacking->projRegHeight != pOriRWPK->rectRegionPacking->projRegHeight) ||
        (RWPK.rectRegionPacking->projRegWidth != pOriRWPK->rectRegionPacking->projRegWidth))
        ret = 1;

    delete[] RWPK.rectRegionPacking;
    delete pOriRWPK;
    I360SCVP_unInit(pI360SCVP);
    EXPECT_TRUE(ret == 0);
}

}
