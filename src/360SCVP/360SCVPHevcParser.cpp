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
#include "assert.h"
#include "360SCVPHevcParser.h"
#include "360SCVPHevcTilestream.h"

uint32_t gts_media_nalu_emulation_bytes_remove_count(const int8_t *buffer, uint32_t size_nal)
{
    uint32_t n = 0, emulation_bytes_count = 0;
    uint8_t zero_counter = 0;

    while (n < size_nal)
    {
        if (zero_counter == 2 && buffer[n] == 0x03 && n + 1 < size_nal && buffer[n + 1] < 0x04)
        {
            zero_counter = 0;
            emulation_bytes_count++;
            n++;
        }
        if (!buffer[n])
            zero_counter++;
        else
            zero_counter = 0;
        n++;
    }

    return emulation_bytes_count;
}

uint32_t gts_media_nalu_remove_emulation_bytes(const int8_t *src_buffer, int8_t *dst_buffer, uint32_t size_nal)
{
    uint32_t n = 0, emulation_bytes_count = 0;
    uint8_t zero_counter = 0;
    while (n < size_nal)
    {
        if (zero_counter == 2 && src_buffer[n] == 0x03 && n + 1 < size_nal && src_buffer[n + 1] < 0x04)
        {
            zero_counter = 0;
            emulation_bytes_count++;
            n++;
        }
        dst_buffer[n - emulation_bytes_count] = src_buffer[n];
        if (!src_buffer[n])
            zero_counter++;
        else
            zero_counter = 0;

        n++;
    }

    return size_nal - emulation_bytes_count;
}


static uint8_t digits_of_agm[128] = {
    8, 7, 6, 6, 5, 5, 5, 5,  4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3,  3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2,  2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2,  2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1
};

static uint32_t bs_get_ue(GTS_BitStream *gts_bitstream)
{
    uint8_t flag_c;
    uint32_t data = 0, flag_r = 0;
    while (1) {
        flag_r = gts_bs_peek_bits(gts_bitstream, 8, 0);
        if (flag_r) break;
        //check whether we still have data once the peek is done since we may have less than 8 data available
        if (!gts_bs_available(gts_bitstream)) {
            //GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("[AVC/HEVC] Not enough data in bitstream !!\n"));
            return 0;
        }
        gts_bs_read_int(gts_bitstream, 8);
        data += 8;
    }
    if (flag_r < 128)
        flag_c = digits_of_agm[flag_r];
    else
        flag_c = 0;
    gts_bs_read_int(gts_bitstream, flag_c);
    data += flag_c;
    return gts_bs_read_int(gts_bitstream, data + 1) - 1;
}

static int32_t bs_get_se(GTS_BitStream *bs)
{
    uint32_t v = bs_get_ue(bs);
    if ((v & 0x1) == 0) return (int32_t)(0 - (v >> 1));
    return (v + 1) >> 1;
}

uint32_t gts_media_nalu_is_start_code(GTS_BitStream *bs)
{
    uint8_t s1, s2, s3, s4;
    bool is_sc = (bool)0;
    uint64_t pos = gts_bs_get_position(bs);
    s1 = gts_bs_read_int(bs, 8);
    s2 = gts_bs_read_int(bs, 8);
    if (!s1 && !s2) {
        s3 = gts_bs_read_int(bs, 8);
        if (s3 == 0x01) is_sc = (bool)3;
        else if (!s3) {
            s4 = gts_bs_read_int(bs, 8);
            if (s4 == 0x01) is_sc = (bool)4;
        }
    }
    gts_bs_seek(bs, pos + is_sc);
    return is_sc;
}

/**********
HEVC parsing
**********/

bool gts_media_hevc_slice_is_intra(HEVCState *hevc)
{
    switch (hevc->s_info.nal_unit_type) {
    case GTS_HEVC_NALU_SLICE_BLA_W_LP:
    case GTS_HEVC_NALU_SLICE_BLA_W_DLP:
    case GTS_HEVC_NALU_SLICE_BLA_N_LP:
    case GTS_HEVC_NALU_SLICE_IDR_W_DLP:
    case GTS_HEVC_NALU_SLICE_IDR_N_LP:
    case GTS_HEVC_NALU_SLICE_CRA:
        return true;
    default:
        return false;
    }
}

bool gts_media_hevc_slice_is_IDR(HEVCState *hevc)
{
    if (hevc->sei.recovery_point.valid)
    {
        hevc->sei.recovery_point.valid = 0;
        return true;
    }
    switch (hevc->s_info.nal_unit_type) {
    case GTS_HEVC_NALU_SLICE_IDR_W_DLP:
    case GTS_HEVC_NALU_SLICE_IDR_N_LP:
        return true;
    default:
        return false;
    }
}

static bool parse_short_term_ref_pic_set(GTS_BitStream *bs, HEVC_SPS *sps, HEVCSliceInfo *si, uint32_t idx_rps)
{
    uint32_t i;
    si->rps[idx_rps].inter_ref_pic_set_prediction_flag = (bool)0;
    if (idx_rps != 0)
        si->rps[idx_rps].inter_ref_pic_set_prediction_flag = (bool)gts_bs_read_int(bs, 1);

    if (si->rps[idx_rps].inter_ref_pic_set_prediction_flag ) {
        HEVC_ReferencePictureSets *hevc_pic_set, *ref_pic_set;
        si->rps[idx_rps].index_num = 0;
        uint32_t index_reference;
        //uint32_t delta_rps_sign;
        uint32_t reference_set_nb;
        int32_t reference_pic_del;
        uint32_t i = 0, i0 = 0, i1 = 0;
        if (idx_rps == sps->num_short_term_ref_pic_sets)
            si->rps[idx_rps].index_num = bs_get_ue(bs);

        assert(si->rps[idx_rps].index_num <= idx_rps - 1);
        index_reference = idx_rps - 1 - si->rps[idx_rps].index_num;
        si->rps[idx_rps].delta_rps_sign = (bool)gts_bs_read_int(bs, 1);
        si->rps[idx_rps].index_num_absolute = bs_get_ue(bs);
        reference_pic_del = (1 - (si->rps[idx_rps].delta_rps_sign<<1)) * (si->rps[idx_rps].index_num_absolute + 1);

        ref_pic_set = &si->rps[idx_rps];
        hevc_pic_set = &si->rps[index_reference];
        reference_set_nb = hevc_pic_set->num_negative_pics + hevc_pic_set->num_positive_pics;
        for (i=0; i<=reference_set_nb; i++) {
            int32_t ref_idc;
            ref_pic_set->used_by_curr_pic_flag[i] = (bool)gts_bs_read_int(bs, 1);
            ref_idc = ref_pic_set->used_by_curr_pic_flag[i] ? 1 : 0;
            if ( !ref_pic_set->used_by_curr_pic_flag[i] ) {
                ref_pic_set->used_delta_flag[i] = (bool)gts_bs_read_int(bs, 1);
                ref_idc = ref_pic_set->used_delta_flag[i] << 1;
            }
            if ((ref_idc==1) || (ref_idc== 2)) {
                int32_t deltaPOC = reference_pic_del;
                if (i < reference_set_nb)
                    deltaPOC += hevc_pic_set->delta_poc[i];

                ref_pic_set->delta_poc[i] = deltaPOC;

                if (deltaPOC < 0)  i0++;
                else i1++;

                i++;
            }
        }
        ref_pic_set->num_negative_pics = i0;
        ref_pic_set->num_positive_pics = i1;
    } else {
        //int32_t prev = 0, poc = 0;
        si->rps[idx_rps].num_negative_pics = bs_get_ue(bs);
        si->rps[idx_rps].num_positive_pics = bs_get_ue(bs);
        if (si->rps[idx_rps].num_negative_pics>16)
            return false;
        if (si->rps[idx_rps].num_positive_pics>16)
            return false;
        for (i=0; i<si->rps[idx_rps].num_negative_pics; i++) {
            //uint32_t delta_poc_s0_minus1 = bs_get_ue(bs);
            //poc = prev - delta_poc_s0_minus1 - 1;
            //prev = poc;
            si->rps[idx_rps].delta_poc0[i] = bs_get_ue(bs) + 1;
            si->rps[idx_rps].used_by_curr_pic_s0_flag[i] = (bool)gts_bs_read_int(bs, 1);
        }
        for (i=0; i<si->rps[idx_rps].num_positive_pics; i++) {
            //uint32_t deltaPocS1 = 0;
            si->rps[idx_rps].delta_poc1[i] = bs_get_ue(bs) + 1;
            //poc = prev + deltaPocS1;
            //prev = poc;
            //si->rps[idx_rps].delta_poc[i] = poc;
            si->rps[idx_rps].used_by_curr_pic_s1_flag[i] = (bool)gts_bs_read_int(bs, 1);
        }
    }
    return true;
}

void hevc_PredWeightTable(GTS_BitStream *bs, HEVCState *hevc, HEVCSliceInfo *si, HEVC_PPS *pps, HEVC_SPS *sps, uint32_t num_ref_idx_l0_active, uint32_t num_ref_idx_l1_active)
{
    uint32_t i = 0;
    bool first_pass=true;
    uint8_t luma_weights[20], chroma_weights[20];
    uint32_t ChromaArrayType = 0;
    uint32_t num_ref_idx = num_ref_idx_l0_active;
    if (!sps->separate_colour_plane_flag)
        ChromaArrayType = sps->chroma_format_idc;
    i=bs_get_ue(bs);
    if (ChromaArrayType != 0)
        i=bs_get_se(bs);

parse_weights:
    for (i=0; i<num_ref_idx; i++) {
        luma_weights[i] = (bool)gts_bs_read_int(bs, 1);
        chroma_weights[i] = 0; //infered to be 0 if not present
    }
    if (ChromaArrayType != 0) {
        for (i=0; i<num_ref_idx; i++) {
            chroma_weights[i] = (bool)gts_bs_read_int(bs, 1);
        }
    }
    for (i=0; i<num_ref_idx; i++) {
        if (luma_weights[i]) {
            bs_get_se(bs);
            bs_get_se(bs);
        }
        if (chroma_weights[i]) {
            bs_get_se(bs);
            bs_get_se(bs);

            bs_get_se(bs);
            bs_get_se(bs);
        }
    }
    if (si->slice_type == GF_HEVC_SLICE_TYPE_B) {
        if (!first_pass) return;
        first_pass=false;
        num_ref_idx = num_ref_idx_l1_active;
        goto parse_weights;
    }
}

static
bool ref_pic_lists_modification(GTS_BitStream *bs, uint32_t slice_type, uint32_t num_ref_idx_l0_active, uint32_t num_ref_idx_l1_active)
{
    //uint32_t i;
    bool ref_pic_list_modification_flag_l0 = (bool)gts_bs_read_int(bs, 1);
    if (ref_pic_list_modification_flag_l0) {
        /*for (i=0; i<num_ref_idx_l0_active; i++) {
            list_entry_l0[i] = *//*gts_bs_read_int(bs, (uint32_t)ceil(log(getNumPicTotalCurr())/log(2)));
        }*/
        return false;
    }
    if (slice_type == GF_HEVC_SLICE_TYPE_B) {
        bool ref_pic_list_modification_flag_l1 = (bool)gts_bs_read_int(bs, 1);
        if (ref_pic_list_modification_flag_l1) {
            /*for (i=0; i<num_ref_idx_l1_active; i++) {
                list_entry_l1[i] = *//*gts_bs_read_int(bs, (uint32_t)ceil(log(getNumPicTotalCurr()) / log(2)));
            }*/
            return false;
        }
    }

    return true;
}

static int32_t hevc_parse_slice_segment(GTS_BitStream *gts_bitstream, HEVCState *state, HEVCSliceInfo *slice_info)
{
    uint32_t m;
    //uint32_t count_index_reference_0=0, count_index_reference_1=0;
    HEVC_PPS *hevc_pps;
    HEVC_SPS *hevc_sps;
    //int32_t index_hevc_pps;
    bool RapPicFlag = false;
    bool IDRPicFlag = false;

    slice_info->first_slice_segment_in_pic_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
    //printf("Get first slice %d for slice %p \n", slice_info->first_slice_segment_in_pic_flag, state);

    switch (slice_info->nal_unit_type) {
    case GTS_HEVC_NALU_SLICE_IDR_W_DLP:
    case GTS_HEVC_NALU_SLICE_IDR_N_LP:
        IDRPicFlag = true;
        RapPicFlag = true;
        break;
    case GTS_HEVC_NALU_SLICE_BLA_W_LP:
    case GTS_HEVC_NALU_SLICE_BLA_W_DLP:
    case GTS_HEVC_NALU_SLICE_BLA_N_LP:
    case GTS_HEVC_NALU_SLICE_CRA:
        RapPicFlag = true;
        break;
    }

    if (RapPicFlag) {
        slice_info->no_output_of_prior_pics_flag = gts_bs_read_int(gts_bitstream, 1);
    }

    slice_info->index_hevc_pps = bs_get_ue(gts_bitstream);
    if (slice_info->index_hevc_pps>=64)
        return -1;

    hevc_pps = &state->pps[slice_info->index_hevc_pps];
    hevc_sps = &state->sps[hevc_pps->sps_id];
    slice_info->sps = hevc_sps;
    slice_info->pps = hevc_pps;

    if (!slice_info->first_slice_segment_in_pic_flag && hevc_pps->dependent_slice_segments_enabled_flag) {
        slice_info->dependent_slice_segment_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
    } else {
        slice_info->dependent_slice_segment_flag = false;
    }

    if (!slice_info->first_slice_segment_in_pic_flag) {
        slice_info->slice_segment_address = gts_bs_read_int(gts_bitstream, hevc_sps->bitsSliceSegmentAddress);
    } else {
        slice_info->slice_segment_address = 0;
    }

    if( !slice_info->dependent_slice_segment_flag ) {
        slice_info->info_of_dfof = (bool)0;
        //bool slice_sao_luma_flag= (bool)0;
        //bool slice_sao_chroma_flag= (bool)0;
        slice_info->info_of_sdfdf= (bool)0;

        //"slice_reserved_undetermined_flag[]"
        //gts_bs_read_int(gts_bitstream, hevc_pps->num_extra_slice_header_bits);
        for (uint32_t k = 0; k < hevc_pps->num_extra_slice_header_bits; k++)
        {
            slice_info->slice_reserved_flag[k] = gts_bs_read_int(gts_bitstream, 1);
        }

        slice_info->slice_type = bs_get_ue(gts_bitstream);
        /*
        if (slice_info->slice_type == GF_HEVC_SLICE_TYPE_P)
            slice_info->slice_type = GF_HEVC_SLICE_TYPE_P;
        */

        if(hevc_pps->output_flag_present_flag)
            slice_info->pic_output_flag = (bool)(gts_bs_read_int(gts_bitstream, 1));

        if (hevc_sps->separate_colour_plane_flag == 1)
            slice_info->colour_plane_id = gts_bs_read_int(gts_bitstream, 2);

        if (IDRPicFlag) {
            slice_info->poc_lsb = 0;

            //if not asked to parse full header, abort since we know the poc
            //if (!state->full_slice_header_parse) return 0;

        } else {
            slice_info->poc_lsb = gts_bs_read_int(gts_bitstream, hevc_sps->log2_max_pic_order_cnt_lsb);

            //if not asked to parse full header, abort once we have the poc
            //if (!state->full_slice_header_parse) return 0;

            slice_info->short_term_ref_pic_set_sps_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
            if (slice_info->short_term_ref_pic_set_sps_flag == 0) {
                bool ret = parse_short_term_ref_pic_set(gts_bitstream, hevc_sps, slice_info, hevc_sps->num_short_term_ref_pic_sets );
                if (!ret)
                    return -1;
            } else if( hevc_sps->num_short_term_ref_pic_sets > 1 ) {
                uint32_t numbits = 0;

                while ( (uint32_t) (1 << numbits) < hevc_sps->num_short_term_ref_pic_sets)
                    numbits++;
                if (numbits > 0)
                    slice_info->short_term_ref_pic_set_idx = gts_bs_read_int(gts_bitstream, numbits);
                else
                    slice_info->short_term_ref_pic_set_idx = 0;
            }
            if (hevc_sps->long_term_ref_pics_present_flag ) {
                //uint8_t poc_MCL_del[32];
                //uint32_t count_hevc_lts = 0;
                //uint32_t count_hevc_ltp = 0;
                if (hevc_sps->num_long_term_ref_pic_sps > 0 ) {
                    slice_info->count_hevc_lts = bs_get_ue(gts_bitstream);
                }
                slice_info->count_hevc_ltp = bs_get_ue(gts_bitstream);

                for (m = 0; m < slice_info->count_hevc_lts + slice_info->count_hevc_ltp; m++ ) {
                    if( m < slice_info->count_hevc_lts ) {
                        if (hevc_sps->num_long_term_ref_pic_sps > 1)
                            slice_info->lt_idx_sps[m] = gts_bs_read_int(gts_bitstream, gts_get_bit_size(hevc_sps->num_long_term_ref_pic_sps) );
                    } else {
                        slice_info->poc_lsb_lt[m] = gts_bs_read_int(gts_bitstream, hevc_sps->log2_max_pic_order_cnt_lsb);
                        slice_info->used_by_curr_pic_lt_flag[m] = gts_bs_read_int(gts_bitstream, 1);
                    }
                    slice_info->delta_poc_msb_present_flag[m] = gts_bs_read_int(gts_bitstream, 1);
                    if (slice_info->delta_poc_msb_present_flag[m]) {
                        if( m == 0 || m == slice_info->count_hevc_lts )
                            slice_info->poc_MCL_del[m] = bs_get_ue(gts_bitstream);
                        else
                            slice_info->poc_MCL_del[m] = bs_get_ue(gts_bitstream) + slice_info->poc_MCL_del[m-1];
                    }
                }
            }
            if (hevc_sps->temporal_mvp_enable_flag)
                slice_info->slice_temporal_mvp_enabled_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
        }
        if (hevc_sps->sample_adaptive_offset_enabled_flag) {
            uint32_t ChromaArrayType = hevc_sps->separate_colour_plane_flag ? 0 : hevc_sps->chroma_format_idc;
            slice_info->slice_sao_luma_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
            if (ChromaArrayType!=0)
                slice_info->slice_sao_chroma_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
        }

        slice_info->count_index_reference_0 = 0;
        slice_info->count_index_reference_1 = 0;
        if (slice_info->slice_type == GF_HEVC_SLICE_TYPE_P || slice_info->slice_type == GF_HEVC_SLICE_TYPE_B) {
            //uint32_t NumPocTotalCurr;
            slice_info->count_index_reference_0 = hevc_pps->num_ref_idx_l0_default_active;
            slice_info->count_index_reference_1 = 0;
            if (slice_info->slice_type == GF_HEVC_SLICE_TYPE_B)
                slice_info->count_index_reference_1 = hevc_pps->num_ref_idx_l1_default_active;

            slice_info->num_ref_idx_active_override_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
            if (slice_info->num_ref_idx_active_override_flag) {
                slice_info->count_index_reference_0 = 1 + bs_get_ue(gts_bitstream);
                if (slice_info->slice_type == GF_HEVC_SLICE_TYPE_B)
                    slice_info->count_index_reference_1 = 1 + bs_get_ue(gts_bitstream);
            }

            if (hevc_pps->lists_modification_present_flag /*TODO: && NumPicTotalCurr > 1*/) {
                if (!ref_pic_lists_modification(gts_bitstream, slice_info->slice_type, slice_info->count_index_reference_0, slice_info->count_index_reference_1)) {
                    //GF_LOG(GF_LOG_WARNING, GF_LOG_CODING, ("[state] ref_pic_lists_modification( ) not implemented\n"));
                    return -1;
                }
            }

            if (slice_info->slice_type == GF_HEVC_SLICE_TYPE_B)
                slice_info->mvd_l1_zero_flag = gts_bs_read_int(gts_bitstream, 1);
            //printf("slice_info->mvd_l1_zero_flag %d \n", slice_info->mvd_l1_zero_flag);
            if (hevc_pps->cabac_init_present_flag)
                slice_info->cabac_init_flag = gts_bs_read_int(gts_bitstream, 1);

            if (slice_info->slice_temporal_mvp_enabled_flag) {
                // When info_of_cl0 is not present, it is inferred to be equal to 1.
                slice_info->info_of_cl0 = (bool)1;
                if (slice_info->slice_type == GF_HEVC_SLICE_TYPE_B)
                    slice_info->info_of_cl0 = (bool)gts_bs_read_int(gts_bitstream, 1);

                if ( (slice_info->info_of_cl0 && (slice_info->count_index_reference_0 > 1) )
                        || ( !slice_info->info_of_cl0 && (slice_info->count_index_reference_1 > 1) )
                   ) {
                    slice_info->collocated_ref_idx = bs_get_ue(gts_bitstream);
                }
            }

            if ( (hevc_pps->weighted_pred_flag && slice_info->slice_type == GF_HEVC_SLICE_TYPE_P )
                    || ( hevc_pps->weighted_bipred_flag && slice_info->slice_type == GF_HEVC_SLICE_TYPE_B)
               ) {
				hevc_PredWeightTable(gts_bitstream, state, slice_info, hevc_pps, hevc_sps, slice_info->count_index_reference_0, slice_info->count_index_reference_1);
            }
            slice_info->five_minus_max_num_merge_cand = bs_get_ue(gts_bitstream);
        }
        slice_info->slice_qp_delta = bs_get_se(gts_bitstream);
        if( hevc_pps->slice_chroma_qp_offsets_present_flag ) {
            slice_info->slice_cb_qp_offset= bs_get_se(gts_bitstream);
            slice_info->slice_cr_qp_offset= bs_get_se(gts_bitstream);
        }
        if ( hevc_pps->deblocking_filter_override_enabled_flag ) {
            slice_info->info_of_dfof = (bool)gts_bs_read_int(gts_bitstream, 1);
        }

        if (slice_info->info_of_dfof) {
            slice_info->info_of_sdfdf = (bool)gts_bs_read_int(gts_bitstream, 1);
            if ( !slice_info->info_of_sdfdf) {
                slice_info->slice_beta_offset_div2 = bs_get_se(gts_bitstream);
                slice_info->slice_tc_offset_div2 = bs_get_se(gts_bitstream);
            }
        }
        if( hevc_pps->loop_filter_across_slices_enabled_flag
                && ( slice_info->slice_sao_luma_flag || slice_info->slice_sao_chroma_flag || !slice_info->info_of_sdfdf )
          ) {
            slice_info->slice_loop_filter_across_slices_enabled_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
        }
    }
    //dependent slice segment
    else {
        //if not asked to parse full header, abort
        //if (!state->full_slice_header_parse) return 0;
    }

    slice_info->entry_point_start_bits = ((uint32_t)gts_bs_get_position(gts_bitstream)-1)*8 + gts_bs_get_bit_position(gts_bitstream);

    if (hevc_pps->tiles_enabled_flag || hevc_pps->entropy_coding_sync_enabled_flag ) {
        slice_info->num_entry_point_offsets = bs_get_ue(gts_bitstream);
        if ( slice_info->num_entry_point_offsets > 0) {
            slice_info->offset_len = bs_get_ue(gts_bitstream) + 1;
            //uint32_t segments = slice_info->offset_len >> 4;
            //int32_t remain = (slice_info->offset_len & 15);

            //for (m = 0; m < slice_info->num_entry_point_offsets; m++) {
                //slice_info->entry_point_offset_minus1[m] = gts_bs_read_int(gts_bitstream, slice_info->offset_len);
                //uint32_t res = 0;
                //for (n=0; n<segments; n++) {
                //    res <<= 16;
                //    res += gts_bs_read_int(gts_bitstream, 16);
                //}
                //if (remain) {
                //    res <<= remain;
                //    res += gts_bs_read_int(gts_bitstream, remain);
                //}
                // entry_point_offset = val + 1; // +1; // +1 to get the size
            //}
        }
    }

    if (hevc_pps->slice_segment_header_extension_present_flag) {
        slice_info->size_ext = bs_get_ue(gts_bitstream);
        while (slice_info->size_ext) {
            uint8_t byte = gts_bs_read_int(gts_bitstream, 8);
            slice_info->ext_bytes.push_back(byte);
            slice_info->size_ext--;
        }
    }

    if (gts_bs_read_int(gts_bitstream, 1) == 0) {
        ;// GF_LOG(GF_LOG_WARNING, GF_LOG_CODING, ("Error parsing slice header: byte_align not found at end of header !\n"));
    }

    gts_bs_align(gts_bitstream);
    slice_info->payload_start_offset = (int32_t)gts_bs_get_position(gts_bitstream);
    return 0;
}

static void hevc_compute_poc(HEVCSliceInfo *slice_info)
{
    uint32_t hevc_pic_order_cnt_top = 1 << (slice_info->sps->log2_max_pic_order_cnt_lsb);

    /*POC reset for IDR frames, NOT for CRA*/
    switch (slice_info->nal_unit_type) {
    case GTS_HEVC_NALU_SLICE_IDR_W_DLP:
    case GTS_HEVC_NALU_SLICE_IDR_N_LP:
        slice_info->poc_lsb_prev = 0;
        slice_info->poc_msb_prev = 0;
        break;
    }

    if ((slice_info->poc_lsb < slice_info->poc_lsb_prev) && (    slice_info->poc_lsb_prev - slice_info->poc_lsb >= hevc_pic_order_cnt_top / 2) )
        slice_info->poc_msb = slice_info->poc_msb_prev + hevc_pic_order_cnt_top;
    else if ((slice_info->poc_lsb > slice_info->poc_lsb_prev) && (slice_info->poc_lsb - slice_info->poc_lsb_prev > hevc_pic_order_cnt_top / 2))
        slice_info->poc_msb = slice_info->poc_msb_prev - hevc_pic_order_cnt_top;
    else
        slice_info->poc_msb = slice_info->poc_msb_prev;

    switch (slice_info->nal_unit_type) {
    case GTS_HEVC_NALU_SLICE_BLA_W_LP:
    case GTS_HEVC_NALU_SLICE_BLA_W_DLP:
    case GTS_HEVC_NALU_SLICE_BLA_N_LP:
        slice_info->poc_msb  = 0;
        break;
    }
    slice_info->poc = slice_info->poc_msb + slice_info->poc_lsb;
}


/*read that amount of data at each IO access rather than fetching byte by byte...*/
#define AVC_CACHE_SIZE    4096

static uint32_t gts_media_nalu_locate_start_code_bs(GTS_BitStream *bs, bool locate_trailing)
{
    uint32_t v, bpos, nb_cons_zeros = 0;
    int8_t avc_cache[AVC_CACHE_SIZE];
    uint64_t end, cache_start, load_size;
    uint64_t start = gts_bs_get_position(bs);
    if (start<3) return 0;

    load_size = 0;
    bpos = 0;
    cache_start = 0;
    end = 0;
    v = 0xffffffff;
    while (!end) {
        /*refill cache*/
        if (bpos == (uint32_t)load_size) {
            if (!gts_bs_available(bs)) break;
            load_size = gts_bs_available(bs);
            if (load_size>AVC_CACHE_SIZE) load_size = AVC_CACHE_SIZE;
            bpos = 0;
            cache_start = gts_bs_get_position(bs);
            gts_bs_read_data(bs, avc_cache, (uint32_t)load_size);
        }
        v = ((v << 8) & 0xFFFFFF00) | ((uint32_t)avc_cache[bpos]);

        bpos++;
        if (locate_trailing) {
            if ((v & 0x000000FF) == 0) nb_cons_zeros++;
            else nb_cons_zeros = 0;
        }

        if (v == 0x00000001) end = cache_start + bpos - 4;
        else if ((v & 0x00FFFFFF) == 0x00000001) end = cache_start + bpos - 3;
    }
    gts_bs_seek(bs, start);
    if (!end) end = gts_bs_get_size(bs);
    if (locate_trailing) {
        if (nb_cons_zeros >= 3)
            return (uint32_t)(end - start - nb_cons_zeros);
    }
    return (uint32_t)(end - start);
}

uint32_t gts_media_nalu_next_start_code_bs(GTS_BitStream *bs)
{
    return gts_media_nalu_locate_start_code_bs(bs, (bool)0);
}

static bool hevc_parse_nal_header(GTS_BitStream *bs, uint8_t *nal_unit_type, uint8_t *temporal_id, uint8_t *layer_id, uint16_t *payloadType)
{
    uint32_t val;
    val = gts_bs_read_int(bs, 1);
    if (val) return false;

    val = gts_bs_read_int(bs, 6);
    if (nal_unit_type) *nal_unit_type = val;

    val = gts_bs_read_int(bs, 6);
    if (layer_id) *layer_id = val;

    val = gts_bs_read_int(bs, 3);
    if (! val)
        return false;
    val -= 1;
    if (temporal_id) *temporal_id = val;

    // add the SEI information parser if the naltype is GTS_HEVC_NALU_PREFIX_SEI, GTS_HEVC_NALU_SUFFIX_SEI
    if (nal_unit_type)
    {
        if (*nal_unit_type == GTS_HEVC_NALU_PREFIX_SEI || *nal_unit_type == GTS_HEVC_NALU_SUFFIX_SEI)
        {
            val = gts_bs_read_int(bs, 8);
            *payloadType = val;
        }
    }

    return true;
}

void sub_profile_flag(GTS_BitStream *bs, bool ProfilePresentFlag, HEVC_ProfileTierLevel *pPTLHEVC)
{
    if (!ProfilePresentFlag)
    {
        pPTLHEVC->level_idc = gts_bs_read_int(bs, 8);
    }
    else
    {
        pPTLHEVC->profile_space = gts_bs_read_int(bs, 2);
        pPTLHEVC->tier_flag = gts_bs_read_int(bs, 1);
        pPTLHEVC->profile_idc = gts_bs_read_int(bs, 5);
        pPTLHEVC->profile_compatibility_flag = gts_bs_read_int(bs, 32);
        pPTLHEVC->general_progressive_source_flag = (bool)gts_bs_read_int(bs, 1);
        pPTLHEVC->general_interlaced_source_flag = (bool)gts_bs_read_int(bs, 1);
        pPTLHEVC->general_non_packed_constraint_flag = (bool)gts_bs_read_int(bs, 1);
        pPTLHEVC->general_frame_only_constraint_flag = (bool)gts_bs_read_int(bs, 1);
        pPTLHEVC->general_reserved_44bits = gts_bs_read_long_int(bs, 44);
        pPTLHEVC->level_idc = gts_bs_read_int(bs, 8);
    }
}

void HEVC_profiletierevel(GTS_BitStream *bs, bool ProfilePresentFlag, uint8_t MaxNumSubLayersMinus1, HEVC_ProfileTierLevel *pPTLHEVC)
{
    uint32_t i = 0;
    uint8_t MaxNum = MaxNumSubLayersMinus1;
    sub_profile_flag(bs, ProfilePresentFlag, pPTLHEVC);

    for (i=0; i< MaxNum; i++) {
        pPTLHEVC->sub_ptl[i].profile_present_flag = (bool)gts_bs_read_int(bs, 1);
        pPTLHEVC->sub_ptl[i].level_present_flag = (bool)gts_bs_read_int(bs, 1);
    }
    if (MaxNum >0) {
        for (i= MaxNum; i<8; i++) {
            gts_bs_read_int(bs, 2);
        }
    }

    for (i=0; i< MaxNum; i++) {
        if (pPTLHEVC->sub_ptl[i].profile_present_flag) {
            pPTLHEVC->sub_ptl[i].profile_space = gts_bs_read_int(bs, 2);
            pPTLHEVC->sub_ptl[i].tier_flag = (bool)gts_bs_read_int(bs, 1);
            pPTLHEVC->sub_ptl[i].profile_idc = gts_bs_read_int(bs, 5);
            pPTLHEVC->sub_ptl[i].profile_compatibility_flag = gts_bs_read_int(bs, 32);
            gts_bs_read_int(bs, 1);
            gts_bs_read_int(bs, 1);
            gts_bs_read_int(bs, 1);
            gts_bs_read_int(bs, 1);
            gts_bs_read_long_int(bs, 44);
        }
        if (pPTLHEVC->sub_ptl[i].level_present_flag)
            pPTLHEVC->sub_ptl[i].level_idc = gts_bs_read_int(bs, 8);
    }
}

static uint32_t scalability_type_to_idx(HEVC_VPS *vps, uint32_t scalability_type)
{
    uint32_t ret = 0;
    uint32_t type = 0;
    uint32_t value = 0;
    for (; type < scalability_type; type++) {
        if (vps->vps_extension.scalability_mask[type])
            value = 1;
        else
            value = 0;
        ret += value;
    }
    return ret;
}

#define VIEW_ORDER_INDEX  1

static uint32_t get_scalability_id(HEVC_VPS *vps, uint32_t layer_id_in_vps, uint32_t scalability_type )
{
    uint32_t idx = 0;
    uint32_t ret = 0;
    if (!vps->vps_extension.scalability_mask[scalability_type])
        return ret;
    idx = scalability_type_to_idx(vps, scalability_type);
    ret = vps->vps_extension.dimension_id[layer_id_in_vps][idx];
    return ret;
}

static uint32_t get_num_views(HEVC_VPS *vps)
{
    uint32_t numViews = 1, i;
    for (i=0; i<vps->max_layers; i++ ) {
        uint32_t layer_id = vps->vps_extension.layer_id_in_nuh[i];
	uint32_t viewIndex = get_scalability_id(vps, vps->vps_extension.layer_id_in_vps[layer_id], VIEW_ORDER_INDEX);
        if (i>0 && (viewIndex != get_scalability_id( vps, i-1, VIEW_ORDER_INDEX) )) {
            numViews++;
        }
    }
    return numViews;
}

static void parse_rep_format(HEVC_RepFormat *pRepFormatHevc, GTS_BitStream *bs)
{
    if (!pRepFormatHevc || !bs)
        return;
    uint8_t chroma_bitdepth_present_flag = 0;
    pRepFormatHevc->pic_width_luma_samples = gts_bs_read_int(bs, 16);
    pRepFormatHevc->pic_height_luma_samples = gts_bs_read_int(bs, 16);
    chroma_bitdepth_present_flag = gts_bs_read_int(bs, 1);
    if (chroma_bitdepth_present_flag) {
        pRepFormatHevc->chroma_format_idc = gts_bs_read_int(bs, 2);

        if (pRepFormatHevc->chroma_format_idc == 3)
            pRepFormatHevc->separate_colour_plane_flag = gts_bs_read_int(bs, 1);
        pRepFormatHevc->bit_depth_luma = 8 + gts_bs_read_int(bs, 4);
        pRepFormatHevc->bit_depth_chroma = 8 + gts_bs_read_int(bs, 4);
    }
    if (gts_bs_read_int(bs, 1)) {
        bs_get_ue(bs);
        bs_get_ue(bs);
        bs_get_ue(bs);
        bs_get_ue(bs);
    }
}


static bool hevc_parseVPSExt(HEVC_VPS *pVPSHevc, GTS_BitStream *bs)
{
    bool OutputLayerFlag[MAX_LHVC_LAYERS][MAX_LHVC_LAYERS];
    uint8_t splitting_flag, vps_nuh_layer_id_present_flag, view_id_len;
    uint8_t dimension_id_len[16], dim_bit_offset[16];
    uint8_t rep_format_idx_present_flag;
    uint8_t idxolsIds;
    uint8_t layerSetsNum;
    uint8_t layerSetIdxForOlsMinus1[MAX_LHVC_LAYERS];
    uint8_t nb_output_layers_in_output_layer_set[MAX_LHVC_LAYERS+1];
    uint8_t ols_highest_output_layer_id[MAX_LHVC_LAYERS+1];
    uint8_t num_direct_ref_layers[64], num_pred_layers[64], num_layers_in_tree_partition[MAX_LHVC_LAYERS];
    uint8_t flagDepend[MAX_LHVC_LAYERS][MAX_LHVC_LAYERS], id_pred_layers[64][MAX_LHVC_LAYERS];
    uint8_t layer_id_in_list_flag[64];
    uint32_t i, j, num_add_layer_set, num_indepentdent_layers, nb_bits;
    uint32_t numScalTypes = 0;;
    uint32_t maxLayers = pVPSHevc->max_layers;
    uint32_t k, r, iNuhLId, jNuhLId;
    uint32_t direct;
    uint32_t pred;
    uint32_t independ;
    uint32_t addOlssNum = 0;
    uint32_t outputLayerIDC = 0;
    uint32_t loopCnt = 0;

    //pVPSHevc->vps_extension_found=(bool)1;
    if ((maxLayers > 1) && pVPSHevc->base_layer_internal_flag)
        HEVC_profiletierevel(bs, (bool)0, pVPSHevc->max_sub_layers-1, &pVPSHevc->vps_extension.ext_ptl[0]);

    splitting_flag = gts_bs_read_int(bs, 1);
    for (i=0; i<16; i++) {
        pVPSHevc->vps_extension.scalability_mask[i] = gts_bs_read_int(bs, 1);
        numScalTypes += pVPSHevc->vps_extension.scalability_mask[i];
    }
    if (numScalTypes >=16) {
        numScalTypes =16;
    }
    dimension_id_len[0] = 0;
    for (i=0; i<(numScalTypes - splitting_flag); i++) {
        dimension_id_len[i] = 1 + gts_bs_read_int(bs, 3);
    }

    if (splitting_flag) {
        for (i = 0; i < numScalTypes; i++) {
            dim_bit_offset[i] = 0;
            for (j = 0; j < i; j++)
                dim_bit_offset[i] +=  dimension_id_len[j];
        }
        dimension_id_len[numScalTypes -1] = 1 + (5 - dim_bit_offset[numScalTypes -1]);
        dim_bit_offset[numScalTypes] = 6;
    }

    vps_nuh_layer_id_present_flag = gts_bs_read_int(bs, 1);
    pVPSHevc->vps_extension.layer_id_in_nuh[0] = 0;
    pVPSHevc->vps_extension.layer_id_in_vps[0] = 0;
    for (i=1; i<maxLayers; i++) {
        if (vps_nuh_layer_id_present_flag) {
            pVPSHevc->vps_extension.layer_id_in_nuh[i] = gts_bs_read_int(bs, 6);
        } else {
            pVPSHevc->vps_extension.layer_id_in_nuh[i] = i;
        }
        pVPSHevc->vps_extension.layer_id_in_vps[pVPSHevc->vps_extension.layer_id_in_nuh[i]] = i;

        if (!splitting_flag) {
            for (j=0; j< numScalTypes; j++) {
                pVPSHevc->vps_extension.dimension_id[i][j] = gts_bs_read_int(bs, dimension_id_len[j]);
            }
        }
    }

    if (splitting_flag) {
        for (i = 0; i<maxLayers; i++)
            for (j=0; j< numScalTypes; j++)
                pVPSHevc->vps_extension.dimension_id[i][j] = ((pVPSHevc->vps_extension.layer_id_in_nuh[i] & ((1 << dim_bit_offset[j+1]) -1)) >> dim_bit_offset[j]);
    } else {
        for (j=0; j< numScalTypes; j++)
            pVPSHevc->vps_extension.dimension_id[0][j] = 0;
    }

    view_id_len = gts_bs_read_int(bs, 4);
    if (view_id_len > 0) {
        for( i = 0; i < get_num_views(pVPSHevc); i++ ) {
            gts_bs_read_int(bs, view_id_len);
        }
    }

    for (i=1; i<maxLayers; i++) {
        for (j=0; j<i; j++) {
            pVPSHevc->vps_extension.direct_dependency_flag[i][j] = gts_bs_read_int(bs, 1);
        }
    }
    for (i = 0; i < MAX_LHVC_LAYERS; i++) {
        if (i >= maxLayers) break;
        for (j = 0; j < maxLayers; j++) {
            flagDepend[i][j] = pVPSHevc->vps_extension.direct_dependency_flag[i][j];
            for (k = 0; k < i; k++)
                if (pVPSHevc->vps_extension.direct_dependency_flag[i][k] && pVPSHevc->vps_extension.direct_dependency_flag[k][j])
                    flagDepend[i][j] = 1;
        }
    }

    for (i = 0; i < maxLayers; i++) {
        iNuhLId = pVPSHevc->vps_extension.layer_id_in_nuh[i];
        direct = 0;
        pred = 0;
        r = 0;
        for (j = 0; j < maxLayers; j++) {
            jNuhLId = pVPSHevc->vps_extension.layer_id_in_nuh[j];
            if (pVPSHevc->vps_extension.direct_dependency_flag[i][j]) {
                direct++;
            }
            if (flagDepend[i][j]) {
                r++;
            }

            if (flagDepend[j][i])
                id_pred_layers[iNuhLId][pred++] = jNuhLId;
        }
        num_direct_ref_layers[iNuhLId] = direct;
        num_pred_layers[iNuhLId] = pred;
    }

    memset_s(layer_id_in_list_flag, 64*sizeof(uint8_t), 0);
    independ = 0;
    for (i = 0; i < maxLayers; i++) {
        iNuhLId = pVPSHevc->vps_extension.layer_id_in_nuh[i];
        if (!num_direct_ref_layers[iNuhLId]) {
            uint32_t h = 1;
            for (j = 0; j < num_pred_layers[iNuhLId]; j++) {
                uint32_t predLId = id_pred_layers[iNuhLId][j];
                if (!layer_id_in_list_flag[predLId]) {
                    layer_id_in_list_flag[predLId] = 1;
                }
            }
            num_layers_in_tree_partition[independ++] = h;
        }
    }
    num_indepentdent_layers = independ;

    num_add_layer_set = 0;
    if (num_indepentdent_layers > 1)
        num_add_layer_set = bs_get_ue(bs);

    for (i = 0; i < num_add_layer_set; i++)
        for (j = 1; j < num_indepentdent_layers; j++) {
            nb_bits =1;
             while ((1 << nb_bits) < (num_layers_in_tree_partition[j] + 1))
                nb_bits++;
            gts_bs_read_int(bs, nb_bits);
        }


    if (gts_bs_read_int(bs, 1)) {
        for (i = 0; i < maxLayers; i++) {
            gts_bs_read_int(bs, 3);
        }
    }

    if (gts_bs_read_int(bs, 1)) {
        for (i=0; i<(maxLayers-1) ; i++) {
            for (j= i+1; j < maxLayers; j++) {
                if (pVPSHevc->vps_extension.direct_dependency_flag[j][i])
                    gts_bs_read_int(bs, 3);
            }
        }
    }
    gts_bs_read_int(bs, 1);

    pVPSHevc->vps_extension.num_profile_tier_level = 1+bs_get_ue(bs);
    if (pVPSHevc->vps_extension.num_profile_tier_level > MAX_LHVC_LAYERS) {
        pVPSHevc->vps_extension.num_profile_tier_level=1;
        return false;
    }

    i = 1;
    if (pVPSHevc->base_layer_internal_flag)
	i = 2;
    for (; i < pVPSHevc->vps_extension.num_profile_tier_level; i++) {
        bool flagProfileVPS = (bool)gts_bs_read_int(bs, 1);
        HEVC_profiletierevel(bs, flagProfileVPS, pVPSHevc->max_sub_layers-1, &pVPSHevc->vps_extension.ext_ptl[i-1] );
    }
    addOlssNum = 0;
    layerSetsNum = pVPSHevc->num_layer_sets + num_add_layer_set;

    if (layerSetsNum > 1) {
        addOlssNum = bs_get_ue(bs);
        outputLayerIDC = gts_bs_read_int(bs,2);
        if (outputLayerIDC > 2)
            outputLayerIDC = 2;
    }
    pVPSHevc->vps_extension.num_output_layer_sets = addOlssNum + layerSetsNum;
    layerSetIdxForOlsMinus1[0] = 1;
    pVPSHevc->vps_extension.output_layer_flag[0][0] = (bool)1;

    for (i = 0; i < pVPSHevc->vps_extension.num_output_layer_sets; i++) {
        if ((layerSetsNum > 2) && (i >= layerSetsNum)) {
            nb_bits = 1;
            while ((1 << nb_bits) < (layerSetsNum - 1))
                nb_bits++;
            layerSetIdxForOlsMinus1[i] = gts_bs_read_int(bs, nb_bits);
        }
        else
            layerSetIdxForOlsMinus1[i] = 0;
        idxolsIds = i < layerSetsNum ? i : layerSetIdxForOlsMinus1[i] + 1;
        loopCnt = pVPSHevc->vps_extension.num_layers_in_id_list[idxolsIds];
        if ((i > (pVPSHevc->num_layer_sets - 1)) || (outputLayerIDC == 2)) {
            for (j = 0; j < loopCnt; j++)
                pVPSHevc->vps_extension.output_layer_flag[i][j] = (bool)gts_bs_read_int(bs, 1);
        }

        if ((outputLayerIDC == 0) || (outputLayerIDC == 1)) {
            for (j = 0; j < loopCnt; j++) {
                if ((outputLayerIDC == 0) || (pVPSHevc->vps_extension.LayerSetLayerIdList[i][j] == pVPSHevc->vps_extension.LayerSetLayerIdListMax[i]))
                    OutputLayerFlag[i][j] = true;
                else
                    OutputLayerFlag[i][j] = false;
            }
        }
        for (j = 0; j < loopCnt; j++) {
            if (OutputLayerFlag[i][j]) {
                uint32_t layeridCur;
                uint32_t k;
                pVPSHevc->vps_extension.necessary_layers_flag[i][j] = true;
                layeridCur = pVPSHevc->vps_extension.LayerSetLayerIdList[i][j];
                for (k = 0; k < j; k++) {
                    uint32_t LayerIdref = pVPSHevc->vps_extension.LayerSetLayerIdList[i][k];
                    if (flagDepend[pVPSHevc->vps_extension.layer_id_in_vps[layeridCur]][pVPSHevc->vps_extension.layer_id_in_vps[LayerIdref]])
                        pVPSHevc->vps_extension.necessary_layers_flag[i][k] = true;
                }
            }
        }
        pVPSHevc->vps_extension.num_necessary_layers[i] = 0;
        for (j = 0; j < loopCnt; j++) {
            if (pVPSHevc->vps_extension.necessary_layers_flag[i][j])
                pVPSHevc->vps_extension.num_necessary_layers[i] += 1;
        }

        if (i == 0) {
            if (pVPSHevc->base_layer_internal_flag) {
                if (maxLayers > 1)
                    pVPSHevc->vps_extension.profile_tier_level_idx[0][0] = 1;
                else
                    pVPSHevc->vps_extension.profile_tier_level_idx[0][0] = 0;
            }
            continue;
        }
        nb_bits = 1;
        while ((uint32_t)(1 << nb_bits) < pVPSHevc->vps_extension.num_profile_tier_level)
            nb_bits++;
        for (j = 0; j < loopCnt; j++)
            if (pVPSHevc->vps_extension.necessary_layers_flag[i][j] && pVPSHevc->vps_extension.num_profile_tier_level)
                pVPSHevc->vps_extension.profile_tier_level_idx[i][j] = gts_bs_read_int(bs, nb_bits);
            else
                pVPSHevc->vps_extension.profile_tier_level_idx[i][j] = 0;


        nb_output_layers_in_output_layer_set[i] = 0;
        ols_highest_output_layer_id[i] = 0;
        for (j = 0; j < loopCnt; j++) {
            nb_output_layers_in_output_layer_set[i] += OutputLayerFlag[i][j];
            if (OutputLayerFlag[i][j]) {
                ols_highest_output_layer_id[i] = pVPSHevc->vps_extension.LayerSetLayerIdList[idxolsIds][j];
            }
        }
        if (nb_output_layers_in_output_layer_set[i] == 1 && ols_highest_output_layer_id[i] > 0)
             pVPSHevc->vps_extension.alt_output_layer_flag[i] = (bool)gts_bs_read_int(bs, 1);
    }

    pVPSHevc->vps_extension.num_rep_formats = 1 + bs_get_ue(bs);
    if (pVPSHevc->vps_extension.num_rep_formats > 16) {
        pVPSHevc->vps_extension.num_rep_formats = 0;
        return false;
    }
    for (i = 0; i < pVPSHevc->vps_extension.num_rep_formats; i++) {
	 parse_rep_format(&pVPSHevc->vps_extension.rep_formats[i], bs);
    }
    if (pVPSHevc->vps_extension.num_rep_formats > 1)
        rep_format_idx_present_flag = gts_bs_read_int(bs, 1);
    else
        rep_format_idx_present_flag = 0;

    pVPSHevc->vps_extension.rep_format_idx[0] = 0;
    nb_bits = 1;
    while ((uint32_t)(1 << nb_bits) < pVPSHevc->vps_extension.num_rep_formats)
        nb_bits++;
    for (i = pVPSHevc->base_layer_internal_flag ? 1 : 0; i < maxLayers; i++) {
        if (rep_format_idx_present_flag) {
            pVPSHevc->vps_extension.rep_format_idx[i] = gts_bs_read_int(bs, nb_bits);
        }
        else {
            pVPSHevc->vps_extension.rep_format_idx[i] = i < pVPSHevc->vps_extension.num_rep_formats - 1 ? i : pVPSHevc->vps_extension.num_rep_formats - 1;
        }
    }
//TODO - we don't use the rest ...

    return true;
}
/*
static void sub_layer_hrd_parameters(GTS_BitStream *bs, int32_t subLayerId, uint32_t cpb_cnt, bool flagHdrParaPresentSubpic) {
    uint32_t i;
    for (i = 0; i <= cpb_cnt; i++) {
        bs_get_ue(bs);
        bs_get_ue(bs);
        if (flagHdrParaPresentSubpic) {
            bs_get_ue(bs);
            bs_get_ue(bs);
        }
        gts_bs_read_int(bs, 1);
    }
}
*/
static void hevc_parse_hrd_parameters(GTS_BitStream *bs, HEVC_VPS *vps, bool commonInfPresentFlag, int32_t maxNumSubLayersMinus1)
{
    int32_t i;
    //uint32_t flagHdrParaPresent = 0;
    //uint32_t flagHdrParaPresentVcl = 0;
    //bool flagHdrParaPresentSubpic = false;
    HEVC_HrdInfo hrd_info;
    memset_s(&hrd_info, sizeof(HEVC_HrdInfo), 0);
    if (commonInfPresentFlag) {
        hrd_info.nal_hrd_param_present_flag = gts_bs_read_int(bs, 1);
        hrd_info.vcl_hrd_param_present_flag = gts_bs_read_int(bs, 1);
        if (hrd_info.nal_hrd_param_present_flag || hrd_info.vcl_hrd_param_present_flag) {
            hrd_info.sub_pic_cpb_param_present_flag = (bool)gts_bs_read_int(bs, 1);
            if (hrd_info.sub_pic_cpb_param_present_flag) {
                hrd_info.tick_divisor_minus2 = gts_bs_read_int(bs, 8);
                hrd_info.du_cpb_removal_delay_len_minus1 = gts_bs_read_int(bs, 5);
                hrd_info.sub_pic_cpb_param_in_picTSEI_flag = gts_bs_read_int(bs, 1);
                hrd_info.dpb_out_delay_du_len_minus1 = gts_bs_read_int(bs, 5);
            }
            hrd_info.bit_rate_scale = gts_bs_read_int(bs, 4);
            hrd_info.cpb_size_scale = gts_bs_read_int(bs, 4);
            if (hrd_info.sub_pic_cpb_param_present_flag) {
                hrd_info.du_cpb_size_scale = gts_bs_read_int(bs, 4);
            }
            hrd_info.init_cpb_removal_delay_len_minus1 = gts_bs_read_int(bs, 5);
            hrd_info.cpb_removal_delay_len_minus1 = gts_bs_read_int(bs, 5);
            hrd_info.dpb_out_delay_len_minus1 = gts_bs_read_int(bs, 5);
        }
    }
    for (i = 0; i <= maxNumSubLayersMinus1; i++) {
        //uint32_t fixed_pic_rate_general_flag_i = 0;
        //uint32_t fixed_pic_rate_within_cvs_flag_i = 1;
        //uint32_t low_delay_hrd_flag_i = 0;
        //uint32_t cpb_cnt_minus1_i = 0;
        hrd_info.hrd_sub_layer_infos[i].fixed_pic_rate_flag = gts_bs_read_int(bs, 1);
        if (!hrd_info.hrd_sub_layer_infos[i].fixed_pic_rate_flag) {
            hrd_info.hrd_sub_layer_infos[i].fixed_pic_rate_within_cvs_flag = gts_bs_read_int(bs, 1);
        }
        else
        {
            hrd_info.hrd_sub_layer_infos[i].fixed_pic_rate_within_cvs_flag = true;
        }
        hrd_info.hrd_sub_layer_infos[i].low_delay_hrd_flag = 0;
        hrd_info.hrd_sub_layer_infos[i].cpb_cnt_minus1 = 0;

        if (hrd_info.hrd_sub_layer_infos[i].fixed_pic_rate_within_cvs_flag)
        {
            hrd_info.hrd_sub_layer_infos[i].pic_dur_inTc_minus1 = bs_get_ue(bs);
        }
        else
        {
            hrd_info.hrd_sub_layer_infos[i].low_delay_hrd_flag = gts_bs_read_int(bs, 1);
        }

        if (!hrd_info.hrd_sub_layer_infos[i].low_delay_hrd_flag) {
            hrd_info.hrd_sub_layer_infos[i].cpb_cnt_minus1 = bs_get_ue(bs);
        }

        uint32_t nal_or_vcl = 0;
        uint32_t j = 0;
        for (nal_or_vcl = 0; nal_or_vcl < 2; nal_or_vcl++)
        {
            if (((nal_or_vcl == 0) && hrd_info.nal_hrd_param_present_flag) ||
                ((nal_or_vcl == 1) && hrd_info.vcl_hrd_param_present_flag))
            {
                for (j = 0; j <= hrd_info.hrd_sub_layer_infos[i].cpb_cnt_minus1; j++)
                {
                    hrd_info.hrd_sub_layer_infos[i].bitrate_val_minus1[j][nal_or_vcl] = bs_get_ue(bs);
                    hrd_info.hrd_sub_layer_infos[i].cpb_size_value[j][nal_or_vcl] = bs_get_ue(bs);
                    if (hrd_info.sub_pic_cpb_param_present_flag)
                    {
                        hrd_info.hrd_sub_layer_infos[i].du_cpb_size_value[j][nal_or_vcl] = bs_get_ue(bs);
                        hrd_info.hrd_sub_layer_infos[i].du_bitrate_value[j][nal_or_vcl] = bs_get_ue(bs);
                    }
                    hrd_info.hrd_sub_layer_infos[i].cbr_flag[j][nal_or_vcl] = gts_bs_read_int(bs, 1);
                }
            }
        }
        //if (flagHdrParaPresent) {
        //    sub_layer_hrd_parameters(bs, i, cpb_cnt_minus1_i, flagHdrParaPresentSubpic);
        //}
        //if (flagHdrParaPresentVcl) {
        //    sub_layer_hrd_parameters(bs, i, cpb_cnt_minus1_i, flagHdrParaPresentSubpic);
        //}
    }
    vps->hrd_infos.push_back(hrd_info);
}

static int32_t gts_media_hevc_read_vps_bs(GTS_BitStream *bs, HEVCState *hevc, bool stop_at_vps_ext)
{
    //uint8_t subLayerOrderInfoPresentFlag;
    //uint8_t vps_extension_flag;
    //uint8_t layer_id_included_flag[MAX_LHVC_LAYERS][64];
    uint32_t i = 0;
    uint32_t j;
    int32_t vps_id = -1;
    HEVC_VPS *vps;
    uint32_t layerIDMax = 0;
    uint32_t layerSetsNum = 0;
    //nalu header already parsed
    vps_id = gts_bs_read_int(bs, 4);

    if (vps_id>=16)
       return -1;

    vps = &hevc->vps[vps_id];
    vps->bit_pos_vps_extensions = -1;
    if (!vps->state) {
        vps->id = vps_id;
        vps->state = 1;
    }

    vps->base_layer_internal_flag = (bool)gts_bs_read_int(bs, 1);
    vps->base_layer_available_flag = (bool)gts_bs_read_int(bs, 1);
    vps->max_layers = 1 + gts_bs_read_int(bs, 6);
    if (vps->max_layers>MAX_LHVC_LAYERS)
    {
        return -1;
    }
    vps->max_sub_layers = gts_bs_read_int(bs, 3) + 1;
    //printf("max_layers %d, max_sub_layers %d \n", vps->max_layers, vps->max_sub_layers);
    vps->temporal_id_nesting = (bool)gts_bs_read_int(bs, 1);
    gts_bs_read_int(bs, 16);
    HEVC_profiletierevel(bs, (bool)1, vps->max_sub_layers-1, &vps->ptl);

    vps->sub_layer_ordering_info_present_flag = gts_bs_read_int(bs, 1);
    if (vps->sub_layer_ordering_info_present_flag)
        i = 0;
    else
        i = vps->max_sub_layers - 1;
    for (; i < vps->max_sub_layers; i++)
    {
        vps->vps_max_dec_pic_buffering_minus1[i] = bs_get_ue(bs);
        vps->vps_max_num_reorder_pics[i] = bs_get_ue(bs);
        vps->vps_max_latency_increase_plus1[i] = bs_get_ue(bs);
        //printf("i %d, %d, %d, %d \n", i, vps->vps_max_dec_pic_buffering_minus1[i], vps->vps_max_num_reorder_pics[i], vps->vps_max_latency_increase_plus1[i]);
    }
    layerIDMax = gts_bs_read_int(bs, 6);
    vps->max_layer_id = layerIDMax;
    if (layerIDMax > MAX_LHVC_LAYERS)
    {
        return -1;
    }
    layerSetsNum = bs_get_ue(bs) + 1;
    vps->num_layer_sets = layerSetsNum;
    if (layerSetsNum > MAX_LHVC_LAYERS)
    {
        return -1;
    }
    for (i=1; i < layerSetsNum; i++)
    {
        for (j=0; j <= layerIDMax; j++)
        {
            vps->layer_id_included_flag[ i ][ j ] = gts_bs_read_int(bs, 1);
        }
    }
    //vps->num_layers_in_id_list[0] = 1;
    //for (i = 1; i < layerSetsNum; i++)
    //{
    //    uint32_t n = 0;
    //    uint32_t m = 0;
    //    for (m = 0; m <= layerIDMax; m++)
    //        if (vps->layer_id_included_flag[i][m])
    //        {
    //            if(n < MAX_LHVC_LAYERS)
    //                vps->LayerSetLayerIdList[i][n++] = m;
    //            if (vps->LayerSetLayerIdListMax[i] < m)
    //                vps->LayerSetLayerIdListMax[i] = m;
    //        }
    //    vps->num_layers_in_id_list[i] = n;
    //}
    vps->timing_info_present_flag = gts_bs_read_int(bs, 1);
    if (vps->timing_info_present_flag)
    {
        //uint32_t numHrdParamVPS;
        vps->num_units_in_tick = gts_bs_read_int(bs, 32);
        vps->time_scale = gts_bs_read_int(bs, 32);
        vps->poc_proportional_to_timing_flag = gts_bs_read_int(bs, 1);
        if (vps->poc_proportional_to_timing_flag) {
            vps->num_ticks_poc_diff_one = bs_get_ue(bs) + 1;
        }
        //numHrdParamVPS = bs_get_ue(bs);
        vps->num_hrd_parameters = bs_get_ue(bs);
        for( i = 0; i < vps->num_hrd_parameters; i++ ) {
            //bool flagPresentCPRMS = true;
            uint32_t one_hrd_layer_set_idx = bs_get_ue(bs);
            vps->hrd_layer_set_idx[i] = one_hrd_layer_set_idx;
            bool cprms_present_flag = true;
            if (i > 0)
            {
                cprms_present_flag = (bool)gts_bs_read_int(bs, 1);
                vps->cprms_present_flags.push_back(cprms_present_flag);
                //vps->cprms_present_flag[i] = (bool)gts_bs_read_int(bs, 1) ;
            }
            else
            {
                cprms_present_flag = true;
                vps->cprms_present_flags.push_back(cprms_present_flag);
                //vps->cprms_present_flag[i] = true;
            }
            hevc_parse_hrd_parameters(bs, vps, cprms_present_flag, vps->max_sub_layers - 1);
        }
    }
    //if (stop_at_vps_ext)
    //{
    //    return vps_id;
    //}

    vps->vps_extension_flag = gts_bs_read_int(bs, 1);
    if (vps->vps_extension_flag) {
        bool res;
        gts_bs_align(bs);
        res = hevc_parseVPSExt(vps, bs);
        if (res!=true) {
            return -1;
        }
        if (gts_bs_read_int(bs, 1)) {
            while (gts_bs_available(bs)) {
                gts_bs_read_int(bs, 1);
            }
        }
    }
    return vps_id;
}


static void hevc_scaling_list_data(GTS_BitStream *bs)
{
    uint32_t i, sizeId, matrixId;
    for (sizeId = 0; sizeId < 4; sizeId++) {
        for (matrixId=0; matrixId<6; matrixId += (sizeId == 3) ? 3:1 ) {
            uint32_t scaling_list_pred_mode_flag_sizeId_matrixId = gts_bs_read_int(bs, 1);
            if( ! scaling_list_pred_mode_flag_sizeId_matrixId ) {
                bs_get_ue(bs);
            } else {
                uint32_t coefNum = MIN(64, (1 << (4+(sizeId << 1))));
                if ( sizeId > 1 ) {
                    bs_get_se(bs);
                }
                for (i = 0; i<coefNum; i++) {
                    bs_get_se(bs);
                }
            }
        }
    }
}


static const struct {
    uint32_t w, h;
} hevc_sar[17] =
{
    { 0,   0 }, { 1,   1 }, { 12, 11 }, { 10, 11 },
    { 16, 11 }, { 40, 33 }, { 24, 11 }, { 20, 11 },
    { 32, 11 }, { 80, 33 }, { 18, 11 }, { 15, 11 },
    { 64, 33 }, { 160,99 }, { 4,3}, { 3,2}, { 2,1}
};

static int32_t gts_media_hevc_read_sps_bs(GTS_BitStream *bs, HEVCState *hevc, uint8_t layer_id, uint32_t *vui_flag_pos)
{
    int32_t vps_id, sps_id = -1;
    uint8_t max_sub_layers_minus1;
    bool scaling_list_enable_flag;
    uint32_t i, nb_CTUs, depth;
    uint32_t DiffMaxMinLumaCodingBlockSizeLog2;
    uint32_t MinTransforBlockSizeLog2;
    uint32_t MinLumaCodingBlockSizeLog2;
    bool sps_sub_layer_ordering_info_present_flag;
    HEVC_SPS *spsInforHevc;
    HEVC_VPS *vps;
    HEVC_ProfileTierLevel ptl;
    uint32_t sps_ext_or_max_sub_layers_minus1;
    bool multiLayerExtSpsFlag;

    if (vui_flag_pos) *vui_flag_pos = 0;

    //nalu header already parsed
    vps_id = gts_bs_read_int(bs, 4);
    if (vps_id>=16) {
        return -1;
    }
    memset_s(&ptl, sizeof(ptl), 0);
    max_sub_layers_minus1 = 0;
    sps_ext_or_max_sub_layers_minus1 = 0;
    if (layer_id == 0)
        max_sub_layers_minus1 = gts_bs_read_int(bs, 3);
    else
        sps_ext_or_max_sub_layers_minus1 = gts_bs_read_int(bs, 3);
    multiLayerExtSpsFlag = (bool)((layer_id != 0) && (sps_ext_or_max_sub_layers_minus1 == 7));
    if (!multiLayerExtSpsFlag) {
        gts_bs_read_int(bs, 1);
		HEVC_profiletierevel(bs, (bool)1, max_sub_layers_minus1, &ptl);
    }

    sps_id = bs_get_ue(bs);
    if ((sps_id<0) ||(sps_id>=16)) {
        return -1;
    }

    spsInforHevc = &hevc->sps[sps_id];
    if (!spsInforHevc->state) {
        spsInforHevc->state = 1;
        spsInforHevc->id = sps_id;
        spsInforHevc->vps_id = vps_id;
    }
    spsInforHevc->ptl = ptl;
    vps = &hevc->vps[vps_id];

    spsInforHevc->max_sub_layers_minus1 = max_sub_layers_minus1;

    if (multiLayerExtSpsFlag) {
        uint8_t update_rep_format_flag = gts_bs_read_int(bs, 1);
        if (update_rep_format_flag) {
            spsInforHevc->rep_format_idx = gts_bs_read_int(bs, 8);
        } else {
            spsInforHevc->rep_format_idx = vps->vps_extension.rep_format_idx[layer_id];
        }
        spsInforHevc->width = vps->vps_extension.rep_formats[spsInforHevc->rep_format_idx].pic_width_luma_samples;
        spsInforHevc->height = vps->vps_extension.rep_formats[spsInforHevc->rep_format_idx].pic_height_luma_samples;
        spsInforHevc->chroma_format_idc = vps->vps_extension.rep_formats[spsInforHevc->rep_format_idx].chroma_format_idc;
        spsInforHevc->bit_depth_luma = vps->vps_extension.rep_formats[spsInforHevc->rep_format_idx].bit_depth_luma;
        spsInforHevc->bit_depth_chroma = vps->vps_extension.rep_formats[spsInforHevc->rep_format_idx].bit_depth_chroma;
        spsInforHevc->separate_colour_plane_flag = (bool)vps->vps_extension.rep_formats[spsInforHevc->rep_format_idx].separate_colour_plane_flag;

        //TODO this is crude ...
        spsInforHevc->ptl = vps->vps_extension.ext_ptl[0];
    } else {
        spsInforHevc->chroma_format_idc = bs_get_ue(bs);
        if (spsInforHevc->chroma_format_idc==3)
            spsInforHevc->separate_colour_plane_flag = (bool)gts_bs_read_int(bs, 1);
        spsInforHevc->width = bs_get_ue(bs);
        spsInforHevc->height = bs_get_ue(bs);
        if (gts_bs_read_int(bs, 1)) {
            uint32_t width_sub, height_sub;

            if (spsInforHevc->chroma_format_idc==1) {
                width_sub = height_sub = 2;
            }
            else if (spsInforHevc->chroma_format_idc==2) {
                width_sub = 2;
                height_sub = 1;
            } else {
                width_sub = height_sub = 1;
            }

            spsInforHevc->cw_left = bs_get_ue(bs);
            spsInforHevc->cw_right = bs_get_ue(bs);
            spsInforHevc->cw_top = bs_get_ue(bs);
            spsInforHevc->cw_bottom = bs_get_ue(bs);

            spsInforHevc->width -= width_sub * (spsInforHevc->cw_left + spsInforHevc->cw_right);
            spsInforHevc->height -= height_sub * (spsInforHevc->cw_top + spsInforHevc->cw_bottom);
        }
        spsInforHevc->bit_depth_luma = 8 + bs_get_ue(bs);
        spsInforHevc->bit_depth_chroma = 8 + bs_get_ue(bs);
    }

    spsInforHevc->log2_max_pic_order_cnt_lsb = 4 + bs_get_ue(bs);

    if (!multiLayerExtSpsFlag) {
        sps_sub_layer_ordering_info_present_flag = (bool)gts_bs_read_int(bs, 1);
        for(i=sps_sub_layer_ordering_info_present_flag ? 0 : spsInforHevc->max_sub_layers_minus1; i<=spsInforHevc->max_sub_layers_minus1; i++) {
            spsInforHevc->max_dec_pic_buffering =  bs_get_ue(bs);
            spsInforHevc->num_reorder_pics =  bs_get_ue(bs);
            spsInforHevc->max_latency_increase =  bs_get_ue(bs);
        }
    }

    MinLumaCodingBlockSizeLog2 = bs_get_ue(bs) + 3;
    DiffMaxMinLumaCodingBlockSizeLog2 = bs_get_ue(bs);
    spsInforHevc->max_CU_width = ( 1<<(MinLumaCodingBlockSizeLog2 + DiffMaxMinLumaCodingBlockSizeLog2) );
    spsInforHevc->max_CU_height = ( 1<<(MinLumaCodingBlockSizeLog2 + DiffMaxMinLumaCodingBlockSizeLog2) );

    MinTransforBlockSizeLog2 = 2 + bs_get_ue(bs);
    bs_get_ue(bs);

    depth = 0;
    spsInforHevc->max_transform_hierarchy_depth_inter = bs_get_ue(bs);
    spsInforHevc->max_transform_hierarchy_depth_intra = bs_get_ue(bs);
    while( (uint32_t) ( spsInforHevc->max_CU_width >> DiffMaxMinLumaCodingBlockSizeLog2 ) > (uint32_t) ( 1 << ( MinTransforBlockSizeLog2 + depth )  ) )
    {
        depth++;
    }
    spsInforHevc->max_CU_depth = DiffMaxMinLumaCodingBlockSizeLog2 + depth;

    nb_CTUs = ((spsInforHevc->width + spsInforHevc->max_CU_width -1) / spsInforHevc->max_CU_width) * ((spsInforHevc->height + spsInforHevc->max_CU_height-1) / spsInforHevc->max_CU_height);
    spsInforHevc->bitsSliceSegmentAddress = 0;
    while (nb_CTUs > (uint32_t) (1 << spsInforHevc->bitsSliceSegmentAddress)) {
        spsInforHevc->bitsSliceSegmentAddress++;
    }

    scaling_list_enable_flag = (bool)gts_bs_read_int(bs, 1);
    if (scaling_list_enable_flag) {
        bool sps_infer_scaling_list_flag = (bool)0;
        if (multiLayerExtSpsFlag) {
            sps_infer_scaling_list_flag = (bool)gts_bs_read_int(bs, 1);
        }

        if (sps_infer_scaling_list_flag) {
            gts_bs_read_int(bs, 6);
        } else {
            if (gts_bs_read_int(bs, 1) ) {
                hevc_scaling_list_data(bs);
            }
        }
    }
    gts_bs_read_int(bs, 1);
    spsInforHevc->sample_adaptive_offset_enabled_flag = (bool)gts_bs_read_int(bs, 1);
    if (gts_bs_read_int(bs, 1) ) {
        gts_bs_read_int(bs, 4);
        gts_bs_read_int(bs, 4);
        bs_get_ue(bs);
        bs_get_ue(bs);
        gts_bs_read_int(bs, 1);
    }
    spsInforHevc->num_short_term_ref_pic_sets = bs_get_ue(bs);
    if (spsInforHevc->num_short_term_ref_pic_sets>64) {
        return -1;
    }

    spsInforHevc->long_term_ref_pics_present_flag = (bool)gts_bs_read_int(bs, 1);
    if (spsInforHevc->long_term_ref_pics_present_flag) {
        spsInforHevc->num_long_term_ref_pic_sps = bs_get_ue(bs);
        for (i=0; i<spsInforHevc->num_long_term_ref_pic_sps; i++) {
            gts_bs_read_int(bs, spsInforHevc->log2_max_pic_order_cnt_lsb);
            gts_bs_read_int(bs, 1);
        }
    }
    spsInforHevc->temporal_mvp_enable_flag = (bool)gts_bs_read_int(bs, 1);
    spsInforHevc->strong_intra_smoothing_enable_flag = (bool)gts_bs_read_int(bs, 1);

    if (vui_flag_pos)
        *vui_flag_pos = (uint32_t)gts_bs_get_bit_offset(bs);

    spsInforHevc->vui_parameters_present_flag = (bool)gts_bs_read_int(bs, 1);
    if (spsInforHevc->vui_parameters_present_flag) {

        spsInforHevc->vui.aspect_ratio_info_present_flag = (bool)gts_bs_read_int(bs, 1);
        if (spsInforHevc->vui.aspect_ratio_info_present_flag) {
            spsInforHevc->vui.sar_idc = gts_bs_read_int(bs, 8);
            if (spsInforHevc->vui.sar_idc == 255) {
                spsInforHevc->vui.sar_width = gts_bs_read_int(bs, 16);
                spsInforHevc->vui.sar_height = gts_bs_read_int(bs, 16);
            } else if (spsInforHevc->vui.sar_idc<17) {
                spsInforHevc->vui.sar_width = hevc_sar[spsInforHevc->vui.sar_idc].w;
                spsInforHevc->vui.sar_height = hevc_sar[spsInforHevc->vui.sar_idc].h;
            }
        }
        spsInforHevc->vui.overscan_info_present = (bool)gts_bs_read_int(bs, 1);
        if (spsInforHevc->vui.overscan_info_present)
            spsInforHevc->vui.overscan_appropriate = (bool)gts_bs_read_int(bs, 1);

        spsInforHevc->vui.video_signal_type_present_flag =(bool)gts_bs_read_int(bs, 1);
        if (spsInforHevc->vui.video_signal_type_present_flag) {
            spsInforHevc->vui.video_format = gts_bs_read_int(bs, 3);
            spsInforHevc->vui.video_full_range_flag =(bool)gts_bs_read_int(bs, 1);
            spsInforHevc->vui.colour_description_present_flag = (bool)gts_bs_read_int(bs, 1);
            if (spsInforHevc->vui.colour_description_present_flag) {
                spsInforHevc->vui.colour_primaries =  gts_bs_read_int(bs, 8);
                spsInforHevc->vui.transfer_characteristic =  gts_bs_read_int(bs, 8);
                spsInforHevc->vui.matrix_coeffs =  gts_bs_read_int(bs, 8);
            }
        }
        spsInforHevc->vui.chroma_loc_info_present_flag = (bool)gts_bs_read_int(bs, 1);
        if (spsInforHevc->vui.chroma_loc_info_present_flag) {
            spsInforHevc->vui.chroma_sample_loc_type_top_field = bs_get_ue(bs);
            spsInforHevc->vui.chroma_sample_loc_type_bottom_field = bs_get_ue(bs);
        }

        spsInforHevc->vui.neutra_chroma_indication_flag =(bool)gts_bs_read_int(bs, 1);
        spsInforHevc->vui.field_seq_flag =(bool)gts_bs_read_int(bs, 1);
        spsInforHevc->vui.frame_field_info_present_flag =(bool)gts_bs_read_int(bs, 1);
        spsInforHevc->vui.default_display_window_flag = (bool)gts_bs_read_int(bs, 1);
        if (spsInforHevc->vui.default_display_window_flag) {
            spsInforHevc->vui.left_offset = bs_get_ue(bs);
            spsInforHevc->vui.right_offset = bs_get_ue(bs);
            spsInforHevc->vui.top_offset = bs_get_ue(bs);
            spsInforHevc->vui.bottom_offset = bs_get_ue(bs);
        }

        spsInforHevc->vui.has_timing_info = (bool)gts_bs_read_int(bs, 1);
        if (spsInforHevc->vui.has_timing_info ) {
            spsInforHevc->vui.num_units_in_tick = gts_bs_read_int(bs, 32);
            spsInforHevc->vui.time_scale = gts_bs_read_int(bs, 32);
            spsInforHevc->vui.poc_proportional_to_timing_flag = (bool)gts_bs_read_int(bs, 1);
            if (spsInforHevc->vui.poc_proportional_to_timing_flag)
                spsInforHevc->vui.num_ticks_poc_diff_one_minus1 = bs_get_ue(bs);
            if (gts_bs_read_int(bs, 1) ) {
                return sps_id;
            }
        }

        if (gts_bs_read_int(bs, 1)) {
            gts_bs_read_int(bs, 1);
            gts_bs_read_int(bs, 1);
            gts_bs_read_int(bs, 1);
            bs_get_ue(bs);
            bs_get_ue(bs);
            bs_get_ue(bs);
            bs_get_ue(bs);
            bs_get_ue(bs);
        }
    }

    if (gts_bs_read_int(bs, 1)) {
        while (gts_bs_available(bs)) {
            gts_bs_read_int(bs, 1);
        }
    }

    return sps_id;
}

static int32_t gts_media_hevc_read_pps_bs(GTS_BitStream *gts_bitstream, HEVCState *state_info)
{
    uint32_t i;
    int32_t index_hevc_pps = -1;
    HEVC_PPS *hevc_pps;

    //NAL header already read
    index_hevc_pps = bs_get_ue(gts_bitstream);

    if ((index_hevc_pps<0) || (index_hevc_pps>=64)) {
        return -1;
    }
    hevc_pps = &state_info->pps[index_hevc_pps];

    if (!hevc_pps->state) {
        hevc_pps->id = index_hevc_pps;
        hevc_pps->state = 1;
    }
    hevc_pps->sps_id = bs_get_ue(gts_bitstream);
    if (hevc_pps->sps_id>16) {
        //GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("[HEVC] wrong SPS ID %d in PPS\n", hevc_pps->sps_id));
        return -1;
    }
    state_info->sps_active_idx = hevc_pps->sps_id; /*set active sps*/
    hevc_pps->dependent_slice_segments_enabled_flag = (bool)gts_bs_read_int(gts_bitstream, 1);

    hevc_pps->output_flag_present_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
    hevc_pps->num_extra_slice_header_bits = gts_bs_read_int(gts_bitstream, 3);
    gts_bs_read_int(gts_bitstream, 1);
    hevc_pps->cabac_init_present_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
    hevc_pps->num_ref_idx_l0_default_active = 1 + bs_get_ue(gts_bitstream);
    hevc_pps->num_ref_idx_l1_default_active = 1 + bs_get_ue(gts_bitstream);
    hevc_pps->pic_init_qp_minus26 = bs_get_se(gts_bitstream);
    gts_bs_read_int(gts_bitstream, 1);
    gts_bs_read_int(gts_bitstream, 1);
    hevc_pps->cu_qp_delta_enabled_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
    if (hevc_pps->cu_qp_delta_enabled_flag )
        hevc_pps->diff_cu_qp_delta_depth = bs_get_ue(gts_bitstream);

    bs_get_se(gts_bitstream);
    bs_get_se(gts_bitstream);
    hevc_pps->slice_chroma_qp_offsets_present_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
    hevc_pps->weighted_pred_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
    hevc_pps->weighted_bipred_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
    gts_bs_read_int(gts_bitstream, 1);
    hevc_pps->tiles_enabled_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
    hevc_pps->entropy_coding_sync_enabled_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
    if (hevc_pps->tiles_enabled_flag) {
        hevc_pps->num_tile_columns = 1 + bs_get_ue(gts_bitstream);
        hevc_pps->num_tile_rows = 1 + bs_get_ue(gts_bitstream);
        state_info->tile_slice_count = hevc_pps->num_tile_columns * hevc_pps->num_tile_rows;
        hevc_pps->uniform_spacing_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
        if (!hevc_pps->uniform_spacing_flag ) {
            for (i=0; i<hevc_pps->num_tile_columns-1; i++) {
                hevc_pps->column_width[i] = 1 + bs_get_ue(gts_bitstream);
            }
            for (i=0; i<hevc_pps->num_tile_rows-1; i++) {
                hevc_pps->row_height[i] = 1+bs_get_ue(gts_bitstream);
            }
        }
        hevc_pps->loop_filter_across_tiles_enabled_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
    }
    else
    {
        state_info->tile_slice_count = 1;
    }
    hevc_pps->loop_filter_across_slices_enabled_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
    hevc_pps->deblocking_filter_control_present_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
    if (hevc_pps->deblocking_filter_control_present_flag) {
        hevc_pps->deblocking_filter_override_enabled_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
        if (! gts_bs_read_int(gts_bitstream, 1) ) {
            /*beta_offset_div2 = */bs_get_se(gts_bitstream);
            /*tc_offset_div2 = */bs_get_se(gts_bitstream);
        }
    }
    if (/*pic_scaling_list_data_present_flag    = */gts_bs_read_int(gts_bitstream, 1) ) {
        hevc_scaling_list_data(gts_bitstream);
    }
    hevc_pps->lists_modification_present_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
    /*log2_parallel_merge_level_minus2 = */bs_get_ue(gts_bitstream);
    hevc_pps->slice_segment_header_extension_present_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
    if ( /*pps_extension_flag= */gts_bs_read_int(gts_bitstream, 1) ) {
        while (gts_bs_available(gts_bitstream) ) {
            /*pps_extension_data_flag */ gts_bs_read_int(gts_bitstream, 1);
        }
    }
    return index_hevc_pps;
}

int32_t gts_media_hevc_parse_nalu(hevc_specialInfo* pSpecialInfo, int8_t *data, uint32_t size, HEVCState *hevc)
{
    GTS_BitStream *bs=NULL;
    int8_t *data_without_emulation_bytes = NULL;
    uint32_t data_without_emulation_bytes_size = 0;
    bool is_slice = false;
    int32_t ret = -1;
    HEVCSliceInfo *SliceInfo;
    uint8_t *nal_unit_type = &pSpecialInfo->naluType;
    uint8_t *temporal_id = &pSpecialInfo->temporal_id;
    uint8_t *layer_id = &pSpecialInfo->layer_id;
    uint16_t* slicehdrlen = &pSpecialInfo->sliceHeaderLen;
    uint16_t* payloadType = &pSpecialInfo->seiPayloadType;
    int32_t specialID = 0;

    SliceInfo = &hevc->s_info;

    hevc->s_info.entry_point_start_bits = -1;
    hevc->s_info.payload_start_offset = -1;

    data_without_emulation_bytes_size = gts_media_nalu_emulation_bytes_remove_count(data, size);
    if (!data_without_emulation_bytes_size) {
        bs = gts_bs_new(data, size, GTS_BITSTREAM_READ);
    } else {
        /*still contains emulation bytes*/
        data_without_emulation_bytes = (int8_t*) gts_malloc(size*sizeof(int8_t));
        if(data_without_emulation_bytes){
            data_without_emulation_bytes_size = gts_media_nalu_remove_emulation_bytes(data, data_without_emulation_bytes, size);
        }
        bs = gts_bs_new(data_without_emulation_bytes, data_without_emulation_bytes_size, GTS_BITSTREAM_READ);
    }
    if (!bs) goto exit;

    if (! hevc_parse_nal_header(bs, nal_unit_type, temporal_id, layer_id, payloadType)) goto exit;
    SliceInfo->nal_unit_type = *nal_unit_type;
    SliceInfo->temporal_id = *temporal_id;

    //switch (SliceInfo.nal_unit_type) {
    switch (SliceInfo->nal_unit_type) {
    case GTS_HEVC_NALU_ACCESS_UNIT:
    case GTS_HEVC_NALU_END_OF_SEQ:
    case GTS_HEVC_NALU_END_OF_STREAM:
        ret = 1;
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
        is_slice = true;
        ret = hevc_parse_slice_segment(bs, hevc, SliceInfo);
        if (ret<0)
            goto exit;
        ret = 0;

        *slicehdrlen = SliceInfo->payload_start_offset;
        hevc_compute_poc(SliceInfo);
        if (hevc->s_info.poc != SliceInfo->poc)
        {
            ret = 1;
            break;
        }
        if (SliceInfo->first_slice_segment_in_pic_flag)
        {
            if (!(*layer_id)
            || (SliceInfo->prev_layer_id_plus1 && ((*layer_id) <= SliceInfo->prev_layer_id_plus1 - 1)) )
            {
                ret = 1;
                break;
            }
        }
        break;
    case GTS_HEVC_NALU_SEQ_PARAM:
        specialID = gts_media_hevc_read_sps_bs(bs, hevc, *layer_id, NULL);
        hevc->last_parsed_sps_id = specialID;
        ret = 0;
        break;
    case GTS_HEVC_NALU_PIC_PARAM:
        specialID = gts_media_hevc_read_pps_bs(bs, hevc);
        hevc->last_parsed_pps_id = specialID;
        ret = 0;
        break;
    case GTS_HEVC_NALU_VID_PARAM:
        specialID = gts_media_hevc_read_vps_bs(bs, hevc, false);
        hevc->last_parsed_vps_id = specialID;
        ret = 0;
        break;
    default:
        ret = 0;
        break;
    }
    //save the previous values
    if (ret && hevc->s_info.sps) {
        SliceInfo->frame_num_offset_prev = hevc->s_info.frame_num_offset;
        SliceInfo->frame_num_prev = hevc->s_info.frame_num;

        SliceInfo->poc_lsb_prev = hevc->s_info.poc_lsb;
        SliceInfo->poc_msb_prev = hevc->s_info.poc_msb;
        SliceInfo->prev_layer_id_plus1 = *layer_id + 1;
    }
    if (is_slice)
    {
        hevc_compute_poc(SliceInfo);
    }

exit:
    if (bs) gts_bs_del(bs);
    if (data_without_emulation_bytes) gts_free(data_without_emulation_bytes);
    return ret;
}

int32_t gts_media_hevc_stitch_sps(HEVCState *hevc, uint32_t frameWidth, uint32_t frameHeight)
{
    HEVC_SPS *sps = &hevc->sps[0];//&hevc->sps[hevc->last_parsed_sps_id];
    sps->width  = frameWidth;
    sps->height = frameHeight;

    return 0;
}

int32_t gts_media_hevc_stitch_pps(HEVCState *hevc, void *gentiledstream)
{
    hevc_gen_tiledstream *pgentiledstream = (hevc_gen_tiledstream *)gentiledstream;
    uint32_t tilesWidthCount = pgentiledstream->outTilesWidthCount;
    uint32_t tilesHeightCount = pgentiledstream->outTilesHeightCount;
    if(tilesWidthCount < 1 || tilesHeightCount < 1)
        return -1;

    HEVC_PPS *pps = &hevc->pps[hevc->last_parsed_pps_id];
    pps->dependent_slice_segments_enabled_flag = (bool)false;
    pps->constrained_intra_pred_flag = (bool)0;
    pps->org_tiles_enabled_flag = pps->tiles_enabled_flag;
    pps->tiles_enabled_flag = (bool)(tilesWidthCount > 1 ||
                               tilesHeightCount > 1);
    pps->num_tile_columns = tilesWidthCount;
    pps->num_tile_rows = tilesHeightCount;
    pps->uniform_spacing_flag = (bool)pgentiledstream->tilesUniformSpacing;
    if(!pps->uniform_spacing_flag)
    {
        for (uint32_t i = 0; i < pps->num_tile_columns - 1; ++i)
        {
            pps->column_width[i] = pgentiledstream->columnWidth[i];
        }
        for (uint32_t i = 0; i < pps->num_tile_rows - 1; ++i)
        {
            pps->row_height[i] = pgentiledstream->rowHeight[i];
        }
    }

    pps->loop_filter_across_tiles_enabled_flag = (bool)false;
    pps->pps_loop_filter_across_slices_enabled_flag = (bool)0;

    return 0;
}

int32_t gts_media_hevc_stitch_slice_segment(HEVCState *hevc, void* slice, uint32_t frameWidth, uint32_t sub_tile_index)
{
    HEVCSliceInfo *si  = &hevc->s_info;
    oneStream_info *pSlice = (oneStream_info *)slice;

    if(pSlice->address == 0 && sub_tile_index == 0)
    {
        si->first_slice_segment_in_pic_flag = (bool)1;
    }
    else
    {
        si->first_slice_segment_in_pic_flag = (bool)0;
    }

    int32_t tile_ctu_column_count = pSlice->tilesWidthCount;
    if(sub_tile_index == 0)
    {
        si->slice_segment_address = (uint32_t)pSlice->address;
    }
    else
    {
        int32_t width_addr = 0, height_addr = 0;
        for(int32_t i = 0 ; i < (int32_t)sub_tile_index % tile_ctu_column_count ; i++)
        {
            width_addr += pSlice->columnWidth[i];
        }
        for(int32_t i = 0 ; i < (int32_t)sub_tile_index / tile_ctu_column_count ; i++)
        {
            height_addr += pSlice->rowHeight[i];
        }
        si->slice_segment_address = (uint32_t)pSlice->address + height_addr * frameWidth / LCU_SIZE + width_addr;
    }
    si->slice_qp_delta += hevc->pps[hevc->last_parsed_pps_id].pic_init_qp_minus26;

    return 0;
}

int32_t hevc_read_RwpkSEI(int8_t *pRWPKBits, uint32_t RWPKBitsSize, RegionWisePacking* pRWPK)
{
    int numRegions = 0;
    GTS_BitStream *bs = NULL;
    int8_t *data_without_emulation_bytes = NULL;
    uint32_t data_without_emulation_bytes_size = 0;
    int32_t ret = 0;
    unsigned int type = 0xff;
    unsigned short payloadSize = 0xff;
    int32_t start_code = 0;
    data_without_emulation_bytes_size = gts_media_nalu_emulation_bytes_remove_count(pRWPKBits, RWPKBitsSize);
    if (!data_without_emulation_bytes_size) {
        bs = gts_bs_new(pRWPKBits, RWPKBitsSize, GTS_BITSTREAM_READ);
    }
    else {
        /*still contains emulation bytes*/
        data_without_emulation_bytes = (int8_t*)gts_malloc(RWPKBitsSize * sizeof(int8_t));
        if (!data_without_emulation_bytes) goto exit;
        data_without_emulation_bytes_size = gts_media_nalu_remove_emulation_bytes(pRWPKBits, data_without_emulation_bytes, RWPKBitsSize);
        bs = gts_bs_new(data_without_emulation_bytes, data_without_emulation_bytes_size, GTS_BITSTREAM_READ);
    }
    if (!bs) goto exit;

    start_code = gts_bs_read_U32(bs);

    if (start_code != 0x00000001) {
        gts_bs_del(bs);
        return -1;
    }

    uint8_t nal_unit_type, temporal_id, layer_id;
    //uint16_t playload;
    if (!hevc_parse_nal_header(bs, &nal_unit_type, &temporal_id, &layer_id, &payloadSize))
        goto exit;
    if (nal_unit_type != GTS_HEVC_NALU_PREFIX_SEI)
        goto exit;

    while(type == 0xff)
    {
        type = gts_bs_read_int(bs, 8);
    }

    while (payloadSize == 0xff)
    {
        payloadSize = gts_bs_read_int(bs, 8);
    }

    // read region wise packing struct
    pRWPK->constituentPicMatching = (gts_bs_read_int(bs, 8) >> 7) & 0x1;
    numRegions = gts_bs_read_int(bs, 8);
    if (numRegions > DEFAULT_REGION_NUM)
       goto exit;
    pRWPK->numRegions = numRegions;
    //if caller set pRWPK->rectRegionPacking as null and didn't allocate memory
    //due to they don't know how many tiles there are,
    //the following code will malloc() for caller, finally the caller must free() memory after use
    if (!pRWPK->rectRegionPacking) {
        pRWPK->rectRegionPacking =
            (RectangularRegionWisePacking*)malloc(numRegions * sizeof(RectangularRegionWisePacking));
    }
    if (!pRWPK->rectRegionPacking)
        goto exit;

    pRWPK->projPicWidth = gts_bs_read_int(bs, 32);
    pRWPK->projPicHeight = gts_bs_read_int(bs, 32);
    pRWPK->packedPicWidth = gts_bs_read_int(bs, 16); //bitstr.read16Bits();
    pRWPK->packedPicHeight = gts_bs_read_int(bs, 16); //bitstr.read16Bits();

    for (int i = 0; i < pRWPK->numRegions; ++i)
    {
        RectangularRegionWisePacking region;
        uint8_t packed8Bits = gts_bs_read_int(bs, 8);
        memset_s(&region, sizeof(RectangularRegionWisePacking), 0);
        region.guardBandFlag = (packed8Bits >> 4) & 0x01;
        // read RectRegionPacking
        region.projRegWidth = gts_bs_read_int(bs, 32);
        region.projRegHeight = gts_bs_read_int(bs, 32);
        region.projRegTop = gts_bs_read_int(bs, 32);
        region.projRegLeft = gts_bs_read_int(bs, 32);
        region.transformType = packed8Bits >> 5;//gts_bs_read_int(bs, 8) >> 5;
        region.packedRegWidth = gts_bs_read_int(bs, 16);
        region.packedRegHeight = gts_bs_read_int(bs, 16);
        region.packedRegTop = gts_bs_read_int(bs, 16);
        region.packedRegLeft = gts_bs_read_int(bs, 16);
        if (region.guardBandFlag)
        {
            // read GuardBand
            region.leftGbWidth = gts_bs_read_int(bs, 8);
            region.rightGbWidth = gts_bs_read_int(bs, 8);
            region.topGbHeight = gts_bs_read_int(bs, 8);
            region.bottomGbHeight = gts_bs_read_int(bs, 8);

            uint16_t packed16Bits = gts_bs_read_int(bs, 16);
            region.gbNotUsedForPredFlag = packed16Bits >> 15 == 1;
            region.gbType0 = (packed16Bits >> 12) & 0x07;
            region.gbType1 = (packed16Bits >> 9) & 0x07;
            region.gbType2 = (packed16Bits >> 6) & 0x07;
            region.gbType3 = (packed16Bits >> 3) & 0x07;
        }
        memcpy_s(&pRWPK->rectRegionPacking[i], sizeof(RectangularRegionWisePacking), &region, sizeof(RectangularRegionWisePacking));
    }
    pRWPK->numHiRegions = gts_bs_read_int(bs, 8);
    pRWPK->lowResPicWidth = gts_bs_read_int(bs, 32);
    pRWPK->lowResPicHeight = gts_bs_read_int(bs, 32);
    pRWPK->timeStamp = gts_bs_read_int(bs, 32);

exit:
    if (bs) gts_bs_del(bs);
    if (data_without_emulation_bytes) gts_free(data_without_emulation_bytes);
    return ret;
}
