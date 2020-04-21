#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER mthq_tp_provider

#if !defined(_TRACEPOINT_MTHQ_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define _TRACEPOINT_MTHQ_H

#include <lttng/tracepoint.h>
//T1
TRACEPOINT_EVENT(
    mthq_tp_provider,
    T1_change_to_lowQ,
    TP_ARGS(
        int,        change_times
    ),

    TP_FIELDS(
        ctf_integer(int, change_times_field, change_times)
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
    T5_read_start_time,
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
    T6_read_end_time,
    TP_ARGS(
        int,        segment_id
    ),

    TP_FIELDS(
        ctf_integer(int, segment_id_field, segment_id)
    )
)
//T7
TRACEPOINT_EVENT(
    mthq_tp_provider,
    T7_get_packet,
    TP_ARGS(
        int,        segment_id
    ),

    TP_FIELDS(
        ctf_integer(int, segment_id_field, segment_id)
    )
)
//T8
TRACEPOINT_EVENT(
    mthq_tp_provider,
    T8_decode_finish,
    TP_ARGS(
        int,        render_fifo_num
    ),

    TP_FIELDS(
        ctf_integer(int, render_fifo_num_field, render_fifo_num)
    )
)
//T9
TRACEPOINT_EVENT(
    mthq_tp_provider,
    T9_render,
    TP_ARGS(
        uint64_t,   render_times
    ),

    TP_FIELDS(
        ctf_integer(uint64_t, render_times_field, render_times)
    )
)
//T9'
TRACEPOINT_EVENT(
    mthq_tp_provider,
    T9n_change_to_highQ,
    TP_ARGS(
        int,        change_times
    ),

    TP_FIELDS(
        ctf_integer(int, change_times_field, change_times)
    )
)
//fifo num
TRACEPOINT_EVENT(
    mthq_tp_provider,
    decode_fifo,
    TP_ARGS(
        int,        decode_fifo_num
    ),

    TP_FIELDS(
        ctf_integer(int, decode_fifo_num_field, decode_fifo_num)
    )
)
//stream information
TRACEPOINT_EVENT(
    mthq_tp_provider,
    stream_information,
    TP_ARGS(
        char*,        dash_mode,
        int,          segment_duration,
        float,        total_duration,
        float,        frame_rate,
        int,          frame_num
    ),

    TP_FIELDS(
        ctf_string(dash_mode_field, dash_mode)
        ctf_integer(int, segment_duration_field, segment_duration)
        ctf_float(float, total_duration_field, total_duration)
        ctf_float(float, frame_rate_field, frame_rate)
        ctf_integer(int, frame_num_field, frame_num)
    )
)


#endif /* _TRACEPOINT_MTHQ_H */

#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_INCLUDE "./MtHQ_tp.h"

#include <lttng/tracepoint-event.h>
