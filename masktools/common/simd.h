#ifndef __Mt_SIMD_H__
#define __Mt_SIMD_H__

#include <emmintrin.h>
#include <immintrin.h>
#include "common.h"

namespace Filtering {

//because ICC is smart enough on its own and force inlining actually makes it slower
#ifdef __INTEL_COMPILER
#define MT_FORCEINLINE inline
#elif _MSC_VER
#define MT_FORCEINLINE __forceinline
#else
#define MT_FORCEINLINE inline
#endif

#define USE_MOVPS

enum class MemoryMode {
    SSE2_UNALIGNED,
    SSE2_ALIGNED
};


template<MemoryMode mem_mode, typename T>
static MT_FORCEINLINE __m128i simd_load_si128(const T* ptr) {
#ifdef USE_MOVPS
    if (mem_mode == MemoryMode::SSE2_ALIGNED) {
        return _mm_castps_si128(_mm_load_ps(reinterpret_cast<const float*>(ptr)));
    } else {
        return _mm_castps_si128(_mm_loadu_ps(reinterpret_cast<const float*>(ptr)));
    }
#else
    if (mem_mode == MemoryMode::SSE2_ALIGNED) {
        return _mm_load_si128(reinterpret_cast<const __m128i*>(ptr));
    } else {
        return _mm_loadu_si128(reinterpret_cast<const __m128i*>(ptr));
    }
#endif
}

template<MemoryMode mem_mode, typename T>
static MT_FORCEINLINE __m128 simd_load_ps(const T* ptr) {
#ifdef USE_MOVPS
  if (mem_mode == MemoryMode::SSE2_ALIGNED) {
    return _mm_load_ps(reinterpret_cast<const float*>(ptr));
  }
  else {
    return _mm_loadu_ps(reinterpret_cast<const float*>(ptr));
  }
#else
  if (mem_mode == MemoryMode::SSE2_ALIGNED) {
    return _mm_castsi128_ps(_mm_load_si128(reinterpret_cast<const __m128i*>(ptr)));
  }
  else {
    return _mm_castsi128_ps(_mm_loadu_si128(reinterpret_cast<const __m128i*>(ptr)));
  }
#endif
}


template<MemoryMode mem_mode, typename T>
static MT_FORCEINLINE void simd_store_si128(T *ptr, __m128i value) {
#ifdef USE_MOVPS
    if (mem_mode == MemoryMode::SSE2_ALIGNED) {
        _mm_store_ps(reinterpret_cast<float*>(ptr), _mm_castsi128_ps(value));
    } else {
        _mm_storeu_ps(reinterpret_cast<float*>(ptr), _mm_castsi128_ps(value));
    }
#else
    if (mem_mode == MemoryMode::SSE2_ALIGNED) {
        _mm_store_si128(reinterpret_cast<__m128i*>(ptr), value);
    } else {   
        _mm_storeu_si128(reinterpret_cast<__m128i*>(ptr), value);
    }
#endif
}

template<MemoryMode mem_mode, typename T>
static MT_FORCEINLINE void simd_store_ps(T *ptr, __m128 value) {
#ifdef USE_MOVPS
  if (mem_mode == MemoryMode::SSE2_ALIGNED) {
    _mm_store_ps(reinterpret_cast<float*>(ptr), value);
}
  else {
    _mm_storeu_ps(reinterpret_cast<float*>(ptr), value);
  }
#else
  if (mem_mode == MemoryMode::SSE2_ALIGNED) {
    _mm_store_si128(reinterpret_cast<__m128i*>(ptr), _mm_castps_si128(value));
  }
  else {
    _mm_storeu_si128(reinterpret_cast<__m128i*>(ptr), _mm_castps_si128(value));
  }
#endif
}

template<MemoryMode mem_mode, typename T>
static RGY_TARGET("avx") MT_FORCEINLINE __m256i simd256_load_si256(const T* ptr) {
#ifdef USE_MOVPS
  if (mem_mode == MemoryMode::SSE2_ALIGNED) {
    return _mm256_castps_si256(_mm256_load_ps(reinterpret_cast<const float*>(ptr)));
}
  else {
    return _mm256_castps_si256(_mm256_loadu_ps(reinterpret_cast<const float*>(ptr)));
  }
#else
  if (mem_mode == MemoryMode::SSE2_ALIGNED) {
    return _mm256_load_si256(reinterpret_cast<const __m256i*>(ptr));
  }
  else {
    return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr));
  }
#endif
}

template<MemoryMode mem_mode, typename T>
static RGY_TARGET("avx") MT_FORCEINLINE __m256 simd256_load_ps(const T* ptr) {
#ifdef USE_MOVPS
  if (mem_mode == MemoryMode::SSE2_ALIGNED) {
    return _mm256_load_ps(reinterpret_cast<const float*>(ptr));
  }
  else {
    return _mm256_loadu_ps(reinterpret_cast<const float*>(ptr));
  }
#else
  if (mem_mode == MemoryMode::SSE2_ALIGNED) {
    return _mm256_castsi256_ps(_mm256_load_si256(reinterpret_cast<const __m256i*>(ptr)));
  }
  else {
    return _mm256_castsi256_ps(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr)));
  }
#endif
}

template<MemoryMode mem_mode, typename T>
static RGY_TARGET("avx") MT_FORCEINLINE __m256 simd256_128_load_ps(const T* ptr) {
  if (mem_mode == MemoryMode::SSE2_ALIGNED) {
    return _mm256_loadu2_m128(reinterpret_cast<const float*>(ptr) + 4, reinterpret_cast<const float*>(ptr));
  }
  else {
    return _mm256_loadu2_m128(reinterpret_cast<const float*>(ptr) + 4, reinterpret_cast<const float*>(ptr));
  }
}

template<MemoryMode mem_mode, typename T>
static RGY_TARGET("avx") MT_FORCEINLINE void simd256_store_si256(T *ptr, __m256i value) {
#ifdef USE_MOVPS
  if (mem_mode == MemoryMode::SSE2_ALIGNED) {
    _mm256_store_ps(reinterpret_cast<float*>(ptr), _mm256_castsi256_ps(value));
  }
  else {
    _mm256_storeu_ps(reinterpret_cast<float*>(ptr), _mm256_castsi256_ps(value));
  }
#else
  if (mem_mode == MemoryMode::SSE2_ALIGNED) {
    _mm256_store_si256(reinterpret_cast<__m256i*>(ptr), value);
  }
  else {
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(ptr), value);
  }
#endif
}

template<MemoryMode mem_mode, typename T>
static RGY_TARGET("avx") MT_FORCEINLINE void simd256_store_ps(T *ptr, __m256 value) {
#ifdef USE_MOVPS
  if (mem_mode == MemoryMode::SSE2_ALIGNED) {
    _mm256_store_ps(reinterpret_cast<float*>(ptr), value);
  }
  else {
    _mm256_storeu_ps(reinterpret_cast<float*>(ptr), value);
  }
#else
  if (mem_mode == MemoryMode::SSE2_ALIGNED) {
    _mm256_store_si256(reinterpret_cast<__m256i*>(ptr), _mm256_castps_si256(value));
  }
  else {
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(ptr), _mm256_castps_si256(value));
  }
#endif
}



static MT_FORCEINLINE int simd_bit_scan_forward(int value) {
#ifdef __INTEL_COMPILER
    return _bit_scan_forward(value);
#elif defined(_MSC_VER)
    unsigned long index;
    _BitScanForward(&index, value);
    return index;
#else
    return __builtin_ctz(value);
#endif
}



enum class Border {
    Left,
    Right,
    None
};

#pragma warning(disable: 4309)

template<Border border_mode, MemoryMode mem_mode>
static MT_FORCEINLINE __m128i load_one_to_left(const Byte *ptr) {
    if (border_mode == Border::Left) {
        auto mask_left = _mm_setr_epi8(0xFF, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00);
        auto val = simd_load_si128<mem_mode>(ptr);
        return _mm_or_si128(_mm_slli_si128(val, 1), _mm_and_si128(val, mask_left)); // clone leftmost
    } else {
        return simd_load_si128<MemoryMode::SSE2_UNALIGNED>(ptr - 1);
    }
}

template<Border border_mode, MemoryMode mem_mode>
static MT_FORCEINLINE __m128i load_one_to_right(const Byte *ptr) {
    if (border_mode == Border::Right) {
        auto mask_right = _mm_setr_epi8(00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 0xFF);
        auto val = simd_load_si128<mem_mode>(ptr);
        return _mm_or_si128(_mm_srli_si128(val, 1), _mm_and_si128(val, mask_right));
    } else {
        return simd_load_si128<MemoryMode::SSE2_UNALIGNED>(ptr + 1);
    }
}

template<Border border_mode, MemoryMode mem_mode>
static MT_FORCEINLINE __m256i load_one_to_left_si256(const Byte *ptr) {
  if (border_mode == Border::Left) {
    auto lo128 = load_one_to_left<border_mode, mem_mode>(ptr); // really left!
    auto hi128 = simd_load_si128<MemoryMode::SSE2_UNALIGNED>(ptr + 16 - 1);
    return _mm256_set_m128i(hi128, lo128);
  }
  else {
    return simd256_load_si256<MemoryMode::SSE2_UNALIGNED>(ptr - 1);
  }
}

template<Border border_mode, MemoryMode mem_mode>
static MT_FORCEINLINE __m256i load_one_to_right_si256(const Byte *ptr) {
  if (border_mode == Border::Right) {
    auto lo128 = simd_load_si128<MemoryMode::SSE2_UNALIGNED>(ptr + 1);
    auto hi128 = load_one_to_right<border_mode, mem_mode>(ptr + 16); // really right!
    return _mm256_set_m128i(hi128, lo128);
  }
  else {
    return simd256_load_si256<MemoryMode::SSE2_UNALIGNED>(ptr + 1);
  }
}

template<Border border_mode, MemoryMode mem_mode>
static MT_FORCEINLINE __m128i load16_one_to_left(const Byte *ptr) {
  if (border_mode == Border::Left) {
    auto mask_left = _mm_setr_epi8(0xFF, 0xFF, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00);
    auto val = simd_load_si128<mem_mode>(ptr);
    return _mm_or_si128(_mm_slli_si128(val, 2), _mm_and_si128(val, mask_left));
  }
  else {
    return simd_load_si128<MemoryMode::SSE2_UNALIGNED>(ptr - 2);
  }
}

template<Border border_mode, MemoryMode mem_mode>
static MT_FORCEINLINE __m256i load16_one_to_left_si256(const Byte *ptr) {
  if (border_mode == Border::Left) {
    auto lo128 = load16_one_to_left<border_mode, mem_mode>(ptr); // really left!
    auto hi128 = simd_load_si128<MemoryMode::SSE2_UNALIGNED>(ptr + 16 - 2);
    return _mm256_set_m128i(hi128, lo128);
  }
  else {
    return simd256_load_si256<MemoryMode::SSE2_UNALIGNED>(ptr - 2);
  }
}

template<Border border_mode, MemoryMode mem_mode>
static MT_FORCEINLINE __m128i load16_one_to_right(const Byte *ptr) {
  if (border_mode == Border::Right) {
    auto mask_right = _mm_setr_epi8(00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 0xFF, 0xFF);
    auto val = simd_load_si128<mem_mode>(ptr);
    return _mm_or_si128(_mm_srli_si128(val, 2), _mm_and_si128(val, mask_right));
  }
  else {
    return simd_load_si128<MemoryMode::SSE2_UNALIGNED>(ptr + 2);
  }
}

template<Border border_mode, MemoryMode mem_mode>
static MT_FORCEINLINE __m256i load16_one_to_right_si256(const Byte *ptr) {
  if (border_mode == Border::Right) {
    auto lo128 = simd_load_si128<MemoryMode::SSE2_UNALIGNED>(ptr + 2);
    auto hi128 = load16_one_to_right<border_mode, mem_mode>(ptr + 16); // really right!
    return _mm256_set_m128i(hi128, lo128);
  }
  else {
    return simd256_load_si256<MemoryMode::SSE2_UNALIGNED>(ptr + 2);
  }
}

template<Border border_mode, MemoryMode mem_mode>
static MT_FORCEINLINE __m128 load32_one_to_left(const Byte *ptr) {
  if (border_mode == Border::Left) {
    auto mask_left = _mm_setr_epi8(0xFF, 0xFF, 0xFF, 0xFF, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00);
    auto val = simd_load_si128<mem_mode>(ptr);
    return _mm_castsi128_ps(_mm_or_si128(_mm_slli_si128(val, 4), _mm_and_si128(val, mask_left)));
  }
  else {
    return simd_load_ps<MemoryMode::SSE2_UNALIGNED>(ptr - 4);
  }
}

template<Border border_mode, MemoryMode mem_mode>
static MT_FORCEINLINE __m128 load32_one_to_right(const Byte *ptr) {
  if (border_mode == Border::Right) {
    auto mask_right = _mm_setr_epi8(00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 0xFF, 0xFF, 0xFF, 0xFF);
    auto val = simd_load_si128<mem_mode>(ptr);
    return _mm_castsi128_ps(_mm_or_si128(_mm_srli_si128(val, 4), _mm_and_si128(val, mask_right)));
  }
  else {
    return simd_load_ps<MemoryMode::SSE2_UNALIGNED>(ptr + 4);
  }
}

template<Border border_mode, MemoryMode mem_mode>
static MT_FORCEINLINE __m256 load32_one_to_left_si256(const Byte *ptr) {
  if (border_mode == Border::Left) {
    auto lo128 = load32_one_to_left<border_mode, mem_mode>(ptr); // really left!
    auto hi128 = simd_load_ps<MemoryMode::SSE2_UNALIGNED>(ptr + 16 - 4);
    return _mm256_set_m128(hi128, lo128);
  }
  else {
    return simd256_load_ps<MemoryMode::SSE2_UNALIGNED>(ptr - 4);
  }
}

template<Border border_mode, MemoryMode mem_mode>
static MT_FORCEINLINE __m256 load32_one_to_right_si256(const Byte *ptr) {
  if (border_mode == Border::Right) {
    auto lo128 = simd_load_ps<MemoryMode::SSE2_UNALIGNED>(ptr+4);
    auto hi128 = load32_one_to_right<border_mode, mem_mode>(ptr + 16); // really right!
    return _mm256_set_m128(hi128, lo128);
  }
  else {
    return simd256_load_ps<MemoryMode::SSE2_UNALIGNED>(ptr + 4);
  }
}

#pragma warning(default: 4309)

static MT_FORCEINLINE __m128i simd_movehl_si128(const __m128i &a, const __m128i &b) {
    return _mm_castps_si128(_mm_movehl_ps(_mm_castsi128_ps(a), _mm_castsi128_ps(b)));
}

template<CpuFlags flags>
static RGY_TARGET("sse4.1") MT_FORCEINLINE __m128i simd_blend_epi8(__m128i const &selector, __m128i const &a, __m128i const &b) {
  if (flags >= CPU_SSE4_1) {
    return _mm_blendv_epi8(b, a, selector);
  }
  else {
    return _mm_or_si128(_mm_and_si128(selector, a), _mm_andnot_si128(selector, b));
  }
}

static RGY_TARGET("avx2") MT_FORCEINLINE __m256i simd256_blend_epi8(__m256i const &selector, __m256i const &a, __m256i const &b) {
  return _mm256_blendv_epi8(b, a, selector);
}

// another blendv, good param order
template<CpuFlags flags>
static RGY_TARGET("sse4.1") MT_FORCEINLINE __m128i simd_blendv_epi8(__m128i x, __m128i y, __m128i mask)
{
  if (flags >= CPU_SSE4_1) {
    return _mm_blendv_epi8(x, y, mask);
  }
  else {
    // Replace bit in x with bit in y when matching bit in mask is set:
    return _mm_or_si128(_mm_andnot_si128(mask, x), _mm_and_si128(mask, y));
  }
}

template<CpuFlags flags>
static RGY_TARGET("sse4.1") MT_FORCEINLINE __m128 simd_blendv_ps(__m128 x, __m128 y, __m128 mask)
{
  if (flags >= CPU_SSE4_1) {
    return _mm_blendv_ps(x, y, mask);
  }
  else {
    // Replace bit in x with bit in y when matching bit in mask is set:
    return _mm_or_ps(_mm_andnot_ps(mask, x), _mm_and_ps(mask, y));
  }
}

static RGY_TARGET("avx") MT_FORCEINLINE __m256 simd256_blendv_ps(__m256 x, __m256 y, __m256 mask)
{
  return _mm256_blendv_ps(x, y, mask);
}

static RGY_TARGET("avx2") MT_FORCEINLINE __m256i simd256_blendv_epi8(__m256i x, __m256i y, __m256i mask)
{
  return _mm256_blendv_epi8(x, y, mask);
}



static MT_FORCEINLINE __m128i threshold_sse2(const __m128i &value, const __m128i &lowThresh, const __m128i &highThresh, const __m128i &v128) {
    auto sat = _mm_sub_epi8(value, v128);
    auto low = _mm_cmpgt_epi8(sat, lowThresh);
    auto high = _mm_cmpgt_epi8(sat, highThresh);
    auto result = _mm_and_si128(value, low);
    return _mm_or_si128(result, high);
}

static RGY_TARGET("avx2") MT_FORCEINLINE __m256i threshold_avx2(const __m256i &value, const __m256i &lowThresh, const __m256i &highThresh, const __m256i &v128) {
  auto sat = _mm256_sub_epi8(value, v128);
  auto low = _mm256_cmpgt_epi8(sat, lowThresh);
  auto high = _mm256_cmpgt_epi8(sat, highThresh);
  auto result = _mm256_and_si256(value, low);
  return _mm256_or_si256(result, high);
}

//  thresholds are decreased by half range in order to do signed comparison
template<int bits_per_pixel>
static MT_FORCEINLINE __m128i threshold16_sse2(const __m128i &value, const __m128i &lowThresh, const __m128i &highThresh, const __m128i &vHalf, const __m128i &maxMask) {
  auto sat = _mm_sub_epi16(value, vHalf);
  auto low = _mm_cmpgt_epi16(sat, lowThresh);
  auto high = _mm_cmpgt_epi16(sat, highThresh);
  if (bits_per_pixel < 16)
    high = _mm_and_si128(high, maxMask); // clamp FFFF to 03FF 0FFF or 3FFF at 10, 12 and 14 bits
  auto result = _mm_and_si128(value, low);
  return _mm_or_si128(result, high); 
}

//  thresholds are decreased by half range in order to do signed comparison
template<int bits_per_pixel>
static RGY_TARGET("avx2") MT_FORCEINLINE __m256i threshold16_avx2(const __m256i &value, const __m256i &lowThresh, const __m256i &highThresh, const __m256i &vHalf, const __m256i &maxMask) {
  auto sat = _mm256_sub_epi16(value, vHalf);
  auto low = _mm256_cmpgt_epi16(sat, lowThresh);
  auto high = _mm256_cmpgt_epi16(sat, highThresh);
  if (bits_per_pixel < 16)
    high = _mm256_and_si256(high, maxMask); // clamp FFFF to 03FF 0FFF or 3FFF at 10, 12 and 14 bits
  auto result = _mm256_and_si256(value, low);
  return _mm256_or_si256(result, high);
}

template<CpuFlags flags>
static MT_FORCEINLINE __m128 threshold32_sse2(const __m128 &value, const __m128 &lowThresh, const __m128 &highThresh) {
  // create final mask 0.0 or 1.0 or x if between
  // value <= low ? 0.0f : value > high ? 1.0 : x
  auto tOne = _mm_set1_ps(1.0f);
  auto lowMask = _mm_cmpgt_ps(value, lowThresh);   // (value > lowTh) ? FFFFFFFF : 00000000
  auto tmpValue = _mm_and_ps(lowMask, value); // 0 where value <= lowTh, value otherwise

  auto highMask = _mm_cmpgt_ps(tmpValue, highThresh); // value > highTh) ? FFFFFFFF : 00000000
  auto result = simd_blendv_ps<flags>(tmpValue, tOne, highMask);
  return result;
}

static RGY_TARGET("avx") MT_FORCEINLINE __m256 _mm256_cmpgt_ps(__m256 a, __m256 b) {
  return _mm256_cmp_ps(a, b, _CMP_NLE_US); // NLE = GT
}

static RGY_TARGET("avx") MT_FORCEINLINE __m256 threshold32_avx(const __m256 &value, const __m256 &lowThresh, const __m256 &highThresh) {
  // create final mask 0.0 or 1.0 or x if between
  // value <= low ? 0.0f : value > high ? 1.0 : x
  auto tOne = _mm256_set1_ps(1.0f);
  auto lowMask = _mm256_cmpgt_ps(value, lowThresh);   // (value > lowTh) ? FFFFFFFF : 00000000
  auto tmpValue = _mm256_and_ps(lowMask, value); // 0 where value <= lowTh, value otherwise

  auto highMask = _mm256_cmpgt_ps(tmpValue, highThresh); // value > highTh) ? FFFFFFFF : 00000000
  auto result = simd256_blendv_ps(tmpValue, tOne, highMask);
  return result;
}

template<CpuFlags flags>
static RGY_TARGET("sse4.1") MT_FORCEINLINE __m128i simd_mullo_epi32(__m128i &a, __m128i &b) {
    if (flags >= CPU_SSE4_1) {
        return _mm_mullo_epi32(a, b);
    } else {
        auto a13    = _mm_shuffle_epi32(a, 0xF5);          // (-,a3,-,a1)
        auto b13    = _mm_shuffle_epi32(b, 0xF5);          // (-,b3,-,b1)
        auto prod02 = _mm_mul_epu32(a, b);                 // (-,a2*b2,-,a0*b0)
        auto prod13 = _mm_mul_epu32(a13, b13);             // (-,a3*b3,-,a1*b1)
        auto prod01 = _mm_unpacklo_epi32(prod02,prod13);   // (-,-,a1*b1,a0*b0) 
        auto prod23 = _mm_unpackhi_epi32(prod02,prod13);   // (-,-,a3*b3,a2*b2) 
        return _mm_unpacklo_epi64(prod01,prod23);   // (ab3,ab2,ab1,ab0)
    }
}

static RGY_TARGET("avx2") MT_FORCEINLINE __m256i simd256_mullo_epi32(__m256i &a, __m256i &b) {
  return _mm256_mullo_epi32(a, b);
}

// sse2 replacement of _mm_mullo_epi32 in SSE4.1
// another way for do mullo for SSE2, actually not used, there is simd_mullo_epi32
// use it after speed test, may have too much overhead and C is faster
static MT_FORCEINLINE __m128i _MM_MULLO_EPI32(const __m128i &a, const __m128i &b)
{
  // for SSE 4.1: return _mm_mullo_epi32(a, b);
  __m128i tmp1 = _mm_mul_epu32(a, b); // mul 2,0
  __m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(a, 4), _mm_srli_si128(b, 4)); // mul 3,1
                                                                            // shuffle results to [63..0] and pack. a2->a1, a0->a0
  return _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE(0, 0, 2, 0)));
}

#pragma warning(disable: 4309)
// fake _mm_packus_epi32 (orig is SSE4.1 only)
static MT_FORCEINLINE __m128i _MM_PACKUS_EPI32(__m128i a, __m128i b)
{
  
  const static __m128i val_32 = _mm_set1_epi32(0x8000);
  const static __m128i val_16 = _mm_set1_epi16(0x8000);

  a = _mm_sub_epi32(a, val_32);
  b = _mm_sub_epi32(b, val_32);
  a = _mm_packs_epi32(a, b);
  a = _mm_add_epi16(a, val_16);
  return a;
}
#pragma warning(default: 4309)


// non-existant in simd
static MT_FORCEINLINE __m128i _MM_CMPLE_EPU16(__m128i x, __m128i y)
{
  // Returns 0xFFFF where x <= y:
  return _mm_cmpeq_epi16(_mm_subs_epu16(x, y), _mm_setzero_si128());
}

// non-existant in simd
static RGY_TARGET("avx2") MT_FORCEINLINE __m256i _MM256_CMPLE_EPU16(__m256i x, __m256i y)
{
  // Returns 0xFFFF where x <= y:
  return _mm256_cmpeq_epi16(_mm256_subs_epu16(x, y), _mm256_setzero_si256());
}

// SSE2 version of SSE4.1-only _mm_max_epu16
template<CpuFlags flags>
static RGY_TARGET("sse4.1") MT_FORCEINLINE __m128i simd_max_epu16(__m128i x, __m128i y)
{
  if (flags >= CPU_SSE4_1) {
    return _mm_max_epu16(x, y);
  }
  else {
    return simd_blendv_epi8<flags>(x, y, _MM_CMPLE_EPU16(x, y)); // blendv (not blend) watch param order !
  }
}

// SSE2 version of SSE4.1-only _mm_min_epu16
template<CpuFlags flags>
static RGY_TARGET("sse4.1") MT_FORCEINLINE __m128i simd_min_epu16(__m128i x, __m128i y)
{
  if (flags >= CPU_SSE4_1) {
    return _mm_min_epu16(x, y);
  }
  else {
    return simd_blendv_epi8<flags>(y, x, _MM_CMPLE_EPU16(x, y)); // blendv (not blend) watch param order !
  }
}

template<CpuFlags flags>
static RGY_TARGET("sse4.1") MT_FORCEINLINE __m128i simd_packus_epi32(__m128i &a, __m128i &b) {
  if (flags >= CPU_SSE4_1) {
    return _mm_packus_epi32(a, b);
  }
  else {
    return _MM_PACKUS_EPI32(a, b);
  }
}

static MT_FORCEINLINE __m128 simd_abs_ps(__m128 a) {
  // maybe not optimal, mask may be generated 
  const __m128 absmask = _mm_castsi128_ps(_mm_set1_epi32(~(1 << 31))); // 0x7FFFFFFF
  return _mm_and_ps(a, absmask);
}

static MT_FORCEINLINE __m128 simd_abs_diff_ps(__m128 a, __m128 b) {
  // maybe not optimal
  const __m128 absmask = _mm_castsi128_ps(_mm_set1_epi32(~(1 << 31))); // 0x7FFFFFFF
  return _mm_and_ps(_mm_sub_ps(a, b), absmask);
}

static RGY_TARGET("avx") MT_FORCEINLINE __m256 simd256_abs_ps(__m256 a) {
  // maybe not optimal, mask may be generated 
  const __m256 absmask = _mm256_castsi256_ps(_mm256_set1_epi32(~(1 << 31))); // 0x7FFFFFFF
  return _mm256_and_ps(a, absmask);
}

static RGY_TARGET("avx") MT_FORCEINLINE __m256 simd256_abs_diff_ps(__m256 a, __m256 b) {
  // maybe not optimal
  const __m256 absmask = _mm256_castsi256_ps(_mm256_set1_epi32(~(1 << 31))); // 0x7FFFFFFF
  return _mm256_and_ps(_mm256_sub_ps(a, b), absmask);
}

static MT_FORCEINLINE __m128i read_word_stacked_simd(const Byte *pMsb, const Byte *pLsb, int x) {
    auto msb = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(pMsb+x));
    auto lsb = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(pLsb+x));
    return _mm_unpacklo_epi8(lsb, msb);
}

static MT_FORCEINLINE void write_word_stacked_simd(Byte *pMsb, Byte *pLsb, int x, const __m128i &value, const __m128i &ff, const __m128i &zero) {
    auto result_lsb = _mm_and_si128(value, ff);
    auto result_msb = _mm_srli_epi16(value, 8);

    result_lsb = _mm_packus_epi16(result_lsb, zero);
    result_msb = _mm_packus_epi16(result_msb, zero);

    _mm_storel_epi64(reinterpret_cast<__m128i*>(pMsb+x), result_msb);
    _mm_storel_epi64(reinterpret_cast<__m128i*>(pLsb+x), result_lsb);
}

// simulate real 256 bit byte-shift (not 2x128 lanes)
template<BYTE shiftcount>
RGY_TARGET("avx2") MT_FORCEINLINE __m256i _MM256_SLLI_SI256(__m256i a)
{
  if (shiftcount == 0)
    return a;
  if (shiftcount < 16)
    return _mm256_alignr_epi8(a, _mm256_permute2x128_si256(a, a, _MM_SHUFFLE(0, 0, 2, 0)), 16 - shiftcount);
  if (shiftcount == 16)
    return _mm256_permute2x128_si256(a, a, _MM_SHUFFLE(0, 0, 2, 0));
  if (shiftcount < 32)
    return _mm256_slli_si256(_mm256_permute2x128_si256(a, a, _MM_SHUFFLE(0, 0, 2, 0)), shiftcount - 16);
  return _mm256_setzero_si256();
}

// simulate real 256 bit byte-shift (not 2x128 lanes)
template<BYTE shiftcount>
RGY_TARGET("avx2") MT_FORCEINLINE __m256i _MM256_SRLI_SI256(__m256i a)
{
  if (shiftcount == 0)
    return a;
  if (shiftcount < 16)
    return _mm256_alignr_epi8(_mm256_permute2x128_si256(a, a, _MM_SHUFFLE(2, 0, 0, 1)), a, shiftcount);
  if (shiftcount == 16)
    return _mm256_permute2x128_si256(a, a, _MM_SHUFFLE(2, 0, 0, 1));
  if (shiftcount < 32)
    return _mm256_srli_si256(_mm256_permute2x128_si256(a, a, _MM_SHUFFLE(2, 0, 0, 1)), shiftcount - 16);
  return _mm256_setzero_si256();
}

}

#endif //__Mt_SIMD_H__
