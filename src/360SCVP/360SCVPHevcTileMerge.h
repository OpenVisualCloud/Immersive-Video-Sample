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
#ifndef _360SCVP_HEVC_TILEMERGE_H_
#define _360SCVP_HEVC_TILEMERGE_H_

#include "360SCVPHevcTilestream.h"

typedef struct ONE_RES
{
    int32_t                tile_height;
    int32_t                tile_width;
    uint32_t               width;
    uint32_t               height;
    int32_t                totalTilesCount;
    int32_t                selectedTilesCount;
    oneStream_info       **pTiledBitstreams;
    oneStream_info        *pHeader;
    int32_t                num_tile_columns;
    int32_t                num_tile_rows;
    bool                   bOrdered;
}one_res;

typedef struct HEVC_MERGEBITSTREAM
{
    one_res        highRes;
    one_res        lowRes;
    uint32_t       inputBistreamsLen;
    uint8_t       *pOutputBitstream;
    uint32_t       outputiledbistreamlen;
    HEVC_PPS       pps;
    int32_t        pic_width;
    int32_t        pic_height;
    int32_t       *slice_segment_address;
    bool           bWroteHeader;
}hevc_mergeStream;

//modify resolution and tile segmentation
int32_t modify_parameter_sets(HEVCState *hevc, hevc_mergeStream *mergeStream);
int32_t init_one_bitstream(oneStream_info **pBs);
int32_t destory_one_bitstream(oneStream_info **pBs);
int32_t modify_slice_header(HEVCState *hevc, hevc_mergeStream *mergeStream, uint32_t tile_index);
int32_t merge_header(GTS_BitStream *bs, oneStream_info* pSlice, uint8_t **pBitstream, bool isHR, hevc_mergeStream *mergeStream, HEVCState *orgHevc);
// Put all LR tiles at the right of HR tiles
int32_t get_merge_solution(hevc_mergeStream *mergeStream);

#endif //_360SCVP_HEVC_TILEMERGE_H_
