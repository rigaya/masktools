#ifndef __MT_COMMON_FUNCTIONS_H__
#define __MT_COMMON_FUNCTIONS_H__

#include <cuda_runtime.h>
#include <device_launch_parameters.h>

// CUDAマクロの定義
#ifndef __host__
#define __host__
#endif

#ifndef __device__
#define __device__
#endif

// CUDAエラー処理関数
inline void OnCudaError(cudaError_t err) {
    if (err != cudaSuccess) {
        throw std::runtime_error(cudaGetErrorString(err));
    }
}

// 共通関数の定義
template<typename T>
__host__ __device__ T min(T v1, T v2) {
    return (v1 < v2) ? v1 : v2;
}

template<typename T>
__host__ __device__ T max(T v1, T v2) {
    return (v1 > v2) ? v1 : v2;
}

template<typename T>
__host__ __device__ T abs(T v1) {
    return (v1 < 0) ? -v1 : v1;
}

template<typename T>
__host__ __device__ T clamp(T n, T minv, T maxv) {
    return min(max(n, minv), maxv);
}

// CUDAエラーチェックマクロ
#define CUDA_CHECK(call) \
    do { \
        cudaError_t err__ = call; \
        if (err__ != cudaSuccess) { \
            OnCudaError(err__); \
        } \
    } while(0)

#endif // __MT_COMMON_FUNCTIONS_H__ 