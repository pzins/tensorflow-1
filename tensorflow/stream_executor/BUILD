licenses(["restricted"])

load("@local_config_cuda//cuda:build_defs.bzl", "if_cuda_is_configured")
load("@local_config_rocm//rocm:build_defs.bzl", "if_rocm_is_configured")

cc_library(
    name = "stream_executor",
    srcs = glob(
        [
            "*.cc",
            "host/*.cc",
            "cuda/cuda_platform_id.cc",
            "rocm/rocm_platform_id.cc",
            "lib/*.cc",
            "platform/default/*.cc",
            "streamTracer.o"
        ],
        exclude = [
            "**/*_test.cc",
        ],
    ),
    hdrs = glob([
        "*.h",
        "cuda/*.h",
        "rocm/*.h",
        "host/*.h",
        "lib/*.h",
        "lib/gtl/*.h",
        "platform/**/*.h",
    ]),
    linkopts = select({
        "//tensorflow:freebsd": [],
        "//conditions:default": ["-ldl"],
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//tensorflow/core:lib",
        "@local_config_cuda//cuda:cuda_headers",
        "@local_config_rocm//rocm:rocm_headers",
    ],
    alwayslink = 1,
)

cc_library(
    name = "cuda_platform",
    srcs = if_cuda_is_configured(
        glob(
            [
                "cuda/*.cc",
            ],
            exclude = ["cuda/cuda_platform_id.cc"],
        ),
    ),
    hdrs = ["//tensorflow/core:tensorflowTracer.h"],
    linkopts = select({
        "//tensorflow:freebsd": [],
        "//conditions:default": ["-ldl", "-llttng-ust"],
    }),
    visibility = ["//visibility:public"],
    deps = [
        ":stream_executor",
        "//tensorflow/core:lib",
        "@local_config_cuda//cuda:cuda_headers",
    ] + if_cuda_is_configured([
        "//tensorflow/core:cuda",
        "@local_config_cuda//cuda:cublas",
        "@local_config_cuda//cuda:cuda_driver",
        "@local_config_cuda//cuda:cudnn",
        "@local_config_cuda//cuda:cufft",
        "@local_config_cuda//cuda:curand",
    ]),
    alwayslink = 1,
)

cc_library(
    name = "rocm_platform",
    srcs = if_rocm_is_configured(
        glob(
            [
                "rocm/*.cc",
            ],
            exclude = ["rocm/rocm_platform_id.cc"],
        ),
    ),
    linkopts = select({
        "//tensorflow:freebsd": [],
        "//conditions:default": ["-ldl"],
    }),
    visibility = ["//visibility:public"],
    deps = [
        ":stream_executor",
        "//tensorflow/core:lib",
        "@local_config_rocm//rocm:rocm_headers",
    ] + if_rocm_is_configured([
        "//tensorflow/core:rocm",
        "@local_config_rocm//rocm:hiprand",
        "@local_config_rocm//rocm:rocfft",
        "@local_config_rocm//rocm:hipblas",
        "@local_config_rocm//rocm:miopen",
    ]),
    alwayslink = 1,
)
