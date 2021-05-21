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
class I360SCVPTest_erp : public testing::Test {
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
      param.paramViewPort.paramVideoFP.cols = 1;
      param.paramViewPort.paramVideoFP.rows = 1;


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

TEST_F(I360SCVPTest_erp, SetviewportAndMergeProcess)
{
    int32_t referenceRWPKprojRegTop[35] = { 0, 0, 0, 0, 0, 0, 0, 0, 512, 512, 512, 768,
                                            768, 768, 1024, 1024, 1024, 1280, 1280, 1280, 0, 0, 0, 0,
                                            0, 682, 682, 682, 682, 682, 1364, 1364, 1364, 1364, 1364 };
    int32_t referenceRWPKprojRegLeft[35] = { 0, 384, 768, 1152, 1536, 1920, 2304, 2688, 384, 768, 1152, 384,
                                             768, 1152, 384, 768, 1152, 384, 768, 1152, 0, 768, 1536, 2304,
                                             3072, 0, 768, 1536, 2304, 3072, 0, 768, 1536, 2304, 3072 };
    int32_t referenceRWPKpackedRegTop[35] = { 0, 256, 512, 768, 1024, 0, 256, 512, 768, 1024,
                                              0, 256, 512, 768, 1024, 0, 256, 512, 768, 1024,
                                              0, 256, 512, 768, 1024, 0, 256, 512, 768, 1024,
                                              0, 256, 512, 768, 1024 };
    int32_t referenceRWPKpackedRegLeft[35] = {0, 0, 0, 0, 0, 384, 384, 384, 384, 384,
                                              768, 768, 768, 768, 768, 1152, 1152, 1152, 1152, 1152,
                                              1536, 1536, 1536, 1536, 1536, 1792, 1792, 1792, 1792, 1792,
                                              2048, 2048, 2048, 2048, 2048 };
    int32_t regionIdx;

    RegionWisePacking* pOriRWPK = NULL;
    pOriRWPK = new RegionWisePacking;
    EXPECT_TRUE( pOriRWPK != NULL);
    if (!pOriRWPK )
    {
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
        delete pOriRWPK;
        pOriRWPK = NULL;        I360SCVP_unInit(pI360SCVP);
        return;
    }

    I360SCVP_setViewPort(pI360SCVP, param.paramViewPort.viewPortYaw, param.paramViewPort.viewPortPitch);
    ret = I360SCVP_process(&param, pI360SCVP);
    EXPECT_TRUE(ret == 0);
    if (ret)
    {
        delete pOriRWPK;
        pOriRWPK = NULL;
        I360SCVP_unInit(pI360SCVP);
        return;
    }
    EXPECT_TRUE(param.outputBitstreamLen > 0);
    if (param.outputBitstreamLen <= 0)
    {
        delete pOriRWPK;
        pOriRWPK = NULL;
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    ret = I360SCVP_GetParameter(pI360SCVP, ID_SCVP_RWPK_INFO, (void**)&pOriRWPK);
    EXPECT_TRUE(ret == 0);
    if (ret)
    {
        delete pOriRWPK;
        pOriRWPK = NULL;
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    int32_t tileWidth = 384;
    int32_t tileHeight = 256;
    int32_t tileWidth_low = 768;
    int32_t tileHeight_low = 682;

    RectangularRegionWisePacking* pRectRegionPacking = pOriRWPK->rectRegionPacking;

    if ( (pOriRWPK->numRegions != 35)
        || (pOriRWPK->numHiRegions != 20)
        || (pOriRWPK->packedPicHeight != 1280)
        || (pOriRWPK->packedPicWidth != 2304)
        || (pOriRWPK->projPicHeight != frameHeight)
        || (pOriRWPK->projPicWidth != frameWidth)
        || (pOriRWPK->lowResPicWidth != frameWidthlow)
        || (pOriRWPK->lowResPicHeight != frameHeightlow) )
        ret = 1;

    for (regionIdx = 0; regionIdx < pOriRWPK->numHiRegions; regionIdx++) {
        if ( (pRectRegionPacking->transformType != NO_TRANSFORM)
            || (pRectRegionPacking->projRegHeight != tileHeight)
            || (pRectRegionPacking->projRegWidth != tileWidth)
            || (pRectRegionPacking->packedRegHeight != tileHeight)
            || (pRectRegionPacking->packedRegWidth != tileWidth)
            || (pRectRegionPacking->projRegTop != referenceRWPKprojRegTop[regionIdx])
            || (pRectRegionPacking->projRegLeft != referenceRWPKprojRegLeft[regionIdx])
            || (pRectRegionPacking->packedRegTop != referenceRWPKpackedRegTop[regionIdx])
            || (pRectRegionPacking->packedRegLeft != referenceRWPKpackedRegLeft[regionIdx]) )
            ret = 1;
         pRectRegionPacking++;
    }
    for (; regionIdx < pOriRWPK->numRegions; regionIdx++) {
        if ( (pRectRegionPacking->transformType != NO_TRANSFORM)
            || (pRectRegionPacking->projRegHeight != tileHeight_low)
            || (pRectRegionPacking->projRegWidth != tileWidth_low)
            || (pRectRegionPacking->packedRegHeight != 256)
            || (pRectRegionPacking->packedRegWidth != 256)
            || (pRectRegionPacking->projRegTop != referenceRWPKprojRegTop[regionIdx])
            || (pRectRegionPacking->projRegLeft != referenceRWPKprojRegLeft[regionIdx])
            || (pRectRegionPacking->packedRegTop != referenceRWPKpackedRegTop[regionIdx])
            || (pRectRegionPacking->packedRegLeft != referenceRWPKpackedRegLeft[regionIdx]) )
            ret = 1;
        pRectRegionPacking++;
    }

    delete pOriRWPK;
    pOriRWPK = NULL;
    I360SCVP_unInit(pI360SCVP);
    EXPECT_TRUE(ret == 0);
}

TEST_F(I360SCVPTest_erp, GetTilesInViewport)
{
    int32_t tileNum_fast, tileNum_legacy;
    TileDef pOutTile[1024];
    Param_ViewportOutput paramViewportOutput;

    param.paramViewPort.faceWidth = 7680;
    param.paramViewPort.faceHeight = 3840;
    param.paramViewPort.geoTypeInput = EGeometryType(E_SVIDEO_EQUIRECT);
    param.paramViewPort.viewportHeight = 1024;
    param.paramViewPort.viewportWidth = 1024;
    param.paramViewPort.geoTypeOutput = E_SVIDEO_VIEWPORT;
    param.paramViewPort.tileNumCol = 20;
    param.paramViewPort.tileNumRow = 10;
    param.paramViewPort.viewPortYaw = 0;
    param.paramViewPort.viewPortPitch = 0;
    param.paramViewPort.viewPortFOVH = 80;
    param.paramViewPort.viewPortFOVV = 90;
    param.usedType = E_VIEWPORT_ONLY;
    param.paramViewPort.paramVideoFP.cols = 1;
    param.paramViewPort.paramVideoFP.rows = 1;
    param.paramViewPort.paramVideoFP.faces[0][0].faceWidth = param.paramViewPort.faceWidth;
    param.paramViewPort.paramVideoFP.faces[0][0].faceHeight = param.paramViewPort.faceHeight;
    param.paramViewPort.paramVideoFP.faces[0][0].idFace = 1;
    param.paramViewPort.paramVideoFP.faces[0][0].rotFace = NO_TRANSFORM;

    void* pI360SCVP = I360SCVP_Init(&param);
    EXPECT_TRUE(pI360SCVP != NULL);
    if (!pI360SCVP)
    {
        I360SCVP_unInit(pI360SCVP);
        printf( "Init 360SCVP failure: pI360SCVP is NULL!!!\n");
        return;
    }

    tileNum_fast = I360SCVP_getTilesInViewport(pOutTile, &paramViewportOutput, pI360SCVP);

    I360SCVP_process(&param, pI360SCVP);
    I360SCVP_SetParameter(pI360SCVP, ID_SCVP_PARAM_VIEWPORT, &param.paramViewPort);
    tileNum_legacy = I360SCVP_GetTilesByLegacyWay(&pOutTile[tileNum_fast], pI360SCVP);

    I360SCVP_unInit(pI360SCVP);
    EXPECT_TRUE(tileNum_fast >= 0);
    EXPECT_TRUE(tileNum_legacy >= 0);
}

}
