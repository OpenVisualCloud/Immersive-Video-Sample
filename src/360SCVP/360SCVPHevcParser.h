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
#ifndef _360SCVP_HEVC_PARSER_H_
#define _360SCVP_HEVC_PARSER_H_

#include <vector>
#include "360SCVPBitstream.h"
#include "360SCVPCommonDef.h"
#include "360SCVPAPI.h"
#ifndef MIN
#define MIN(X, Y) ((X)<(Y)?(X):(Y))
#endif
#ifndef MAX
#define MAX(X, Y) ((X)>(Y)?(X):(Y))
#endif

#define LCU_SIZE 64

enum chroma_format {
    CSP_400 = 0,
    CSP_420 = 1,
    CSP_422 = 2,
    CSP_444 = 3,
};

/*!
HEVC NAL unit types
*/
enum
{
    /*! Trail N HEVC slice*/
    GTS_HEVC_NALU_SLICE_TRAIL_N = 0,
    /*! Trail R HEVC slice*/
    GTS_HEVC_NALU_SLICE_TRAIL_R = 1,
    /*! TSA N HEVC slice*/
    GTS_HEVC_NALU_SLICE_TSA_N = 2,
    /*! TSA R HEVC slice*/
    GTS_HEVC_NALU_SLICE_TSA_R = 3,
    /*! STSA N HEVC slice*/
    GTS_HEVC_NALU_SLICE_STSA_N = 4,
    /*! STSA R HEVC slice*/
    GTS_HEVC_NALU_SLICE_STSA_R = 5,
    /*! RADL N HEVC slice*/
    GTS_HEVC_NALU_SLICE_RADL_N = 6,
    /*! RADL R HEVC slice*/
    GTS_HEVC_NALU_SLICE_RADL_R = 7,
    /*! RASL N HEVC slice*/
    GTS_HEVC_NALU_SLICE_RASL_N = 8,
    /*! RASL R HEVC slice*/
    GTS_HEVC_NALU_SLICE_RASL_R = 9,
    /*! BLA LP HEVC slice*/
    GTS_HEVC_NALU_SLICE_BLA_W_LP = 16,
    /*! BLA DLP HEVC slice*/
    GTS_HEVC_NALU_SLICE_BLA_W_DLP = 17,
    /*! BLA no LP HEVC slice*/
    GTS_HEVC_NALU_SLICE_BLA_N_LP = 18,
    /*! IDR DLP HEVC slice*/
    GTS_HEVC_NALU_SLICE_IDR_W_DLP = 19,
    /*! IDR HEVC slice*/
    GTS_HEVC_NALU_SLICE_IDR_N_LP = 20,
    /*! CRA HEVC slice*/
    GTS_HEVC_NALU_SLICE_CRA = 21,
    /*! Video Parameter Set*/
    GTS_HEVC_NALU_VID_PARAM = 32,
    /*! Sequence Parameter Set*/
    GTS_HEVC_NALU_SEQ_PARAM = 33,
    /*! Picture Parameter Set*/
    GTS_HEVC_NALU_PIC_PARAM = 34,
    /*! AU delimiter*/
    GTS_HEVC_NALU_ACCESS_UNIT = 35,
    /*! End of sequence*/
    GTS_HEVC_NALU_END_OF_SEQ = 36,
    /*! End of stream*/
    GTS_HEVC_NALU_END_OF_STREAM = 37,
    //SEI
    GTS_HEVC_NALU_FILLER_DATA = 38,
    GTS_HEVC_NALU_PREFIX_SEI = 39,
    GTS_HEVC_NALU_SUFFIX_SEI = 40,

};

struct EquirectangularProjectionSEI
{
    bool erpCancel;
    bool erpPersistence;
    bool erpGuardBand;
    uint8_t erpGuardBandType;
    uint8_t erpLeftGuardBandWidth;
    uint8_t erpRightGuardBandWidth;
};

struct CubemapProjectionSEI
{
    bool cmpCancel;
    bool cmpPersistence;
};

struct FramePackingSEI
{
    uint32_t fpArrangementId;
    bool fpCancel;
    uint8_t fpArrangementType;
    bool quincunxSamplingFlag;
    uint8_t contentInterpretationType;
    bool spatialFlipping;
    bool frame0Flipped;
    bool fieldViews;
    bool currentFrameIsFrame0;
    bool frame0SelfContained;
    bool frame1SelfContained;
    uint8_t frame0GridX;
    uint8_t frame0GridY;
    uint8_t frame1GridX;
    uint8_t frame1GridY;
    bool fpArrangementPersistence;
    bool upsampledAspectRatio;
};

struct SphereRotationSEI
{
    bool sphereRotationCancel;
    bool sphereRotationPersistence;
    int32_t yawRotation;
    int32_t pitchRotation;
    int32_t rollRotation;
};

struct RegionStruct
{
    uint8_t  rwpReserved4Bits;
    uint8_t  rwpTransformType;
    bool     rwpGuardBand;
    uint32_t projRegionWidth;
    uint32_t projRegionHeight;
    uint32_t projRegionTop;
    uint32_t projRegionLeft;
    uint16_t packedRegionWidth;
    uint16_t packedRegionHeight;
    uint16_t packedRegionTop;
    uint16_t packedRegionLeft;

    uint8_t leftGbWidth;
    uint8_t rightGbWidth;
    uint8_t topGbHeight;
    uint8_t bottomGbHeight;
    bool    gbNotUsedForPredFlag;
    uint8_t gbType0;
    uint8_t gbType1;
    uint8_t gbType2;
    uint8_t gbType3;
    //+  reserved 3 bits
};

struct RegionWisePackingSEI
{
    bool rwpCancel;
    bool rwpPersistence;
    bool constituentPictureMatching;
    uint32_t projPictureWidth;
    uint32_t projPictureHeight;
    uint16_t packedPictureWidth;
    uint16_t packedPictureHeight;
    uint16_t regionsSize;
    RegionStruct* pRegions;
    uint8_t  numHiRegions;
    uint32_t lowResPicWidth;
    uint32_t lowResPicHeight;
    uint32_t timeStamp;
};

struct ViewportStruct
{
    int32_t  AzimuthCentre;
    int32_t  ElevationCentre;
    int32_t  tiltCentre;
    uint32_t HorzRange;
    uint32_t VertRange;
};

struct ViewportSEI
{
    bool vpCancel;
    bool vpPersistence;
    uint32_t vpId;
    uint16_t viewportsSize;
    ViewportStruct* pViewports;
};

typedef struct
{
    bool profile_present_flag, level_present_flag, tier_flag;
    uint8_t profile_space;
    uint8_t profile_idc;
    uint32_t profile_compatibility_flag;
    uint8_t level_idc;
} HEVC_SublayerPTL;

typedef struct
{
    uint8_t profile_space, tier_flag, profile_idc, level_idc;
    uint32_t profile_compatibility_flag;
    bool general_progressive_source_flag;
    bool general_interlaced_source_flag;
    bool general_non_packed_constraint_flag;
    bool general_frame_only_constraint_flag;
    uint64_t general_reserved_44bits;

    HEVC_SublayerPTL sub_ptl[8];
} HEVC_ProfileTierLevel;

typedef struct
{
    bool     inter_ref_pic_set_prediction_flag;
    uint32_t index_num;
    bool     delta_rps_sign;
    uint32_t index_num_absolute;
    bool     used_by_curr_pic_flag[256];
    bool     used_delta_flag[256];
    uint32_t num_negative_pics;
    uint32_t num_positive_pics;
    int32_t  delta_poc[16];
    int32_t  delta_poc0[16];
    bool     used_by_curr_pic_s0_flag[16];
    int32_t  delta_poc1[16];
    bool     used_by_curr_pic_s1_flag[16];
} HEVC_ReferencePictureSets;

#define MAX_CPB_CNT 32
#define MAX_LHVC_LAYERS    7
#define NUM_LAYER_SETS_MAX 1024

typedef struct
{
    bool fixed_pic_rate_flag;
    bool fixed_pic_rate_within_cvs_flag;
    uint32_t pic_dur_inTc_minus1;
    bool low_delay_hrd_flag;
    uint32_t cpb_cnt_minus1;
    uint32_t bitrate_val_minus1[MAX_CPB_CNT][2];
    uint32_t cpb_size_value[MAX_CPB_CNT][2];
    uint32_t du_cpb_size_value[MAX_CPB_CNT][2];
    bool cbr_flag[MAX_CPB_CNT][2];
    uint32_t du_bitrate_value[MAX_CPB_CNT][2];
} HEVC_HrdSubLayerInfo;

typedef struct
{
    bool nal_hrd_param_present_flag;
    bool vcl_hrd_param_present_flag;
    bool sub_pic_cpb_param_present_flag;
    uint32_t tick_divisor_minus2;
    uint32_t du_cpb_removal_delay_len_minus1;
    bool sub_pic_cpb_param_in_picTSEI_flag;
    uint32_t dpb_out_delay_du_len_minus1;
    uint32_t bit_rate_scale;
    uint32_t cpb_size_scale;
    uint32_t du_cpb_size_scale;
    uint32_t init_cpb_removal_delay_len_minus1;
    uint32_t cpb_removal_delay_len_minus1;
    uint32_t dpb_out_delay_len_minus1;
    HEVC_HrdSubLayerInfo hrd_sub_layer_infos[MAX_LHVC_LAYERS];
} HEVC_HrdInfo;

typedef struct
{
    bool aspect_ratio_info_present_flag;
    uint8_t sar_idc;
    uint16_t sar_width, sar_height;
    bool overscan_info_present, overscan_appropriate;
    bool video_signal_type_present_flag, video_full_range_flag, colour_description_present_flag;
    uint8_t video_format, colour_primaries, transfer_characteristic, matrix_coeffs;
    bool chroma_loc_info_present_flag;
    uint32_t chroma_sample_loc_type_top_field, chroma_sample_loc_type_bottom_field;

    bool neutra_chroma_indication_flag, field_seq_flag, frame_field_info_present_flag, default_display_window_flag;
    uint32_t left_offset, right_offset, top_offset, bottom_offset;
    bool has_timing_info;
    uint32_t num_units_in_tick, time_scale;
    bool poc_proportional_to_timing_flag;
    uint32_t num_ticks_poc_diff_one_minus1;

}HEVC_VUI;

typedef struct
{
    int32_t id, vps_id;
    /*used to discard repeated SPSs - 0: not parsed, 1 parsed, 2 stored*/
    uint32_t state;
    uint32_t crc;
    uint32_t width, height;

    HEVC_ProfileTierLevel ptl;

    uint8_t chroma_format_idc;
    bool cw_flag;
    uint32_t cw_left, cw_right, cw_top, cw_bottom;
    uint8_t bit_depth_luma;
    uint8_t bit_depth_chroma;
    uint8_t log2_max_pic_order_cnt_lsb;
    uint8_t max_sub_layers_minus1;
    bool separate_colour_plane_flag;

    uint32_t max_CU_width, max_CU_height, max_CU_depth;
    uint32_t bitsSliceSegmentAddress;
    uint32_t max_dec_pic_buffering, num_reorder_pics, max_latency_increase;

    uint32_t num_short_term_ref_pic_sets, num_long_term_ref_pic_sps;

    uint32_t max_transform_hierarchy_depth_inter;
    uint32_t max_transform_hierarchy_depth_intra;

    bool vui_parameters_present_flag, long_term_ref_pics_present_flag, temporal_mvp_enable_flag, sample_adaptive_offset_enabled_flag;
    HEVC_VUI vui;

    bool strong_intra_smoothing_enable_flag;

    uint32_t rep_format_idx;
} HEVC_SPS;

typedef struct
{
    int32_t id;
    uint32_t sps_id;
    /*used to discard repeated SPSs - 0: not parsed, 1 parsed, 2 stored*/
    uint32_t state;
    uint32_t crc;

    bool dependent_slice_segments_enabled_flag, tiles_enabled_flag, uniform_spacing_flag, constrained_intra_pred_flag, org_tiles_enabled_flag;
    uint32_t num_extra_slice_header_bits, num_ref_idx_l0_default_active, num_ref_idx_l1_default_active;
    bool slice_segment_header_extension_present_flag, output_flag_present_flag, lists_modification_present_flag, cabac_init_present_flag;
    bool weighted_pred_flag, weighted_bipred_flag, slice_chroma_qp_offsets_present_flag, deblocking_filter_control_present_flag, deblocking_filter_override_enabled_flag, loop_filter_across_slices_enabled_flag, entropy_coding_sync_enabled_flag;
    bool loop_filter_across_tiles_enabled_flag, pps_loop_filter_across_slices_enabled_flag, cu_qp_delta_enabled_flag;

    uint32_t num_tile_columns, num_tile_rows, pic_init_qp_minus26, diff_cu_qp_delta_depth;
    uint32_t column_width[22], row_height[20];
} HEVC_PPS;

typedef struct RepFormat
{
    uint32_t chroma_format_idc;
    uint32_t pic_width_luma_samples;
    uint32_t pic_height_luma_samples;
    uint32_t bit_depth_luma;
    uint32_t bit_depth_chroma;
    uint8_t separate_colour_plane_flag;
} HEVC_RepFormat;

//#define MAX_LHVC_LAYERS    7
//#define NUM_LAYER_SETS_MAX 1024
typedef struct
{
    uint8_t num_profile_tier_level;
    uint8_t num_output_layer_sets;
    uint32_t num_rep_formats;
    uint32_t rep_format_idx[16];
    uint32_t scalability_mask[16];
    uint32_t dimension_id[MAX_LHVC_LAYERS][16];
    uint32_t layer_id_in_nuh[MAX_LHVC_LAYERS];
    uint32_t layer_id_in_vps[MAX_LHVC_LAYERS];
    bool output_layer_flag[MAX_LHVC_LAYERS][MAX_LHVC_LAYERS];
    bool alt_output_layer_flag[MAX_LHVC_LAYERS];
    bool necessary_layers_flag[MAX_LHVC_LAYERS][MAX_LHVC_LAYERS];
    uint8_t num_layers_in_id_list[NUM_LAYER_SETS_MAX];
    uint8_t direct_dependency_flag[MAX_LHVC_LAYERS][MAX_LHVC_LAYERS];
    uint8_t profile_tier_level_idx[MAX_LHVC_LAYERS][MAX_LHVC_LAYERS];
    uint8_t num_necessary_layers[MAX_LHVC_LAYERS];
    uint8_t LayerSetLayerIdList[MAX_LHVC_LAYERS][MAX_LHVC_LAYERS];
    uint8_t LayerSetLayerIdListMax[MAX_LHVC_LAYERS];
    HEVC_ProfileTierLevel ext_ptl[MAX_LHVC_LAYERS];
    HEVC_RepFormat rep_formats[16];
} HEVC_VPSEx;

typedef struct
{
    int32_t id;
    bool vps_extension_flag;
    bool temporal_id_nesting;
    bool base_layer_internal_flag;
    bool base_layer_available_flag;
    //uint8_t num_profile_tier_level;
    //uint8_t num_output_layer_sets;
    int32_t bit_pos_vps_extensions;
    bool     timing_info_present_flag;
    uint32_t num_units_in_tick;
    int32_t num_ticks_poc_diff_one;
    bool    poc_proportional_to_timing_flag;
    uint32_t time_scale;
    uint32_t max_layers;
    uint32_t max_sub_layers;
    uint32_t max_layer_id;
    uint32_t num_layer_sets;
    uint32_t sub_layer_ordering_info_present_flag;
    uint32_t vps_max_dec_pic_buffering_minus1[MAX_LHVC_LAYERS];
    uint32_t vps_max_num_reorder_pics[MAX_LHVC_LAYERS];
    uint32_t vps_max_latency_increase_plus1[MAX_LHVC_LAYERS];
    uint8_t  layer_id_included_flag[MAX_LHVC_LAYERS][64];
    std::vector<uint32_t> hrd_layer_set_idx;
    std::vector<bool> cprms_present_flags;
    //uint32_t num_rep_formats;
    uint32_t state;
    uint32_t crc;
    //uint32_t rep_format_idx[16];
    //uint32_t scalability_mask[16];
    //uint32_t dimension_id[MAX_LHVC_LAYERS][16];
    //uint32_t layer_id_in_nuh[MAX_LHVC_LAYERS];
    //uint32_t layer_id_in_vps[MAX_LHVC_LAYERS];
    HEVC_ProfileTierLevel ptl;
    //bool output_layer_flag[MAX_LHVC_LAYERS][MAX_LHVC_LAYERS];
    //bool alt_output_layer_flag[MAX_LHVC_LAYERS];
    //bool necessary_layers_flag[MAX_LHVC_LAYERS][MAX_LHVC_LAYERS];
    //uint8_t num_layers_in_id_list[NUM_LAYER_SETS_MAX];
    //uint8_t direct_dependency_flag[MAX_LHVC_LAYERS][MAX_LHVC_LAYERS];
    //uint8_t profile_tier_level_idx[MAX_LHVC_LAYERS][MAX_LHVC_LAYERS];
    //uint8_t num_necessary_layers[MAX_LHVC_LAYERS];
    //uint8_t LayerSetLayerIdList[MAX_LHVC_LAYERS][MAX_LHVC_LAYERS];
    //uint8_t LayerSetLayerIdListMax[MAX_LHVC_LAYERS];
    //uint32_t profile_level_tier_idx[MAX_LHVC_LAYERS];
    //HEVC_ProfileTierLevel ext_ptl[MAX_LHVC_LAYERS];
    HEVC_SublayerPTL sub_ptl[8];
    //HEVC_RepFormat rep_formats[16];
    uint32_t num_hrd_parameters;
    std::vector<HEVC_HrdInfo> hrd_infos;
    HEVC_VPSEx vps_extension;
} HEVC_VPS;


typedef struct
{
    uint32_t frame_cnt;
    uint8_t exact_match_flag;
    uint8_t broken_link_flag;
    uint8_t changing_slice_group_idc;
    uint8_t valid;
} AVCSeiRecoveryPoint;


typedef struct
{
    uint8_t pic_struct;
    /*to be eventually completed by other pic_timing members*/
} AVCSeiPicTiming;

typedef struct
{
    AVCSeiRecoveryPoint recovery_point;
    AVCSeiPicTiming pic_timing;

} HEVC_SEI;

#define MAX_NUM_LONG_TERM_REF_PICS 33

typedef struct
{
    uint8_t nal_unit_type;
    uint8_t temporal_id;
    uint32_t frame_num, poc_lsb, slice_type;

    int32_t redundant_pic_cnt, short_term_ref_pic_set_idx;

    int32_t poc;
    uint32_t poc_msb, poc_msb_prev, poc_lsb_prev, frame_num_prev;
    int32_t frame_num_offset, frame_num_offset_prev;

    bool dependent_slice_segment_flag, short_term_ref_pic_set_sps_flag, slice_sao_luma_flag, slice_sao_chroma_flag;
    bool no_output_of_prior_pics_flag;
    int32_t index_hevc_pps;
    uint32_t slice_reserved_flag[16];
    bool pic_output_flag;
    uint32_t colour_plane_id;
    uint32_t count_hevc_lts, count_hevc_ltp;
    uint8_t  lt_idx_sps[MAX_NUM_LONG_TERM_REF_PICS];
    uint32_t poc_lsb_lt[MAX_NUM_LONG_TERM_REF_PICS];
    uint8_t  used_by_curr_pic_lt_flag[MAX_NUM_LONG_TERM_REF_PICS];
    uint8_t  delta_poc_msb_present_flag[MAX_NUM_LONG_TERM_REF_PICS];
    uint8_t  poc_MCL_del[MAX_NUM_LONG_TERM_REF_PICS];
    uint32_t count_index_reference_0, count_index_reference_1;
    uint8_t  mvd_l1_zero_flag, cabac_init_flag;
    bool     info_of_cl0;
    uint32_t collocated_ref_idx;
    uint32_t five_minus_max_num_merge_cand;
    bool     info_of_dfof, info_of_sdfdf;
    int32_t  slice_beta_offset_div2, slice_tc_offset_div2;
    bool     slice_loop_filter_across_slices_enabled_flag;
    bool first_slice_segment_in_pic_flag, num_ref_idx_active_override_flag, slice_temporal_mvp_enabled_flag;
    uint32_t slice_segment_address;
    uint8_t prev_layer_id_plus1;

    //bit offset of the num_entry_point (if present) field
    int32_t entry_point_start_bits;
    uint32_t num_entry_point_offsets;
    uint32_t offset_len;
    //uint32_t entry_point_offset_minus1[1024];
    //byte offset of the payload start (after byte alignment)
    int32_t payload_start_offset;

    HEVC_SPS *sps;
    HEVC_PPS *pps;

    int32_t slice_qp_delta, slice_cb_qp_offset, slice_cr_qp_offset;
    //bool used_by_curr_pic_s0_flag[16];
    HEVC_ReferencePictureSets rps[64];

    uint32_t size_ext;
    std::vector<uint8_t> ext_bytes;
} HEVCSliceInfo;

typedef struct _hevc_state
{
    //set by user
    bool full_slice_header_parse;

    //all other vars set by parser

    HEVC_SPS sps[16]; /* range allowed in the spec is 0..15 */
    int8_t sps_active_idx;    /*currently active sps; must be initalized to -1 in order to discard not yet decodable SEIs*/

    HEVC_PPS pps[64];

    HEVC_VPS vps[16];

    HEVCSliceInfo s_info;
    HEVC_SEI sei;

    //-1 or the value of the vps/sps/pps ID of the nal just parsed
    int32_t last_parsed_vps_id;
    int32_t last_parsed_sps_id;
    int32_t last_parsed_pps_id;

    bool first_nal;
    int32_t  tile_slice_count;
} HEVCState;

enum
{
    GF_HEVC_SLICE_TYPE_B = 0,
    GF_HEVC_SLICE_TYPE_P = 1,
    GF_HEVC_SLICE_TYPE_I = 2,
};

int32_t gts_media_hevc_parse_nalu(hevc_specialInfo* pSpecialInfo, int8_t *data, uint32_t size, HEVCState *hevc);
bool gts_media_hevc_slice_is_intra(HEVCState *hevc);
bool gts_media_hevc_slice_is_IDR(HEVCState *hevc);

int32_t gts_media_hevc_stitch_sps(HEVCState *hevc, uint32_t frameWidth, uint32_t frameHeight);
int32_t gts_media_hevc_stitch_pps(HEVCState *hevc, void *gentiledstream);
int32_t gts_media_hevc_stitch_slice_segment(HEVCState *hevc, void* slice, uint32_t frameWidth, uint32_t sub_tile_index);

uint32_t gts_media_nalu_next_start_code_bs(GTS_BitStream *bs);
int32_t hevc_read_RwpkSEI(int8_t *pRWPKBits, uint32_t RWPKBitsSize, RegionWisePacking* pRWPK);
#define MAX_TILE_ROWS 64
#define MAX_TILE_COLS 64

#endif
