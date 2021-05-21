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
//! \file:   HevcNaluParser.cpp
//! \brief:  Hevc nalu parser class implementation
//!
//! Created on April 30, 2019, 6:04 AM
//!

#include <stdio.h>
#include "OmafPackingLog.h"
#include "error.h"
#include "HevcNaluParser.h"
#include "../../../../utils/safe_mem.h"

HevcNaluParser::~HevcNaluParser()
{
    DELETE_ARRAY(m_vpsNalu->data);
    DELETE_MEMORY(m_vpsNalu);
    DELETE_ARRAY(m_spsNalu->data);
    DELETE_MEMORY(m_spsNalu);
    DELETE_ARRAY(m_ppsNalu->data);
    DELETE_MEMORY(m_ppsNalu);
    DELETE_ARRAY(m_projNalu->data);
    DELETE_MEMORY(m_projNalu);
    DELETE_MEMORY(m_picInfo);
}

int32_t HevcNaluParser::ParseHeaderData()
{
    if (!m_360scvpHandle || !m_360scvpParam)
        return OMAF_ERROR_NULL_PTR;

    m_vpsNalu = new Nalu;
    if (!m_vpsNalu)
        return OMAF_ERROR_NULL_PTR;

    m_vpsNalu->data = m_360scvpParam->pInputBitstream;
    m_vpsNalu->dataSize = m_360scvpParam->inputBitstreamLen;

    int32_t ret = I360SCVP_ParseNAL(m_vpsNalu, m_360scvpHandle);
    if (ret)
        return OMAF_ERROR_NALU_NOT_FOUND;

    if (m_vpsNalu->data != m_360scvpParam->pInputBitstream)
        return OMAF_ERROR_INVALID_HEADER;

    uint8_t *temp = m_vpsNalu->data;
    m_vpsNalu->data = new uint8_t[m_vpsNalu->dataSize];
    if (!m_vpsNalu->data)
        return OMAF_ERROR_NULL_PTR;

    memcpy_s(m_vpsNalu->data, m_vpsNalu->dataSize, temp, m_vpsNalu->dataSize);

    m_spsNalu = new Nalu;
    if (!m_spsNalu)
        return OMAF_ERROR_NULL_PTR;

    m_spsNalu->data = temp + m_vpsNalu->dataSize;
    m_spsNalu->dataSize = m_360scvpParam->inputBitstreamLen - m_vpsNalu->dataSize;
    ret = I360SCVP_ParseNAL(m_spsNalu, m_360scvpHandle);
    if (ret)
        return OMAF_ERROR_NALU_NOT_FOUND;

    if (m_spsNalu->data != (temp + m_vpsNalu->dataSize))
        return OMAF_ERROR_INVALID_HEADER;

    temp = m_spsNalu->data;
    m_spsNalu->data = new uint8_t[m_spsNalu->dataSize];
    if (!m_spsNalu->data)
        return OMAF_ERROR_NULL_PTR;

    memcpy_s(m_spsNalu->data, m_spsNalu->dataSize, temp, m_spsNalu->dataSize);

    m_ppsNalu = new Nalu;
    if (!m_ppsNalu)
        return OMAF_ERROR_NULL_PTR;

    m_ppsNalu->data = temp + m_spsNalu->dataSize;
    m_ppsNalu->dataSize = m_360scvpParam->inputBitstreamLen - m_vpsNalu->dataSize - m_spsNalu->dataSize;
    ret = I360SCVP_ParseNAL(m_ppsNalu, m_360scvpHandle);
    if (ret)
        return OMAF_ERROR_NALU_NOT_FOUND;

    if (m_ppsNalu->data != (temp + m_spsNalu->dataSize))
        return OMAF_ERROR_INVALID_HEADER;

    temp = m_ppsNalu->data;
    m_ppsNalu->data = new uint8_t[m_ppsNalu->dataSize];
    if (!m_ppsNalu->data)
        return OMAF_ERROR_NULL_PTR;

    memcpy_s(m_ppsNalu->data, m_ppsNalu->dataSize, temp, m_ppsNalu->dataSize);

    m_projNalu = new Nalu;
    if (!m_projNalu)
        return OMAF_ERROR_NULL_PTR;

    m_projNalu->data = temp + m_ppsNalu->dataSize;
    m_projNalu->dataSize = m_360scvpParam->inputBitstreamLen - m_vpsNalu->dataSize - m_spsNalu->dataSize - m_ppsNalu->dataSize;
    ret = I360SCVP_ParseNAL(m_projNalu, m_360scvpHandle);
    if (ret)
        return OMAF_ERROR_NALU_NOT_FOUND;

    if (m_projNalu->data != (temp + m_ppsNalu->dataSize))
        return OMAF_ERROR_INVALID_HEADER;

    temp = m_projNalu->data;
    m_projNalu->data = new uint8_t[m_projNalu->dataSize];
    if (!m_projNalu->data)
        return OMAF_ERROR_NULL_PTR;

    memcpy_s(m_projNalu->data, m_projNalu->dataSize, temp, m_projNalu->dataSize);

    m_picInfo = new Param_PicInfo;
    if (!m_picInfo)
        return OMAF_ERROR_NULL_PTR;

    I360SCVP_GetParameter(m_360scvpHandle, ID_SCVP_PARAM_PICINFO, (void**)(&m_picInfo));

    return ERROR_NONE;
}

uint16_t HevcNaluParser::GetSrcWidth()
{
    if (!m_picInfo)
        return 0;

    return m_picInfo->picWidth;
}

uint16_t HevcNaluParser::GetSrcHeight()
{
    if (!m_picInfo)
        return 0;

    return m_picInfo->picHeight;
}

uint8_t HevcNaluParser::GetTileInRow()
{
    if (!m_picInfo)
        return 0;

    return m_picInfo->tileWidthNum;
}

uint8_t HevcNaluParser::GetTileInCol()
{
    if (!m_picInfo)
        return 0;

    return m_picInfo->tileHeightNum;
}

static int32_t HevcGetTileRect(Param_PicInfo *picInfo, uint16_t tileIdx, TileInfo *tileInfo)
{

    uint32_t i, tbX, tbY, PicWidthInCtbsY, PicHeightInCtbsY, tileX, tileY, oX, oY, val;

    if ((0 == picInfo->maxCUWidth) ||
        (0 == picInfo->tileWidthNum) ||
        (0 == picInfo->tileHeightNum))
        return OMAF_ERROR_INVALID_HEADER;

    PicWidthInCtbsY = picInfo->picWidth / picInfo->maxCUWidth;
    if (PicWidthInCtbsY * (uint32_t)(picInfo->maxCUWidth) < (uint32_t)(picInfo->picWidth)) PicWidthInCtbsY++;

    PicHeightInCtbsY = picInfo->picHeight / picInfo->maxCUWidth;
    if (PicHeightInCtbsY * (uint32_t)(picInfo->maxCUWidth) < (uint32_t)(picInfo->picHeight)) PicHeightInCtbsY++;

    tbX = tileIdx % picInfo->tileWidthNum;
    tbY = tileIdx / picInfo->tileWidthNum;

    tileX = tileY = 0;
    oX = oY = 0;
    val = 0;
    for (i=0; i < (uint32_t)(picInfo->tileWidthNum); i++) {
        if(picInfo->tileIsUniform) {
            val = (i+1)*PicWidthInCtbsY / picInfo->tileWidthNum - (i)*PicWidthInCtbsY / picInfo->tileWidthNum;
        }

        tileInfo->horizontalPos = oX;
        tileInfo->tileWidth = val;

        if (oX >= (tbX * (PicWidthInCtbsY /picInfo->tileWidthNum))) break;
        oX += val;
        tileX++;
    }
    for (i=0; i<(uint32_t)(picInfo->tileHeightNum); i++) {
        if (picInfo->tileIsUniform) {
            val = (i+1)*PicHeightInCtbsY / picInfo->tileHeightNum - (i)*PicHeightInCtbsY / picInfo->tileHeightNum;
        }

        tileInfo->verticalPos = oY;
        tileInfo->tileHeight = val;

        if (oY >= (tbY * (PicHeightInCtbsY / picInfo->tileHeightNum))) break;
        oY += val;
        tileY++;
    }

    tileInfo->horizontalPos = tileInfo->horizontalPos * picInfo->maxCUWidth;
    tileInfo->verticalPos = tileInfo->verticalPos * picInfo->maxCUWidth;
    tileInfo->tileWidth = tileInfo->tileWidth * picInfo->maxCUWidth;
    tileInfo->tileHeight = tileInfo->tileHeight * picInfo->maxCUWidth;


    if (tileInfo->horizontalPos + tileInfo->tileWidth > picInfo->picWidth)
        tileInfo->tileWidth = picInfo->picWidth - tileInfo->horizontalPos;
    if (tileInfo->verticalPos + tileInfo->tileHeight > picInfo->picHeight)
        tileInfo->tileHeight = picInfo->picHeight - tileInfo->verticalPos;

    return ERROR_NONE;
}

int32_t HevcNaluParser::GetTileInfo(uint16_t tileIdx, TileInfo *tileInfo)
{
    if (!m_picInfo)
        return OMAF_ERROR_NULL_PTR;

    if (m_picInfo->tileIsUniform)
    {
        int32_t ret = HevcGetTileRect(m_picInfo, tileIdx, tileInfo);
        if (ret)
            return ret;
    } else {
        return OMAF_ERROR_UNDEFINED_OPERATION;
    }

    return ERROR_NONE;
}

int16_t HevcNaluParser::ParseProjectionTypeSei()
{
    if (!m_projNalu)
        return -1;

    if (m_projNalu->seiPayloadType == 150)
    {
        return 0;
    } else if (m_projNalu->seiPayloadType == 151) {
        return 1;
    } else {
        return -1;
    }
}

int32_t HevcNaluParser::ParseSliceNalu(
        uint8_t *frameData,
        int32_t frameDataSize,
        uint16_t tilesNum,
        TileInfo *tilesInfo)
{
    if (!frameData || !frameDataSize || !tilesNum || !tilesInfo)
        return OMAF_ERROR_BAD_PARAM;

    m_360scvpParam->pInputBitstream = frameData;
    m_360scvpParam->inputBitstreamLen = frameDataSize;
    uint32_t restBSBytes = m_360scvpParam->inputBitstreamLen;

    while (restBSBytes > 0)
    {
        Nalu *tempNalu = new Nalu;
        tempNalu->data = m_360scvpParam->pInputBitstream;
        tempNalu->dataSize = restBSBytes;
        I360SCVP_ParseNAL(tempNalu, m_360scvpHandle);
        if (tempNalu->naluType == 32 || tempNalu->naluType == 33
        || tempNalu->naluType == 34 || tempNalu->naluType == 39
        || tempNalu->naluType == 40) // skip VPS/SPS/PPS/SEI
        {
            m_360scvpParam->pInputBitstream = m_360scvpParam->pInputBitstream + tempNalu->dataSize;
            restBSBytes -= tempNalu->dataSize;
            delete tempNalu;
            tempNalu = NULL;
        }
        else
        {
            delete tempNalu;
            tempNalu = NULL;
            break;
        }
    }

    uint32_t restBitstreamLen = restBSBytes;
    for (uint16_t tileIdx = 0; tileIdx < tilesNum; tileIdx++)
    {
        TileInfo *tileInfo = &(tilesInfo[tileIdx]);
        Nalu *nalu         = tileInfo->tileNalu;

        if (tileIdx == 0)
        {
            nalu->data       = m_360scvpParam->pInputBitstream;
            nalu->dataSize   = restBitstreamLen;
        }
        else
        {
            TileInfo *tileInfoPrev = &(tilesInfo[tileIdx - 1]);
            Nalu *naluPrev         = tileInfoPrev->tileNalu;
            nalu->data             = naluPrev->data + naluPrev->dataSize;
            nalu->dataSize         = restBitstreamLen;
        }

        uint8_t *startPos = nalu->data;

        I360SCVP_ParseNAL(nalu, m_360scvpHandle);

        if (nalu->data != startPos)
            return OMAF_ERROR_INVALID_FRAME_BITSTREAM;

        nalu->sliceHeaderLen = nalu->sliceHeaderLen - HEVC_NALUHEADER_LEN;

        restBitstreamLen = restBitstreamLen - nalu->dataSize;

        uint64_t actualSize = nalu->dataSize - HEVC_STARTCODES_LEN;
        nalu->data[0] = (uint8_t)((0xff000000 & actualSize) >> 24);
        nalu->data[1] = (uint8_t)((0x00ff0000 & actualSize) >> 16);
        nalu->data[2] = (uint8_t)((0x0000ff00 & actualSize) >> 8);
        nalu->data[3] = (uint8_t)((uint8_t)actualSize);
    }

    return ERROR_NONE;
}
