#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER mthq_tp_provider

#if !defined(_TRACEPOINT_MTHQ_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define _TRACEPOINT_MTHQ_H

#include <lttng/tracepoint.h>

//T0
TRACEPOINT_EVENT(
    mthq_tp_provider,
    T0_change_to_lowQ,
    TP_ARGS(
        int,        change_times,
        uint64_t,   pts
    ),

    TP_FIELDS(
        ctf_integer(int, change_times_field, change_times)
        ctf_integer(uint64_t, pts_field, pts)
    )
)
//T1
TRACEPOINT_EVENT(
    mthq_tp_provider,
    T1_select_tracks,
    TP_ARGS(
        char*,      track_type
    ),

    TP_FIELDS(
        ctf_string(track_type_field, track_type)
    )
)
//T2
TRACEPOINT_EVENT(
    mthq_tp_provider,
    T2_detect_pose_change,
    TP_ARGS(
        int,        is_changed
    ),

    TP_FIELDS(
        ctf_integer(int, is_changed_field, is_changed)
    )
)
//T3
TRACEPOINT_EVENT(
    mthq_tp_provider,
    T3_start_download_time,
    TP_ARGS(
        int,        segment_id
    ),

    TP_FIELDS(
        ctf_integer(int, segment_id_field, segment_id)
    )
)
//T4
TRACEPOINT_EVENT(
    mthq_tp_provider,
    T4_parse_start_time,
    TP_ARGS(
        int,        segment_id
    ),

    TP_FIELDS(
        ctf_integer(int, segment_id_field, segment_id)
    )
)
//T5
TRACEPOINT_EVENT(
    mthq_tp_provider,
    T5_parse_end_time,
    TP_ARGS(
        int,        segment_id
    ),

    TP_FIELDS(
        ctf_integer(int, segment_id_field, segment_id)
    )
)
//T6
TRACEPOINT_EVENT(
    mthq_tp_provider,
    T6_stitch_start_time,
    TP_ARGS(
        uint64_t,        pts
    ),

    TP_FIELDS(
        ctf_integer(uint64_t, pts_field, pts)
    )
)
//T7
TRACEPOINT_EVENT(
    mthq_tp_provider,
    T7_stitch_end_time,
    TP_ARGS(
        int,             segment_id,
        uint64_t,        pts,
        int,             video_num
    ),

    TP_FIELDS(
        ctf_integer(int, segment_id_field, segment_id)
        ctf_integer(uint64_t, pts_field, pts)
        ctf_integer(int, video_num_field, video_num)
    )
)
//T8
TRACEPOINT_EVENT(
    mthq_tp_provider,
    T8_get_packet,
    TP_ARGS(
        uint64_t,        pts
    ),

    TP_FIELDS(
        ctf_integer(uint64_t, pts_field, pts)
    )
)
//T9
TRACEPOINT_EVENT(
    mthq_tp_provider,
    T9_push_frame_to_fifo,
    TP_ARGS(
        uint64_t,        pts,
        int,             video_id
    ),

    TP_FIELDS(
        ctf_integer(uint64_t, pts_field, pts)
        ctf_integer(int, video_id_field, video_id)
    )
)
//T10
TRACEPOINT_EVENT(
    mthq_tp_provider,
    T10_decode_time_cost,
    TP_ARGS(
        uint64_t,        pts,
        int,             video_id,
        uint64_t,        timeMS,
        int,             width,
        int,             height
    ),

    TP_FIELDS(
        ctf_integer(uint64_t, pts_field, pts)
        ctf_integer(int, video_id_field, video_id)
        ctf_integer(uint64_t, timeMS_field, timeMS)
        ctf_integer(int, width_field, width)
        ctf_integer(int, height_field, height)
    )
)
//T11
TRACEPOINT_EVENT(
    mthq_tp_provider,
    T11_update_time,
    TP_ARGS(
        uint64_t,        pts
    ),

    TP_FIELDS(
        ctf_integer(uint64_t, pts_field, pts)
    )
)
//T12
TRACEPOINT_EVENT(
    mthq_tp_provider,
    T12_change_to_highQ,
    TP_ARGS(
        int,        change_times,
        uint64_t,   pts
    ),

    TP_FIELDS(
        ctf_integer(int, change_times_field, change_times)
        ctf_integer(uint64_t, pts_field, pts)
    )
)
//T13
TRACEPOINT_EVENT(
    mthq_tp_provider,
    T13_render_time,
    TP_ARGS(
        uint64_t,    render_count
    ),

    TP_FIELDS(
        ctf_integer(uint64_t, render_count_field, render_count)
    )
)
//stream information
TRACEPOINT_EVENT(
    mthq_tp_provider,
    stream_information,
    TP_ARGS(
        char*,        dash_mode,
        int,          projection_format,
        int,          segment_duration,
        float,        total_duration,
	    float,        frame_rate,
        int,          frame_num,
        uint32_t,     highreso_width,
        uint32_t,     highreso_height
    ),

    TP_FIELDS(
        ctf_string(dash_mode_field, dash_mode)
        ctf_integer(int, projection_format_field, projection_format)
        ctf_integer(int, segment_duration_field, segment_duration)
        ctf_float(float, total_duration_field, total_duration)
        ctf_float(float, frame_rate_field, frame_rate)
        ctf_integer(int, frame_num_field, frame_num)
        ctf_integer(uint32_t, highreso_width_field, highreso_width)
        ctf_integer(uint32_t, highreso_height_field, highreso_height)
    )
)


#endif /* _TRACEPOINT_MTHQ_H */

#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_INCLUDE "./MtHQ_tp.h"

#include <lttng/tracepoint-event.h>
