/* Copyright 2016 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#if GOOGLE_CUDA || TENSORFLOW_USE_ROCM

#define EIGEN_USE_GPU


#include "tensorflow/core/kernels/segment_reduction_ops.h"

#include "tensorflow/core/framework/register_types.h"
#include "tensorflow/core/util/gpu_kernel_helper.h"

namespace tensorflow {

using GPUDevice = Eigen::GpuDevice;

// Helper for UnusortedSegmentSumCustomKernel that adds value into dest
// atomically.
template <typename T>
static __device__ __forceinline__ void AccumulateInto(T* dest, const T& value) {
  GpuAtomicAdd(dest, value);
}

// Specializations of AccumulateInto for complex types, which GpuAtomicAdd does
// not support. We treat a std::complex<T>* as a T* (the C++ standard section
// 26.4.4 allows this explicitly) and atomic add the real and imaginary
// components individually. The operation as a whole is not atomic, but we can
// safely treat the components independently for the purpose of accumulating.
template <>
__device__ __forceinline__ void AccumulateInto(
    std::complex<float>* dest, const std::complex<float>& value) {
  auto dest_scalar = reinterpret_cast<float*>(dest);
  GpuAtomicAdd(dest_scalar, value.real());
  GpuAtomicAdd(dest_scalar + 1, value.imag());
}

template <>
__device__ __forceinline__ void AccumulateInto(
    std::complex<double>* dest, const std::complex<double>& value) {
  auto dest_scalar = reinterpret_cast<double*>(dest);
  GpuAtomicAdd(dest_scalar, value.real());
  GpuAtomicAdd(dest_scalar + 1, value.imag());
}

// UnsortedSegmentSumFunctor kernel processes 'input_total_size' elements.
// Each element is mapped from input to output by a combination of its
// 'segment_ids' mapping and 'inner_dim_size'.
template <typename T, typename Index>
__global__ void UnsortedSegmentSumCustomKernel(
    const Index input_outer_dim_size, const Index inner_dim_size,
    const Index output_outer_dim_size, const Index* segment_ids, const T* input,
    T* output) {
  const Index input_total_size = input_outer_dim_size * inner_dim_size;
  const Index output_total_size = output_outer_dim_size * inner_dim_size;
  GPU_1D_KERNEL_LOOP(input_index, input_total_size) {
    const Index input_segment_index = input_index / inner_dim_size;
    const Index segment_offset = input_index % inner_dim_size;
    const Index output_segment_index = segment_ids[input_segment_index];

    if (output_segment_index < 0 || output_segment_index >= output_total_size) {
      continue;
    }
    const Index output_index =
        output_segment_index * inner_dim_size + segment_offset;
    AccumulateInto<T>(output + output_index, ldg(input + input_index));
  }
}

namespace functor {

// UnsortedSegmentSumFunctor implementation for GPUDevice.
template <typename T, typename Index>
struct UnsortedSegmentSumFunctor<GPUDevice, T, Index>: UnsortedSegmentBaseFunctor<GPUDevice, T, Index> {
  void operator()(OpKernelContext* ctx, const GPUDevice& d,
                  const Index output_rows, const TensorShape& segment_ids_shape,
                  typename TTypes<Index>::ConstFlat segment_ids,
                  const Index data_size, const T* data,
                  typename TTypes<T, 2>::Tensor output) override {
    if (output.size() == 0) {
      return;
    }

    // Set 'output' to zeros.
    GpuLaunchConfig config = GetGpuLaunchConfig(output.size(), d);
    GPU_LAUNCH_KERNEL(SetZero<T>,
        dim3(config.block_count), dim3(config.thread_per_block), 0, d.stream(),
        output.size(), output.data());
    if (data_size == 0 || segment_ids_shape.num_elements() == 0) {
      return;
    }

    // Launch kernel to compute unsorted segment sum.
    // Notes:
    // *) 'input_total_size' is the total number of elements to process.
    // *) 'segment_ids.shape' is a prefix of data's shape.
    // *) 'input_outer_dim_size' is the total number of segments to process.
    const Index input_total_size = data_size;
    const Index input_outer_dim_size = segment_ids.dimension(0);
    const Index input_inner_dim_size = input_total_size / input_outer_dim_size;

    config = GetGpuLaunchConfig(input_total_size, d);
    GPU_LAUNCH_KERNEL(UnsortedSegmentSumCustomKernel<T, Index>,
        dim3(config.block_count), dim3(config.thread_per_block), 0, d.stream(),
        input_outer_dim_size, input_inner_dim_size, output_rows,
        segment_ids.data(), data, output.data());
  }
};

#define DEFINE_GPU_SPECS_INDEX(T, Index) \
  template struct UnsortedSegmentSumFunctor<GPUDevice, T, Index>

#define DEFINE_GPU_SPECS(T)         \
  DEFINE_GPU_SPECS_INDEX(T, int32); \
  DEFINE_GPU_SPECS_INDEX(T, int64);

TF_CALL_GPU_NUMBER_TYPES_NO_HALF(DEFINE_GPU_SPECS);
TF_CALL_complex64(DEFINE_GPU_SPECS);
TF_CALL_complex128(DEFINE_GPU_SPECS);

#undef DEFINE_GPU_SPECS
#undef DEFINE_GPU_SPECS_INDEX

}  // namespace functor
}  // namespace tensorflow

#endif  // GOOGLE_CUDA || TENSORFLOW_USE_ROCM
