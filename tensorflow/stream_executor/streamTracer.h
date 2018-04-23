#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER streamTracer

#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_INCLUDE "./streamTracer.h"

#if !defined(_streamTracer_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define _streamTracer_H

#include <lttng/tracepoint.h>

TRACEPOINT_EVENT(
    streamTracer,
    stream_then_memcpy_host_to_device_start,
    TP_ARGS(
        const char*, cat_arg,
        const char*, name_arg
    ),
    TP_FIELDS(
        ctf_string(cat, cat_arg)
        ctf_string(name, name_arg)
    )
)
TRACEPOINT_EVENT(
    streamTracer,
    stream_then_memcpy_host_to_device_end,
    TP_ARGS(
        const char*, cat_arg,
        const char*, name_arg
    ),
    TP_FIELDS(
        ctf_string(cat, cat_arg)
        ctf_string(name, name_arg)
    )
)
TRACEPOINT_EVENT(
    streamTracer,
    stream_then_memcpy_device_to_host_start,
    TP_ARGS(
        const char*, cat_arg,
        const char*, name_arg
    ),
    TP_FIELDS(
        ctf_string(cat, cat_arg)
        ctf_string(name, name_arg)
    )
)
TRACEPOINT_EVENT(
    streamTracer,
    stream_then_memcpy_device_to_host_end,
    TP_ARGS(
        const char*, cat_arg,
        const char*, name_arg
    ),
    TP_FIELDS(
        ctf_string(cat, cat_arg)
        ctf_string(name, name_arg)
    )
)

TRACEPOINT_EVENT(
    streamTracer,
    stream_then_memcpy_device_to_device_start,
    TP_ARGS(
        const char*, cat_arg,
        const char*, name_arg
    ),
    TP_FIELDS(
        ctf_string(cat, cat_arg)
        ctf_string(name, name_arg)
    )
)
TRACEPOINT_EVENT(
    streamTracer,
    stream_then_memcpy_device_to_device_end,
    TP_ARGS(
        const char*, cat_arg,
        const char*, name_arg
    ),
    TP_FIELDS(
        ctf_string(cat, cat_arg)
        ctf_string(name, name_arg)
    )
)

#endif

#include <lttng/tracepoint-event.h>
