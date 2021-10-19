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
#include "360SCVPHevcEncHdr.h"
#include "360SCVPCommonDef.h"
#include "360SCVPBitstream.h"

#define MRG_MAX_NUM_CANDS 5

//! Maximum CU depth when descending form LCU level.
//! spec: log2_diff_max_min_luma_coding_block_size
#define MAX_DEPTH 3
//! Minimum log2 size of CUs.
//! spec: MinCbLog2SizeY
#define MIN_SIZE 3
#define LCU_WIDTH (1 << (MIN_SIZE + MAX_DEPTH))

static uint32_t math_floor_log2(uint32_t value)
{
    assert(value > 0);

    uint32_t result = 0;

    for (int32_t i = 4; i >= 0; --i) {
        uint32_t bits = 1ull << i;
        uint32_t shift = value >= (uint32_t)(1 << bits) ? bits : 0;
        result += shift;
        value >>= shift;
    }

    return result;
}

static uint32_t math_ceil_log2(uint32_t value)
{
    assert(value > 0);

    // The ceil_log2 is just floor_log2 + 1, except for exact powers of 2.
    return math_floor_log2(value) + ((value & (value - 1)) ? 1 : 0);
}

/**
* \brief Chroma subsampling format used for encoding.
* \since 3.15.0
*/
enum hevc_slices {
    HEVC_SLICES_NONE,
    HEVC_SLICES_TILES = (1 << 0), /*!< \brief Put each tile in a slice. */
    HEVC_SLICES_WPP = (1 << 1), /*!< \brief Put each row in a slice. */
};

/**
* \brief Write a Network Abstraction Layer (NAL) packet to the output.
*/
void nal_write(GTS_BitStream * const bitstream, const uint8_t nal_type,
    const uint8_t temporal_id, const int32_t long_start_code)
{
    uint8_t byte;

    // Some useful constants
    const uint8_t start_code_prefix_one_3bytes = 0x01;
    //const uint8_t zero = 0x00;

    // zero_byte (0x00) shall be present in the byte stream NALU of VPS, SPS
    // and PPS, or the first NALU of an access unit
    if (long_start_code)
        gts_bs_write_byte(bitstream, 0, 1);

    // start_code_prefix_one_3bytes
    gts_bs_write_byte(bitstream, 0, 1);
    gts_bs_write_byte(bitstream, 0, 1);
    gts_bs_write_byte(bitstream, start_code_prefix_one_3bytes, 1);

    // Handle header bits with full bytes instead of using bitstream
    // forbidden_zero_flag(1) + nal_unit_type(6) + 1bit of nuh_layer_id
    byte = nal_type << 1;
    gts_bs_write_byte(bitstream, byte, 1);

    // 5bits of nuh_layer_id + nuh_temporal_id_plus1(3)
    byte = (temporal_id + 1) & 7;
    gts_bs_write_byte(bitstream, byte, 1);
}


/**
* \brief Add rbsp_trailing_bits syntax element, which aligns the bitstream.
*/
void hevc_bitstream_add_rbsp_trailing_bits(GTS_BitStream * const stream)
{
    gts_bs_write_int(stream, 1, 1);
    if ((stream->nbBits & 7) != 0) {
        gts_bs_write_int(stream, 0, 8 - (stream->nbBits & 7));
    }
}

/**
* \brief Write uint32_t Exp-Golomb bit string
*/
static void bitstream_put_ue(GTS_BitStream *stream, uint32_t code_num)
{
    uint32_t code_num_log2 = math_floor_log2(code_num + 1);
    uint32_t prefix = 1 << code_num_log2;
    uint32_t suffix = code_num + 1 - prefix;
    uint32_t num_bits = code_num_log2 * 2 + 1;
    uint32_t value = prefix | suffix;

    gts_bs_write_int(stream, value, num_bits);
}

/**
* \brief Write signed Exp-Golomb bit string
*/
static void bitstream_put_se(GTS_BitStream *stream, int32_t data)
{
    // Map positive values to even and negative to odd values.
    uint32_t code_num = data <= 0 ? (-data) << 1 : (data << 1) - 1;
    bitstream_put_ue(stream, code_num);
}

static void hevc_write_bitstream_PTL(GTS_BitStream *stream,
    HEVCState * const state)
{
    // PTL
    // Profile Tier
    gts_bs_write_int(stream, state->sps->ptl.profile_space, 2); //, "general_profile_space"
    gts_bs_write_int(stream, state->sps->ptl.tier_flag, 1); //, "general_tier_flag"
    // Main Profile == 1,  Main 10 profile == 2
    gts_bs_write_int(stream, state->sps->ptl.profile_idc, 5); //, "general_profile_idc"
    /* Compatibility flags should be set at general_profile_idc
    *  (so with general_profile_idc = 1, compatibility_flag[1] should be 1)
    * According to specification, when compatibility_flag[1] is set,
    *  compatibility_flag[2] should be set too.
    */
    gts_bs_write_int(stream, state->sps->ptl.profile_compatibility_flag, 32); //, "general_profile_compatibility_flag[]"

    gts_bs_write_int(stream, state->sps->ptl.general_progressive_source_flag, 1); //, "general_progressive_source_flag"
    gts_bs_write_int(stream, state->sps->ptl.general_interlaced_source_flag != 0, 1); //, "general_interlaced_source_flag"
    gts_bs_write_int(stream, state->sps->ptl.general_non_packed_constraint_flag, 1); //, "general_non_packed_constraint_flag"
    gts_bs_write_int(stream, state->sps->ptl.general_frame_only_constraint_flag, 1); //, "general_frame_only_constraint_flag"

    if (state->sps->ptl.profile_idc < 4)
    {
        gts_bs_write_int(stream, 0, 16);
    }
    else
    {
        gts_bs_write_int(stream, 1, 1);

        //now for encoderBitDepth == EB_8BIT
        gts_bs_write_int(stream, 1, 1);
        gts_bs_write_int(stream, 1, 1); //(encoderBitDepth == EB_8BIT && (hromaFormatIdc == EB_YUV444 || constrainedIntra == EB_TRUE))
        gts_bs_write_int(stream, 1, 1); //(chromaFormatIdc == EB_YUV422 || (chromaFormatIdc == EB_YUV420 && constrainedIntra == EB_TRUE))
        //now for chromaFormatIdc == EB_YUV420
        gts_bs_write_int(stream, 1, 1);

        //"general_max_monochrome_constraint_flag"
        gts_bs_write_int(stream, 0, 1);
        gts_bs_write_int(stream, 0, 1); //for constrainedIntra == EB_TRUE

        //"general_one_picture_only_constraint_flag"
        gts_bs_write_int(stream, 0, 1);

        //"general_lower_bit_rate_constraint_flag"
        gts_bs_write_int(stream, 1, 1);

        // "XXX_reserved_zero_44bits[9..15]"
        gts_bs_write_int(stream, 0, 7);
    }

    gts_bs_write_int(stream, 0, 16); //, "XXX_reserved_zero_44bits[16..31]"
    gts_bs_write_int(stream, 0, 12); //, "XXX_reserved_zero_44bits[32..43]"
    // end Profile Tier

    uint8_t level = state->sps->ptl.level_idc;
    gts_bs_write_int(stream, level, 8); //, "general_level_idc"

    uint32_t layer_idx;
    for (layer_idx = 0; layer_idx < (state->vps[0].max_sub_layers - 1); layer_idx++)
    {
        gts_bs_write_int(stream, 0, 1); //, "sub_layer_profile_present_flag"
        gts_bs_write_int(stream, 0, 1); //, "sub_layer_level_present_flag"
    }
    if (state->vps[0].max_sub_layers > 1)
    {
        for (uint32_t i = (state->vps[0].max_sub_layers - 1); i < 8; i++) {
            gts_bs_write_int(stream, 0, 2); //, "reserved_zero_2bits"
        }
    }

    // end PTL
}

static void hevc_write_bitstream_VUI(GTS_BitStream* stream, HEVCState * const state)
{
    HEVC_SPS    *sps = &state->sps[state->last_parsed_sps_id];
    gts_bs_write_int(stream, sps->vui.aspect_ratio_info_present_flag, 1);
    if(sps->vui.aspect_ratio_info_present_flag)
    {
        gts_bs_write_int(stream, sps->vui.sar_idc, 8);
        if(sps->vui.sar_idc == 255)// EXTENDED_SAR
        {
            gts_bs_write_int(stream, sps->vui.sar_width, 16);
            gts_bs_write_int(stream, sps->vui.sar_height, 16);
        }
    }

    gts_bs_write_int(stream, sps->vui.overscan_info_present, 1);
    if(sps->vui.overscan_info_present)
    {
        gts_bs_write_int(stream, sps->vui.overscan_appropriate, 1);
    }

    gts_bs_write_int(stream, sps->vui.video_signal_type_present_flag, 1);
    if(sps->vui.video_signal_type_present_flag)
    {
        gts_bs_write_int(stream, sps->vui.video_format, 3);
        gts_bs_write_int(stream, sps->vui.video_full_range_flag, 1);
        gts_bs_write_int(stream, sps->vui.colour_description_present_flag, 1);

        if(sps->vui.colour_description_present_flag)
        {
            gts_bs_write_int(stream, sps->vui.colour_primaries, 8);
            gts_bs_write_int(stream, sps->vui.transfer_characteristic, 8);
            gts_bs_write_int(stream, sps->vui.matrix_coeffs, 8);
        }
    }

    gts_bs_write_int(stream, sps->vui.chroma_loc_info_present_flag, 1);
    if(sps->vui.chroma_loc_info_present_flag)
    {
        bitstream_put_ue(stream, sps->vui.chroma_sample_loc_type_top_field);
        bitstream_put_ue(stream, sps->vui.chroma_sample_loc_type_bottom_field);
    }

    gts_bs_write_int(stream, sps->vui.neutra_chroma_indication_flag, 1);
    gts_bs_write_int(stream, sps->vui.field_seq_flag, 1);
    gts_bs_write_int(stream, sps->vui.frame_field_info_present_flag, 1);

    gts_bs_write_int(stream, sps->vui.default_display_window_flag, 1);
    if(sps->vui.default_display_window_flag)
    {
        bitstream_put_ue(stream, sps->vui.left_offset);
        bitstream_put_ue(stream, sps->vui.right_offset);
        bitstream_put_ue(stream, sps->vui.top_offset);
        bitstream_put_ue(stream, sps->vui.bottom_offset);
    }

    gts_bs_write_int(stream, sps->vui.has_timing_info, 1);
    if(sps->vui.has_timing_info)
    {
        gts_bs_write_int(stream, sps->vui.num_units_in_tick, 32);
        gts_bs_write_int(stream, sps->vui.time_scale, 32);
        gts_bs_write_int(stream, sps->vui.poc_proportional_to_timing_flag, 1);
        if(sps->vui.poc_proportional_to_timing_flag)
        {
            bitstream_put_ue(stream, sps->vui.num_ticks_poc_diff_one_minus1);
        }
        gts_bs_write_int(stream, 0, 1);//vui_hrd_parameters_present_flag
    }
    gts_bs_write_int(stream, 0, 1);//bitstream_restriction_flag
}

static void hevc_wirte_bitstream_hrd_parameters(GTS_BitStream* stream, HEVC_HrdInfo *hrd_info, bool cprms_present_flag, uint32_t max_sub_layers)
{
    if (cprms_present_flag)
    {
        gts_bs_write_int(stream, hrd_info->nal_hrd_param_present_flag, 1);
        gts_bs_write_int(stream, hrd_info->vcl_hrd_param_present_flag, 1);
        if (hrd_info->nal_hrd_param_present_flag ||
            hrd_info->vcl_hrd_param_present_flag)
        {
            gts_bs_write_int(stream, hrd_info->sub_pic_cpb_param_present_flag, 1);
            if (hrd_info->sub_pic_cpb_param_present_flag)
            {
                gts_bs_write_int(stream, hrd_info->tick_divisor_minus2, 8);
                gts_bs_write_int(stream, hrd_info->du_cpb_removal_delay_len_minus1, 5);
                gts_bs_write_int(stream, hrd_info->sub_pic_cpb_param_in_picTSEI_flag, 1);
                gts_bs_write_int(stream, hrd_info->dpb_out_delay_du_len_minus1, 5);
            }

            gts_bs_write_int(stream, hrd_info->bit_rate_scale, 4);
            gts_bs_write_int(stream, hrd_info->cpb_size_scale, 4);
            if (hrd_info->sub_pic_cpb_param_present_flag)
            {
                gts_bs_write_int(stream, hrd_info->du_cpb_size_scale, 4);
            }
            gts_bs_write_int(stream, hrd_info->init_cpb_removal_delay_len_minus1, 5);
            gts_bs_write_int(stream, hrd_info->cpb_removal_delay_len_minus1, 5);
            gts_bs_write_int(stream, hrd_info->dpb_out_delay_len_minus1, 5);
        }
    }

    for (uint32_t layer_idx = 0; layer_idx <= (max_sub_layers - 1); layer_idx++)
    {
        gts_bs_write_int(stream, hrd_info->hrd_sub_layer_infos[layer_idx].fixed_pic_rate_flag, 1);
        if (!hrd_info->hrd_sub_layer_infos[layer_idx].fixed_pic_rate_flag)
        {
            gts_bs_write_int(stream, hrd_info->hrd_sub_layer_infos[layer_idx].fixed_pic_rate_within_cvs_flag, 1);
        }

        if (hrd_info->hrd_sub_layer_infos[layer_idx].fixed_pic_rate_within_cvs_flag)
        {
            bitstream_put_ue(stream, hrd_info->hrd_sub_layer_infos[layer_idx].pic_dur_inTc_minus1);
        }
        else
        {
            gts_bs_write_int(stream, hrd_info->hrd_sub_layer_infos[layer_idx].low_delay_hrd_flag, 1);
        }

        if (!hrd_info->hrd_sub_layer_infos[layer_idx].low_delay_hrd_flag)
        {
            bitstream_put_ue(stream, hrd_info->hrd_sub_layer_infos[layer_idx].cpb_cnt_minus1);
        }

        for (uint32_t nal_or_vcl = 0; nal_or_vcl < 2; nal_or_vcl++)
        {
            if (((nal_or_vcl == 0) && hrd_info->nal_hrd_param_present_flag) ||
                ((nal_or_vcl == 1) && hrd_info->vcl_hrd_param_present_flag))
            {
                for (uint32_t k = 0; k <= hrd_info->hrd_sub_layer_infos[layer_idx].cpb_cnt_minus1; k++)
                {
                    bitstream_put_ue(stream, hrd_info->hrd_sub_layer_infos[layer_idx].bitrate_val_minus1[k][nal_or_vcl]);
                    bitstream_put_ue(stream, hrd_info->hrd_sub_layer_infos[layer_idx].cpb_size_value[k][nal_or_vcl]);

                    if (hrd_info->sub_pic_cpb_param_present_flag)
                    {
                        bitstream_put_ue(stream, hrd_info->hrd_sub_layer_infos[layer_idx].du_cpb_size_value[k][nal_or_vcl]);
                        bitstream_put_ue(stream, hrd_info->hrd_sub_layer_infos[layer_idx].du_bitrate_value[k][nal_or_vcl]);
                    }
                    gts_bs_write_int(stream, hrd_info->hrd_sub_layer_infos[layer_idx].cbr_flag[k][nal_or_vcl], 1);
                }
            }
        }
    }
}

static void hevc_write_bitstream_vid_parameter_set(GTS_BitStream* stream,
    HEVCState * const state)
{
    HEVC_VPS *vps = &state->vps[0];
    gts_bs_write_int(stream, vps->id, 4); //, "vps_video_parameter_set_id"
    //gts_bs_write_int(stream, 3, 2); //, "vps_reserved_three_2bits"
    gts_bs_write_int(stream, vps->base_layer_internal_flag, 1); //, "base_layer_internal_flag"
    gts_bs_write_int(stream, vps->base_layer_available_flag, 1); //, "base_layer_available_flag"
    //gts_bs_write_int(stream, 0, 6); //, "vps_reserved_zero_6bits"
    gts_bs_write_int(stream, vps->max_layers - 1, 6); //, "vps_max_layers_minus1"
    gts_bs_write_int(stream, vps->max_sub_layers - 1, 3); //, "vps_max_sub_layers_minus1"
    gts_bs_write_int(stream, vps->temporal_id_nesting, 1); //, "vps_temporal_id_nesting_flag"
    gts_bs_write_int(stream, 0xffff, 16); //, "vps_reserved_ffff_16bits"

    hevc_write_bitstream_PTL(stream, state);

    gts_bs_write_int(stream, vps->sub_layer_ordering_info_present_flag, 1); //, "vps_sub_layer_ordering_info_present_flag"

    //for each layer
    uint32_t i = vps->sub_layer_ordering_info_present_flag ? 0 : (vps->max_sub_layers - 1);
    for ( ; i < vps->max_sub_layers; i++) {
        //printf("write %d, %d, %d \n", vps->vps_max_dec_pic_buffering_minus1[i], vps->vps_max_num_reorder_pics[i], vps->vps_max_latency_increase_plus1[i]);
        bitstream_put_ue(stream, vps->vps_max_dec_pic_buffering_minus1[i]); //, "vps_max_dec_pic_buffering"
        bitstream_put_ue(stream, vps->vps_max_num_reorder_pics[i]); //, "vps_num_reorder_pics"
        bitstream_put_ue(stream, vps->vps_max_latency_increase_plus1[i]); //, "vps_max_latency_increase"
    }

    gts_bs_write_int(stream, vps->max_layer_id, 6); //, "vps_max_nuh_reserved_zero_layer_id"
    bitstream_put_ue(stream, vps->num_layer_sets - 1); //, "vps_max_op_sets_minus1"
    for (uint32_t m = 1; m <= (vps->num_layer_sets - 1); m++)
    {
        for (uint32_t n = 0; n <= vps->max_layer_id; n++)
        {
            gts_bs_write_int(stream, vps->layer_id_included_flag[m][n], 1);
        }
    }

    gts_bs_write_int(stream, vps->timing_info_present_flag, 1); //, "vps_timing_info_present_flag"

    //IF timing info
    //END IF

    if (vps->timing_info_present_flag)
    {
        gts_bs_write_int(stream, vps->num_units_in_tick, 32);
        gts_bs_write_int(stream, vps->time_scale, 32);
        gts_bs_write_int(stream, vps->poc_proportional_to_timing_flag, 1);
        if (vps->poc_proportional_to_timing_flag)
            bitstream_put_ue(stream, vps->num_ticks_poc_diff_one - 1);

        bitstream_put_ue(stream, vps->num_hrd_parameters);
        for (uint32_t m = 0; m < vps->num_hrd_parameters; m++)
        {
            bitstream_put_ue(stream, vps->hrd_layer_set_idx[m]);

            if (m > 0)
                gts_bs_write_int(stream, vps->cprms_present_flags[m], 1);

            hevc_wirte_bitstream_hrd_parameters(stream, &(vps->hrd_infos[m]), vps->cprms_present_flags[m], vps->max_sub_layers);
        }
    }

    gts_bs_write_int(stream, 0, 1); //, "vps_extension_flag"

    hevc_bitstream_add_rbsp_trailing_bits(stream);
}


static void hevc_write_bitstream_seq_parameter_set(GTS_BitStream* stream,
    HEVCState * const state)
{
    HEVC_SPS *sps = &state->sps[state->last_parsed_sps_id];

    //const encoder_control_t * encoder = state->encoder_control;

    // TODO: profile IDC and level IDC should be defined later on
    gts_bs_write_int(stream, 0, 4); //, "sps_video_parameter_set_id"
    gts_bs_write_int(stream, sps->max_sub_layers_minus1, 3); //, "sps_max_sub_layers_minus1"
    gts_bs_write_int(stream, 0, 1); //, "sps_temporal_id_nesting_flag"

    hevc_write_bitstream_PTL(stream, state);

    bitstream_put_ue(stream, 0); //, "sps_seq_parameter_set_id"
    bitstream_put_ue(stream, state->sps->chroma_format_idc); //, "chroma_format_idc"

    //if (state->sps->chroma_format_idc == KVZ_CSP_444) {
    //    gts_bs_write_int(stream, 0, 1); //, "separate_colour_plane_flag"
    //}

    bitstream_put_ue(stream, state->sps->width); //, "pic_width_in_luma_samples"
    bitstream_put_ue(stream, state->sps->height); //, "pic_height_in_luma_samples"
    gts_bs_write_int(stream, 0, 1); //, "conformance_window_flag"

    //IF window flag
    //END IF

    bitstream_put_ue(stream, state->sps->bit_depth_luma - 8); //, "bit_depth_luma_minuS8"
    bitstream_put_ue(stream, state->sps->bit_depth_luma - 8); //, "bit_depth_chroma_minuS8"
    bitstream_put_ue(stream, sps->log2_max_pic_order_cnt_lsb - 4); //, "log2_max_pic_order_cnt_lsb_minus4"
    gts_bs_write_int(stream, 0, 1); //, "sps_sub_layer_ordering_info_present_flag"
    bitstream_put_ue(stream, sps->max_dec_pic_buffering); //, "sps_max_dec_pic_buffering"
    bitstream_put_ue(stream, sps->num_reorder_pics); //, "sps_num_reorder_pics"

    bitstream_put_ue(stream, sps->max_latency_increase); //, "sps_max_latency_increase"
    //end for

    bitstream_put_ue(stream, MIN_SIZE - 3); //, "log2_min_coding_block_size_minus3"
    bitstream_put_ue(stream, MAX_DEPTH); //, "log2_diff_max_min_coding_block_size"
    bitstream_put_ue(stream, 0);   // 4x4 , "log2_min_transform_block_size_minus2"
    bitstream_put_ue(stream, 3); // 4x4...32x32 , "log2_diff_max_min_transform_block_size"
    bitstream_put_ue(stream, sps->max_transform_hierarchy_depth_inter); //, "max_transform_hierarchy_depth_inter"
    bitstream_put_ue(stream, sps->max_transform_hierarchy_depth_intra); //, "max_transform_hierarchy_depth_intra"

    // scaling list
    gts_bs_write_int(stream, 0, 1); //, "scaling_list_enable_flag"

    gts_bs_write_int(stream,  0, 1); //, "amp_enabled_flag"

    gts_bs_write_int(stream,  sps->sample_adaptive_offset_enabled_flag, 1); //,    "sample_adaptive_offset_enabled_flag"
    gts_bs_write_int(stream, 0, 1); //, "pcm_enabled_flag"
#if ENABLE_PCM == 1
    WRITE_U(stream, 7, 4, "pcm_sample_bit_depth_luma_minus1");
    WRITE_U(stream, 7, 4, "pcm_sample_bit_depth_chroma_minus1");
    WRITE_UE(stream, 0, "log2_min_pcm_coding_block_size_minus3");
    WRITE_UE(stream, 2, "log2_diff_max_min_pcm_coding_block_size");
    WRITE_U(stream, 1, 1, "pcm_loop_filter_disable_flag");
#endif

    bitstream_put_ue(stream, sps->num_short_term_ref_pic_sets); //, "num_short_term_ref_pic_sets"

    //IF num short term ref pic sets
    //ENDIF

    gts_bs_write_int(stream, 0, 1); //, "long_term_ref_pics_present_flag"

    //IF long_term_ref_pics_present
    //ENDIF

    gts_bs_write_int(stream, sps->temporal_mvp_enable_flag, 1); //,"sps_temporal_mvp_enable_flag"
    gts_bs_write_int(stream, sps->strong_intra_smoothing_enable_flag, 1); //, "sps_strong_intra_smoothing_enable_flag"

    gts_bs_write_int(stream, sps->vui_parameters_present_flag, 1); //, "vui_parameters_present_flag"

    if(sps->vui_parameters_present_flag)
    {
        hevc_write_bitstream_VUI(stream, state);
    }

    gts_bs_write_int(stream, 0, 1); //, "sps_extension_present_flag"
    //encoder_state_write_bitstream_SPS_extension(stream, state);

    hevc_bitstream_add_rbsp_trailing_bits(stream);
}

static void hevc_write_bitstream_pic_parameter_set(GTS_BitStream* stream,
    HEVCState * const state)
{
    HEVC_PPS      *pps = &state->pps[state->last_parsed_pps_id];
    //const encoder_control_t * const encoder = state->encoder_control;

    bitstream_put_ue(stream, 0); //, "pic_parameter_set_id"
    bitstream_put_ue(stream, 0); //, "seq_parameter_set_id"
    gts_bs_write_int(stream, state->pps[state->last_parsed_pps_id].dependent_slice_segments_enabled_flag, 1); //, "dependent_slice_segments_enabled_flag"
    gts_bs_write_int(stream, 0, 1); //, "output_flag_present_flag"
    gts_bs_write_int(stream, 0, 3); //, "num_extra_slice_header_bits"
    gts_bs_write_int(stream, 0, 1); //, "sign_data_hiding_flag"
    gts_bs_write_int(stream, 0, 1); //, "cabac_init_present_flag"

    bitstream_put_ue(stream, 0); //, "num_ref_idx_l0_default_active_minus1"
    bitstream_put_ue(stream, 0); //, "num_ref_idx_l1_default_active_minus1"

    // If tiles and slices = tiles is enabled, signal QP in the slice header. Keeping the PPS constant for OMAF etc
    bool signal_qp_in_slice_header = 1;//(encoder->cfg.slices & HEVC_SLICES_TILES) && encoder->tiles_enable;
    bitstream_put_se(stream, signal_qp_in_slice_header ? 0 : (((int8_t)state->pps[state->last_parsed_pps_id].pic_init_qp_minus26))); //, "pic_init_qp_minus26"

    gts_bs_write_int(stream, 0, 1); //, "constrained_intra_pred_flag"
    gts_bs_write_int(stream, 0, 1); //, "transform_skip_enabled_flag"

    // Use separate QP for each LCU when rate control is enabled.
    gts_bs_write_int(stream, pps->cu_qp_delta_enabled_flag, 1); //, "cu_qp_delta_enabled_flag"
    if(pps->cu_qp_delta_enabled_flag)
    {
        bitstream_put_ue(stream, pps->diff_cu_qp_delta_depth); //, "diff_cu_qp_delta_depth"
    }

    //TODO: add QP offsets
    bitstream_put_se(stream, 0); //, "pps_cb_qp_offset"
    bitstream_put_se(stream, 0); //, "pps_cr_qp_offset"
    gts_bs_write_int(stream, pps->slice_chroma_qp_offsets_present_flag, 1); //, "pps_slice_chroma_qp_offsets_present_flag"
    gts_bs_write_int(stream, 0, 1); //, "weighted_pred_flag"
    gts_bs_write_int(stream, 0, 1); //, "weighted_bipred_idc"

    //WRITE_U(stream, 0, 1, "dependent_slices_enabled_flag");
    gts_bs_write_int(stream, 0, 1); //, "transquant_bypass_enable_flag"
    gts_bs_write_int(stream, pps->tiles_enabled_flag, 1); //, "tiles_enabled_flag"
    //wavefronts
    gts_bs_write_int(stream, 0, 1); //, "entropy_coding_sync_enabled_flag"

    if (pps->tiles_enabled_flag)
    {
        bitstream_put_ue(stream, pps->num_tile_columns - 1); //, "num_tile_columns_minus1"
        bitstream_put_ue(stream, pps->num_tile_rows - 1); //, "num_tile_rows_minus1"

        gts_bs_write_int(stream, pps->uniform_spacing_flag, 1); //, "uniform_spacing_flag"

        if (!pps->uniform_spacing_flag)
        {
            for (uint32_t i = 0; i < pps->num_tile_columns - 1; ++i) {
                bitstream_put_ue(stream, pps->column_width[i] - 1); //, "column_width_minus1[...]"
            }
            for (uint32_t i = 0; i < pps->num_tile_rows - 1; ++i) {
                bitstream_put_ue(stream, pps->row_height[i] - 1);//, "row_height_minus1[...]"
            }
        }

        gts_bs_write_int(stream, pps->loop_filter_across_tiles_enabled_flag, 1);//, "loop_filter_across_tiles_enabled_flag"

    }

    gts_bs_write_int(stream, pps->loop_filter_across_slices_enabled_flag, 1); //, "loop_filter_across_slice_flag"
    gts_bs_write_int(stream, pps->deblocking_filter_control_present_flag, 1); //, "deblocking_filter_control_present_flag"

    //IF deblocking_filter
    gts_bs_write_int(stream, pps->deblocking_filter_override_enabled_flag, 1); //, "deblocking_filter_override_enabled_flag"
    gts_bs_write_int(stream,  1, 1); //,    "pps_disable_deblocking_filter_flag"

    gts_bs_write_int(stream, 0, 1); //, "pps_scaling_list_data_present_flag"

    gts_bs_write_int(stream, 0, 1); //, "lists_modification_present_flag"
    bitstream_put_ue(stream, 0); //, "log2_parallel_merge_level_minus2"
    gts_bs_write_int(stream, 0, 1); //, "slice_segment_header_extension_present_flag"
    gts_bs_write_int(stream, 0, 1); //, "pps_extension_flag"

    hevc_bitstream_add_rbsp_trailing_bits(stream);
}

void hevc_write_bitstream_aud(GTS_BitStream *stream,    HEVCState * const state)
{
    // Access Unit Delimiter (AUD)
    nal_write(stream, GTS_HEVC_NALU_ACCESS_UNIT, 0, 1);

    uint8_t picType = state->s_info.slice_type == GF_HEVC_SLICE_TYPE_I ? 0
                : state->s_info.slice_type == GF_HEVC_SLICE_TYPE_P ? 1 : 2;
    gts_bs_write_int(stream, picType, 3);//pic_type

    hevc_bitstream_add_rbsp_trailing_bits(stream);
}

void hevc_write_parameter_sets(GTS_BitStream *stream,    HEVCState * const state)
{
    // Video Parameter Set (VPS)
    nal_write(stream, GTS_HEVC_NALU_VID_PARAM, 0, 1);
    hevc_write_bitstream_vid_parameter_set(stream, state);

    // Sequence Parameter Set (SPS)
    nal_write(stream, GTS_HEVC_NALU_SEQ_PARAM, 0, 1);
    hevc_write_bitstream_seq_parameter_set(stream, state);

    // Picture Parameter Set (PPS)
    nal_write(stream, GTS_HEVC_NALU_PIC_PARAM, 0, 1);
    hevc_write_bitstream_pic_parameter_set(stream, state);
}


void hevc_write_sps(GTS_BitStream *stream, HEVCState * const state)
{
    // Sequence Parameter Set (SPS)
    nal_write(stream, GTS_HEVC_NALU_SEQ_PARAM, 0, 1);
    hevc_write_bitstream_seq_parameter_set(stream, state);
}

void hevc_write_pps(GTS_BitStream *stream, HEVCState * const state)
{
    // Picture Parameter Set (PPS)
    nal_write(stream, GTS_HEVC_NALU_PIC_PARAM, 0, 1);
    hevc_write_bitstream_pic_parameter_set(stream, state);
}

static void hevc_write_bitstream_slice_header_independent(GTS_BitStream * stream, HEVCState * state)
{
    HEVC_SPS      *sps = &state->sps[state->last_parsed_sps_id];
    HEVC_PPS      *pps = &state->pps[state->last_parsed_pps_id];
    HEVCSliceInfo *si  = &state->s_info;

    for (uint32_t k = 0; k < pps->num_extra_slice_header_bits; k++)
    {
        gts_bs_write_int(stream, si->slice_reserved_flag[k], 1);
    }

    bitstream_put_ue(stream, state->s_info.slice_type); //, "slice_type"

    if (pps->output_flag_present_flag)
        gts_bs_write_int(stream, si->pic_output_flag, 1);

    if (sps->separate_colour_plane_flag == 1)
        gts_bs_write_int(stream, si->colour_plane_id, 2);

    if (state->s_info.nal_unit_type != GTS_HEVC_NALU_SLICE_IDR_W_DLP
        && state->s_info.nal_unit_type != GTS_HEVC_NALU_SLICE_IDR_N_LP)
    {
        gts_bs_write_int(stream, si->poc_lsb, sps->log2_max_pic_order_cnt_lsb);//, "pic_order_cnt_lsb");
        gts_bs_write_int(stream, si->short_term_ref_pic_set_sps_flag, 1);//, "short_term_ref_pic_set_sps_flag");
        // !!NOTE!! only support 1 reference frame, sps->num_short_term_ref_pic_sets == 0 
        if (si->short_term_ref_pic_set_sps_flag == 0)
        {
            uint32_t idx_rps = sps->num_short_term_ref_pic_sets;
            if (idx_rps != 0)
                gts_bs_write_int(stream, si->rps[idx_rps].inter_ref_pic_set_prediction_flag, 1);

            if (si->rps[idx_rps].inter_ref_pic_set_prediction_flag)
            {
                if (idx_rps == sps->num_short_term_ref_pic_sets)
                    bitstream_put_ue(stream, si->rps[idx_rps].index_num);

                gts_bs_write_int(stream, si->rps[idx_rps].delta_rps_sign, 1);
                bitstream_put_ue(stream, si->rps[idx_rps].index_num_absolute);
                uint32_t index_reference, reference_set_nb;
                index_reference = idx_rps - 1 - si->rps[idx_rps].index_num;
                HEVC_ReferencePictureSets *hevc_pic_set = &si->rps[index_reference];
                reference_set_nb = hevc_pic_set->num_negative_pics + hevc_pic_set->num_positive_pics;
                for (uint32_t i=0; i<=reference_set_nb; i++)
                {
                    gts_bs_write_int(stream, si->rps[idx_rps].used_by_curr_pic_flag[i], 1);
                    if (!si->rps[idx_rps].used_by_curr_pic_flag[i])
                        gts_bs_write_int(stream, si->rps[idx_rps].used_delta_flag[i], 1);

                }
            }
            else
            {
                bitstream_put_ue(stream, si->rps[idx_rps].num_negative_pics);
                bitstream_put_ue(stream, si->rps[idx_rps].num_positive_pics);
                for (uint32_t i = 0; i < si->rps[idx_rps].num_negative_pics; i++)
                {
                     bitstream_put_ue(stream, si->rps[idx_rps].delta_poc0[i] - 1);
                     gts_bs_write_int(stream, si->rps[idx_rps].used_by_curr_pic_s0_flag[i], 1);
                }
                for (uint32_t i = 0; i < si->rps[idx_rps].num_positive_pics; i++)
                {
                     bitstream_put_ue(stream, si->rps[idx_rps].delta_poc1[i] - 1);
                     gts_bs_write_int(stream, si->rps[idx_rps].used_by_curr_pic_s1_flag[i], 1);
                }
            }
        }
        else if (sps->num_short_term_ref_pic_sets > 1)
        {
            uint32_t numbits = 0;
            while ( (uint32_t) (1 << numbits) < sps->num_short_term_ref_pic_sets)
                numbits++;

            if (numbits > 0)
                gts_bs_write_int(stream, si->short_term_ref_pic_set_idx, numbits);
        }

        if (sps->long_term_ref_pics_present_flag)
        {
            if (sps->num_long_term_ref_pic_sps > 0)
            {
                bitstream_put_ue(stream, si->count_hevc_lts);
            }

            bitstream_put_ue(stream, si->count_hevc_ltp);

            for (uint32_t m = 0; m < si->count_hevc_lts + si->count_hevc_ltp; m++)
            {

                if (m < si->count_hevc_lts)
                {
                    if (sps->num_long_term_ref_pic_sps > 1)
                        gts_bs_write_int(stream, si->lt_idx_sps[m], gts_get_bit_size(sps->num_long_term_ref_pic_sps));
                }
                else
                {
                    gts_bs_write_int(stream, si->poc_lsb_lt[m], sps->log2_max_pic_order_cnt_lsb);
                    gts_bs_write_int(stream, si->used_by_curr_pic_lt_flag[m], 1);
                }

                gts_bs_write_int(stream, si->delta_poc_msb_present_flag[m], 1);
                if (si->delta_poc_msb_present_flag[m])
                    bitstream_put_ue(stream, si->poc_MCL_del[m]);
            }
        }

        if (sps->temporal_mvp_enable_flag)
            gts_bs_write_int(stream, si->slice_temporal_mvp_enabled_flag, 1);

    }
    /*
        bitstream_put_ue(stream, si->rps[0].num_negative_pics);//, "num_negative_pics");
        bitstream_put_ue(stream, si->rps[0].num_positive_pics);//, "num_positive_pics");
        for (uint32_t j = 0; j < si->rps[0].num_negative_pics; j++) {
            //refer to previous frame
            bitstream_put_ue(stream, si->rps[0].delta_poc[j] - 1);//, "delta_poc_s0_minus1");
            gts_bs_write_int(stream, si->used_by_curr_pic_s0_flag[j], 1);//, "used_by_curr_pic_s0_flag");
        }

        if (sps->temporal_mvp_enable_flag) {
            gts_bs_write_int(stream, si->slice_temporal_mvp_enabled_flag, 1);//, "slice_temporal_mvp_enabled_flag");
        }
    }
    */

    if (state->sps[state->last_parsed_sps_id].sample_adaptive_offset_enabled_flag) {
        gts_bs_write_int(stream, si->slice_sao_luma_flag, 1); //, "slice_sao_luma_flag"
        if (state->sps[state->last_parsed_sps_id].chroma_format_idc != CSP_400) {
            gts_bs_write_int(stream, si->slice_sao_chroma_flag, 1); //, "slice_sao_chroma_flag"
        }
    }

    if (state->s_info.slice_type != SLICE_I) {
        gts_bs_write_int(stream, si->num_ref_idx_active_override_flag, 1); //, "num_ref_idx_active_override_flag"
        if(si->num_ref_idx_active_override_flag)
        {
            bitstream_put_ue(stream, si->count_index_reference_0 - 1); //, "num_ref_idx_l0_active_minus1"

            if (state->s_info.slice_type == SLICE_B)
            {
                bitstream_put_ue(stream, si->count_index_reference_1 - 1);
            }
        }

        //ref_pic_lists_modification()
        //if (pps->lists_modification_present_flag)
        //{
        //}
        //printf("si->mvd_l1_zero_flag %d \n", si->mvd_l1_zero_flag);
        if (si->slice_type == SLICE_B)
        {
            gts_bs_write_int(stream, si->mvd_l1_zero_flag, 1);
            //printf("Write si->mvd_l1_zero_flag %d \n", si->mvd_l1_zero_flag);
        }

        if (pps->cabac_init_present_flag)
        {
            gts_bs_write_int(stream, si->cabac_init_flag, 1);
        }
        if (si->slice_temporal_mvp_enabled_flag)
        {
            if (si->slice_type == SLICE_B)
            {
                gts_bs_write_int(stream, si->info_of_cl0, 1);
            }

            if ((si->info_of_cl0 && (si->count_index_reference_0 > 1)) ||
                (!si->info_of_cl0 && (si->count_index_reference_1 > 1)))
            {
                bitstream_put_ue(stream, si->collocated_ref_idx);
            }
        }

        //weighted prediction
        //if ((pps->weighted_pred_flag && si->slice_type == SLICE_P) ||
        //    (pps->weighted_bipred_flag && si->slice_type == SLICE_B))
        //{
        //}
        bitstream_put_ue(stream, si->five_minus_max_num_merge_cand);
    }

    // If tiles are enabled, signal the full QP here (relative to the base value of 26)
    bitstream_put_se(stream, si->slice_qp_delta); //, "slice_qp_delta"
    if(pps->slice_chroma_qp_offsets_present_flag)
    {
        bitstream_put_se(stream, si->slice_cb_qp_offset);
        bitstream_put_se(stream, si->slice_cr_qp_offset);
    }

    if (pps->deblocking_filter_override_enabled_flag)
    {
        gts_bs_write_int(stream, si->info_of_dfof, 1);
    }
    if (si->info_of_dfof)
    {
        gts_bs_write_int(stream, si->info_of_sdfdf, 1);
        if (!si->info_of_sdfdf)
        {
            bitstream_put_se(stream, si->slice_beta_offset_div2);
            bitstream_put_se(stream, si->slice_tc_offset_div2);
        }
    }

    if (pps->loop_filter_across_slices_enabled_flag &&
        (si->slice_sao_luma_flag || si->slice_sao_chroma_flag || !si->info_of_sdfdf))
    {
        gts_bs_write_int(stream, si->slice_loop_filter_across_slices_enabled_flag, 1);
    }

}

void hevc_write_bitstream_slice_header(GTS_BitStream * stream, HEVCState * state)
{
    //printf("Will write first slice %d for slice %p \n", state->s_info.first_slice_segment_in_pic_flag, state);
    bool first_slice_segment_in_pic = state->s_info.first_slice_segment_in_pic_flag;

    HEVCSliceInfo *slice_info = &(state->s_info);
    gts_bs_write_int(stream, first_slice_segment_in_pic, 1); //, "first_slice_segment_in_pic_flag"

    if (state->s_info.nal_unit_type >= GTS_HEVC_NALU_SLICE_BLA_W_LP
        && state->s_info.nal_unit_type < GTS_HEVC_NALU_VID_PARAM) { //23
        gts_bs_write_int(stream, slice_info->no_output_of_prior_pics_flag, 1); //, "no_output_of_prior_pics_flag"
    }

    bitstream_put_ue(stream, slice_info->index_hevc_pps); //, "slice_pic_parameter_set_id"

    if (!first_slice_segment_in_pic) {
         int32_t lcu_cnt = state->sps[state->last_parsed_sps_id].width / LCU_WIDTH * state->sps[state->last_parsed_sps_id].height / LCU_WIDTH;
        int32_t num_bits = math_ceil_log2(lcu_cnt);
        gts_bs_write_int(stream, state->s_info.slice_segment_address, num_bits); //, "slice_segment_address"
    }

    hevc_write_bitstream_slice_header_independent(stream, state);

    if (state->pps[state->last_parsed_pps_id].tiles_enabled_flag ||
        state->pps[state->last_parsed_pps_id].entropy_coding_sync_enabled_flag)
    {
        bitstream_put_ue(stream, state->s_info.num_entry_point_offsets);
        if (state->s_info.num_entry_point_offsets > 0)
        {
            bitstream_put_ue(stream, state->s_info.offset_len - 1);

            //for (uint32_t k = 0; k < state->s_info.num_entry_point_offsets; k++)
            //{
            //    gts_bs_write_int(stream, state->s_info.entry_point_offset_minus1[k], state->s_info.offset_len);
            //}
        }
    }

    if (state->pps[state->last_parsed_pps_id].slice_segment_header_extension_present_flag)
    {
        bitstream_put_ue(stream, state->s_info.size_ext);
        for (uint32_t k = 0; k < state->s_info.size_ext; k++)
        {
            gts_bs_write_int(stream, state->s_info.ext_bytes[k], 8);
        }
    }

}

 void hevc_write_slice_header(GTS_BitStream * stream, HEVCState * state)
{
    state->first_nal = true;
    nal_write(stream, state->s_info.nal_unit_type, state->s_info.temporal_id, state->first_nal);
    state->first_nal = false;

    hevc_write_bitstream_slice_header(stream, state);
    hevc_bitstream_add_rbsp_trailing_bits(stream);
}

 void writeSEINalHeader(GTS_BitStream *bs, H265SEIType payloadType, unsigned int payloadSize, int temporalIdPlus1)
 {
     // add NAL header
     nal_write(bs, GTS_HEVC_NALU_PREFIX_SEI, 0, 1);

     unsigned int type = (unsigned int)payloadType;
     for (; type >= 0xff; type -= 0xff)
     {
         gts_bs_write_U8(bs, 0xff);
     }
     gts_bs_write_U8(bs, type);

     for (; payloadSize >= 0xff; payloadSize -= 0xff)
     {
         gts_bs_write_U8(bs, 0xff);
     }
     gts_bs_write_U8(bs, payloadSize);

 }

 uint32_t writeEquiProjectionSEINal(GTS_BitStream *bs, EquirectangularProjectionSEI& projection, int temporalIdPlus1)
 {
     uint32_t payloadSize = 0;
     uint64_t posStart = gts_bs_get_position(bs);

     if (!projection.erpCancel)
     {
         if (projection.erpGuardBand)
             payloadSize = 3;
     }
     writeSEINalHeader(bs, E_EQUIRECT_PROJECTION, payloadSize, temporalIdPlus1);

     gts_bs_write_int(bs, projection.erpCancel, 1);
     if (!projection.erpCancel)
     {
         gts_bs_write_int(bs, projection.erpPersistence, 1);
         gts_bs_write_int(bs, projection.erpGuardBand, 1);
         gts_bs_write_int(bs, 0, 2);
         if (projection.erpGuardBand)
         {
             gts_bs_write_int(bs, projection.erpGuardBandType, 3);
             gts_bs_write_U8(bs, projection.erpLeftGuardBandWidth);
             gts_bs_write_U8(bs, projection.erpRightGuardBandWidth);
         }
     }

     hevc_bitstream_add_rbsp_trailing_bits(bs);

     return (uint32_t)(gts_bs_get_position(bs)-posStart);
 }

uint32_t writeCubeProjectionSEINal(GTS_BitStream *bs, CubemapProjectionSEI& projection, int temporalIdPlus1)
 {
    uint64_t posStart = gts_bs_get_position(bs);

    writeSEINalHeader(bs, E_CUBEMAP_PROJECTION, 1, temporalIdPlus1);

    if (!projection.cmpCancel)
     {
        gts_bs_write_int(bs, projection.cmpPersistence, 1);
     }

     hevc_bitstream_add_rbsp_trailing_bits(bs);

     return (uint32_t)(gts_bs_get_position(bs) - posStart);
}

 uint32_t writeFramePackingSEINal(GTS_BitStream *bs, FramePackingSEI& packing, int temporalIdPlus1)
 {
     int64_t posStart = gts_bs_get_position(bs);

     writeSEINalHeader(bs, E_FRAMEPACKING_ARRANGEMENT, 1, temporalIdPlus1);

     gts_bs_write_int(bs, packing.fpArrangementId, 32);
     gts_bs_write_int(bs, packing.fpCancel, 1);
     if (!packing.fpCancel)
     {
         gts_bs_write_int(bs, packing.fpArrangementType, 7);
         gts_bs_write_int(bs, packing.quincunxSamplingFlag, 1);
         gts_bs_write_int(bs, packing.contentInterpretationType, 6);
         gts_bs_write_int(bs, packing.spatialFlipping, 1);
         gts_bs_write_int(bs, packing.frame0Flipped, 1);
         gts_bs_write_int(bs, packing.fieldViews, 1);
         gts_bs_write_int(bs, packing.currentFrameIsFrame0, 1);
         gts_bs_write_int(bs, packing.frame0SelfContained, 1);
         gts_bs_write_int(bs, packing.frame1SelfContained, 1);
         if (!packing.quincunxSamplingFlag && packing.fpArrangementType != 5)
         {
             gts_bs_write_int(bs, packing.frame0GridX, 4);
             gts_bs_write_int(bs, packing.frame0GridY, 4);
             gts_bs_write_int(bs, packing.frame1GridX, 4);
             gts_bs_write_int(bs, packing.frame1GridY, 4);
         }
         gts_bs_write_U8(bs, 0); // //reserved

         gts_bs_write_int(bs, packing.fpArrangementPersistence, 1);
     }
     gts_bs_write_int(bs, packing.upsampledAspectRatio, 1);

     hevc_bitstream_add_rbsp_trailing_bits(bs);
     return (uint32_t)(gts_bs_get_position(bs) - posStart);
 }

 uint32_t writeRwpkSEINal(GTS_BitStream *bs, RegionWisePackingSEI& packing, int temporalIdPlus1)
 {
     unsigned int payloadSize = 1;
     int64_t posStart = gts_bs_get_position(bs);

     if (!packing.rwpCancel)
     {
         payloadSize = 14;
         RegionStruct* pRegion = packing.pRegions;
         for (int32_t i = 0; i < packing.regionsSize; i++)
         {
             payloadSize += 29;
             if (pRegion->rwpGuardBand)
                 payloadSize += 6;
             pRegion++;
         }
     }

     writeSEINalHeader(bs, E_REGIONWISE_PACKING, payloadSize, temporalIdPlus1);

     gts_bs_write_int(bs, packing.rwpCancel, 1);
     if (!packing.rwpCancel)
     {
         gts_bs_write_int(bs, packing.rwpPersistence, 1);
         gts_bs_write_int(bs, packing.constituentPictureMatching, 1);
         gts_bs_write_int(bs, 0, 5);

         gts_bs_write_int(bs, packing.regionsSize,8);
         gts_bs_write_int(bs, packing.projPictureWidth, 32);
         gts_bs_write_int(bs, packing.projPictureHeight, 32);
         gts_bs_write_int(bs, packing.packedPictureWidth, 16);
         gts_bs_write_int(bs, packing.packedPictureHeight, 16);
         RegionStruct* pRegion = packing.pRegions;
         for (int32_t i = 0; i < packing.regionsSize; i++)
         {
             gts_bs_write_int(bs, 0, 4); //reserved_zero_4bits
             gts_bs_write_int(bs, pRegion->rwpTransformType, 3);
             gts_bs_write_int(bs, pRegion->rwpGuardBand, 1);
             gts_bs_write_int(bs, pRegion->projRegionWidth, 32);
             gts_bs_write_int(bs, pRegion->projRegionHeight, 32);
             gts_bs_write_int(bs, pRegion->projRegionTop, 32);
             gts_bs_write_int(bs, pRegion->projRegionLeft, 32);
             gts_bs_write_int(bs, pRegion->packedRegionWidth, 16);
             gts_bs_write_int(bs, pRegion->packedRegionHeight, 16);
             gts_bs_write_int(bs, pRegion->packedRegionTop, 16);
             gts_bs_write_int(bs, pRegion->packedRegionLeft, 16);

             if (pRegion->rwpGuardBand)
             {
                 gts_bs_write_U8(bs, pRegion->leftGbWidth);
                 gts_bs_write_U8(bs, pRegion->rightGbWidth);
                 gts_bs_write_U8(bs, pRegion->topGbHeight);
                 gts_bs_write_U8(bs, pRegion->bottomGbHeight);

                 gts_bs_write_int(bs, pRegion->gbNotUsedForPredFlag, 1);
                 gts_bs_write_int(bs, pRegion->gbType0, 3);
                 gts_bs_write_int(bs, pRegion->gbType1, 3);
                 gts_bs_write_int(bs, pRegion->gbType2, 3);
                 gts_bs_write_int(bs, pRegion->gbType3, 3);
                 gts_bs_write_int(bs, 0, 3); // reserved_zero_3bits
             }
             pRegion++;
         }
         gts_bs_write_int(bs, packing.numHiRegions, 8);
         gts_bs_write_int(bs, packing.lowResPicWidth, 32);
         gts_bs_write_int(bs, packing.lowResPicHeight, 32);
         gts_bs_write_int(bs, packing.timeStamp, 32);
     }

     hevc_bitstream_add_rbsp_trailing_bits(bs);

     return (uint32_t)(gts_bs_get_position(bs) - posStart);
 }

 uint32_t writeRotationSEINal(GTS_BitStream *bs, SphereRotationSEI& rotation, int temporalIdPlus1)
 {
     unsigned int payloadSize = 1;
     int64_t posStart = gts_bs_get_position(bs);

     if (!rotation.sphereRotationCancel)
         payloadSize = 13;

     writeSEINalHeader(bs, E_SPHERE_ROTATION, payloadSize, temporalIdPlus1);

     gts_bs_write_int(bs, rotation.sphereRotationCancel, 1);

     if (!rotation.sphereRotationCancel)
     {
         gts_bs_write_int(bs, rotation.sphereRotationPersistence, 1);
         gts_bs_write_int(bs, 0, 6); // reserved_zero_6bits

         gts_bs_write_int(bs, rotation.yawRotation, 32);
         gts_bs_write_int(bs, rotation.pitchRotation, 32);
         gts_bs_write_int(bs, rotation.rollRotation, 32);

     }

     hevc_bitstream_add_rbsp_trailing_bits(bs);

     return (uint32_t)(gts_bs_get_position(bs) - posStart);
 }

 uint32_t writeRotationSEINal(GTS_BitStream *bs, ViewportSEI& viewport, int temporalIdPlus1)
 {
     unsigned int payloadSize = 1;
     int64_t posStart = gts_bs_get_position(bs);

     if (!viewport.vpCancel) //the payload size need to confirm further
         payloadSize = 13;

     writeSEINalHeader(bs, E_OMNI_VIEWPORT, payloadSize, temporalIdPlus1);

     gts_bs_write_int(bs, viewport.vpCancel, 1);

     if (!viewport.vpCancel)
     {
         gts_bs_write_int(bs, viewport.vpPersistence, 1);
         gts_bs_write_int(bs, 0, 6); // reserved_zero_6bits
         for (int32_t i = 0; i < viewport.viewportsSize; i++)
         {
             gts_bs_write_int(bs, viewport.pViewports[i].AzimuthCentre, 32);
             gts_bs_write_int(bs, viewport.pViewports[i].ElevationCentre, 32);
             gts_bs_write_int(bs, viewport.pViewports[i].tiltCentre, 32);
             gts_bs_write_int(bs, viewport.pViewports[i].HorzRange, 32);
             gts_bs_write_int(bs, viewport.pViewports[i].VertRange, 32);
         }
     }

     hevc_bitstream_add_rbsp_trailing_bits(bs);

     return (uint32_t)(gts_bs_get_position(bs) - posStart);
 }

 uint32_t hevc_write_ProjectionSEI(GTS_BitStream * stream, int32_t projType, int32_t temporalIdPlus1)
 {
     if (projType == E_EQUIRECT_PROJECTION)
     {
         EquirectangularProjectionSEI projection;
         projection.erpCancel = false;
         projection.erpPersistence = true; // valid until updated
         projection.erpGuardBand = false;
         return writeEquiProjectionSEINal(stream, projection, temporalIdPlus1);
     }
     else if (projType == E_CUBEMAP_PROJECTION)
     {
         CubemapProjectionSEI projectionCube;
         projectionCube.cmpCancel = false;
         projectionCube.cmpPersistence = true; // valid until updated
         return writeCubeProjectionSEINal(stream, projectionCube, temporalIdPlus1);
     }
     else
         return 0;
 }

 uint32_t hevc_write_RwpkSEI(GTS_BitStream * stream, const RegionWisePacking* pRegion, int32_t temporalIdPlus1)
 {
     RegionWisePackingSEI packing;
     packing.rwpCancel = false;
     packing.rwpPersistence = true; // valid until updated
     packing.constituentPictureMatching = false; // 0 for mono

     packing.projPictureWidth = pRegion->projPicWidth;
     packing.projPictureHeight = pRegion->projPicHeight;
     packing.packedPictureWidth = pRegion->packedPicWidth;
     packing.packedPictureHeight = pRegion->packedPicHeight;
     packing.numHiRegions = pRegion->numHiRegions;
     packing.lowResPicWidth = pRegion->lowResPicWidth;
     packing.lowResPicHeight = pRegion->lowResPicHeight;
     packing.timeStamp = pRegion->timeStamp;
     RectangularRegionWisePacking* inputRegion = pRegion->rectRegionPacking;
     packing.regionsSize = pRegion->numRegions;
     packing.pRegions = new RegionStruct[pRegion->numRegions];
     if (packing.pRegions == NULL)
         return -1;
     RegionStruct* pRegionSructTmp = packing.pRegions;
     for (int32_t i = 0; i < pRegion->numRegions; i++)
     {
        // projected as such without any moves
        pRegionSructTmp->projRegionTop = inputRegion->projRegTop;
        pRegionSructTmp->projRegionLeft = inputRegion->projRegLeft;
        pRegionSructTmp->projRegionWidth = inputRegion->projRegWidth;
        pRegionSructTmp->projRegionHeight = inputRegion->projRegHeight;
        pRegionSructTmp->packedRegionTop = inputRegion->packedRegTop;
        pRegionSructTmp->packedRegionLeft = inputRegion->packedRegLeft;
        pRegionSructTmp->packedRegionWidth = inputRegion->packedRegWidth;
        pRegionSructTmp->packedRegionHeight = inputRegion->packedRegHeight;

        pRegionSructTmp->rwpTransformType = inputRegion->transformType;

        pRegionSructTmp->rwpGuardBand = false;

        pRegionSructTmp++;
        inputRegion++;
     }

     uint32_t ret = writeRwpkSEINal(stream, packing, temporalIdPlus1);

     SAFE_DELETE_ARRAY(packing.pRegions);
     return ret;
 }

 uint32_t hevc_write_SphereRotSEI(GTS_BitStream * stream, const SphereRotation* pSphereRot, int32_t temporalIdPlus1)
 {
     SphereRotationSEI sphereRot;
     sphereRot.sphereRotationCancel = false;
     sphereRot.sphereRotationPersistence = true; // valid until updated
     sphereRot.pitchRotation = pSphereRot->pitchRotation;
     sphereRot.rollRotation = pSphereRot->rollRotation;
     sphereRot.yawRotation = pSphereRot->yawRotation;
     uint32_t ret = writeRotationSEINal(stream, sphereRot, temporalIdPlus1);
     return ret;
 }

 uint32_t hevc_write_FramePackingSEI(GTS_BitStream * stream, const FramePacking* pFramePacking, int32_t temporalIdPlus1)
 {
     FramePackingSEI framePacking;
     framePacking.fpCancel = false;
     framePacking.fpArrangementPersistence = true; // valid until updated
     framePacking.contentInterpretationType = pFramePacking->contentInterpretationType;
     framePacking.currentFrameIsFrame0 = pFramePacking->currentFrameIsFrame0;
     framePacking.fieldViews = pFramePacking->fieldViews;
     framePacking.fpArrangementId = pFramePacking->fpArrangementId;
     framePacking.fpArrangementType = pFramePacking->fpArrangementType;
     framePacking.frame0Flipped = pFramePacking->frame0Flipped;
     framePacking.frame0GridX = pFramePacking->frame0GridX;
     framePacking.frame0GridY = pFramePacking->frame0GridY;
     framePacking.frame1GridX = pFramePacking->frame0GridX;
     framePacking.frame1GridY = pFramePacking->frame0GridY;
     framePacking.frame0SelfContained = pFramePacking->frame0SelfContained;
     framePacking.frame1SelfContained = pFramePacking->frame1SelfContained;
     framePacking.quincunxSamplingFlag = pFramePacking->quincunxSamplingFlag;
     framePacking.spatialFlipping = pFramePacking->spatialFlipping;
     framePacking.upsampledAspectRatio = pFramePacking->upsampledAspectRatio;
     uint32_t ret = writeFramePackingSEINal(stream, framePacking, temporalIdPlus1);
     return ret;
 }

 uint32_t hevc_write_ViewportSEI(GTS_BitStream * stream, const OMNIViewPort* pViewport, int32_t temporalIdPlus1)
 {
     ViewportSEI viewPort;
     viewPort.vpCancel = false;
     viewPort.vpPersistence = true; // valid until updated
     viewPort.vpId = pViewport->vpId;
     viewPort.viewportsSize = pViewport->viewportsSize;
     oneViewport* pViewportInput = pViewport->pViewports;
     ViewportStruct viewportTmp;
     viewPort.pViewports = new ViewportStruct[pViewport->viewportsSize];
     if (viewPort.pViewports == NULL)
         return -1;
     ViewportStruct* pViewportTmp = viewPort.pViewports;

     for (int32_t i = 0; i < pViewport->viewportsSize; i++)
     {
         viewportTmp.AzimuthCentre = pViewportInput->AzimuthCentre;
         viewportTmp.ElevationCentre = pViewportInput->ElevationCentre;
         viewportTmp.HorzRange = pViewportInput->HorzRange;
         viewportTmp.tiltCentre = pViewportInput->tiltCentre;
         viewportTmp.VertRange = pViewportInput->VertRange;
         pViewportTmp = &viewportTmp;
         pViewportTmp++;
         pViewportInput++;
     }
     uint32_t ret = writeRotationSEINal(stream, viewPort, temporalIdPlus1);
     SAFE_DELETE_ARRAY(viewPort.pViewports);
     return ret;
 }
