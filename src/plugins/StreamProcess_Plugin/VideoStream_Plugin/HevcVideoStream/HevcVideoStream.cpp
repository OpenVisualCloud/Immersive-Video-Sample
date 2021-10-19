/*
 * Copyright (c) 2020, Intel Corporation
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
//! \file:   HevcVideoStream.cpp
//! \brief:  HEVC Video stream process class implementation
//!
//! Created on November 6, 2020, 6:04 AM
//!

#include "HevcVideoStream.h"
#include "OmafPackingLog.h"
#include "error.h"

HevcVideoStream::HevcVideoStream()
{
    m_streamIdx = 0;
    m_codecId = CODEC_ID_H265;
    m_width = 0;
    m_height = 0;
    m_tileInRow = 0;
    m_tileInCol = 0;
    m_tileRowsInFace = 0;
    m_tileColsInFace = 0;
    m_tilesInfo = NULL;
    m_projType = VCD::OMAF::ProjectionFormat::PF_ERP;
    memset_s(m_cubeMapInfo, CUBEMAP_FACES_NUM * sizeof(CubeMapFaceInfo), 0);
    m_frameRate.num = 0;
    m_frameRate.den = 0;
    m_bitRate = 0;

    m_srcRwpk = NULL;
    m_srcCovi = NULL;

    m_videoSegInfoGen = NULL;
    m_currFrameInfo = NULL;

    m_360scvpParam = NULL;
    m_360scvpHandle = NULL;
    m_naluParser = NULL;
    m_isEOS = false;
    m_lastKeyFramePTS = 0;
    m_gopSize = 0;
}

HevcVideoStream::HevcVideoStream(const HevcVideoStream& src)
{
    m_streamIdx = src.m_streamIdx;
    m_codecId = src.m_codecId;
    m_width = src.m_width;
    m_height = src.m_height;
    m_tileInRow = src.m_tileInRow;
    m_tileInCol = src.m_tileInCol;
    m_tileRowsInFace = src.m_tileRowsInFace;
    m_tileColsInFace = src.m_tileColsInFace;
    m_tilesInfo = std::move(src.m_tilesInfo);
    m_projType = src.m_projType;
    memcpy_s(m_cubeMapInfo, CUBEMAP_FACES_NUM * sizeof(CubeMapFaceInfo), src.m_cubeMapInfo, CUBEMAP_FACES_NUM * sizeof(CubeMapFaceInfo));
    m_frameRate.num = src.m_frameRate.num;
    m_frameRate.den = src.m_frameRate.den;
    m_bitRate = src.m_bitRate;

    m_srcRwpk = std::move(src.m_srcRwpk);
    m_srcCovi = std::move(src.m_srcCovi);

    m_videoSegInfoGen = std::move(src.m_videoSegInfoGen);
    m_currFrameInfo = std::move(src.m_currFrameInfo);

    m_360scvpParam = std::move(src.m_360scvpParam);
    m_360scvpHandle = std::move(src.m_360scvpHandle);
    m_naluParser = std::move(src.m_naluParser);
    m_isEOS = src.m_isEOS;
    m_lastKeyFramePTS = 0;
    m_gopSize = 0;
}

HevcVideoStream& HevcVideoStream::operator=(HevcVideoStream&& other)
{
    m_streamIdx = other.m_streamIdx;
    m_codecId = other.m_codecId;
    m_width = other.m_width;
    m_height = other.m_height;
    m_tileInRow = other.m_tileInRow;
    m_tileInCol = other.m_tileInCol;
    m_tileRowsInFace = other.m_tileRowsInFace;
    m_tileColsInFace = other.m_tileColsInFace;
    m_tilesInfo = std::move(other.m_tilesInfo);
    m_projType = other.m_projType;
    memcpy_s(m_cubeMapInfo, CUBEMAP_FACES_NUM * sizeof(CubeMapFaceInfo), other.m_cubeMapInfo, CUBEMAP_FACES_NUM * sizeof(CubeMapFaceInfo));
    m_frameRate.num = other.m_frameRate.num;
    m_frameRate.den = other.m_frameRate.den;
    m_bitRate = other.m_bitRate;

    m_srcRwpk = std::move(other.m_srcRwpk);
    m_srcCovi = std::move(other.m_srcCovi);

    m_videoSegInfoGen = std::move(other.m_videoSegInfoGen);
    m_currFrameInfo = std::move(other.m_currFrameInfo);

    m_360scvpParam = std::move(other.m_360scvpParam);
    m_360scvpHandle = std::move(other.m_360scvpHandle);
    m_naluParser = std::move(other.m_naluParser);
    m_isEOS = other.m_isEOS;

    return *this;
}

HevcVideoStream::~HevcVideoStream()
{
    if (m_srcRwpk)
    {
        DELETE_ARRAY(m_srcRwpk->rectRegionPacking);

        delete m_srcRwpk;
        m_srcRwpk = NULL;
    }

    if (m_srcCovi)
    {
        DELETE_ARRAY(m_srcCovi->sphereRegions);

        delete m_srcCovi;
        m_srcCovi = NULL;
    }

    if (m_tilesInfo)
    {
        uint16_t tilesNum = m_tileInRow * m_tileInCol;
        for (uint16_t tileIdx = 0; tileIdx < tilesNum; tileIdx++)
        {
            DELETE_MEMORY(m_tilesInfo[tileIdx].tileNalu);
        }

        delete[] m_tilesInfo;
        m_tilesInfo = NULL;
    }

    DELETE_MEMORY(m_videoSegInfoGen);

    std::list<FrameBSInfo*>::iterator it1;
    for (it1 = m_frameInfoList.begin(); it1 != m_frameInfoList.end();)
    {
        FrameBSInfo *frameInfo = *it1;
        if (frameInfo)
        {
            DELETE_ARRAY(frameInfo->data);

            delete frameInfo;
            frameInfo = NULL;
        }

        it1 = m_frameInfoList.erase(it1);
    }
    m_frameInfoList.clear();

    std::list<FrameBSInfo*>::iterator it2;
    for (it2 = m_framesToOneSeg.begin(); it2 != m_framesToOneSeg.end();)
    {
        FrameBSInfo *frameInfo = *it2;
        if (frameInfo)
        {
            DELETE_ARRAY(frameInfo->data);

            delete frameInfo;
            frameInfo = NULL;
        }

        it2 = m_framesToOneSeg.erase(it2);
    }
    m_framesToOneSeg.clear();

    DELETE_MEMORY(m_360scvpParam);

    if (m_360scvpHandle)
    {
        I360SCVP_unInit(m_360scvpHandle);
    }

    DELETE_MEMORY(m_naluParser);
}

int32_t HevcVideoStream::ParseHeader()
{
    m_naluParser->ParseHeaderData();
    m_width = m_naluParser->GetSrcWidth();
    m_height = m_naluParser->GetSrcHeight();
    m_tileInRow = m_naluParser->GetTileInRow();
    m_tileInCol = m_naluParser->GetTileInCol();
    m_projType = (VCD::OMAF::ProjectionFormat)(m_naluParser->GetProjectionType());

    uint16_t tilesNum = m_tileInRow * m_tileInCol;
    m_tilesInfo = new TileInfo[tilesNum];
    if (!m_tilesInfo)
        return OMAF_ERROR_NULL_PTR;

    memset_s(m_tilesInfo, tilesNum * sizeof(TileInfo), 0);
    for (uint16_t tileIdx = 0; tileIdx < tilesNum; tileIdx++)
    {
        m_naluParser->GetTileInfo(tileIdx, &(m_tilesInfo[tileIdx]));
        m_tilesInfo[tileIdx].tileNalu = new Nalu;
        if (!(m_tilesInfo[tileIdx].tileNalu))
            return OMAF_ERROR_NULL_PTR;
    }

    return ERROR_NONE;
}

int32_t HevcVideoStream::FillRegionWisePackingForERP()
{
    if (!m_srcRwpk)
        return OMAF_ERROR_NULL_PTR;

    if (!m_tilesInfo)
        return OMAF_ERROR_NULL_PTR;

    m_srcRwpk->constituentPicMatching = 0;
    m_srcRwpk->numRegions             = m_tileInRow * m_tileInCol;
    m_srcRwpk->projPicWidth           = m_width;
    m_srcRwpk->projPicHeight          = m_height;
    m_srcRwpk->packedPicWidth         = m_width;
    m_srcRwpk->packedPicHeight        = m_height;

    m_srcRwpk->rectRegionPacking      = new RectangularRegionWisePacking[m_srcRwpk->numRegions];
    if (!(m_srcRwpk->rectRegionPacking))
        return OMAF_ERROR_NULL_PTR;

    for (uint8_t regionIdx = 0; regionIdx < m_srcRwpk->numRegions; regionIdx++)
    {
        RectangularRegionWisePacking *rectRwpk = &(m_srcRwpk->rectRegionPacking[regionIdx]);
        TileInfo *tileInfo                     = &(m_tilesInfo[regionIdx]);
        tileInfo->projFormat                   = VCD::OMAF::ProjectionFormat::PF_ERP;

        memset_s(rectRwpk, sizeof(RectangularRegionWisePacking), 0);
        rectRwpk->transformType = 0;
        rectRwpk->guardBandFlag = 0;
        rectRwpk->projRegWidth  = tileInfo->tileWidth;
        rectRwpk->projRegHeight = tileInfo->tileHeight;
        rectRwpk->projRegLeft   = tileInfo->horizontalPos;
        rectRwpk->projRegTop    = tileInfo->verticalPos;

        rectRwpk->packedRegWidth  = tileInfo->tileWidth;
        rectRwpk->packedRegHeight = tileInfo->tileHeight;
        rectRwpk->packedRegLeft   = tileInfo->horizontalPos;
        rectRwpk->packedRegTop    = tileInfo->verticalPos;

        rectRwpk->leftGbWidth          = 0;
        rectRwpk->rightGbWidth         = 0;
        rectRwpk->topGbHeight          = 0;
        rectRwpk->bottomGbHeight       = 0;
        rectRwpk->gbNotUsedForPredFlag = true;
        rectRwpk->gbType0              = 0;
        rectRwpk->gbType1              = 0;
        rectRwpk->gbType2              = 0;
        rectRwpk->gbType3              = 0;
    }

    return ERROR_NONE;
}

int32_t HevcVideoStream::FillRegionWisePackingForCubeMap()
{
    if (!m_srcRwpk)
        return OMAF_ERROR_NULL_PTR;

    if (!m_tilesInfo)
        return OMAF_ERROR_NULL_PTR;

    m_srcRwpk->constituentPicMatching = 0;
    m_srcRwpk->numRegions             = m_tileInRow * m_tileInCol;
    m_srcRwpk->projPicWidth           = m_width;
    m_srcRwpk->projPicHeight          = m_height;
    m_srcRwpk->packedPicWidth         = m_width;
    m_srcRwpk->packedPicHeight        = m_height;

    m_srcRwpk->rectRegionPacking      = new RectangularRegionWisePacking[m_srcRwpk->numRegions];
    if (!(m_srcRwpk->rectRegionPacking))
        return OMAF_ERROR_NULL_PTR;

    for (uint8_t regionIdx = 0; regionIdx < m_srcRwpk->numRegions; regionIdx++)
    {
        RectangularRegionWisePacking *rectRwpk = &(m_srcRwpk->rectRegionPacking[regionIdx]);
        TileInfo *tileInfo                     = &(m_tilesInfo[regionIdx]);
        tileInfo->projFormat                   = VCD::OMAF::ProjectionFormat::PF_CUBEMAP;

        memset_s(rectRwpk, sizeof(RectangularRegionWisePacking), 0);

        uint8_t regColId = regionIdx % m_tileInRow;
        uint8_t regRowId = regionIdx / m_tileInRow;
        uint8_t faceId = 0;
        faceId = (uint8_t)(regRowId / m_tileRowsInFace) * 3 + (uint8_t)(regColId / m_tileColsInFace);
        uint8_t mappedDefaultFaceId = m_cubeMapInfo[faceId].mappedStandardFaceId;
        uint8_t tileColIdInFace = regColId - (uint8_t)(faceId % 3) * m_tileColsInFace;
        uint8_t tileRowIdInFace = regRowId - (uint8_t)(faceId / 3) * m_tileRowsInFace;
        uint8_t regIdInDefaultLayout = (uint8_t)(mappedDefaultFaceId / 3) * m_tileInRow * m_tileRowsInFace + (uint8_t)(mappedDefaultFaceId % 3) * m_tileColsInFace;
        regIdInDefaultLayout += tileRowIdInFace * m_tileInRow + tileColIdInFace;

        rectRwpk->transformType = m_cubeMapInfo[faceId].transformType;
        rectRwpk->guardBandFlag = 0;
        rectRwpk->projRegWidth  = tileInfo->tileWidth;
        rectRwpk->projRegHeight = tileInfo->tileHeight;
        rectRwpk->projRegLeft   = (regIdInDefaultLayout % m_tileInRow) * (tileInfo->tileWidth); //projected region left is calculated according to default Cube-3x2 layout
        rectRwpk->projRegTop    = (regIdInDefaultLayout / m_tileInRow) * (tileInfo->tileHeight);

        tileInfo->defaultHorPos = rectRwpk->projRegLeft;
        tileInfo->defaultVerPos = rectRwpk->projRegTop;
        tileInfo->tileIdxInProjPic = regionIdx;
        uint32_t faceWidth  = m_width / 3;
        uint32_t faceHeight = m_height / 2;
        uint32_t faceColId  = tileInfo->defaultHorPos / faceWidth;
        uint32_t faceRowId  = tileInfo->defaultVerPos / faceHeight;
        uint16_t localX     = tileInfo->defaultHorPos % faceWidth;
        uint16_t localY     = tileInfo->defaultVerPos % faceHeight;
        if (faceRowId == 0)
        {
            if (faceColId == 0) //face PY in OMAF spec for Cube-3x2
            {
                tileInfo->corresFaceIdTo360SCVP = FACE_PY_IN_360SCVP; //convert face PY into number used in 360SCVP library
                tileInfo->corresHorPosTo360SCVP = localX;
                tileInfo->corresVerPosTo360SCVP = localY;
            }
            else if (faceColId == 1) //face PX in OMAF spec for Cube-3x2
            {
                tileInfo->corresFaceIdTo360SCVP = FACE_PX_IN_360SCVP;
                tileInfo->corresHorPosTo360SCVP = localX;
                tileInfo->corresVerPosTo360SCVP = localY;
            }
            else if (faceColId == 2) //face NY in OMAF spec for Cube-3x2
            {
                tileInfo->corresFaceIdTo360SCVP = FACE_NY_IN_360SCVP;
                tileInfo->corresHorPosTo360SCVP = localX;
                tileInfo->corresVerPosTo360SCVP = localY;
            }
        }
        else if (faceRowId == 1)
        {
            if (faceColId == 0) //face NZ in OMAF spec for Cube-3x2
            {
                tileInfo->corresFaceIdTo360SCVP = FACE_NZ_IN_360SCVP;
                tileInfo->corresVerPosTo360SCVP = localX;
                tileInfo->corresHorPosTo360SCVP = faceHeight - tileInfo->tileHeight - localY;
            }
            else if (faceColId == 1) //face NX in OMAF spec for Cube-3x2
            {
                tileInfo->corresFaceIdTo360SCVP = FACE_NX_IN_360SCVP;
                tileInfo->corresHorPosTo360SCVP = localX;
                tileInfo->corresVerPosTo360SCVP = localY;
            }
            else if (faceColId == 2) //face PZ in OMAF spec for Cube-3x2
            {
                tileInfo->corresFaceIdTo360SCVP = FACE_PZ_IN_360SCVP;
                tileInfo->corresVerPosTo360SCVP = faceWidth - tileInfo->tileWidth - localX;
                tileInfo->corresHorPosTo360SCVP = localY;
            }
        }

        rectRwpk->packedRegWidth  = tileInfo->tileWidth;
        rectRwpk->packedRegHeight = tileInfo->tileHeight;
        rectRwpk->packedRegLeft   = tileInfo->horizontalPos; //packed region left is left position in actual input source picture
        rectRwpk->packedRegTop    = tileInfo->verticalPos;

        rectRwpk->leftGbWidth          = 0;
        rectRwpk->rightGbWidth         = 0;
        rectRwpk->topGbHeight          = 0;
        rectRwpk->bottomGbHeight       = 0;
        rectRwpk->gbNotUsedForPredFlag = true;
        rectRwpk->gbType0              = 0;
        rectRwpk->gbType1              = 0;
        rectRwpk->gbType2              = 0;
        rectRwpk->gbType3              = 0;
    }

    return ERROR_NONE;
}

int32_t HevcVideoStream::FillContentCoverageForERP()
{
    if (!m_srcCovi)
        return OMAF_ERROR_NULL_PTR;

    if (!m_srcRwpk)
        return OMAF_ERROR_NULL_PTR;

    if (m_projType == VCD::OMAF::ProjectionFormat::PF_ERP) //ERP projection type
    {
        m_srcCovi->coverageShapeType = 1;// TwoAzimuthAndTwoElevationCircles
    }
    else
    {
        m_srcCovi->coverageShapeType = 0; //FourGreatCircles
    }

    m_srcCovi->numRegions          = m_tileInRow * m_tileInCol;
    m_srcCovi->viewIdcPresenceFlag = false;
    m_srcCovi->defaultViewIdc      = 0;

    m_srcCovi->sphereRegions = new SphereRegion[m_srcCovi->numRegions];
    if (!(m_srcCovi->sphereRegions))
        return OMAF_ERROR_NULL_PTR;

    // Fill sphere region information for each tile
    for (uint8_t regionIdx = 0; regionIdx < m_srcCovi->numRegions; regionIdx++)
    {
        SphereRegion *sphereRegion             = &(m_srcCovi->sphereRegions[regionIdx]);
        RectangularRegionWisePacking *rectRwpk = &(m_srcRwpk->rectRegionPacking[regionIdx]);

        memset_s(sphereRegion, sizeof(SphereRegion), 0);
        sphereRegion->viewIdc         = 0; //doesn't take effect when viewIdcPresenceFlag is 0
        sphereRegion->centreAzimuth   = (int32_t)((((m_width / 2) - (float)(rectRwpk->projRegLeft + rectRwpk->projRegWidth / 2)) * 360 * 65536) / m_width);
        sphereRegion->centreElevation = (int32_t)((((m_height / 2) - (float)(rectRwpk->projRegTop + rectRwpk->projRegHeight / 2)) * 180 * 65536) / m_height);
        sphereRegion->centreTilt      = 0;
        sphereRegion->azimuthRange    = (uint32_t)((rectRwpk->projRegWidth * 360.f * 65536) / m_width);
        sphereRegion->elevationRange  = (uint32_t)((rectRwpk->projRegHeight * 180.f * 65536) / m_height);
        sphereRegion->interpolate     = 0;
    }

    return ERROR_NONE;
}

int32_t HevcVideoStream::Initialize(
    uint8_t streamIdx,
    BSBuffer *bs,
    InitialInfo *initInfo)
{
    if (!bs || !initInfo)
        return OMAF_ERROR_NULL_PTR;

    m_srcRwpk = new RegionWisePacking;
    if (!m_srcRwpk)
        return OMAF_ERROR_NULL_PTR;

    m_streamIdx = streamIdx;

    m_codecId = bs->codecId;
    m_frameRate = bs->frameRate;
    m_bitRate = bs->bitRate;

    m_360scvpParam = new param_360SCVP;
    if (!m_360scvpParam)
        return OMAF_ERROR_NULL_PTR;

    memset_s(m_360scvpParam, sizeof(param_360SCVP), 0);

    m_360scvpParam->usedType                         = E_PARSER_ONENAL;
    m_360scvpParam->pInputBitstream                  = bs->data;
    m_360scvpParam->inputBitstreamLen                = bs->dataSize;
    m_360scvpParam->logFunction                      = (void*)logCallBack;

    m_360scvpHandle = I360SCVP_Init(m_360scvpParam);
    if (!m_360scvpHandle)
        return OMAF_ERROR_SCVP_INIT_FAILED;

    //if (m_codecId == 0) //CODEC_ID_H264
    //{
    //    m_naluParser = new AvcNaluParser(m_360scvpHandle, m_360scvpParam);
    //    if (!m_naluParser)
    //        return OMAF_ERROR_NULL_PTR;
    //} else if (m_codecId == 1) { //CODEC_ID_H265
    //    m_naluParser = new HevcNaluParser(m_360scvpHandle, m_360scvpParam);
    //    if (!m_naluParser)
    //        return OMAF_ERROR_NULL_PTR;
    //} else {
    //    return OMAF_ERROR_UNDEFINED_OPERATION;
    //}

    if (m_codecId == 1) { //CODEC_ID_H265
        m_naluParser = new HevcNaluParser(m_360scvpHandle, m_360scvpParam);
    }
    else {
        return OMAF_ERROR_INVALID_CODEC;
    }

    int32_t ret = ParseHeader();
    if (ret)
        return ret;

    if (initInfo->projType != (EGeometryType)(m_projType))
    {
        OMAF_LOG(LOG_ERROR, "Not matched projection type in bitstream and initial information !\n");
        return OMAF_ERROR_INVALID_PROJECTIONTYPE;
    }

    if (m_projType == VCD::OMAF::ProjectionFormat::PF_ERP)
    {
        m_srcCovi = new ContentCoverage;
        if (!m_srcCovi)
            return OMAF_ERROR_NULL_PTR;
    }

    if (m_projType == VCD::OMAF::ProjectionFormat::PF_CUBEMAP)
    {
        if (!(initInfo->cubeMapInfo))
        {
            OMAF_LOG(LOG_ERROR, "There is no input CubeMap information in initial information !\n");
            return OMAF_ERROR_BAD_PARAM;
        }
        m_cubeMapInfo[0] = (initInfo->cubeMapInfo)->face0MapInfo;
        m_cubeMapInfo[1] = (initInfo->cubeMapInfo)->face1MapInfo;
        m_cubeMapInfo[2] = (initInfo->cubeMapInfo)->face2MapInfo;
        m_cubeMapInfo[3] = (initInfo->cubeMapInfo)->face3MapInfo;
        m_cubeMapInfo[4] = (initInfo->cubeMapInfo)->face4MapInfo;
        m_cubeMapInfo[5] = (initInfo->cubeMapInfo)->face5MapInfo;

        if (m_tileInCol % 2)
        {
            OMAF_LOG(LOG_ERROR, "Each face in CubeMap should have the same tile rows !\n");
            return OMAF_ERROR_BAD_PARAM;
        }
        m_tileRowsInFace = m_tileInCol / 2;

        if (m_tileInRow % 3)
        {
            OMAF_LOG(LOG_ERROR, "Each face in CubeMap should have the same tile cols !\n");
            return OMAF_ERROR_BAD_PARAM;
        }
        m_tileColsInFace = m_tileInRow / 3;
    }

    m_videoSegInfoGen = new VideoSegmentInfoGenerator(
                                bs, initInfo, m_streamIdx,
                                m_width, m_height,
                                m_tileInRow, m_tileInCol);
    if (!m_videoSegInfoGen)
        return OMAF_ERROR_NULL_PTR;

    ret = m_videoSegInfoGen->Initialize(m_tilesInfo);
    if (ret)
        return ret;

    if (m_projType == VCD::OMAF::ProjectionFormat::PF_ERP)
    {
        ret = FillRegionWisePackingForERP();
    }
    else if (m_projType == VCD::OMAF::ProjectionFormat::PF_CUBEMAP)
    {
        ret = FillRegionWisePackingForCubeMap();
    }
    if (ret)
        return ret;

    if (m_projType == VCD::OMAF::ProjectionFormat::PF_ERP)
    {
        ret = FillContentCoverageForERP();
        if (ret)
            return ret;
    }

    return ERROR_NONE;
}

int32_t HevcVideoStream::AddFrameInfo(FrameBSInfo *frameInfo)
{
    if (!frameInfo || !(frameInfo->data))
        return OMAF_ERROR_NULL_PTR;

    if (!frameInfo->dataSize)
        return OMAF_ERROR_DATA_SIZE;

    FrameBSInfo *newFrameInfo = new FrameBSInfo;
    if (!newFrameInfo)
        return OMAF_ERROR_NULL_PTR;

    memset_s(newFrameInfo, sizeof(FrameBSInfo), 0);

    uint8_t *localData = new uint8_t[frameInfo->dataSize];
    if (!localData)
    {
        delete newFrameInfo;
        newFrameInfo = NULL;
        return OMAF_ERROR_NULL_PTR;
    }
    memcpy_s(localData, frameInfo->dataSize, frameInfo->data, frameInfo->dataSize);

    newFrameInfo->data = localData;
    newFrameInfo->dataSize = frameInfo->dataSize;
    newFrameInfo->pts = frameInfo->pts;
    newFrameInfo->isKeyFrame = frameInfo->isKeyFrame;

    if (m_gopSize == 0 && newFrameInfo->isKeyFrame) {
        if (m_lastKeyFramePTS != 0) {
            m_gopSize = (uint32_t)(newFrameInfo->pts - m_lastKeyFramePTS);
        }
        else {
            m_lastKeyFramePTS = newFrameInfo->pts;
        }
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    m_frameInfoList.push_back(newFrameInfo);

    return ERROR_NONE;
}

void HevcVideoStream::SetCurrFrameInfo()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_frameInfoList.size() > 0)
    {
        m_currFrameInfo = m_frameInfoList.front();
        m_frameInfoList.pop_front();
    }
}

int32_t HevcVideoStream::UpdateTilesNalu()
{
    if (!m_currFrameInfo)
        return OMAF_ERROR_NULL_PTR;

    uint16_t tilesNum = m_tileInRow * m_tileInCol;
    int32_t ret = m_naluParser->ParseSliceNalu(m_currFrameInfo->data, m_currFrameInfo->dataSize, tilesNum, m_tilesInfo);
    if (ret)
        return ret;

    return ERROR_NONE;
}

TileInfo* HevcVideoStream::GetAllTilesInfo()
{
    return m_tilesInfo;
}

FrameBSInfo* HevcVideoStream::GetCurrFrameInfo()
{
    return m_currFrameInfo;
}

void HevcVideoStream::DestroyCurrSegmentFrames()
{
    std::list<FrameBSInfo*>::iterator it;
    for (it = m_framesToOneSeg.begin(); it != m_framesToOneSeg.end(); )
    {
        FrameBSInfo *frameInfo = *it;
        if (frameInfo)
        {
            DELETE_ARRAY(frameInfo->data);
            delete frameInfo;
            frameInfo = NULL;
        }

        //m_framesToOneSeg.erase(it++);
        it = m_framesToOneSeg.erase(it);

    }
    m_framesToOneSeg.clear();
}

void HevcVideoStream::DestroyCurrFrameInfo()
{
    if (m_currFrameInfo)
    {
        DELETE_ARRAY(m_currFrameInfo->data);

        delete m_currFrameInfo;
        m_currFrameInfo = NULL;
    }
}

Nalu* HevcVideoStream::GetVPSNalu()
{
    if (m_codecId == CODEC_ID_H265)
    {
        return ((HevcNaluParser*)m_naluParser)->GetVPSNalu();
    }
    else
        return NULL;
}

Nalu* HevcVideoStream::GetSPSNalu()
{
    return m_naluParser->GetSPSNalu();
}

Nalu* HevcVideoStream::GetPPSNalu()
{
    return m_naluParser->GetPPSNalu();
}

extern "C" VideoStream* Create()
{
    HevcVideoStream *hevcVS = new HevcVideoStream;
    return (VideoStream*)(hevcVS);
}

extern "C" void Destroy(VideoStream* hevcVS)
{
    delete hevcVS;
    hevcVS = NULL;
}
