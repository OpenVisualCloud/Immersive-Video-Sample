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
#include "stdio.h"
#include "stdlib.h"
#include "assert.h"
#include "360SCVPHevcTilestream.h"
#include "360SCVPHevcParser.h"
#include "360SCVPHevcEncHdr.h"
#include "360SCVPTiledstreamAPI.h"
#include "360SCVPLog.h"

int32_t hevc_import_ffextradata(hevc_specialInfo* pSpecialInfo, HEVCState* hevc, uint32_t *pSize, int32_t *spsCnt, int32_t bParse)
{
    int32_t spsNum = 0;
    const uint8_t *extradata = pSpecialInfo->ptr;
    const uint64_t extradata_size = pSpecialInfo->ptr_size;
    if(!extradata || !hevc || !pSize)
        return GTS_BAD_PARAM;
    int32_t bfinished = 0;
    GTS_BitStream *bs;
    int8_t *buffer = NULL;
    uint32_t buffer_size = 0;
    if (!extradata || (extradata_size < sizeof(uint32_t)))
        return GTS_BAD_PARAM;
    bs = gts_bs_new((const int8_t *)extradata, extradata_size, GTS_BITSTREAM_READ);
    if (!bs)
        return GTS_BAD_PARAM;

    uint32_t nal_size;
    int32_t idx = SLICE_DATA; // the slice header begins from idx == 5

    while (gts_bs_available(bs)) {
        uint8_t nal_unit_type;
        uint8_t startcodeSize = 4;
        uint64_t nal_start, start_code;

        start_code = gts_bs_read_U32(bs);
        if (start_code >> 8 == 0x000001) {
            nal_start = gts_bs_get_position(bs) - 1;
            gts_bs_seek(bs, nal_start);
            start_code = 1;
            startcodeSize = 3;
        }
        if (start_code != 0x00000001) {
            gts_bs_del(bs);
            if (buffer) gts_free(buffer);
            //if (vpss && spss && ppss) return GTS_OK;
            return GTS_BAD_PARAM;
        }
        nal_start = gts_bs_get_position(bs);
        nal_size = gts_media_nalu_next_start_code_bs(bs);
        if (nal_start + nal_size > extradata_size) {
            gts_bs_del(bs);
            return GTS_BAD_PARAM;
        }
        if (nal_size > buffer_size) {
            buffer = (int8_t*)gts_realloc(buffer, nal_size);
            buffer_size = nal_size;
        }
        gts_bs_read_data(bs, buffer, nal_size);

        //uint64_t start_position = bs->position - nal_size + 2;
        //printf("<nalu bs at:%d, %d \n",start_position,nal_size);
        if (buffer) gts_media_hevc_parse_nalu(pSpecialInfo, buffer, nal_size, hevc);

        nal_unit_type = pSpecialInfo->naluType;
        int32_t slicehdr_size = pSpecialInfo->sliceHeaderLen;

        if ((bParse==1) && (hevc->s_info.first_slice_segment_in_pic_flag == 1))
            bfinished++;

        if (pSpecialInfo->layer_id) {
            gts_bs_del(bs);
            gts_free(buffer);
            return GTS_BAD_PARAM;
        }

        switch (nal_unit_type)
        {
        case GTS_HEVC_NALU_SEQ_PARAM:
            pSize[SEQ_PARAM_SET] = nal_size + startcodeSize;
            spsNum++;
            break;
        case GTS_HEVC_NALU_PIC_PARAM:
            pSize[PIC_PARAM_SET] = nal_size + startcodeSize;
            break;
        case GTS_HEVC_NALU_VID_PARAM:
            pSize[VID_PARAM_SET] = nal_size + startcodeSize;
            break;
        case GTS_HEVC_NALU_ACCESS_UNIT:
            pSize[ACCESS_UNIT_DELIMITER] = nal_size + startcodeSize;
            break;
            /*slice_segment_layer_rbsp*/
        case GTS_HEVC_NALU_SLICE_TRAIL_N:
        case GTS_HEVC_NALU_SLICE_TRAIL_R:
        case GTS_HEVC_NALU_SLICE_TSA_N:
        case GTS_HEVC_NALU_SLICE_TSA_R:
        case GTS_HEVC_NALU_SLICE_STSA_N:
        case GTS_HEVC_NALU_SLICE_STSA_R:
        case GTS_HEVC_NALU_SLICE_BLA_W_LP:
        case GTS_HEVC_NALU_SLICE_BLA_W_DLP:
        case GTS_HEVC_NALU_SLICE_BLA_N_LP:
        case GTS_HEVC_NALU_SLICE_IDR_W_DLP:
        case GTS_HEVC_NALU_SLICE_IDR_N_LP:
        case GTS_HEVC_NALU_SLICE_CRA:
        case GTS_HEVC_NALU_SLICE_RADL_N:
        case GTS_HEVC_NALU_SLICE_RADL_R:
        case GTS_HEVC_NALU_SLICE_RASL_N:
        case GTS_HEVC_NALU_SLICE_RASL_R:
            pSize[SLICE_HEADER] = slicehdr_size + startcodeSize;
            if (bParse == 1)
            {
                pSize[idx] = nal_size + startcodeSize;
                idx++;

            }
            else
            {
                //save the slice data lenght in order to fetch the next frame
                pSize[SLICE_DATA] = nal_size - slicehdr_size;
                bfinished = 1;

            }
            break;
        case GTS_HEVC_NALU_PREFIX_SEI:
            pSize[PREFIX_SEI] = nal_size + startcodeSize;
            break;
        default:
            break;
        }
        // the caller just wants to parse one NAL
        if (bParse == 2)
        {
            pSpecialInfo->startCodesSize = startcodeSize;
            pSpecialInfo->ptr_size = nal_size;
            break;
        }
        else if (bParse == 1)
        {
            *spsCnt = spsNum;
            if (bfinished == 2)
                break;
        }
        else
        {
            if (bfinished)
                break;
        }

    }

    gts_bs_del(bs);
    if (buffer) gts_free(buffer);

    return idx;
}

int32_t  parse_hevc_specialinfo(hevc_specialInfo* pSpecialInfo, HEVCState* hevc, uint32_t* nalsize, uint32_t* specialLen, int32_t* spsCnt, int32_t bParse)
{
    if (!pSpecialInfo || !hevc || !nalsize || !specialLen)
        return GTS_BAD_PARAM;

    int32_t headers_cnt = SLICE_HEADER;
    int32_t i = 0;
    int32_t calnalbits = 0;

    //remove the useless byte in the input bitstream
    uint8_t *ptr = pSpecialInfo->ptr;
    uint32_t ptr_size = pSpecialInfo->ptr_size;
    int32_t  byteCnt = 0;
    while (ptr_size > 0)
    {
        if ((*ptr == 0 && *(ptr + 1) == 0 && *(ptr + 2) == 0 && *(ptr + 3) == 1)
            || (*ptr == 0 && *(ptr + 1) == 0 && *(ptr + 2) == 1))
            break;
        ptr++;
        ptr_size--;
        byteCnt++;
    }

    if (byteCnt > 0)
    {
        memmove_s(pSpecialInfo->ptr, ptr_size, pSpecialInfo->ptr + byteCnt, ptr_size);
        pSpecialInfo->ptr_size = ptr_size;
    }
    int32_t nalCnt = hevc_import_ffextradata(pSpecialInfo, hevc, nalsize, spsCnt, bParse);
    for (i = 0; i < headers_cnt; i++)
    {
        calnalbits += nalsize[i];
    }
    *specialLen = calnalbits;
    return nalCnt;
}

int32_t parse_tiles_info(hevc_gen_tiledstream* pGenTilesStream)
{
    if (!pGenTilesStream)
        return GTS_BAD_PARAM;
    uint32_t specialLen = 0;
    // define nxm tiles here, uniform type is default setting
    int32_t tilesWidthCount  = pGenTilesStream->tilesWidthCount;
    int32_t tilesHeightCount = pGenTilesStream->tilesHeightCount;

    // TODO:change tiles count to tiledStream number?
    bool havePPS = false;
    hevc_specialInfo* pSpecialInfo = &pGenTilesStream->specialInfo;
    int32_t totalWidthCount = 0, totalHeightCount = 0;
    for (int32_t i = 0; i < tilesHeightCount; i++)
    {
        totalWidthCount = 0;
        for (int32_t j = 0; j < tilesWidthCount; j++)
        {
            oneStream_info * pSliceCur = pGenTilesStream->pTiledBitstreams[i*tilesWidthCount + j];
            uint8_t * pBufferSliceCur = pSliceCur->pTiledBitstreamBuffer;
            int32_t lenSlice = pSliceCur->inputBufferLen;
            uint32_t nalsize[200];
            memset_s(nalsize, sizeof(nalsize), 0);

            pSpecialInfo->ptr = pBufferSliceCur;
            pSpecialInfo->ptr_size = lenSlice;

            int32_t spsCnt = 0;
            int32_t nalCnt = parse_hevc_specialinfo(pSpecialInfo, pSliceCur->hevcSlice, nalsize, &specialLen, &spsCnt, pGenTilesStream->parseType);
            if (spsCnt > 1)
                pBufferSliceCur = pBufferSliceCur + (spsCnt - 1) * specialLen;

            if (pGenTilesStream->parseType == 1)
            {
                //arrange the nalsize list 0--1st frame, 1--2nd frame
                int32_t pos = specialLen + nalsize[SLICE_DATA];
                pGenTilesStream->pNalInfo[0].nalLen = pos;
                pGenTilesStream->pNalInfo[0].pNalStream = pBufferSliceCur;
                for (int32_t idx = SLICE_DATA; idx < nalCnt; idx++)
                {
                    pGenTilesStream->pNalInfo[idx - SLICE_HEADER].nalLen = nalsize[idx+1];
                    pGenTilesStream->pNalInfo[idx - SLICE_HEADER].pNalStream = pBufferSliceCur + pos;
                    pos += nalsize[idx+1];
                }
                // do not parse these special information
                nalsize[SEQ_PARAM_SET] = 0;
                nalsize[PIC_PARAM_SET] = 0;
            }
            else if (pGenTilesStream->parseType == 2)
            {
                pGenTilesStream->pOutputTiledBitstream = pSpecialInfo->ptr;
            }

            if(nalsize[SEQ_PARAM_SET])
            {
                HEVC_SPS *sps = &pSliceCur->hevcSlice->sps[pSliceCur->hevcSlice->last_parsed_sps_id];
                pSliceCur->width = sps->width;
                pSliceCur->height = sps->height;
            }
/*
            if(pSliceCur->width != (pGenTilesStream->frameWidth / tilesWidthCount)
                || pSliceCur->height != (pGenTilesStream->frameHeight / tilesHeightCount))
            {
                pGenTilesStream->tilesUniformSpacing = false;
            }
*/
            if(nalsize[PIC_PARAM_SET])
            {
                havePPS = true;

                pSliceCur->address = totalWidthCount + totalHeightCount * pGenTilesStream->frameWidth / LCU_SIZE;

                HEVC_PPS *pps = &pSliceCur->hevcSlice->pps[pSliceCur->hevcSlice->last_parsed_pps_id];

                memset_s(pSliceCur->columnWidth, sizeof(pSliceCur->columnWidth), 0);
                memset_s(pSliceCur->rowHeight, sizeof(pSliceCur->rowHeight), 0);
                if(!pps->tiles_enabled_flag)
                {
                    pSliceCur->tilesWidthCount  = 1;
                    pSliceCur->tilesHeightCount = 1;
                    pSliceCur->columnWidth[0]    = pSliceCur->width / LCU_SIZE;
                    pSliceCur->rowHeight[0]      = pSliceCur->height / LCU_SIZE;
                }
                else if(pps->tiles_enabled_flag && pps->uniform_spacing_flag)
                {
                    pSliceCur->tilesWidthCount  = pps->num_tile_columns;
                    pSliceCur->tilesHeightCount = pps->num_tile_rows;
                    int32_t totalLCUinWidth = pSliceCur->width / LCU_SIZE;
                    int32_t totalLCUinHeight = pSliceCur->height / LCU_SIZE;
                    int32_t tmpSum = 0;
                    for(int32_t i = 0; i < pSliceCur->tilesWidthCount - 1; i++)
                    {
                        pSliceCur->columnWidth[i] = totalLCUinWidth * (i+1) / pSliceCur->tilesWidthCount - totalLCUinWidth * i / pSliceCur->tilesWidthCount;
                        tmpSum += pSliceCur->columnWidth[i];
                    }
                    if (pSliceCur->tilesWidthCount < 1 ||  pSliceCur->tilesWidthCount >= 23)
                        return GTS_BAD_PARAM;
                    pSliceCur->columnWidth[pSliceCur->tilesWidthCount-1] = totalLCUinWidth - tmpSum;

                    tmpSum = 0;
                    for(int32_t i = 0; i < pSliceCur->tilesHeightCount - 1; i++)
                    {
                        pSliceCur->rowHeight[i] = totalLCUinHeight * (i+1) / pSliceCur->tilesHeightCount - totalLCUinHeight * i / pSliceCur->tilesHeightCount;
                        tmpSum += pSliceCur->rowHeight[i];
                    }
                    if (pSliceCur->tilesHeightCount < 1 || pSliceCur->tilesHeightCount >= 21)
                        return GTS_BAD_PARAM;
                    pSliceCur->rowHeight[pSliceCur->tilesHeightCount-1] = totalLCUinHeight - tmpSum;
                }
                else
                {
                    pGenTilesStream->tilesUniformSpacing = false;

                    pSliceCur->tilesWidthCount  = pps->num_tile_columns;
                    pSliceCur->tilesHeightCount = pps->num_tile_rows;
                    for(int32_t i = 0; i < pSliceCur->tilesWidthCount; i++)
                    {
                        pSliceCur->columnWidth[i] = pps->column_width[i];
                    }
                    for(int32_t i = 0; i < pSliceCur->tilesHeightCount; i++)
                    {
                        pSliceCur->rowHeight[i]  = pps->row_height[i];
                    }
                }

                totalWidthCount += pSliceCur->width / LCU_SIZE;
            }
        }
        totalHeightCount += pGenTilesStream->pTiledBitstreams[i*tilesWidthCount + 0]->height / LCU_SIZE;
    }

    if(havePPS)
    {
        pGenTilesStream->outTilesWidthCount = 0;
        pGenTilesStream->outTilesHeightCount = 0;
        //TODO:
        //  1. need to check if total width/height count is larger than MAX value
        //  2. check if tile patition of every column/row is aligned
        for (int32_t i = 0; i < tilesWidthCount; i++)
        {
            oneStream_info * pSliceCur = pGenTilesStream->pTiledBitstreams[i];
            for(int32_t j = 0; j < pSliceCur->tilesWidthCount; j++)
            {
                pGenTilesStream->columnWidth[pGenTilesStream->outTilesWidthCount + j] = pSliceCur->columnWidth[j];
            }
            pGenTilesStream->outTilesWidthCount += pSliceCur->tilesWidthCount;

            // Count column number for each sub-stream
            pGenTilesStream->columnCnt[i] = pSliceCur->tilesWidthCount;
        }
        for (int32_t i = 0; i < tilesHeightCount; i++)
        {
            oneStream_info * pSliceCur = pGenTilesStream->pTiledBitstreams[i * tilesWidthCount];
            for(int32_t j = 0; j < pSliceCur->tilesHeightCount; j++)
            {
                pGenTilesStream->rowHeight[pGenTilesStream->outTilesHeightCount + j] = pSliceCur->rowHeight[j];
            }
            pGenTilesStream->outTilesHeightCount += pSliceCur->tilesHeightCount;

            // Count row number for each sub-stream
            pGenTilesStream->rowCnt[i] = pSliceCur->tilesHeightCount;
        }
    }
    return specialLen;
}

int32_t set_genHandle_params(oneStream_info* cur, param_oneStream_info* in)
{
    if (!cur || !in)
        return GTS_BAD_PARAM;
    cur->pTiledBitstreamBuffer = in->pTiledBitstreamBuffer;
    cur->inputBufferLen = in->inputBufferLen;
    cur->curBufferLen = in->curBufferLen;
    cur->outputBufferLen = in->outputBufferLen;

    return 0;
}

void*   genTiledStream_Init(param_gen_tiledStream* pParamGenTiledStream)
{
    if (!pParamGenTiledStream)
        return NULL;

    hevc_gen_tiledstream *pGen = (hevc_gen_tiledstream *)malloc(sizeof(hevc_gen_tiledstream));
    if (!pGen)
        return NULL;
    memset_s(pGen, sizeof(hevc_gen_tiledstream), 0);
    pGen->pTiledBitstreams = (oneStream_info**)malloc(pParamGenTiledStream->tilesHeightCount
                                                       * pParamGenTiledStream->tilesWidthCount
                                                       * sizeof(oneStream_info *));
    if (pGen && pGen->pTiledBitstreams)
    {
        pGen->pNalInfo = pParamGenTiledStream->pNalInfo;
        pGen->frameHeight = pParamGenTiledStream->frameHeight;
        pGen->frameWidth = pParamGenTiledStream->frameWidth;
        pGen->tilesHeightCount = pParamGenTiledStream->tilesHeightCount;
        pGen->tilesWidthCount = pParamGenTiledStream->tilesWidthCount;
        for (int32_t i = 0; i < pGen->tilesHeightCount; i++)
        {
            for (int32_t j = 0; j < pGen->tilesWidthCount; j++)
            {
                pGen->pTiledBitstreams[i*pGen->tilesWidthCount + j] = (oneStream_info *)malloc(sizeof(oneStream_info));
                if (!pGen->pTiledBitstreams[i*pGen->tilesWidthCount + j])
                {
                    free(pGen->pTiledBitstreams);
                    pGen->pTiledBitstreams = NULL;
                    free(pGen);
                    pGen = NULL;
                    return NULL;
                }
                pGen->pTiledBitstreams[i*pGen->tilesWidthCount + j]->hevcSlice = (HEVCState*)malloc(sizeof(HEVCState));
                if (pGen->pTiledBitstreams[i*pGen->tilesWidthCount + j]->hevcSlice)
                {
                    memset_s(pGen->pTiledBitstreams[i*pGen->tilesWidthCount + j]->hevcSlice, sizeof(HEVCState), 0);
                    pGen->pTiledBitstreams[i*pGen->tilesWidthCount + j]->hevcSlice->sps_active_idx = -1;
                }

                pGen->pTiledBitstreams[i*pGen->tilesWidthCount + j]->currentTileIdx = 0;
            }
        }
    }
    //pGen->pOutputTiledBitstream = pParamGenTiledStream->pOutputTiledBitstream;
    pGen->tilesUniformSpacing = pParamGenTiledStream->tilesUniformSpacing;
    pGen->parseType = pParamGenTiledStream->parseType;
    pGen->VUI_enable = pParamGenTiledStream->VUI_enable;
    pGen->AUD_enable = pParamGenTiledStream->AUD_enable;
    pGen->key_frame_flag = false;

    return pGen;

}

int32_t  genTiledStream_process(param_gen_tiledStream* pParamGenTiledStream, void* pGenHandle)
{
   int32_t ret = 0;
 /*    int32_t outputlen = 0;
    if (!pParamGenTiledStream || !pGenHandle)
    {
        ret = 1;
        printf("the pointer of input paramter is NULL\n");
        return ret;
    }

    hevc_gen_tiledstream* pGenTilesStream = (hevc_gen_tiledstream*)pGenHandle;
    int32_t tiled_width = pGenTilesStream->tilesWidthCount;
    int32_t tiled_height = pGenTilesStream->tilesHeightCount;
    for (int32_t i = 0; i < tiled_height; i++)
    {
        for (int32_t j = 0; j < tiled_width; j++)
        {
            set_genHandle_params(pGenTilesStream->pTiledBitstreams[i*tiled_width + j], 
                        pParamGenTiledStream->pTiledBitstream[i*tiled_width + j]);
            pGenTilesStream->pTiledBitstreams[i*tiled_width + j]->inputBufferLen =
                pParamGenTiledStream->pTiledBitstream[i*tiled_width + j]->inputBufferLen;
            pGenTilesStream->pTiledBitstreams[i*tiled_width + j]->pTiledBitstreamBuffer =
                pParamGenTiledStream->pTiledBitstream[i*tiled_width + j]->pTiledBitstreamBuffer;

            pGenTilesStream->pTiledBitstreams[i*tiled_width + j]->curBufferLen = 0;
            pGenTilesStream->pTiledBitstreams[i*tiled_width + j]->outputBufferLen = 0;
            pGenTilesStream->pTiledBitstreams[i*tiled_width + j]->currentTileIdx = 0;
        }
    }

    pGenTilesStream->pOutputTiledBitstream = pParamGenTiledStream->pOutputTiledBitstream;

    ret = merge_partstream_into1bitstream(pGenTilesStream, pParamGenTiledStream->inputBistreamsLen);

    int32_t tiled_idx = 0;
    for (int32_t i = 0; i < tiled_height; i++)
    {
        for (int32_t j = 0; j < tiled_width; j++)
        {
            int32_t outputBufferLen = pGenTilesStream->pTiledBitstreams[tiled_idx]->outputBufferLen;
            int32_t curBufferLen = pGenTilesStream->pTiledBitstreams[tiled_idx]->curBufferLen;
            pParamGenTiledStream->pTiledBitstream[tiled_idx]->outputBufferLen = outputBufferLen;
            pParamGenTiledStream->pTiledBitstream[tiled_idx]->curBufferLen = curBufferLen;
            outputlen += outputBufferLen;
            tiled_idx++;
        }
    }

    oneStream_info * pSlice = pGenTilesStream->pTiledBitstreams[0];
    bool idr_flag = (pSlice->hevcSlice->s_info.nal_unit_type == GTS_HEVC_NALU_SLICE_IDR_W_DLP ||
                       pSlice->hevcSlice->s_info.nal_unit_type == GTS_HEVC_NALU_SLICE_IDR_N_LP);
    pParamGenTiledStream->sliceType = (slice_type)(idr_flag ? SLICE_IDR : pSlice->hevcSlice->s_info.slice_type);
    pParamGenTiledStream->pts = idr_flag ? 0 : pSlice->hevcSlice->s_info.poc_lsb;
    pParamGenTiledStream->outputiledbistreamlen = outputlen;*/
    return ret;

}

int32_t  genTiledStream_parseNals(param_gen_tiledStream* pParamGenTiledStream, void* pGenHandle)
{
    int32_t ret = 0;
    int32_t lenSpecial;
    if (!pParamGenTiledStream || !pGenHandle)
    {
        ret = 1;
        SCVP_LOG(LOG_ERROR, "the pointer of input paramter is NULL\n");
        return ret;
    }
    hevc_gen_tiledstream* pGenTilesStream = (hevc_gen_tiledstream*)pGenHandle;
    oneStream_info * pSlice = pGenTilesStream->pTiledBitstreams[0];
    int32_t tiled_width = pGenTilesStream->tilesWidthCount;
    int32_t tiled_height = pGenTilesStream->tilesHeightCount;
    for (int32_t i = 0; i < tiled_height; i++)
    {
        for (int32_t j = 0; j < tiled_width; j++)
        {
            set_genHandle_params(pGenTilesStream->pTiledBitstreams[i*tiled_width + j],
                pParamGenTiledStream->pTiledBitstream[i*tiled_width + j]);
            pGenTilesStream->pTiledBitstreams[i*tiled_width + j]->inputBufferLen =
                pParamGenTiledStream->pTiledBitstream[i*tiled_width + j]->inputBufferLen;
            pGenTilesStream->pTiledBitstreams[i*tiled_width + j]->pTiledBitstreamBuffer =
                pParamGenTiledStream->pTiledBitstream[i*tiled_width + j]->pTiledBitstreamBuffer;

            pGenTilesStream->pTiledBitstreams[i*tiled_width + j]->curBufferLen = 0;
            pGenTilesStream->pTiledBitstreams[i*tiled_width + j]->outputBufferLen = 0;
            pGenTilesStream->pTiledBitstreams[i*tiled_width + j]->currentTileIdx = 0;
        }
    }

    lenSpecial = parse_tiles_info(pGenTilesStream);

    //output some information
    pParamGenTiledStream->specialLen = lenSpecial;
    bool idr_flag = (pSlice->hevcSlice->s_info.nal_unit_type == GTS_HEVC_NALU_SLICE_IDR_W_DLP ||
        pSlice->hevcSlice->s_info.nal_unit_type == GTS_HEVC_NALU_SLICE_IDR_N_LP);
    pParamGenTiledStream->sliceType = (slice_type)(idr_flag ? SLICE_IDR : pSlice->hevcSlice->s_info.slice_type);
    pParamGenTiledStream->tilesHeightCount = pSlice->hevcSlice->pps[pSlice->hevcSlice->last_parsed_pps_id].num_tile_rows;
    pParamGenTiledStream->tilesWidthCount = pSlice->hevcSlice->pps[pSlice->hevcSlice->last_parsed_pps_id].num_tile_columns;
    pParamGenTiledStream->nalType = pGenTilesStream->specialInfo.naluType;
    pParamGenTiledStream->startCodesSize = pGenTilesStream->specialInfo.startCodesSize;
    pParamGenTiledStream->seiPayloadType = pGenTilesStream->specialInfo.seiPayloadType;
    pParamGenTiledStream->sliceHeaderLen = pGenTilesStream->specialInfo.sliceHeaderLen;
    pParamGenTiledStream->outputiledbistreamlen = pGenTilesStream->specialInfo.ptr_size;
    pParamGenTiledStream->pOutputTiledBitstream = pGenTilesStream->specialInfo.ptr;
    return ret;
}

int32_t   genTiledStream_getParam(void* pGenHandle, uint32_t id, uint8_t** pValue)
{
    int32_t size = 0;

    if (!pGenHandle || !pValue)
    {
        SCVP_LOG(LOG_ERROR, "the input pointer is null\n");
        return -1;
    }
    hevc_gen_tiledstream* pGenTilesStream = (hevc_gen_tiledstream*)pGenHandle;

    switch (id)
    {
    case ID_GEN_TILED_BITSTREAMS_VPS:
        break;
    case ID_GEN_TILED_BITSTREAMS_SPS:
        break;
    case ID_GEN_TILED_BITSTREAMS_PPS:
        break;
    case ID_GEN_TILED_BITSTREAMS_HEADER:
        *pValue = (uint8_t *)pGenTilesStream->headerNal;
        size = pGenTilesStream->headerNalSize;
        break;
    default:
        break;
    }
    return size;
}


int32_t   genTiledStream_setParam(void* pGenHandle, uint32_t id, uint8_t* pValue)
{
    if (!pGenHandle || !pValue)
    {
        SCVP_LOG(LOG_WARNING, "the input pointer is null\n");
        return 1;
    }

    switch (id)
    {
    case ID_GEN_TILED_BITSTREAMS_VPS:
        break;
    case ID_GEN_TILED_BITSTREAMS_SPS:
        break;
    case ID_GEN_TILED_BITSTREAMS_PPS:
        break;
    default:
        break;
    }
    return 0;
}

int32_t  genTiledStream_unInit(void* pGenHandle)
{
    int32_t ret = 0;
    hevc_gen_tiledstream *pGen = (hevc_gen_tiledstream *)pGenHandle;
    if (pGen)
    {
        if (pGen->pTiledBitstreams)
        {
            for (int32_t i = 0; i < pGen->tilesHeightCount; i++)
            {
                for (int32_t j = 0; j < pGen->tilesWidthCount; j++)
                {
                    if (pGen->pTiledBitstreams[i*pGen->tilesWidthCount + j]->hevcSlice)
                    {
                        free(pGen->pTiledBitstreams[i*pGen->tilesWidthCount + j]->hevcSlice);
                        pGen->pTiledBitstreams[i*pGen->tilesWidthCount + j]->hevcSlice = NULL;
                        free(pGen->pTiledBitstreams[i*pGen->tilesWidthCount + j]);
                        pGen->pTiledBitstreams[i*pGen->tilesWidthCount + j] = NULL;
                    }
                }
            }

            free(pGen->pTiledBitstreams);
            pGen->pTiledBitstreams = NULL;
        }

        free(pGen);
        pGen = NULL;
    }

    return ret;
}

