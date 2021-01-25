#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER E2E_latency_tp_provider

#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_INCLUDE "./E2E_latency_tp.h"

#if !defined(_E2E_LATENCY_TP_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define _E2E_LATENCY_TP_H

#include <lttng/tracepoint.h>
#include <lttng/tracepoint-event.h>

// [T1] before distributed encoder encode
TRACEPOINT_EVENT(
    E2E_latency_tp_provider,
    pre_de_info,
    TP_ARGS(
        int,         idx,
        const char*, tag
    ),

    TP_FIELDS(
        ctf_integer(int, idx_field, idx)
        ctf_string(tag_field, tag)
    )
)

// [T2] after distributed encoder encode
TRACEPOINT_EVENT(
    E2E_latency_tp_provider,
    post_de_info,
    TP_ARGS(
        int,         idx,
        const char*, tag
    ),

    TP_FIELDS(
        ctf_integer(int, idx_field, idx)
        ctf_string(tag_field, tag)
    )
)

// [T3] before OMAFPacking operation
TRACEPOINT_EVENT(
    E2E_latency_tp_provider,
    pre_op_info,
    TP_ARGS(
        int,         idx,
        const char*, tag
    ),

    TP_FIELDS(
        ctf_integer(int, idx_field, idx)
        ctf_string(tag_field, tag)
    )
)

// [T4] after OMAFPacking operation
TRACEPOINT_EVENT(
    E2E_latency_tp_provider,
    post_op_info,
    TP_ARGS(
        int,         idx,
        const char*, tag
    ),

    TP_FIELDS(
        ctf_integer(int, idx_field, idx)
        ctf_string(tag_field, tag)
    )
)

// [T5] DashAccess start to download segment
TRACEPOINT_EVENT(
    E2E_latency_tp_provider,
    pre_da_info,
    TP_ARGS(
        int,         idx,
        const char*, tag
    ),

    TP_FIELDS(
        ctf_integer(int, idx_field, idx)
        ctf_string(tag_field, tag)
    )
)

// [T6] after DashAccess download and parse segment
TRACEPOINT_EVENT(
    E2E_latency_tp_provider,
    post_da_info,
    TP_ARGS(
        int,         idx,
        const char*, tag
    ),

    TP_FIELDS(
        ctf_integer(int, idx_field, idx)
        ctf_string(tag_field, tag)
    )
)

// [T7] before player render
TRACEPOINT_EVENT(
    E2E_latency_tp_provider,
    pre_rd_info,
    TP_ARGS(
        int,         idx,
        const char*, tag
    ),

    TP_FIELDS(
        ctf_integer(int, idx_field, idx)
        ctf_string(tag_field, tag)
    )
)

// [T8] after player render
TRACEPOINT_EVENT(
    E2E_latency_tp_provider,
    post_rd_info,
    TP_ARGS(
        int,         idx,
        const char*, tag
    ),

    TP_FIELDS(
        ctf_integer(int, idx_field, idx)
        ctf_string(tag_field, tag)
    )
)

// [S1] index of first segment
TRACEPOINT_EVENT(
    E2E_latency_tp_provider,
    da_ssi_info,
    TP_ARGS(
        int,         idx
    ),

    TP_FIELDS(
        ctf_integer(int, idx_field, idx)
    )
)

#endif
