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
class I360SCVPTest_cubemap : public testing::Test {
public:
    virtual void SetUp()
    {
      pInputFile = fopen("./testCube.265", "rb");
      pInputFileLow = fopen("./testCube_low.265", "rb");
      if((pInputFile==NULL) ||(pInputFileLow==NULL))
        return;
      frameWidth = 2880;
      frameHeight = 1920;
      frameWidthlow = 960;
      frameHeightlow = 640;
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
      param.paramViewPort.paramVideoFP.cols = 3;
      param.paramViewPort.paramVideoFP.rows = 2;
      param.paramViewPort.paramVideoFP.faces[0][0].idFace = OMAF_FACE_PX;
      param.paramViewPort.paramVideoFP.faces[0][0].rotFace = NO_TRANSFORM;
      param.paramViewPort.paramVideoFP.faces[0][1].idFace = OMAF_FACE_NX;
      param.paramViewPort.paramVideoFP.faces[0][1].rotFace = NO_TRANSFORM;
      param.paramViewPort.paramVideoFP.faces[0][2].idFace = OMAF_FACE_PY;
      param.paramViewPort.paramVideoFP.faces[0][2].rotFace = NO_TRANSFORM;
      param.paramViewPort.paramVideoFP.faces[1][0].idFace = OMAF_FACE_NY;
      param.paramViewPort.paramVideoFP.faces[1][0].rotFace = NO_TRANSFORM;
      param.paramViewPort.paramVideoFP.faces[1][1].idFace = OMAF_FACE_PZ;
      param.paramViewPort.paramVideoFP.faces[1][1].rotFace = NO_TRANSFORM;
      param.paramViewPort.paramVideoFP.faces[1][2].idFace = OMAF_FACE_NZ;
      param.paramViewPort.paramVideoFP.faces[1][2].rotFace = NO_TRANSFORM;
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

TEST_F(I360SCVPTest_cubemap, SetviewportAndMergeProcess)
{
    int32_t referenceRWPKprojRegTop[26] = { 0, 0, 0, 320, 320, 320, 640, 640, 640, 320, 320,
                                    320, 320, 320, 320, 320, 320, 0, 0, 0, 0, 0, 0,
                                    960, 960, 960};
    int32_t referenceRWPKprojRegLeft[26] = { 0, 320, 640, 0, 320, 640, 0, 320, 640, 0, 320, 640,
                                     960, 1280, 1600, 1920, 2240, 0, 0, 0, 0, 960, 1920,
                                     0, 960, 1920};
    int32_t referenceRWPKpackedRegTop[26] = { 0, 320, 640, 960, 1280, 0, 320, 640, 960, 1280,
                                    0, 320, 640, 960, 1280, 0, 320, 640, 960, 1280,
                                    0, 320, 640, 960, 1280, 0};
    int32_t referenceRWPKpackedRegLeft[26] = {0, 0, 0, 0, 0, 320, 320, 320, 320, 320,
                                    640, 640, 640, 640, 640, 960, 960, 960, 960, 960,
                                    1280, 1280, 1280, 1280, 1280, 1600 };
    int32_t regionIdx;

    RegionWisePacking* pOriRWPK = NULL;
    pOriRWPK = new RegionWisePacking;
    EXPECT_TRUE( pOriRWPK != NULL);
    if (!pOriRWPK )
    {
        return;
    }

    int ret = 0;
    param.paramViewPort.faceWidth = 960;
    param.paramViewPort.faceHeight = 960;
    param.paramViewPort.geoTypeInput = EGeometryType(E_SVIDEO_CUBEMAP);
    param.paramViewPort.viewportHeight = 1024;
    param.paramViewPort.viewportWidth = 1024;
    param.paramViewPort.geoTypeOutput = E_SVIDEO_VIEWPORT;
    param.paramViewPort.viewPortYaw = 0;
    param.paramViewPort.viewPortPitch = 0;
    param.paramViewPort.viewPortFOVH = 90;
    param.paramViewPort.viewPortFOVV = 90;
    param.paramViewPort.tileNumCol = 3;
    param.paramViewPort.tileNumRow = 3;
    param.usedType = E_MERGE_AND_VIEWPORT;
    void* pI360SCVP = I360SCVP_Init(&param);
    EXPECT_TRUE(pI360SCVP != NULL);
    if (!pI360SCVP)
    {
        delete pOriRWPK;
        pOriRWPK = NULL;
        I360SCVP_unInit(pI360SCVP);
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

    int32_t tileWidth, tileHeight;
    tileWidth = param.paramViewPort.faceWidth / param.paramViewPort.tileNumCol;
    tileHeight = param.paramViewPort.faceHeight / param.paramViewPort.tileNumRow;
    RectangularRegionWisePacking* pRectRegionPacking = pOriRWPK->rectRegionPacking;

    if ( (pOriRWPK->numRegions != 26)
        || (pOriRWPK->numHiRegions != 20)
        || (pOriRWPK->packedPicHeight != 1600)
        || (pOriRWPK->packedPicWidth != 1600)
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
            || (pRectRegionPacking->projRegHeight != param.paramViewPort.faceHeight)
            || (pRectRegionPacking->projRegWidth != param.paramViewPort.faceWidth)
            || (pRectRegionPacking->packedRegHeight != tileHeight)
            || (pRectRegionPacking->packedRegWidth != tileWidth)
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

TEST_F(I360SCVPTest_cubemap, GetTilesInViewport)
{
    TileDef pOutTile[1024];
    Param_ViewportOutput paramViewportOutput;
    int ret = 0;
    param.paramViewPort.faceWidth = 512 * 4;
    param.paramViewPort.faceHeight = 512 * 4;
    param.paramViewPort.geoTypeInput = EGeometryType(E_SVIDEO_CUBEMAP);
    param.paramViewPort.viewportHeight = 960;
    param.paramViewPort.viewportWidth = 960;
    param.paramViewPort.geoTypeOutput = E_SVIDEO_VIEWPORT;
    param.paramViewPort.tileNumCol = 4;
    param.paramViewPort.tileNumRow = 4;
    param.paramViewPort.viewPortYaw = -90;
    param.paramViewPort.viewPortPitch = 0;
    param.paramViewPort.viewPortFOVH = 80;
    param.paramViewPort.viewPortFOVV = 80;
    param.usedType = E_VIEWPORT_ONLY;
    param.paramViewPort.paramVideoFP.cols = 3;
    param.paramViewPort.paramVideoFP.rows = 2;
    param.paramViewPort.paramVideoFP.faces[0][0].idFace = 4;
    param.paramViewPort.paramVideoFP.faces[0][0].rotFace = NO_TRANSFORM;
    param.paramViewPort.paramVideoFP.faces[0][0].faceHeight = 512 * 4;
    param.paramViewPort.paramVideoFP.faces[0][0].faceHeight = 512 * 4;
    param.paramViewPort.paramVideoFP.faces[0][1].idFace = 0;
    param.paramViewPort.paramVideoFP.faces[0][1].rotFace = NO_TRANSFORM;
    param.paramViewPort.paramVideoFP.faces[0][2].idFace = 5;
    param.paramViewPort.paramVideoFP.faces[0][2].rotFace = NO_TRANSFORM;

    param.paramViewPort.paramVideoFP.faces[1][0].idFace = 3;
    param.paramViewPort.paramVideoFP.faces[1][0].rotFace = ROTATION_180_ANTICLOCKWISE;
    param.paramViewPort.paramVideoFP.faces[1][1].idFace = 1;
    param.paramViewPort.paramVideoFP.faces[1][1].rotFace = ROTATION_270_ANTICLOCKWISE;
    param.paramViewPort.paramVideoFP.faces[1][2].idFace = 2;
    param.paramViewPort.paramVideoFP.faces[1][2].rotFace = NO_TRANSFORM;
    void* pI360SCVP = I360SCVP_Init(&param);
    EXPECT_TRUE(pI360SCVP != NULL);
    if (!pI360SCVP)
    {
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    I360SCVP_setViewPort(pI360SCVP, param.paramViewPort.viewPortYaw, param.paramViewPort.viewPortPitch);
    I360SCVP_getTilesInViewport(pOutTile, &paramViewportOutput, pI360SCVP);
    ret = I360SCVP_process(&param, pI360SCVP);

    I360SCVP_unInit(pI360SCVP);
    EXPECT_TRUE(ret >= 0);
}

}
