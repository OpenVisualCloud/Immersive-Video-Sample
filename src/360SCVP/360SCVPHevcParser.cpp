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

uint32_t gts_get_bit_size(uint32_t MaxVal)
{
    uint32_t k = 0;
    while ((int32_t)MaxVal > ((1 << k) - 1)) k += 1;
    return k;
}

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


static uint8_t digits_of_agm[256] = {
    8, 7, 6, 6, 5, 5, 5, 5,
    4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
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
    flag_c = digits_of_agm[flag_r];
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
    bool inter_ref_pic_set_prediction_flag = (bool)0;
    if (idx_rps != 0)
        inter_ref_pic_set_prediction_flag = (bool)gts_bs_read_int(bs, 1);

    if (inter_ref_pic_set_prediction_flag ) {
        HEVC_ReferencePictureSets *hevc_pic_set, *ref_pic_set;
        uint32_t index_num = 0;
        uint32_t index_reference;
        uint32_t delta_rps_sign;
        uint32_t index_num_absolute, reference_set_nb;
        int32_t reference_pic_del;
        uint32_t i = 0, i0 = 0, i1 = 0;
        if (idx_rps == sps->num_short_term_ref_pic_sets)
            index_num = bs_get_ue(bs);

        assert(index_num <= idx_rps - 1);
        index_reference = idx_rps - 1 - index_num;
        delta_rps_sign = (bool)gts_bs_read_int(bs, 1);
        index_num_absolute = bs_get_ue(bs);
        reference_pic_del = (1 - (delta_rps_sign<<1)) * (index_num_absolute + 1);

        ref_pic_set = &si->rps[idx_rps];
        hevc_pic_set = &si->rps[index_reference];
        reference_set_nb = hevc_pic_set->num_negative_pics + hevc_pic_set->num_positive_pics;
        for (i=0; i<=reference_set_nb; i++) {
            int32_t ref_idc;
            int32_t used_by_curr_pic_flag = (bool)gts_bs_read_int(bs, 1);
            ref_idc = used_by_curr_pic_flag ? 1 : 0;
            if ( !used_by_curr_pic_flag ) {
                used_by_curr_pic_flag = (bool)gts_bs_read_int(bs, 1);
                ref_idc = used_by_curr_pic_flag << 1;
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
        int32_t prev = 0, poc = 0;
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
            si->rps[idx_rps].delta_poc[i] = bs_get_ue(bs) + 1;
            si->used_by_curr_pic_s0_flag[i] = (bool)gts_bs_read_int(bs, 1);
        }
        for (i=0; i<si->rps[idx_rps].num_positive_pics; i++) {
            uint32_t delta_poc_s1_minus1 = bs_get_ue(bs);
            poc = prev + delta_poc_s1_minus1 + 1;
            prev = poc;
            si->rps[idx_rps].delta_poc[i] = poc;
            /*used_by_curr_pic_s1_flag[ i ] = */gts_bs_read_int(bs, 1);
        }
    }
    return true;
}

void hevc_pred_weight_table(GTS_BitStream *bs, HEVCState *hevc, HEVCSliceInfo *si, HEVC_PPS *pps, HEVC_SPS *sps, uint32_t num_ref_idx_l0_active, uint32_t num_ref_idx_l1_active)
{
    uint32_t i, num_ref_idx;
    bool first_pass=true;
    uint8_t luma_weights[20], chroma_weights[20];
    uint32_t ChromaArrayType = sps->separate_colour_plane_flag ? 0 : sps->chroma_format_idc;

    num_ref_idx = num_ref_idx_l0_active;

    /*luma_log2_weight_denom=*/i=bs_get_ue(bs);
    if (ChromaArrayType != 0)
        /*delta_chroma_log2_weight_denom=*/i=bs_get_se(bs);

parse_weights:
    for (i=0; i<num_ref_idx; i++) {
        luma_weights[i] = (bool)gts_bs_read_int(bs, 1);
        //infered to be 0 if not present
        chroma_weights[i] = 0;
    }
    if (ChromaArrayType != 0) {
        for (i=0; i<num_ref_idx; i++) {
            chroma_weights[i] = (bool)gts_bs_read_int(bs, 1);
        }
    }
    for (i=0; i<num_ref_idx; i++) {
        if (luma_weights[i]) {
            /*delta_luma_weight_l0[ i ]=*/bs_get_se(bs);
            /*luma_offset_l0[ i ]=*/bs_get_se(bs);
        }
        if (chroma_weights[i]) {
            /*delta_chroma_weight_l0[ i ][ 0 ]=*/bs_get_se(bs);
            /*delta_chroma_offset_l0[ i ][ 0 ]=*/bs_get_se(bs);

            /*delta_chroma_weight_l0[ i ][ 1 ]=*/bs_get_se(bs);
            /*delta_chroma_offset_l0[ i ][ 1 ]=*/bs_get_se(bs);
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
    uint32_t m, n;
    uint32_t count_index_reference_0=0, count_index_reference_1=0;
    HEVC_PPS *hevc_pps;
    HEVC_SPS *hevc_sps;
    int32_t index_hevc_pps;
    bool RapPicFlag = false;
    bool IDRPicFlag = false;

    slice_info->first_slice_segment_in_pic_flag = (bool)gts_bs_read_int(gts_bitstream, 1);

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
        /*bool no_output_of_prior_pics_flag = */gts_bs_read_int(gts_bitstream, 1);
    }

    index_hevc_pps = bs_get_ue(gts_bitstream);
    if (index_hevc_pps>=64)
        return -1;

    hevc_pps = &state->pps[index_hevc_pps];
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
        bool info_of_dfof= (bool)0;
        bool slice_sao_luma_flag= (bool)0;
        bool slice_sao_chroma_flag= (bool)0;
        bool info_of_sdfdf= (bool)0;

        //"slice_reserved_undetermined_flag[]"
        gts_bs_read_int(gts_bitstream, hevc_pps->num_extra_slice_header_bits);

        slice_info->slice_type = bs_get_ue(gts_bitstream);
        if (slice_info->slice_type == GF_HEVC_SLICE_TYPE_P)
            slice_info->slice_type = GF_HEVC_SLICE_TYPE_P;

        if(hevc_pps->output_flag_present_flag)
            /*pic_output_flag = */gts_bs_read_int(gts_bitstream, 1);

        if (hevc_sps->separate_colour_plane_flag == 1)
            /*colour_plane_id = */gts_bs_read_int(gts_bitstream, 2);

        if (IDRPicFlag) {
            slice_info->poc_lsb = 0;

            //if not asked to parse full header, abort since we know the poc
            //if (!state->full_slice_header_parse) return 0;

        } else
              {
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
                uint8_t poc_MCL_del[32];
                uint32_t count_hevc_lts = 0;
                uint32_t count_hevc_ltp = 0;
                if (hevc_sps->num_long_term_ref_pic_sps > 0 ) {
                    count_hevc_lts = bs_get_ue(gts_bitstream);
                }
                count_hevc_ltp = bs_get_ue(gts_bitstream);

                for (m = 0; m < count_hevc_lts + count_hevc_ltp; m++ ) {
                    if( m < count_hevc_lts ) {
                        if (hevc_sps->num_long_term_ref_pic_sps > 1)
                            /*uint8_t lt_idx_sps = */gts_bs_read_int(gts_bitstream, gts_get_bit_size(hevc_sps->num_long_term_ref_pic_sps) );
                    } else {
                        /*PocLsbLt[ m ] = */ gts_bs_read_int(gts_bitstream, hevc_sps->log2_max_pic_order_cnt_lsb);
                        /*UsedByCurrPicLt[ m ] = */ gts_bs_read_int(gts_bitstream, 1);
                    }
                    if (/*delta_poc_msb_present_flag[ m ] = */ gts_bs_read_int(gts_bitstream, 1) ) {
                        if( m == 0 || m == count_hevc_lts )
                            poc_MCL_del[m] = bs_get_ue(gts_bitstream);
                        else
                            poc_MCL_del[m] = bs_get_ue(gts_bitstream) + poc_MCL_del[m-1];
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

        if (slice_info->slice_type == GF_HEVC_SLICE_TYPE_P || slice_info->slice_type == GF_HEVC_SLICE_TYPE_B) {
            //uint32_t NumPocTotalCurr;
            count_index_reference_0 = hevc_pps->num_ref_idx_l0_default_active;
            count_index_reference_1 = 0;
            if (slice_info->slice_type == GF_HEVC_SLICE_TYPE_B)
                count_index_reference_1 = hevc_pps->num_ref_idx_l1_default_active;

            slice_info->num_ref_idx_active_override_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
            if (slice_info->num_ref_idx_active_override_flag) {
                count_index_reference_0 = 1 + bs_get_ue(gts_bitstream);
                if (slice_info->slice_type == GF_HEVC_SLICE_TYPE_B)
                    count_index_reference_1 = 1 + bs_get_ue(gts_bitstream);
            }

            if (hevc_pps->lists_modification_present_flag /*TODO: && NumPicTotalCurr > 1*/) {
                if (!ref_pic_lists_modification(gts_bitstream, slice_info->slice_type, count_index_reference_0, count_index_reference_1)) {
                    //GF_LOG(GF_LOG_WARNING, GF_LOG_CODING, ("[state] ref_pic_lists_modification( ) not implemented\n"));
                    return -1;
                }
            }

            if (slice_info->slice_type == GF_HEVC_SLICE_TYPE_B)
                /*mvd_l1_zero_flag=*/gts_bs_read_int(gts_bitstream, 1);
            if (hevc_pps->cabac_init_present_flag)
                /*cabac_init_flag=*/gts_bs_read_int(gts_bitstream, 1);

            if (slice_info->slice_temporal_mvp_enabled_flag) {
                // When info_of_cl0 is not present, it is inferred to be equal to 1.
                bool info_of_cl0 = (bool)1;
                if (slice_info->slice_type == GF_HEVC_SLICE_TYPE_B)
                    info_of_cl0 = (bool)gts_bs_read_int(gts_bitstream, 1);

                if ( (info_of_cl0 && (count_index_reference_0>1) )
                        || ( !info_of_cl0 && (count_index_reference_1>1) )
                   ) {
                    /*collocated_ref_idx=*/bs_get_ue(gts_bitstream);
                }
            }

            if ( (hevc_pps->weighted_pred_flag && slice_info->slice_type == GF_HEVC_SLICE_TYPE_P )
                    || ( hevc_pps->weighted_bipred_flag && slice_info->slice_type == GF_HEVC_SLICE_TYPE_B)
               ) {
                hevc_pred_weight_table(gts_bitstream, state, slice_info, hevc_pps, hevc_sps, count_index_reference_0, count_index_reference_1);
            }
            /*five_minus_max_num_merge_cand=*/bs_get_ue(gts_bitstream);
        }
        slice_info->slice_qp_delta = bs_get_se(gts_bitstream);
        if( hevc_pps->slice_chroma_qp_offsets_present_flag ) {
            slice_info->slice_cb_qp_offset= bs_get_se(gts_bitstream);
            slice_info->slice_cr_qp_offset= bs_get_se(gts_bitstream);
        }
        if ( hevc_pps->deblocking_filter_override_enabled_flag ) {
            info_of_dfof = (bool)gts_bs_read_int(gts_bitstream, 1);
        }

        if (info_of_dfof) {
            info_of_sdfdf = (bool)gts_bs_read_int(gts_bitstream, 1);
            if ( !info_of_sdfdf) {
                /*slice_beta_offset_div2=*/ bs_get_se(gts_bitstream);
                /*slice_tc_offset_div2=*/bs_get_se(gts_bitstream);
            }
        }
        if( hevc_pps->loop_filter_across_slices_enabled_flag
                && ( slice_sao_luma_flag || slice_sao_chroma_flag || !info_of_sdfdf )
          ) {
            /*slice_loop_filter_across_slices_enabled_flag = */gts_bs_read_int(gts_bitstream, 1);
        }
    }
    //dependent slice segment
    else {
        //if not asked to parse full header, abort
        //if (!state->full_slice_header_parse) return 0;
    }

    slice_info->entry_point_start_bits = ((uint32_t)gts_bs_get_position(gts_bitstream)-1)*8 + gts_bs_get_bit_position(gts_bitstream);

    if (hevc_pps->tiles_enabled_flag || hevc_pps->entropy_coding_sync_enabled_flag ) {
        uint32_t num_entry_point_offsets = bs_get_ue(gts_bitstream);
        if ( num_entry_point_offsets > 0) {
            uint32_t offset = bs_get_ue(gts_bitstream) + 1;
            uint32_t segments = offset >> 4;
            int32_t remain = (offset & 15);

            for (m=0; m<num_entry_point_offsets; m++) {
                uint32_t res = 0;
                for (n=0; n<segments; n++) {
                    res <<= 16;
                    res += gts_bs_read_int(gts_bitstream, 16);
                }
                if (remain) {
                    res <<= remain;
                    res += gts_bs_read_int(gts_bitstream, remain);
                }
                // entry_point_offset = val + 1; // +1; // +1 to get the size
            }
        }
    }

    if (hevc_pps->slice_segment_header_extension_present_flag) {
        uint32_t size_ext = bs_get_ue(gts_bitstream);
        while (size_ext) {
            gts_bs_read_int(gts_bitstream, 8);
            size_ext--;
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


void profile_tier_level(GTS_BitStream *bs, bool ProfilePresentFlag, uint8_t MaxNumSubLayersMinus1, HEVC_ProfileTierLevel *ptl)
{
    uint32_t i;
    if (ProfilePresentFlag) {
        ptl->profile_space = gts_bs_read_int(bs, 2);
        ptl->tier_flag = gts_bs_read_int(bs, 1);
        ptl->profile_idc = gts_bs_read_int(bs, 5);

        ptl->profile_compatibility_flag = gts_bs_read_int(bs, 32);

        ptl->general_progressive_source_flag = (bool)gts_bs_read_int(bs, 1);
        ptl->general_interlaced_source_flag = (bool)gts_bs_read_int(bs, 1);
        ptl->general_non_packed_constraint_flag = (bool)gts_bs_read_int(bs, 1);
        ptl->general_frame_only_constraint_flag = (bool)gts_bs_read_int(bs, 1);
        ptl->general_reserved_44bits = gts_bs_read_long_int(bs, 44);
    }
    ptl->level_idc = gts_bs_read_int(bs, 8);
    for (i=0; i<MaxNumSubLayersMinus1; i++) {
        ptl->sub_ptl[i].profile_present_flag = (bool)gts_bs_read_int(bs, 1);
        ptl->sub_ptl[i].level_present_flag = (bool)gts_bs_read_int(bs, 1);
    }
    if (MaxNumSubLayersMinus1>0) {
        for (i=MaxNumSubLayersMinus1; i<8; i++) {
            /*reserved_zero_2bits*/gts_bs_read_int(bs, 2);
        }
    }

    for (i=0; i<MaxNumSubLayersMinus1; i++) {
        if (ptl->sub_ptl[i].profile_present_flag) {
            ptl->sub_ptl[i].profile_space = gts_bs_read_int(bs, 2);
            ptl->sub_ptl[i].tier_flag = (bool)gts_bs_read_int(bs, 1);
            ptl->sub_ptl[i].profile_idc = gts_bs_read_int(bs, 5);
            ptl->sub_ptl[i].profile_compatibility_flag = gts_bs_read_int(bs, 32);
            /*ptl->sub_ptl[i].progressive_source_flag =*/ gts_bs_read_int(bs, 1);
            /*ptl->sub_ptl[i].interlaced_source_flag =*/ gts_bs_read_int(bs, 1);
            /*ptl->sub_ptl[i].non_packed_constraint_flag =*/ gts_bs_read_int(bs, 1);
            /*ptl->sub_ptl[i].frame_only_constraint_flag =*/ gts_bs_read_int(bs, 1);
            /*ptl->sub_ptl[i].reserved_44bits =*/ gts_bs_read_long_int(bs, 44);
        }
        if (ptl->sub_ptl[i].level_present_flag)
            ptl->sub_ptl[i].level_idc = gts_bs_read_int(bs, 8);
    }
}

static uint32_t scalability_type_to_idx(HEVC_VPS *vps, uint32_t scalability_type)
{
    uint32_t idx = 0, type;
    for (type=0; type < scalability_type; type++) {
        idx += (vps->scalability_mask[type] ? 1 : 0 );
    }
    return idx;
}

#define LHVC_VIEW_ORDER_INDEX  1
#define LHVC_SCALABILITY_INDEX    2

static uint32_t lhvc_get_scalability_id(HEVC_VPS *vps, uint32_t layer_id_in_vps, uint32_t scalability_type )
{
    uint32_t idx;
    if (!vps->scalability_mask[scalability_type]) return 0;
    idx = scalability_type_to_idx(vps, scalability_type);
    return vps->dimension_id[layer_id_in_vps][idx];
}

static uint32_t lhvc_get_view_index(HEVC_VPS *vps, uint32_t id)
{
    return lhvc_get_scalability_id(vps, vps->layer_id_in_vps[id], LHVC_VIEW_ORDER_INDEX);
}

static uint32_t lhvc_get_num_views(HEVC_VPS *vps)
{
    uint32_t numViews = 1, i;
    for (i=0; i<vps->max_layers; i++ ) {
        uint32_t layer_id = vps->layer_id_in_nuh[i];
        if (i>0 && ( lhvc_get_view_index( vps, layer_id) != lhvc_get_scalability_id( vps, i-1, LHVC_VIEW_ORDER_INDEX) )) {
            numViews++;
        }
    }
    return numViews;
}

static void lhvc_parse_rep_format(HEVC_RepFormat *fmt, GTS_BitStream *bs)
{
    uint8_t chroma_bitdepth_present_flag = 0;
    fmt->pic_width_luma_samples = gts_bs_read_int(bs, 16);
    fmt->pic_height_luma_samples = gts_bs_read_int(bs, 16);
    chroma_bitdepth_present_flag = gts_bs_read_int(bs, 1);
    if (chroma_bitdepth_present_flag) {
        fmt->chroma_format_idc = gts_bs_read_int(bs, 2);

        if (fmt->chroma_format_idc == 3)
            fmt->separate_colour_plane_flag = gts_bs_read_int(bs, 1);
        fmt->bit_depth_luma = 8 + gts_bs_read_int(bs, 4);
        fmt->bit_depth_chroma = 8 + gts_bs_read_int(bs, 4);
    }
    if (/*conformance_window_vps_flag*/ gts_bs_read_int(bs, 1)) {
        /*conf_win_vps_left_offset*/bs_get_ue(bs);
        /*conf_win_vps_right_offset*/bs_get_ue(bs);
        /*conf_win_vps_top_offset*/bs_get_ue(bs);
        /*conf_win_vps_bottom_offset*/bs_get_ue(bs);
    }
}


static bool hevc_parse_vps_extension(HEVC_VPS *vps, GTS_BitStream *bs)
{
    uint8_t splitting_flag, vps_nuh_layer_id_present_flag, view_id_len;
    uint32_t i, j, num_scalability_types, num_add_olss, num_add_layer_set, num_indepentdent_layers, nb_bits, default_output_layer_idc=0;
    uint8_t dimension_id_len[16], dim_bit_offset[16];
    uint8_t /*avc_base_layer_flag, */NumLayerSets, /*default_one_target_output_layer_flag, */rep_format_idx_present_flag, ols_ids_to_ls_idx;
    uint8_t layer_set_idx_for_ols_minus1[MAX_LHVC_LAYERS];
    uint8_t nb_output_layers_in_output_layer_set[MAX_LHVC_LAYERS+1];
    uint8_t ols_highest_output_layer_id[MAX_LHVC_LAYERS+1];

    uint32_t k,d, r, p, iNuhLId, jNuhLId;
    uint8_t num_direct_ref_layers[64], num_pred_layers[64], num_layers_in_tree_partition[MAX_LHVC_LAYERS];
    uint8_t dependency_flag[MAX_LHVC_LAYERS][MAX_LHVC_LAYERS], id_pred_layers[64][MAX_LHVC_LAYERS];
//    uint8_t num_ref_layers[64];
//    uint8_t tree_partition_layer_id[MAX_LHVC_LAYERS][MAX_LHVC_LAYERS];
//    uint8_t id_ref_layers[64][MAX_LHVC_LAYERS];
//    uint8_t id_direct_ref_layers[64][MAX_LHVC_LAYERS];
    uint8_t layer_id_in_list_flag[64];
    bool OutputLayerFlag[MAX_LHVC_LAYERS][MAX_LHVC_LAYERS];

    vps->vps_extension_found=(bool)1;
    if ((vps->max_layers > 1) && vps->base_layer_internal_flag)
        profile_tier_level(bs, (bool)0, vps->max_sub_layers-1, &vps->ext_ptl[0]);

    splitting_flag = gts_bs_read_int(bs, 1);
    num_scalability_types = 0;
    for (i=0; i<16; i++) {
        vps->scalability_mask[i] = gts_bs_read_int(bs, 1);
        num_scalability_types += vps->scalability_mask[i];
    }
    if (num_scalability_types>=16) {
        num_scalability_types=16;
    }
    dimension_id_len[0] = 0;
    for (i=0; i<(num_scalability_types - splitting_flag); i++) {
        dimension_id_len[i] = 1 + gts_bs_read_int(bs, 3);
    }

    if (splitting_flag) {
        for (i = 0; i < num_scalability_types; i++) {
            dim_bit_offset[i] = 0;
            for (j = 0; j < i; j++)
                dim_bit_offset[i] +=  dimension_id_len[j];
        }
        dimension_id_len[num_scalability_types-1] = 1 + (5 - dim_bit_offset[num_scalability_types-1]);
        dim_bit_offset[num_scalability_types] = 6;
    }

    vps_nuh_layer_id_present_flag = gts_bs_read_int(bs, 1);
    vps->layer_id_in_nuh[0] = 0;
    vps->layer_id_in_vps[0] = 0;
    for (i=1; i<vps->max_layers; i++) {
        if (vps_nuh_layer_id_present_flag) {
            vps->layer_id_in_nuh[i] = gts_bs_read_int(bs, 6);
        } else {
            vps->layer_id_in_nuh[i] = i;
        }
        vps->layer_id_in_vps[vps->layer_id_in_nuh[i]] = i;

        if (!splitting_flag) {
            for (j=0; j<num_scalability_types; j++) {
                vps->dimension_id[i][j] = gts_bs_read_int(bs, dimension_id_len[j]);
            }
        }
    }

    if (splitting_flag) {
        for (i = 0; i<vps->max_layers; i++)
            for (j=0; j<num_scalability_types; j++)
                vps->dimension_id[i][j] = ((vps->layer_id_in_nuh[i] & ((1 << dim_bit_offset[j+1]) -1)) >> dim_bit_offset[j]);
    } else {
        for (j=0; j<num_scalability_types; j++)
            vps->dimension_id[0][j] = 0;
    }

    view_id_len = gts_bs_read_int(bs, 4);
    if (view_id_len > 0) {
        for( i = 0; i < lhvc_get_num_views(vps); i++ ) {
            /*m_viewIdVal[i] = */ gts_bs_read_int(bs, view_id_len);
        }
    }

    for (i=1; i<vps->max_layers; i++) {
        for (j=0; j<i; j++) {
            vps->direct_dependency_flag[i][j] = gts_bs_read_int(bs, 1);
        }
    }

    //we do the test on MAX_LHVC_LAYERS and break in the loop to avoid a wrong GCC 4.8 warning on array bounds
    for (i = 0; i < MAX_LHVC_LAYERS; i++) {
        if (i >= vps->max_layers) break;
        for (j = 0; j < vps->max_layers; j++) {
            dependency_flag[i][j] = vps->direct_dependency_flag[i][j];
            for (k = 0; k < i; k++)
                if (vps->direct_dependency_flag[i][k] && vps->direct_dependency_flag[k][j])
                    dependency_flag[i][j] = 1;
        }
    }

    for (i = 0; i < vps->max_layers; i++) {
        iNuhLId = vps->layer_id_in_nuh[i];
        d = r = p = 0;
        for (j = 0; j < vps->max_layers; j++) {
            jNuhLId = vps->layer_id_in_nuh[j];
            if (vps->direct_dependency_flag[i][j]) {
//                id_direct_ref_layers[iNuhLId][d] = jNuhLId;
                d++;
            }
            if (dependency_flag[i][j]) {
//                id_ref_layers[iNuhLId][r] = jNuhLId;
                r++;
            }

            if (dependency_flag[j][i])
                id_pred_layers[iNuhLId][p++] = jNuhLId;
        }
        num_direct_ref_layers[iNuhLId] = d;
//        num_ref_layers[iNuhLId] = r;
        num_pred_layers[iNuhLId] = p;
    }

    memset(layer_id_in_list_flag, 0, 64*sizeof(uint8_t));
    k = 0; //num_indepentdent_layers
    for (i = 0; i < vps->max_layers; i++) {
        iNuhLId = vps->layer_id_in_nuh[i];
        if (!num_direct_ref_layers[iNuhLId]) {
            uint32_t h = 1;
            //tree_partition_layer_id[k][0] = iNuhLId;
            for (j = 0; j < num_pred_layers[iNuhLId]; j++) {
                uint32_t predLId = id_pred_layers[iNuhLId][j];
                if (!layer_id_in_list_flag[predLId]) {
                    //tree_partition_layer_id[k][h++] = predLId;
                    layer_id_in_list_flag[predLId] = 1;
                }
            }
            num_layers_in_tree_partition[k++] = h;
        }
    }
    num_indepentdent_layers = k;

    num_add_layer_set = 0;
    if (num_indepentdent_layers > 1)
        num_add_layer_set = bs_get_ue(bs);

    for (i = 0; i < num_add_layer_set; i++)
        for (j = 1; j < num_indepentdent_layers; j++) {
            nb_bits =1;
             while ((1 << nb_bits) < (num_layers_in_tree_partition[j] + 1))
                nb_bits++;
            /*highest_layer_idx_plus1[i][j]*/gts_bs_read_int(bs, nb_bits);
        }


    if (/*vps_sub_layers_max_minus1_present_flag*/gts_bs_read_int(bs, 1)) {
        for (i = 0; i < vps->max_layers; i++) {
            /*sub_layers_vps_max_minus1[ i ]*/gts_bs_read_int(bs, 3);
        }
    }

    if (/*max_tid_ref_present_flag = */gts_bs_read_int(bs, 1)) {
        for (i=0; i<(vps->max_layers-1) ; i++) {
            for (j= i+1; j < vps->max_layers; j++) {
                if (vps->direct_dependency_flag[j][i])
                    /*max_tid_il_ref_pics_plus1[ i ][ j ]*/gts_bs_read_int(bs, 3);
            }
        }
    }
    /*default_ref_layers_active_flag*/gts_bs_read_int(bs, 1);

    vps->num_profile_tier_level = 1+bs_get_ue(bs);
    if (vps->num_profile_tier_level > MAX_LHVC_LAYERS) {
        //GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("[HEVC] Wrong number of PTLs in VPS %d\n", vps->num_profile_tier_level));
        vps->num_profile_tier_level=1;
        return false;
    }

    for (i=vps->base_layer_internal_flag ? 2 : 1; i < vps->num_profile_tier_level; i++) {
        bool vps_profile_present_flag = (bool)gts_bs_read_int(bs, 1);
        profile_tier_level(bs, vps_profile_present_flag, vps->max_sub_layers-1, &vps->ext_ptl[i-1] );
    }

    NumLayerSets = vps->num_layer_sets + num_add_layer_set;
    num_add_olss = 0;

    if (NumLayerSets > 1) {
        num_add_olss = bs_get_ue(bs);
        default_output_layer_idc = gts_bs_read_int(bs,2);
        default_output_layer_idc = default_output_layer_idc < 2 ? default_output_layer_idc : 2;
    }
    vps->num_output_layer_sets = num_add_olss + NumLayerSets;


    layer_set_idx_for_ols_minus1[0] = 1;
    vps->output_layer_flag[0][0] = (bool)1;

    for (i = 0; i < vps->num_output_layer_sets; i++) {
        if ((NumLayerSets > 2) && (i >= NumLayerSets)) {
            nb_bits = 1;
            while ((1 << nb_bits) < (NumLayerSets - 1))
                nb_bits++;
            layer_set_idx_for_ols_minus1[i] = gts_bs_read_int(bs, nb_bits);
        }
        else
            layer_set_idx_for_ols_minus1[i] = 0;
        ols_ids_to_ls_idx = i < NumLayerSets ? i : layer_set_idx_for_ols_minus1[i] + 1;

        if ((i > (vps->num_layer_sets - 1)) || (default_output_layer_idc == 2)) {
            for (j = 0; j < vps->num_layers_in_id_list[ols_ids_to_ls_idx]; j++)
                vps->output_layer_flag[i][j] = (bool)gts_bs_read_int(bs, 1);
        }

        if ((default_output_layer_idc == 0) || (default_output_layer_idc == 1)) {
            for (j = 0; j < vps->num_layers_in_id_list[ols_ids_to_ls_idx]; j++) {
                if ((default_output_layer_idc == 0) || (vps->LayerSetLayerIdList[i][j] == vps->LayerSetLayerIdListMax[i]))
                    OutputLayerFlag[i][j] = true;
                else
                    OutputLayerFlag[i][j] = false;
            }
        }

        for (j = 0; j < vps->num_layers_in_id_list[ols_ids_to_ls_idx]; j++) {
            if (OutputLayerFlag[i][j]) {
                uint32_t curLayerID, k;
                vps->necessary_layers_flag[i][j] = true;
                curLayerID = vps->LayerSetLayerIdList[i][j];
                for (k = 0; k < j; k++) {
                    uint32_t refLayerId = vps->LayerSetLayerIdList[i][k];
                    if (dependency_flag[vps->layer_id_in_vps[curLayerID]][vps->layer_id_in_vps[refLayerId]])
                        vps->necessary_layers_flag[i][k] = true;
                }
            }
        }
        vps->num_necessary_layers[i] = 0;
        for (j = 0; j < vps->num_layers_in_id_list[ols_ids_to_ls_idx]; j++) {
            if (vps->necessary_layers_flag[i][j])
                vps->num_necessary_layers[i] += 1;
        }

        if (i == 0) {
            if (vps->base_layer_internal_flag) {
                if (vps->max_layers > 1)
                    vps->profile_tier_level_idx[0][0] = 1;
                else
                    vps->profile_tier_level_idx[0][0] = 0;
            }
            continue;
        }
        nb_bits = 1;
        while ((uint32_t)(1 << nb_bits) < vps->num_profile_tier_level)
            nb_bits++;
        for (j = 0; j < vps->num_layers_in_id_list[ols_ids_to_ls_idx]; j++)
            if (vps->necessary_layers_flag[i][j] && vps->num_profile_tier_level)
                vps->profile_tier_level_idx[i][j] = gts_bs_read_int(bs, nb_bits);
            else
                vps->profile_tier_level_idx[i][j] = 0;


        nb_output_layers_in_output_layer_set[i] = 0;
        ols_highest_output_layer_id[i] = 0;
        for (j = 0; j < vps->num_layers_in_id_list[ols_ids_to_ls_idx]; j++) {
            nb_output_layers_in_output_layer_set[i] += OutputLayerFlag[i][j];
            if (OutputLayerFlag[i][j]) {
                ols_highest_output_layer_id[i] = vps->LayerSetLayerIdList[ols_ids_to_ls_idx][j];
            }
        }
        if (nb_output_layers_in_output_layer_set[i] == 1 && ols_highest_output_layer_id[i] > 0)
             vps->alt_output_layer_flag[i] = (bool)gts_bs_read_int(bs, 1);
    }

    vps->num_rep_formats = 1 + bs_get_ue(bs);
    if (vps->num_rep_formats > 16) {
        //GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("[HEVC] Wrong number of rep formats in VPS %d\n", vps->num_rep_formats));
        vps->num_rep_formats = 0;
        return false;
    }

    for (i = 0; i < vps->num_rep_formats; i++) {
        lhvc_parse_rep_format(&vps->rep_formats[i], bs);
    }
    if (vps->num_rep_formats > 1)
        rep_format_idx_present_flag = gts_bs_read_int(bs, 1);
    else
        rep_format_idx_present_flag = 0;

    vps->rep_format_idx[0] = 0;
    nb_bits = 1;
    while ((uint32_t)(1 << nb_bits) < vps->num_rep_formats)
        nb_bits++;
    for (i = vps->base_layer_internal_flag ? 1 : 0; i < vps->max_layers; i++) {
        if (rep_format_idx_present_flag) {
            vps->rep_format_idx[i] = gts_bs_read_int(bs, nb_bits);
        }
        else {
            vps->rep_format_idx[i] = i < vps->num_rep_formats - 1 ? i : vps->num_rep_formats - 1;
        }
    }
//TODO - we don't use the rest ...

return true;
}

static void sub_layer_hrd_parameters(GTS_BitStream *bs, int32_t subLayerId, uint32_t cpb_cnt, bool sub_pic_hrd_params_present_flag) {
    uint32_t i;
    for (i = 0; i <= cpb_cnt; i++) {
        /*bit_rate_value_minus1[i] = */bs_get_ue(bs);
        /*cpb_size_value_minus1[i] = */bs_get_ue(bs);
        if (sub_pic_hrd_params_present_flag) {
            /*cpb_size_du_value_minus1[i] = */bs_get_ue(bs);
            /*bit_rate_du_value_minus1[i] = */bs_get_ue(bs);
        }
        /*cbr_flag[i] = */gts_bs_read_int(bs, 1);
    }
}

static void hevc_parse_hrd_parameters(GTS_BitStream *bs, bool commonInfPresentFlag, int32_t maxNumSubLayersMinus1)
{
    int32_t i;
    bool nal_hrd_parameters_present_flag = false;
    bool vcl_hrd_parameters_present_flag = false;
    bool sub_pic_hrd_params_present_flag = false;
    if (commonInfPresentFlag) {
        nal_hrd_parameters_present_flag = (bool)gts_bs_read_int(bs, 1);
        vcl_hrd_parameters_present_flag = (bool)gts_bs_read_int(bs, 1);
        if (nal_hrd_parameters_present_flag || vcl_hrd_parameters_present_flag) {
            sub_pic_hrd_params_present_flag = (bool)gts_bs_read_int(bs, 1);
            if (sub_pic_hrd_params_present_flag) {
                /*tick_divisor_minus2 = */gts_bs_read_int(bs, 8);
                /*du_cpb_removal_delay_increment_length_minus1 = */gts_bs_read_int(bs, 5);
                /*sub_pic_cpb_params_in_pic_timing_sei_flag = */gts_bs_read_int(bs, 1);
                /*dpb_output_delay_du_length_minus1 = */gts_bs_read_int(bs, 5);
            }
            /*bit_rate_scale = */gts_bs_read_int(bs, 4);
            /*cpb_size_scale = */gts_bs_read_int(bs, 4);
            if (sub_pic_hrd_params_present_flag) {
                /*cpb_size_du_scale = */gts_bs_read_int(bs, 4);
            }
            /*initial_cpb_removal_delay_length_minus1 = */gts_bs_read_int(bs, 5);
            /*au_cpb_removal_delay_length_minus1 = */gts_bs_read_int(bs, 5);
            /*dpb_output_delay_length_minus1 = */gts_bs_read_int(bs, 5);
        }
    }
    for (i = 0; i <= maxNumSubLayersMinus1; i++) {
        bool fixed_pic_rate_general_flag_i = (bool)gts_bs_read_int(bs, 1);
        bool fixed_pic_rate_within_cvs_flag_i = true;
        bool low_delay_hrd_flag_i = false;
        uint32_t cpb_cnt_minus1_i = 0;
        if (!fixed_pic_rate_general_flag_i) {
            fixed_pic_rate_within_cvs_flag_i = (bool)gts_bs_read_int(bs, 1);
        }
        if (fixed_pic_rate_within_cvs_flag_i)
            /*elemental_duration_in_tc_minus1[i] = */bs_get_ue(bs);
        else
            low_delay_hrd_flag_i = (bool)gts_bs_read_int(bs, 1);
        if (!low_delay_hrd_flag_i) {
            cpb_cnt_minus1_i = bs_get_ue(bs);
        }
        if (nal_hrd_parameters_present_flag) {
            sub_layer_hrd_parameters(bs, i, cpb_cnt_minus1_i, sub_pic_hrd_params_present_flag);
        }
        if (vcl_hrd_parameters_present_flag) {
            sub_layer_hrd_parameters(bs, i, cpb_cnt_minus1_i, sub_pic_hrd_params_present_flag);
        }
    }
}

static int32_t gts_media_hevc_read_vps_bs(GTS_BitStream *bs, HEVCState *hevc, bool stop_at_vps_ext)
{
    uint8_t vps_sub_layer_ordering_info_present_flag, vps_extension_flag;
    uint32_t i, j;
    int32_t vps_id = -1;
    HEVC_VPS *vps;
    uint8_t layer_id_included_flag[MAX_LHVC_LAYERS][64];

    //nalu header already parsed
    vps_id = gts_bs_read_int(bs, 4);

    if (vps_id>=16) return -1;

    vps = &hevc->vps[vps_id];
    vps->bit_pos_vps_extensions = -1;
    if (!vps->state) {
        vps->id = vps_id;
        vps->state = 1;
    }

    vps->base_layer_internal_flag = (bool)gts_bs_read_int(bs, 1);
    vps->base_layer_available_flag = (bool)gts_bs_read_int(bs, 1);
    vps->max_layers = 1 + gts_bs_read_int(bs, 6);
    if (vps->max_layers>MAX_LHVC_LAYERS) {
        //GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("[HEVC] sorry, %d layers in VPS but only %d supported\n", vps->max_layers, MAX_LHVC_LAYERS));
        return -1;
    }
    vps->max_sub_layers = gts_bs_read_int(bs, 3) + 1;
    vps->temporal_id_nesting = (bool)gts_bs_read_int(bs, 1);
    /* vps_reserved_ffff_16bits = */ gts_bs_read_int(bs, 16);
    profile_tier_level(bs, (bool)1, vps->max_sub_layers-1, &vps->ptl);

    vps_sub_layer_ordering_info_present_flag = gts_bs_read_int(bs, 1);
    for (i=(vps_sub_layer_ordering_info_present_flag ? 0 : vps->max_sub_layers - 1); i < vps->max_sub_layers; i++) {
        vps->vps_max_dec_pic_buffering_minus1 = bs_get_ue(bs);
        vps->vps_max_num_reorder_pics = bs_get_ue(bs);
        vps->vps_max_latency_increase_plus1 = bs_get_ue(bs);
    }
    vps->max_layer_id = gts_bs_read_int(bs, 6);
    if (vps->max_layer_id > MAX_LHVC_LAYERS) {
        //GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("[HEVC] VPS max layer ID %u but GPAC only supports %u\n", vps->max_layer_id,  MAX_LHVC_LAYERS));
        return -1;
    }
    vps->num_layer_sets = bs_get_ue(bs) + 1;
    if (vps->num_layer_sets > MAX_LHVC_LAYERS) {
        //GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("[HEVC] Wrong number of layer sets in VPS %d\n", vps->num_layer_sets));
        return -1;
    }
    for (i=1; i < vps->num_layer_sets; i++) {
        for (j=0; j <= vps->max_layer_id; j++) {
            layer_id_included_flag[ i ][ j ] = gts_bs_read_int(bs, 1);
        }
    }
    vps->num_layers_in_id_list[0] = 1;
    for (i = 1; i < vps->num_layer_sets; i++) {
        uint32_t n, m;
        n = 0;
        for (m = 0; m <= vps->max_layer_id; m++)
            if (layer_id_included_flag[i][m]) {
                if(n < MAX_LHVC_LAYERS)
                    vps->LayerSetLayerIdList[i][n++] = m;
                if (vps->LayerSetLayerIdListMax[i] < m)
                    vps->LayerSetLayerIdListMax[i] = m;
            }
        vps->num_layers_in_id_list[i] = n;
    }
    if (/*vps_timing_info_present_flag*/gts_bs_read_int(bs, 1)) {
        uint32_t vps_num_hrd_parameters;
        /*uint32_t vps_num_units_in_tick = */gts_bs_read_int(bs, 32);
        /*uint32_t vps_time_scale = */gts_bs_read_int(bs, 32);
        if (/*vps_poc_proportional_to_timing_flag*/gts_bs_read_int(bs, 1)) {
            /*vps_num_ticks_poc_diff_one_minus1*/bs_get_ue(bs);
        }
        vps_num_hrd_parameters = bs_get_ue(bs);
        for( i = 0; i < vps_num_hrd_parameters; i++ ) {
            bool cprms_present_flag = true;
            /*hrd_layer_set_idx[i] = */bs_get_ue(bs);
            if (i>0)
                cprms_present_flag = (bool)gts_bs_read_int(bs, 1) ;
            hevc_parse_hrd_parameters(bs, cprms_present_flag, vps->max_sub_layers - 1);
        }
    }
    if (stop_at_vps_ext) {
        return vps_id;
    }

    vps_extension_flag = gts_bs_read_int(bs, 1);
    if (vps_extension_flag ) {
        bool res;
        gts_bs_align(bs);
        res = hevc_parse_vps_extension(vps, bs);
        if (res!=true) {
            //GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("[HEVC] Failed to parse VPS extensions\n"));
            return -1;
        }
        if (/*vps_extension2_flag*/gts_bs_read_int(bs, 1)) {
            while (gts_bs_available(bs)) {
                /*vps_extension_data_flag */ gts_bs_read_int(bs, 1);
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
                /*scaling_list_pred_matrix_id_delta[ sizeId ][ matrixId ] =*/ bs_get_ue(bs);
            } else {
                //uint32_t nextCoef = 8;
                uint32_t coefNum = MIN(64, (1 << (4+(sizeId << 1))));
                if ( sizeId > 1 ) {
                    /*scaling_list_dc_coef_minuS8[ sizeId - 2 ][ matrixId ] = */bs_get_se(bs);
                }
                for (i = 0; i<coefNum; i++) {
                    /*scaling_list_delta_coef = */bs_get_se(bs);
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
    uint32_t log2_diff_max_min_luma_coding_block_size;
    uint32_t log2_min_transform_block_size, log2_min_luma_coding_block_size;
    bool sps_sub_layer_ordering_info_present_flag;
    HEVC_SPS *hevc_sps;
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
    memset(&ptl, 0, sizeof(ptl));
    max_sub_layers_minus1 = 0;
    sps_ext_or_max_sub_layers_minus1 = 0;
    if (layer_id == 0)
        max_sub_layers_minus1 = gts_bs_read_int(bs, 3);
    else
        sps_ext_or_max_sub_layers_minus1 = gts_bs_read_int(bs, 3);
    multiLayerExtSpsFlag = (bool)((layer_id != 0) && (sps_ext_or_max_sub_layers_minus1 == 7));
    if (!multiLayerExtSpsFlag) {
        /*temporal_id_nesting_flag = */gts_bs_read_int(bs, 1);
        profile_tier_level(bs, (bool)1, max_sub_layers_minus1, &ptl);
    }

    sps_id = bs_get_ue(bs);
    if ((sps_id<0) ||(sps_id>=16)) {
        return -1;
    }

    hevc_sps = &hevc->sps[sps_id];
    if (!hevc_sps->state) {
        hevc_sps->state = 1;
        hevc_sps->id = sps_id;
        hevc_sps->vps_id = vps_id;
    }
    hevc_sps->ptl = ptl;
    vps = &hevc->vps[vps_id];

    hevc_sps->max_sub_layers_minus1 = max_sub_layers_minus1;

    //sps_rep_format_idx = 0;
    if (multiLayerExtSpsFlag) {
        uint8_t update_rep_format_flag = gts_bs_read_int(bs, 1);
        if (update_rep_format_flag) {
            hevc_sps->rep_format_idx = gts_bs_read_int(bs, 8);
        } else {
            hevc_sps->rep_format_idx = vps->rep_format_idx[layer_id];
        }
        hevc_sps->width = vps->rep_formats[hevc_sps->rep_format_idx].pic_width_luma_samples;
        hevc_sps->height = vps->rep_formats[hevc_sps->rep_format_idx].pic_height_luma_samples;
        hevc_sps->chroma_format_idc = vps->rep_formats[hevc_sps->rep_format_idx].chroma_format_idc;
        hevc_sps->bit_depth_luma = vps->rep_formats[hevc_sps->rep_format_idx].bit_depth_luma;
        hevc_sps->bit_depth_chroma = vps->rep_formats[hevc_sps->rep_format_idx].bit_depth_chroma;
        hevc_sps->separate_colour_plane_flag = (bool)vps->rep_formats[hevc_sps->rep_format_idx].separate_colour_plane_flag;

        //TODO this is crude ...
        hevc_sps->ptl = vps->ext_ptl[0];
    } else {
        hevc_sps->chroma_format_idc = bs_get_ue(bs);
        if (hevc_sps->chroma_format_idc==3)
            hevc_sps->separate_colour_plane_flag = (bool)gts_bs_read_int(bs, 1);
        hevc_sps->width = bs_get_ue(bs);
        hevc_sps->height = bs_get_ue(bs);
        if (/*conformance_window_flag*/gts_bs_read_int(bs, 1)) {
            uint32_t width_sub, height_sub;

            if (hevc_sps->chroma_format_idc==1) {
                width_sub = height_sub = 2;
            }
            else if (hevc_sps->chroma_format_idc==2) {
                width_sub = 2;
                height_sub = 1;
            } else {
                width_sub = height_sub = 1;
            }

            hevc_sps->cw_left = bs_get_ue(bs);
            hevc_sps->cw_right = bs_get_ue(bs);
            hevc_sps->cw_top = bs_get_ue(bs);
            hevc_sps->cw_bottom = bs_get_ue(bs);

            hevc_sps->width -= width_sub * (hevc_sps->cw_left + hevc_sps->cw_right);
            hevc_sps->height -= height_sub * (hevc_sps->cw_top + hevc_sps->cw_bottom);
        }
        hevc_sps->bit_depth_luma = 8 + bs_get_ue(bs);
        hevc_sps->bit_depth_chroma = 8 + bs_get_ue(bs);
    }

    hevc_sps->log2_max_pic_order_cnt_lsb = 4 + bs_get_ue(bs);

    if (!multiLayerExtSpsFlag) {
        sps_sub_layer_ordering_info_present_flag = (bool)gts_bs_read_int(bs, 1);
        for(i=sps_sub_layer_ordering_info_present_flag ? 0 : hevc_sps->max_sub_layers_minus1; i<=hevc_sps->max_sub_layers_minus1; i++) {
            hevc_sps->max_dec_pic_buffering =  bs_get_ue(bs);
            hevc_sps->num_reorder_pics =  bs_get_ue(bs);
            hevc_sps->max_latency_increase =  bs_get_ue(bs);
        }
    }

    log2_min_luma_coding_block_size = 3 + bs_get_ue(bs);
    log2_diff_max_min_luma_coding_block_size = bs_get_ue(bs);
    hevc_sps->max_CU_width = ( 1<<(log2_min_luma_coding_block_size + log2_diff_max_min_luma_coding_block_size) );
    hevc_sps->max_CU_height = ( 1<<(log2_min_luma_coding_block_size + log2_diff_max_min_luma_coding_block_size) );

    log2_min_transform_block_size = 2 + bs_get_ue(bs);
    /*log2_max_transform_block_size = log2_min_transform_block_size  + */bs_get_ue(bs);

    depth = 0;
    hevc_sps->max_transform_hierarchy_depth_inter = bs_get_ue(bs);
    hevc_sps->max_transform_hierarchy_depth_intra = bs_get_ue(bs);
    while( (uint32_t) ( hevc_sps->max_CU_width >> log2_diff_max_min_luma_coding_block_size ) > (uint32_t) ( 1 << ( log2_min_transform_block_size + depth )  ) )
    {
        depth++;
    }
    hevc_sps->max_CU_depth = log2_diff_max_min_luma_coding_block_size + depth;

    nb_CTUs = ((hevc_sps->width + hevc_sps->max_CU_width -1) / hevc_sps->max_CU_width) * ((hevc_sps->height + hevc_sps->max_CU_height-1) / hevc_sps->max_CU_height);
    hevc_sps->bitsSliceSegmentAddress = 0;
    while (nb_CTUs > (uint32_t) (1 << hevc_sps->bitsSliceSegmentAddress)) {
        hevc_sps->bitsSliceSegmentAddress++;
    }

    scaling_list_enable_flag = (bool)gts_bs_read_int(bs, 1);
    if (scaling_list_enable_flag) {
        bool sps_infer_scaling_list_flag = (bool)0;
        /*uint8_t sps_scaling_list_ref_layer_id = 0;*/
        if (multiLayerExtSpsFlag) {
            sps_infer_scaling_list_flag = (bool)gts_bs_read_int(bs, 1);
        }

        if (sps_infer_scaling_list_flag) {
            /*sps_scaling_list_ref_layer_id = */gts_bs_read_int(bs, 6);
        } else {
            if (/*sps_scaling_list_data_present_flag=*/gts_bs_read_int(bs, 1) ) {
                hevc_scaling_list_data(bs);
            }
        }
    }
    /*asymmetric_motion_partitions_enabled_flag= */ gts_bs_read_int(bs, 1);
    hevc_sps->sample_adaptive_offset_enabled_flag = (bool)gts_bs_read_int(bs, 1);
    if (/*pcm_enabled_flag= */ gts_bs_read_int(bs, 1) ) {
        /*pcm_sample_bit_depth_luma_minus1=*/gts_bs_read_int(bs, 4);
        /*pcm_sample_bit_depth_chroma_minus1=*/gts_bs_read_int(bs, 4);
        /*log2_min_pcm_luma_coding_block_size_minus3= */ bs_get_ue(bs);
        /*log2_diff_max_min_pcm_luma_coding_block_size = */ bs_get_ue(bs);
        /*pcm_loop_filter_disable_flag=*/gts_bs_read_int(bs, 1);
    }
    hevc_sps->num_short_term_ref_pic_sets = bs_get_ue(bs);
    if (hevc_sps->num_short_term_ref_pic_sets>64) {
        //GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("[HEVC] Invalid number of short term reference picture sets %d\n", hevc_sps->num_short_term_ref_pic_sets));
        return -1;
    }
/*
    for (i=0; i<hevc_sps->num_short_term_ref_pic_sets; i++) {
        bool ret = parse_short_term_ref_pic_set(bs, hevc_sps, &hevc->s_info, i);
        //cannot parse short_term_ref_pic_set, skip VUI parsing
        if (!ret) {
            //GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("[HEVC] Invalid short_term_ref_pic_set\n"));
            return -1;
        }
    }
*/
    hevc_sps->long_term_ref_pics_present_flag = (bool)gts_bs_read_int(bs, 1);
    if (hevc_sps->long_term_ref_pics_present_flag) {
        hevc_sps->num_long_term_ref_pic_sps = bs_get_ue(bs);
        for (i=0; i<hevc_sps->num_long_term_ref_pic_sps; i++) {
            /*lt_ref_pic_poc_lsb_sps=*/gts_bs_read_int(bs, hevc_sps->log2_max_pic_order_cnt_lsb);
            /*used_by_curr_pic_lt_sps_flag*/gts_bs_read_int(bs, 1);
        }
    }
    hevc_sps->temporal_mvp_enable_flag = (bool)gts_bs_read_int(bs, 1);
    hevc_sps->strong_intra_smoothing_enable_flag = (bool)gts_bs_read_int(bs, 1);

    if (vui_flag_pos)
        *vui_flag_pos = (uint32_t)gts_bs_get_bit_offset(bs);

    hevc_sps->vui_parameters_present_flag = (bool)gts_bs_read_int(bs, 1);
    if (hevc_sps->vui_parameters_present_flag) {

        hevc_sps->vui.aspect_ratio_info_present_flag = (bool)gts_bs_read_int(bs, 1);
        if (hevc_sps->vui.aspect_ratio_info_present_flag) {
            hevc_sps->vui.sar_idc = gts_bs_read_int(bs, 8);
            if (hevc_sps->vui.sar_idc == 255) {
                hevc_sps->vui.sar_width = gts_bs_read_int(bs, 16);
                hevc_sps->vui.sar_height = gts_bs_read_int(bs, 16);
            } else if (hevc_sps->vui.sar_idc<17) {
                hevc_sps->vui.sar_width = hevc_sar[hevc_sps->vui.sar_idc].w;
                hevc_sps->vui.sar_height = hevc_sar[hevc_sps->vui.sar_idc].h;
            }
        }
        hevc_sps->vui.overscan_info_present = (bool)gts_bs_read_int(bs, 1);
        if (hevc_sps->vui.overscan_info_present)
            hevc_sps->vui.overscan_appropriate = (bool)gts_bs_read_int(bs, 1);

        hevc_sps->vui.video_signal_type_present_flag =(bool)gts_bs_read_int(bs, 1);
        if (hevc_sps->vui.video_signal_type_present_flag) {
            hevc_sps->vui.video_format = gts_bs_read_int(bs, 3);
            hevc_sps->vui.video_full_range_flag =(bool)gts_bs_read_int(bs, 1);
            hevc_sps->vui.colour_description_present_flag = (bool)gts_bs_read_int(bs, 1);
            if (hevc_sps->vui.colour_description_present_flag) {
                hevc_sps->vui.colour_primaries =  gts_bs_read_int(bs, 8);
                hevc_sps->vui.transfer_characteristic =  gts_bs_read_int(bs, 8);
                hevc_sps->vui.matrix_coeffs =  gts_bs_read_int(bs, 8);
            }
        }
        hevc_sps->vui.chroma_loc_info_present_flag = (bool)gts_bs_read_int(bs, 1);
        if (hevc_sps->vui.chroma_loc_info_present_flag) {
            hevc_sps->vui.chroma_sample_loc_type_top_field = bs_get_ue(bs);
            hevc_sps->vui.chroma_sample_loc_type_bottom_field = bs_get_ue(bs);
        }

        hevc_sps->vui.neutra_chroma_indication_flag =(bool)gts_bs_read_int(bs, 1);
        hevc_sps->vui.field_seq_flag =(bool)gts_bs_read_int(bs, 1);
        hevc_sps->vui.frame_field_info_present_flag =(bool)gts_bs_read_int(bs, 1);
        hevc_sps->vui.default_display_window_flag = (bool)gts_bs_read_int(bs, 1);
        if (hevc_sps->vui.default_display_window_flag) {
            hevc_sps->vui.left_offset = bs_get_ue(bs);
            hevc_sps->vui.right_offset = bs_get_ue(bs);
            hevc_sps->vui.top_offset = bs_get_ue(bs);
            hevc_sps->vui.bottom_offset = bs_get_ue(bs);
        }

        hevc_sps->vui.has_timing_info = (bool)gts_bs_read_int(bs, 1);
        if (hevc_sps->vui.has_timing_info ) {
            hevc_sps->vui.num_units_in_tick = gts_bs_read_int(bs, 32);
            hevc_sps->vui.time_scale = gts_bs_read_int(bs, 32);
            hevc_sps->vui.poc_proportional_to_timing_flag = (bool)gts_bs_read_int(bs, 1);
            if (hevc_sps->vui.poc_proportional_to_timing_flag)
                hevc_sps->vui.num_ticks_poc_diff_one_minus1 = bs_get_ue(bs);
            if (/*hrd_parameters_present_flag=*/gts_bs_read_int(bs, 1) ) {
//                GF_LOG(GF_LOG_INFO, GF_LOG_CODING, ("[HEVC] HRD param parsing not implemented\n"));
                return sps_id;
            }
        }

        if (/*bitstream_restriction_flag=*/gts_bs_read_int(bs, 1)) {
            /*tiles_fixed_structure_flag = */gts_bs_read_int(bs, 1);
            /*motion_vectors_over_pic_boundaries_flag = */gts_bs_read_int(bs, 1);
            /*restricted_ref_pic_lists_flag = */gts_bs_read_int(bs, 1);
            /*min_spatial_segmentation_idc = */bs_get_ue(bs);
            /*max_bytes_per_pic_denom = */bs_get_ue(bs);
            /*max_bits_per_min_cu_denom = */bs_get_ue(bs);
            /*log2_max_mv_length_horizontal = */bs_get_ue(bs);
            /*log2_max_mv_length_vertical = */bs_get_ue(bs);
        }
    }

    if (/*sps_extension_flag*/gts_bs_read_int(bs, 1)) {
        while (gts_bs_available(bs)) {
            /*sps_extension_data_flag */ gts_bs_read_int(bs, 1);
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
        //GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("[HEVC] wrong PPS ID %d in PPS\n", index_hevc_pps));
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
    /*sign_data_hiding_flag = */gts_bs_read_int(gts_bitstream, 1);
    hevc_pps->cabac_init_present_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
    hevc_pps->num_ref_idx_l0_default_active = 1 + bs_get_ue(gts_bitstream);
    hevc_pps->num_ref_idx_l1_default_active = 1 + bs_get_ue(gts_bitstream);
    hevc_pps->pic_init_qp_minus26 = bs_get_se(gts_bitstream);
    /*constrained_intra_pred_flag = */gts_bs_read_int(gts_bitstream, 1);
    /*transform_skip_enabled_flag = */gts_bs_read_int(gts_bitstream, 1);
    hevc_pps->cu_qp_delta_enabled_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
    if (hevc_pps->cu_qp_delta_enabled_flag )
        hevc_pps->diff_cu_qp_delta_depth = bs_get_ue(gts_bitstream);

    /*pic_cb_qp_offset = */bs_get_se(gts_bitstream);
    /*pic_cr_qp_offset = */bs_get_se(gts_bitstream);
    hevc_pps->slice_chroma_qp_offsets_present_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
    hevc_pps->weighted_pred_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
    hevc_pps->weighted_bipred_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
    /*transquant_bypass_enable_flag = */gts_bs_read_int(gts_bitstream, 1);
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
    if (/*hevc_pps->deblocking_filter_control_present_flag*/ gts_bs_read_int(gts_bitstream, 1)) {
        hevc_pps->deblocking_filter_override_enabled_flag = (bool)gts_bs_read_int(gts_bitstream, 1);
        if (! /*pic_disable_deblocking_filter_flag= */gts_bs_read_int(gts_bitstream, 1) ) {
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
    HEVCSliceInfo n_state;
    uint8_t *nal_unit_type = &pSpecialInfo->naluType;
    uint8_t *temporal_id = &pSpecialInfo->temporal_id;
    uint8_t *layer_id = &pSpecialInfo->layer_id;
    uint16_t* slicehdrlen = &pSpecialInfo->sliceHeaderLen;
    uint16_t* payloadType = &pSpecialInfo->seiPayloadType;

    memcpy(&n_state, &hevc->s_info, sizeof(HEVCSliceInfo));

    //hevc->last_parsed_vps_id = hevc->last_parsed_sps_id = hevc->last_parsed_pps_id = -1;
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
    n_state.nal_unit_type = *nal_unit_type;

    switch (n_state.nal_unit_type) {
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
        /* slice - read the info and compare.*/
        ret = hevc_parse_slice_segment(bs, hevc, &n_state);
        if (ret<0) goto exit;

        *slicehdrlen = n_state.payload_start_offset;
        hevc_compute_poc(&n_state);

        ret = 0;

        if (hevc->s_info.poc != n_state.poc) {
            ret=1;
            break;
        }
        if (n_state.first_slice_segment_in_pic_flag) {
            if (!(*layer_id) || (n_state.prev_layer_id_plus1 && ((*layer_id) <= n_state.prev_layer_id_plus1 - 1)) ) {
                ret = 1;
                break;
            }
        }
        break;
    case GTS_HEVC_NALU_SEQ_PARAM:
        hevc->last_parsed_sps_id = gts_media_hevc_read_sps_bs(bs, hevc, *layer_id, NULL);
        ret = 0;
        break;
    case GTS_HEVC_NALU_PIC_PARAM:
        hevc->last_parsed_pps_id = gts_media_hevc_read_pps_bs(bs, hevc);
        ret = 0;
        break;
    case GTS_HEVC_NALU_VID_PARAM:
        hevc->last_parsed_vps_id = gts_media_hevc_read_vps_bs(bs, hevc, false);
        ret = 0;
        break;
    default:
        ret = 0;
        break;
    }

    /* save _prev values */
    if (ret && hevc->s_info.sps) {
        n_state.frame_num_offset_prev = hevc->s_info.frame_num_offset;
        n_state.frame_num_prev = hevc->s_info.frame_num;

        n_state.poc_lsb_prev = hevc->s_info.poc_lsb;
        n_state.poc_msb_prev = hevc->s_info.poc_msb;
        n_state.prev_layer_id_plus1 = *layer_id + 1;
    }
    if (is_slice) hevc_compute_poc(&n_state);
    memcpy(&hevc->s_info, &n_state, sizeof(HEVCSliceInfo));

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
    pRWPK->projPicWidth = gts_bs_read_int(bs, 32);
    pRWPK->projPicHeight = gts_bs_read_int(bs, 32);
    pRWPK->packedPicWidth = gts_bs_read_int(bs, 16); //bitstr.read16Bits();
    pRWPK->packedPicHeight = gts_bs_read_int(bs, 16); //bitstr.read16Bits();

    for (int i = 0; i < pRWPK->numRegions; ++i)
    {
        RectangularRegionWisePacking region;
        uint8_t packed8Bits = gts_bs_read_int(bs, 8);
        memset(&region, 0, sizeof(RectangularRegionWisePacking));
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
        memcpy(&pRWPK->rectRegionPacking[i], &region, sizeof(RectangularRegionWisePacking));
    }
    pRWPK->numHiRegions = gts_bs_read_int(bs, 8);
    pRWPK->lowResPicWidth = gts_bs_read_int(bs, 32);
    pRWPK->lowResPicHeight = gts_bs_read_int(bs, 32);

exit:
    if (bs) gts_bs_del(bs);
    if (data_without_emulation_bytes) gts_free(data_without_emulation_bytes);
    return ret;
}
