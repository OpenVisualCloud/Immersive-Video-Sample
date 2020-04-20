#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER bandwidth_tp_provider

#if !defined(_TRACEPOINT_BANDWIDTH_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define _TRACEPOINT_BANDWIDTH_H

#include <lttng/tracepoint.h>

TRACEPOINT_EVENT(
    bandwidth_tp_provider,
    tiles_selection_redundancy,
    TP_ARGS(
        int, viewport_net_width,
        int, viewport_net_height,
        int, viewport_tiled_width,
        int, viewport_tiled_height
    ),

    TP_FIELDS(
        ctf_integer(int, net_width_field, viewport_net_width)
        ctf_integer(int, net_height_field, viewport_net_height)
        ctf_integer(int, tiled_width_field, viewport_tiled_width)
        ctf_integer(int, tiled_height_field, viewport_tiled_height)
    )
)

TRACEPOINT_EVENT(
    bandwidth_tp_provider,
    encoded_frame_size,
    TP_ARGS(
        char*, resolution,
        char*, tile_split,
        int,   frame_index,
        int,   frame_size
    ),

    TP_FIELDS(
        ctf_string(resolution_field, resolution)
        ctf_string(tile_split_field, tile_split)
        ctf_integer(int, frame_index_field, frame_index)
        ctf_integer(int, frame_size_field, frame_size)
    )
)

TRACEPOINT_EVENT(
    bandwidth_tp_provider,
    packed_segment_size,
    TP_ARGS(
        int,         track_index,
        const char*, track_type,
        int,         segment_index,
        int,         segment_size
    ),

    TP_FIELDS(
        ctf_integer(int, track_index_field, track_index)
        ctf_string(track_type_field, track_type)
        ctf_integer(int, segment_index_field, segment_index)
        ctf_integer(int, segment_size_field, segment_size)
    )
)

#endif /* _TRACEPOINT_BANDWIDTH_H */

#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_INCLUDE "./Bandwidth_tp.h"

#include <lttng/tracepoint-event.h>
