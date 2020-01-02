/*
 * Copyright (c) 2018, Intel Corporation
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
#ifndef _360SCVP_HEVC_TILESTREAM_H_
#define _360SCVP_HEVC_TILESTREAM_H_
#include "360SCVPBitstream.h"
#include "360SCVPHevcParser.h"
#include "360SCVPTiledstreamAPI.h"
#include "360SCVPCommonDef.h"

typedef enum NALU_TYPE
{
    VID_PARAM_SET = 0,
    SEQ_PARAM_SET,
    PIC_PARAM_SET,
    ACCESS_UNIT_DELIMITER,
    PREFIX_SEI,
    SLICE_HEADER = 10,
    SLICE_DATA,
    NALU_NUM
}NALU_type;

typedef struct ONESTREAM_INFO
{
    uint32_t            width;
    uint32_t            height;
    int32_t             tilesWidthCount;
    int32_t             tilesHeightCount;

    uint8_t            *pTiledBitstreamBuffer;
    uint32_t            inputBufferLen;
    uint32_t            curBufferLen;
    uint32_t            outputBufferLen;
    HEVCState          *hevcSlice;
    int32_t             columnWidth[22];
    int32_t             rowHeight[20];
    int32_t             address;
    int32_t             currentTileIdx;

}oneStream_info;

typedef struct HEVC_GEN_TILEDSTREAM
{
    uint32_t              frameWidth;
    uint32_t              frameHeight;

    uint8_t              *headerNal;
    uint8_t               headerNalSize;

    int32_t               tilesWidthCount;
    int32_t               tilesHeightCount;
    int32_t               columnCnt[22];
    int32_t               rowCnt[20];

    int32_t               outTilesWidthCount;
    int32_t               outTilesHeightCount;
    int32_t               columnWidth[22];
    int32_t               rowHeight[20];
    bool                  tilesUniformSpacing;
    bool                  AUD_enable;
    bool                  VUI_enable;

    oneStream_info      **pTiledBitstreams;
    uint8_t              *pOutputTiledBitstream;
    nal_info             *pNalInfo;
    int32_t               parseType;
    hevc_specialInfo      specialInfo;

    bool             key_frame_flag;
    HEVC_VPS         vps;
    HEVC_SPS         sps;
    HEVC_PPS         pps;
}hevc_gen_tiledstream;

int32_t set_genHandle_params(oneStream_info* cur, param_oneStream_info* in);
int32_t parse_tiles_info(hevc_gen_tiledstream* pGenTilesStream);
int32_t hevc_import_ffextradata(hevc_specialInfo* pSpecialInfo, HEVCState* hevc, uint32_t *pSize, int32_t *spsCnt, int32_t bParse);
int32_t parse_hevc_specialinfo(hevc_specialInfo* pSpecialInfo, HEVCState* hevc, uint32_t* nalsize, uint32_t* specialLen, int32_t* spsCnt, int32_t bParse);
#endif
