#include <string>
#include <cstdint>

#include <cuda_runtime.h>

#include <libgpuc_cuda.hpp>

// TODO implement stride and split
/// Adds two tensors
template<typename T>
__global__ void add2Kernel(T* out, const T* in1, const T* in2, uint64_t n) {
  uint32_t numThreads = blockDim.x * gridDim.x;
  uint32_t thId = threadIdx.x + blockIdx.x * blockDim.x;

  for(uint64_t i = thId; i < n; i += numThreads) {
    out[i] = in1[i] + in2[i];
  }
}

const char* libtcCudaAdd2Ckern(libtcCudaStream& stream, double* out, const double* in1, const double* in2, uint64_t n) {
  auto err = cudaSetDevice(stream.device);
  if (err != cudaSuccess) {
    return cudaGetErrorString(err);
  }
  cudaDeviceProp props;
  err = cudaGetDeviceProperties(&props, stream.device);
  if (err != cudaSuccess) {
    return cudaGetErrorString(err);
  }
  uint32_t numThreads = props.multiProcessorCount * 128;
  if(numThreads > n) {
    numThreads = n;
  }
  
  cudaLaunchConfig_t config = {
    .stream = stream.stream,
  };
  if(numThreads < props.maxThreadsPerBlock) {
    config.blockDim.x = numThreads;
    config.gridDim.x = 1;
  } else {
    config.blockDim.x = props.maxThreadsPerBlock;
    config.gridDim.x = (n + props.maxThreadsPerBlock - 1) / props.maxThreadsPerBlock;
  }

  err = cudaLaunchKernelEx(&config, add2Kernel<double>, out, in1, in2, n);
  if (err != cudaSuccess) {
    return cudaGetErrorString(err);
  }
  // TODO remove
  err = cudaDeviceSynchronize();
  if (err != cudaSuccess) {
    return cudaGetErrorString(err);
  }
  return nullptr;
}

void add2D(Tensor out, Tensor in1, Tensor in2) {
  uint32_t n = getTensorNel(in1);
  if (n != getTensorNel(in2) || n != getTensorNel(out))
    throw std::string("Size mismatch");

  uint32_t threads = n;
  uint32_t blocks = 1;
  if (n > MAX_THREADS_PER_BLOCK) {
    threads = MAX_THREADS_PER_BLOCK;
    blocks = (n + threads - 1) / threads;
  }
  cudaLaunchConfig_t config = {};
  config.blockDim.x = threads;
  config.gridDim.x = blocks;
  auto err = cudaLaunchKernelEx(&config, add2Kernel<double>, out.mem, in1.mem, in2.mem, n);
  if (err != cudaSuccess) {
    throw std::string(cudaGetErrorString(err));
  }
}