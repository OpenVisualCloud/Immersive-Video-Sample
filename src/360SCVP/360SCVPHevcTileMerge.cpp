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
#include"stdio.h"
#include"stdlib.h"
#include "math.h"
#include "vector"
#include "360SCVPHevcTileMerge.h"
#include "360SCVPBitstream.h"
#include "360SCVPHevcEncHdr.h"
#include "360SCVPHevcParser.h"
#include "360SCVPMergeStreamAPI.h"
 #include "360SCVPLog.h"

int32_t gcd(int32_t a, int32_t b)
{
    for (;;)
    {
        if (a == 0) return b;
        b %= a;
        if (b == 0) return a;
        a %= b;
    }
}

int32_t lcm(int32_t a, int32_t b)
{
    int32_t temp = gcd(a, b);

    return temp ? (a / temp * b) : 0;
}

int32_t init_one_bitstream(oneStream_info **pBs)
{
    if (!pBs)
        return -1;
    oneStream_info *pTiledBitstreams = *pBs;
    pTiledBitstreams = (oneStream_info *)malloc(sizeof(oneStream_info));
    if (!pTiledBitstreams)
        return -1;
    memset_s(pTiledBitstreams, sizeof(oneStream_info), 0);
    pTiledBitstreams->hevcSlice = (HEVCState*)malloc(sizeof(HEVCState));
    if(pTiledBitstreams->hevcSlice)
    {
        memset_s(pTiledBitstreams->hevcSlice, sizeof(HEVCState), 0);
        pTiledBitstreams->hevcSlice->sps_active_idx = -1;
    }
    pTiledBitstreams->outputBufferLen = 0;

    *pBs = pTiledBitstreams;
    return 0;
}

int32_t destory_one_bitstream(oneStream_info **pBs)
{
    oneStream_info *pTiledBitstreams = *pBs;
    if(pTiledBitstreams)
    {
        if(pTiledBitstreams->hevcSlice)
        {
            free(pTiledBitstreams->hevcSlice);
            pTiledBitstreams->hevcSlice = NULL;
        }

        free(pTiledBitstreams);
        pTiledBitstreams = NULL;
    }

    *pBs = pTiledBitstreams;
    return 0;
}

//modify resolution and tile segmentation
int32_t modify_parameter_sets(HEVCState *hevc, hevc_mergeStream *mergeStream)
{
    if (!hevc || !mergeStream)
        return -1;
    HEVC_PPS *m_pps = &(mergeStream->pps);

    HEVC_SPS *sps = &hevc->sps[0];
    HEVC_PPS *pps = &hevc->pps[hevc->last_parsed_pps_id];

    if (!m_pps || !sps || !pps)
        return -1;
    sps->width  = mergeStream->pic_width;
    sps->height = mergeStream->pic_height;

    pps->uniform_spacing_flag = (bool)false;

    pps->num_tile_columns = m_pps->num_tile_columns;
    for (uint32_t i = 0; i < pps->num_tile_columns; i++)
    {
        pps->column_width[i] = m_pps->column_width[i];
    }
    pps->num_tile_rows = m_pps->num_tile_rows;
    for (uint32_t i = 0; i < pps->num_tile_rows; i++)
    {
        pps->row_height[i] =  m_pps->row_height[i];
    }

    return 0;
}

int32_t modify_slice_header(HEVCState *hevc, hevc_mergeStream *mergeStream, uint32_t tile_index)
{
    if (!hevc || !mergeStream)
        return -1;

    HEVCSliceInfo *si  = &hevc->s_info;
    HEVC_SPS *sps = &hevc->sps[0];
    HEVC_PPS *pps = &hevc->pps[hevc->last_parsed_pps_id];
    if (!si || !sps || !pps)
        return -1;

    sps->width  = mergeStream->pic_width;
    sps->height = mergeStream->pic_height;

    si->first_slice_segment_in_pic_flag = tile_index==0 ? true : false;
    si->slice_segment_address = mergeStream->slice_segment_address[tile_index];

    return 0;
}

int32_t merge_header(GTS_BitStream *bs, oneStream_info* pSlice, uint8_t **pBitstream, bool isHR, hevc_mergeStream *mergeStream, HEVCState *orgHevc)
{
    if (!bs || !pSlice || !pBitstream || !mergeStream)
        return -1;
    uint32_t nalsize[NALU_NUM];
    uint32_t specialLen = 0;
    int32_t framesize = 0;

    uint8_t *pBufferSliceCur = pSlice->pTiledBitstreamBuffer;
    int32_t lenSlice = pSlice->inputBufferLen;

    uint8_t *pBitstreamCur = *pBitstream;

    HEVCState *hevc = pSlice->hevcSlice;

    hevc_specialInfo specialInfo;
    memset_s(&specialInfo, sizeof(hevc_specialInfo), 0);
    specialInfo.ptr = pBufferSliceCur;
    specialInfo.ptr_size = lenSlice;
    memset_s(nalsize, sizeof(nalsize), 0);
    uint64_t bs_position = bs->position;
    int32_t spsCnt;
    parse_hevc_specialinfo(&specialInfo, hevc, nalsize, &specialLen, &spsCnt, 0);

    specialLen += nalsize[SLICE_HEADER];
    framesize = specialLen + nalsize[SLICE_DATA];
    lenSlice -= framesize;

    if (nalsize[SEQ_PARAM_SET])
    {
        if (orgHevc)
        {
            memcpy_s(orgHevc, sizeof(HEVCState), hevc, sizeof(HEVCState));
        }

        // Choose LR header if HR and LR exist
        // or return directly after parsing if this frame doesn't need headers
        if ((isHR && mergeStream->highRes.selectedTilesCount && mergeStream->lowRes.selectedTilesCount)
            || (!mergeStream->bWroteHeader))
            return 0;

        modify_parameter_sets(hevc, mergeStream);
        hevc_write_parameter_sets(bs, hevc);
    }
    else
    {
        modify_slice_header(hevc, mergeStream, pSlice->currentTileIdx);
        hevc_write_slice_header(bs, hevc);
    }

    //move to current address
    pBitstreamCur += bs->position - bs_position;
    pSlice->outputBufferLen += (uint32_t)(bs->position - bs_position);
    bs_position = bs->position;

    //copy slice data
    memcpy_s(pBitstreamCur, nalsize[SLICE_DATA], pBufferSliceCur + specialLen, nalsize[SLICE_DATA]);
    pBitstreamCur += nalsize[SLICE_DATA];
    bs->position += nalsize[SLICE_DATA];
    pSlice->outputBufferLen += nalsize[SLICE_DATA];
    pBufferSliceCur += specialLen + nalsize[SLICE_DATA];

    *pBitstream = pBitstreamCur;

    return 0;
}

int32_t copy_tile_params(one_res *dst, one_res_param *src)
{
    if (!dst || !src)
        return -1;
    dst->width = src->width;
    dst->height = src->height;
    dst->totalTilesCount = src->totalTilesCount;
    dst->selectedTilesCount = src->selectedTilesCount;
    dst->tile_height = src->tile_height;
    dst->tile_width = src->tile_width;

    return 0;
}

// Put all LR tiles at the right of HR tiles
int32_t get_merge_solution(hevc_mergeStream *mergeStream)
{
    if (!mergeStream)
        return -1;
    int32_t HR_ntile = mergeStream->highRes.selectedTilesCount;
    int32_t LR_ntile = mergeStream->lowRes.selectedTilesCount;
    int32_t HR_tile_h = mergeStream->highRes.tile_height;
    int32_t HR_tile_w = mergeStream->highRes.tile_width;
    int32_t LR_tile_h = mergeStream->lowRes.tile_height;
    int32_t LR_tile_w = mergeStream->lowRes.tile_width;
    int32_t num_tile_rows = mergeStream->highRes.num_tile_rows;

#define LCU_SIZE 64

    HEVC_PPS *pps = &(mergeStream->pps);

    if(0 != HR_ntile && 0 != LR_ntile)
    {
        int32_t height = 0;
        int32_t HR_hc = 0;
        if(!mergeStream->highRes.bOrdered)
        {
        // Suppose all tiles of one stream have same resolution
            height = lcm(HR_tile_h, LR_tile_h);
            int sqrtH = (int)(ceil((double)sqrt(HR_ntile)));
            //find the max height of the stitched YUV, maybe we can try every possible value later.
            while(sqrtH && HR_ntile%sqrtH){sqrtH--;}

            HR_hc = height / HR_tile_h;
            HR_hc = lcm(HR_hc, sqrtH);
            height = HR_hc * HR_tile_h;
        }
        else
        {
            HR_hc = num_tile_rows;
            height = HR_tile_h * num_tile_rows;
        }

        // Check if the input tile number is legitimate
        if(height == 0 || HR_hc == 0 || height % LR_tile_h || HR_ntile % HR_hc)
        {
            SCVP_LOG(LOG_ERROR, "The input tile number is not legitimate!\n");
            return -1;
        }

        int32_t LR_hc = height / LR_tile_h;
        uint32_t HR_wc = HR_ntile / HR_hc;
        int32_t LR_wc = LR_ntile / LR_hc;

        if (LR_ntile > LR_hc * LR_wc) {
            SCVP_LOG(LOG_INFO, "The low-resolution tiles %d cannot be exactly divided by the tile rows %d!\n", LR_ntile, LR_hc);
            LR_wc++;
        }
        pps->num_tile_columns = HR_wc + LR_wc;
        for (uint32_t i = 0; i < pps->num_tile_columns; i++)
        {
            pps->column_width[i] = i < HR_wc ? HR_tile_w : LR_tile_w;
            pps->column_width[i] /= LCU_SIZE;
        }
        // No tile in rows!
        pps->num_tile_rows = 1;

        // left - HR + right - LR
        int32_t pic_width = HR_wc * HR_tile_w + LR_wc * LR_tile_w;
        int32_t pic_height = HR_hc * HR_tile_h;
        mergeStream->pic_width = pic_width;
        mergeStream->pic_height = pic_height;

        for (int32_t i = 0; i < HR_ntile ; i++)
        {
            int32_t addr = (i % HR_hc) * HR_tile_h / LCU_SIZE * pic_width / LCU_SIZE + (i / HR_hc) * HR_tile_w / LCU_SIZE;
            mergeStream->slice_segment_address[i] = addr;
        }
        for (int32_t i = 0; i < LR_ntile ; i++)
        {
            int32_t addr = (i % LR_hc) * LR_tile_h / LCU_SIZE * pic_width / LCU_SIZE + (i / LR_hc) * LR_tile_w / LCU_SIZE + HR_wc * HR_tile_w / LCU_SIZE;
            mergeStream->slice_segment_address[i + HR_ntile] = addr;
        }
    }
    else if(0 == HR_ntile && 0 != LR_ntile)
    {
        int32_t LR_hc = 0;
        int32_t LR_wc = 0;

        if(!mergeStream->lowRes.bOrdered)
        {
            LR_hc = (int32_t)sqrt(LR_ntile);
            while(LR_hc && HR_ntile%LR_hc){LR_hc--;}
            LR_wc = LR_ntile / LR_hc;
        }
        else
        {
            LR_hc = mergeStream->lowRes.num_tile_rows;
            LR_wc = mergeStream->lowRes.num_tile_columns;
        }

        pps->num_tile_columns = LR_wc;
        for (uint32_t i = 0; i < pps->num_tile_columns; i++)
        {
            pps->column_width[i] = LR_tile_w;
            pps->column_width[i] /= LCU_SIZE;
        }
        // No tile in rows!
        pps->num_tile_rows = 1;

        int32_t pic_width = LR_wc * LR_tile_w;
        int32_t pic_height = LR_hc * LR_tile_h;
        mergeStream->pic_width = pic_width;
        mergeStream->pic_height = pic_height;

        for (int32_t i = 0; i < LR_ntile ; i++)
        {
            int32_t addr = (i % LR_hc) * LR_tile_h / LCU_SIZE * pic_width / LCU_SIZE + (i / LR_hc) * LR_tile_w / LCU_SIZE;
            mergeStream->slice_segment_address[i] = addr;
        }
    }
    else if(0 != HR_ntile && 0 == LR_ntile)
    {
        int32_t HR_hc = 0;
        int32_t HR_wc = 0;
        if(!mergeStream->highRes.bOrdered)
        {
            HR_hc = (int32_t)sqrt(HR_ntile);
            while(HR_hc && HR_ntile%HR_hc){HR_hc--;}
            HR_wc = HR_ntile / HR_hc;
        }
        else
        {
            HR_hc = mergeStream->highRes.num_tile_rows;
            HR_wc = mergeStream->highRes.num_tile_columns;
        }

        pps->num_tile_columns = HR_wc;
        for (uint32_t i = 0; i < pps->num_tile_columns; i++)
        {
            pps->column_width[i] = HR_tile_w;
            pps->column_width[i] /= LCU_SIZE;
        }
        // No tile in rows!
        pps->num_tile_rows = 1;

        int32_t pic_width = HR_wc * HR_tile_w;
        int32_t pic_height = HR_hc * HR_tile_h;
        mergeStream->pic_width = pic_width;
        mergeStream->pic_height = pic_height;

        for (int32_t i = 0; i < HR_ntile ; i++)
        {
            int32_t addr = (i % HR_hc) * HR_tile_h / LCU_SIZE * pic_width / LCU_SIZE + (i / HR_hc) * HR_tile_w / LCU_SIZE;
            mergeStream->slice_segment_address[i] = addr;
        }
    }
    else
    {
        return -1;
    }

    return 0;
}

void* tile_merge_Init(param_mergeStream *mergeStreamParams)
{
    if(!mergeStreamParams)
        return NULL;

    hevc_mergeStream *mergeStream = (hevc_mergeStream *)malloc(sizeof(hevc_mergeStream));
    if(!mergeStream)
        return NULL;
    memset_s(mergeStream, sizeof(hevc_mergeStream), 0);

    int32_t HR_ntile = mergeStreamParams->highRes.selectedTilesCount;
    int32_t LR_ntile = mergeStreamParams->lowRes.selectedTilesCount;
    copy_tile_params(&(mergeStream->highRes), &(mergeStreamParams->highRes));
    copy_tile_params(&(mergeStream->lowRes), &(mergeStreamParams->lowRes));

    int32_t totalTileNum = HR_ntile + LR_ntile;
    mergeStream->slice_segment_address = (int32_t *)malloc(totalTileNum * sizeof(int32_t));
    memset_s(mergeStream->slice_segment_address, totalTileNum * sizeof(int32_t), 0);

    mergeStream->inputBistreamsLen = 0;

    init_one_bitstream(&mergeStream->highRes.pHeader);
    init_one_bitstream(&mergeStream->lowRes.pHeader);
    mergeStream->highRes.pTiledBitstreams = (oneStream_info**)malloc(HR_ntile * sizeof(oneStream_info *));
    for(int32_t i = 0 ; i < HR_ntile ; i++)
    {
        init_one_bitstream(&mergeStream->highRes.pTiledBitstreams[i]);
    }
    mergeStream->lowRes.pTiledBitstreams = (oneStream_info**)malloc(LR_ntile * sizeof(oneStream_info *));
    for(int32_t i = 0 ; i < LR_ntile ; i++)
    {
        init_one_bitstream(&mergeStream->lowRes.pTiledBitstreams[i]);
    }
    return mergeStream;
}

int32_t tile_merge_Close(void* handle)
{
    hevc_mergeStream *mergeStream = (hevc_mergeStream *)handle;
    if (!mergeStream)
        return -1;
    int32_t HR_ntile = mergeStream->highRes.selectedTilesCount;
    int32_t LR_ntile = mergeStream->lowRes.selectedTilesCount;

    destory_one_bitstream(&mergeStream->highRes.pHeader);
    destory_one_bitstream(&mergeStream->lowRes.pHeader);

    for(int32_t i = 0 ; i < HR_ntile ; i++)
    {
        destory_one_bitstream(&mergeStream->highRes.pTiledBitstreams[i]);
    }
    for(int32_t i = 0 ; i < LR_ntile ; i++)
    {
        destory_one_bitstream(&mergeStream->lowRes.pTiledBitstreams[i]);
    }

    if(mergeStream->slice_segment_address)
    {
        free(mergeStream->slice_segment_address);
        mergeStream->slice_segment_address = NULL;
    }

    if(mergeStream->highRes.pTiledBitstreams)
    {
        free(mergeStream->highRes.pTiledBitstreams);
        mergeStream->highRes.pTiledBitstreams = NULL;
    }
    if(mergeStream->lowRes.pTiledBitstreams)
    {
        free(mergeStream->lowRes.pTiledBitstreams);
        mergeStream->lowRes.pTiledBitstreams = NULL;
    }
    if(mergeStream)
    {
        free(mergeStream);
        mergeStream = NULL;
    }

    return 0;
}

int32_t tile_merge_Process(param_mergeStream *mergeStreamParams, void* handle)
{
    hevc_mergeStream *mergeStream = (hevc_mergeStream *)handle;
    if (!mergeStream || !mergeStreamParams)
        return -1;
    int32_t HR_ntile = mergeStream->highRes.selectedTilesCount;
    int32_t LR_ntile = mergeStream->lowRes.selectedTilesCount;

    if( 0 == HR_ntile && 0 == LR_ntile)
        return 0;

    mergeStream->bWroteHeader = mergeStreamParams->bWroteHeader;

    mergeStream->highRes.pHeader->pTiledBitstreamBuffer =  mergeStreamParams->highRes.pHeader->pTiledBitstreamBuffer;
    mergeStream->highRes.pHeader->inputBufferLen =  mergeStreamParams->highRes.pHeader->inputBufferLen;
    mergeStream->lowRes.pHeader->pTiledBitstreamBuffer =  mergeStreamParams->lowRes.pHeader->pTiledBitstreamBuffer;
    mergeStream->lowRes.pHeader->inputBufferLen = mergeStreamParams->lowRes.pHeader->inputBufferLen;
    mergeStream->highRes.bOrdered = mergeStreamParams->highRes.bOrdered;
    mergeStream->lowRes.bOrdered = mergeStreamParams->lowRes.bOrdered;

    // Calculate input length
    mergeStream->inputBistreamsLen += mergeStream->highRes.pHeader->inputBufferLen;
    mergeStream->inputBistreamsLen += mergeStream->lowRes.pHeader->inputBufferLen;
    int32_t num_tile_columns = mergeStreamParams->highRes.num_tile_columns;
    int32_t num_tile_rows = mergeStreamParams->highRes.num_tile_rows;
    if(mergeStream->highRes.bOrdered && (!num_tile_columns || !num_tile_rows))
        return -1;
    if(mergeStream->highRes.bOrdered)
    {
        mergeStream->highRes.num_tile_columns = num_tile_columns;
        mergeStream->highRes.num_tile_rows = num_tile_rows;
    }

    for(int32_t i = 0 ; i < HR_ntile ; i++)
    {
        int32_t x = i % num_tile_rows;
        int32_t y = i / num_tile_rows;
        int32_t inverseIdx = mergeStream->highRes.bOrdered ? (x * num_tile_columns + y) : i;
        //printf("inverseIdx = %d, i = %d\n", inverseIdx, i);
        mergeStream->highRes.pTiledBitstreams[i]->pTiledBitstreamBuffer = mergeStreamParams->highRes.pTiledBitstreams[inverseIdx]->pTiledBitstreamBuffer;
        mergeStream->highRes.pTiledBitstreams[i]->inputBufferLen = mergeStreamParams->highRes.pTiledBitstreams[inverseIdx]->inputBufferLen;
        mergeStream->inputBistreamsLen += mergeStreamParams->highRes.pTiledBitstreams[inverseIdx]->inputBufferLen;
        mergeStream->highRes.pTiledBitstreams[i]->currentTileIdx = i;
    }
    for(int32_t i = 0 ; i < LR_ntile ; i++)
    {
        mergeStream->lowRes.pTiledBitstreams[i]->pTiledBitstreamBuffer = mergeStreamParams->lowRes.pTiledBitstreams[i]->pTiledBitstreamBuffer;
        mergeStream->lowRes.pTiledBitstreams[i]->inputBufferLen = mergeStreamParams->lowRes.pTiledBitstreams[i]->inputBufferLen;
        mergeStream->inputBistreamsLen += mergeStreamParams->lowRes.pTiledBitstreams[i]->inputBufferLen;
        mergeStream->lowRes.pTiledBitstreams[i]->currentTileIdx = HR_ntile + i;
    }

    mergeStream->pOutputBitstream = mergeStreamParams->pOutputBitstream;

    // Get tiles merge solution
    int32_t err = get_merge_solution(mergeStream);
    if(err)
        return err;

    // Merge
    GTS_BitStream *bs = gts_bs_new((const int8_t *)mergeStream->pOutputBitstream, 2*mergeStream->inputBistreamsLen, GTS_BITSTREAM_WRITE);
    uint8_t *pOutBitstream = mergeStream->pOutputBitstream;

    HEVCState *HRhevcSlice = (HEVCState*)malloc(sizeof(HEVCState));
    HEVCState *LRhevcSlice = (HEVCState*)malloc(sizeof(HEVCState));
    HEVCState *tmpHevcSlice = NULL;

    if(HR_ntile)
    {
        merge_header(bs, mergeStream->highRes.pHeader, &pOutBitstream, 1, mergeStream, HRhevcSlice);
    }
    if(LR_ntile)
    {
        merge_header(bs, mergeStream->lowRes.pHeader, &pOutBitstream, 0, mergeStream, LRhevcSlice);
    }
    // Just merge one frame
    for(int32_t i = 0 ; i < HR_ntile; i++)
    {
        tmpHevcSlice = mergeStream->highRes.pTiledBitstreams[i]->hevcSlice;
        mergeStream->highRes.pTiledBitstreams[i]->hevcSlice = HRhevcSlice;
        merge_header(bs, mergeStream->highRes.pTiledBitstreams[i], &pOutBitstream, 1, mergeStream,NULL);
        mergeStream->highRes.pTiledBitstreams[i]->hevcSlice = tmpHevcSlice;
    }
    for(int32_t i = 0 ; i < LR_ntile; i++)
    {
        tmpHevcSlice = mergeStream->lowRes.pTiledBitstreams[i]->hevcSlice;
        mergeStream->lowRes.pTiledBitstreams[i]->hevcSlice = LRhevcSlice;
        merge_header(bs, mergeStream->lowRes.pTiledBitstreams[i], &pOutBitstream, 0, mergeStream,NULL);
        mergeStream->lowRes.pTiledBitstreams[i]->hevcSlice = tmpHevcSlice;
    }

    // Calculate output length
    int32_t outputBufferLen = 0;
    if(LR_ntile)
    {
        outputBufferLen += mergeStream->lowRes.pHeader->outputBufferLen;
    }
    else
    {
        outputBufferLen += mergeStream->highRes.pHeader->outputBufferLen;
    }
    for(int32_t i = 0 ; i < HR_ntile ; i++)
    {
        outputBufferLen += mergeStream->highRes.pTiledBitstreams[i]->outputBufferLen;
    }
    for(int32_t i = 0 ; i < LR_ntile ; i++)
    {
        outputBufferLen += mergeStream->lowRes.pTiledBitstreams[i]->outputBufferLen;
    }
    mergeStream->outputiledbistreamlen = outputBufferLen;
    mergeStreamParams->outputiledbistreamlen = mergeStream->outputiledbistreamlen;

    if (bs) gts_bs_del(bs);

    if(HRhevcSlice)
    {
        free(HRhevcSlice);
        HRhevcSlice = NULL;
    }
    if(LRhevcSlice)
    {
        free(LRhevcSlice);
        LRhevcSlice = NULL;
    }
    return 0;
}

int32_t tile_merge_reset(void* handle)
{
    hevc_mergeStream *mergeStream = (hevc_mergeStream *)handle;
    if (!mergeStream)
        return -1;
    int32_t HR_ntile = mergeStream->highRes.selectedTilesCount;
    int32_t LR_ntile = mergeStream->lowRes.selectedTilesCount;

    if (0 == HR_ntile && 0 == LR_ntile)
        return -1;

    mergeStream->bWroteHeader = 0;

    mergeStream->inputBistreamsLen = 0;
    mergeStream->highRes.pHeader->outputBufferLen = 0;
    mergeStream->lowRes.pHeader->outputBufferLen = 0;

    for (int32_t i = 0; i < HR_ntile; i++)
    {
        mergeStream->highRes.pTiledBitstreams[i]->outputBufferLen = 0;
    }
    for (int32_t i = 0; i < LR_ntile; i++)
    {
        mergeStream->lowRes.pTiledBitstreams[i]->outputBufferLen = 0;
    }
    return 0;
}
