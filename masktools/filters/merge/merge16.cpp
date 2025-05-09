#include "merge.h"
#include "../../common/simd.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Merge {

/* Common */

enum MaskMode {
    MASK420,
    MASK422,
    MASK444
};

template<int bits_per_pixel>
MT_FORCEINLINE static Word merge16_core_c(Word dst, Word src, Word mask) {
    if (mask == 0) {
        return dst;
    } else if (mask == ((1 << bits_per_pixel) - 1)) {
        return src;
    } else {
      if(bits_per_pixel < 16)
        return dst + (((src - dst) * mask) >> bits_per_pixel);
      else
        return dst + (((src - dst) * (mask >> 1)) >> (bits_per_pixel - 1)); // >> 1: avoid overflow
    }
}

template <CpuFlags flags>
RGY_TARGET("ssse3") MT_FORCEINLINE static __m128i get_single_mask_value_420(const __m128i &row1_lo, const __m128i &row1_hi, const __m128i &row2_lo, const __m128i &row2_hi) {
    auto avg_lo = _mm_avg_epu16(row1_lo, row2_lo);
    auto avg_hi = _mm_avg_epu16(row1_hi, row2_hi);

    auto avg_lo_sh = _mm_srli_si128(avg_lo, 2);
    auto avg_hi_sh = _mm_srli_si128(avg_hi, 2);

    avg_lo = _mm_avg_epu16(avg_lo, avg_lo_sh);
    avg_hi = _mm_avg_epu16(avg_hi, avg_hi_sh);

    if (flags >= CPU_SSSE3) {
        avg_lo = _mm_shuffle_epi8(avg_lo, _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 13, 12, 9, 8, 5, 4, 1, 0));
        avg_hi = _mm_shuffle_epi8(avg_hi, _mm_set_epi8(13, 12, 9, 8, 5, 4, 1, 0, 0, 0, 0, 0, 0, 0 ,0 ,0));
    } else {
        avg_lo = _mm_shufflelo_epi16(avg_lo, _MM_SHUFFLE(3, 3, 2, 0));
        avg_lo = _mm_shufflehi_epi16(avg_lo, _MM_SHUFFLE(3, 3, 2, 0));
        avg_lo = _mm_shuffle_epi32(avg_lo, _MM_SHUFFLE(3, 3, 2, 0));

        avg_hi = _mm_shufflelo_epi16(avg_hi, _MM_SHUFFLE(3, 3, 2, 0));
        avg_hi = _mm_shufflehi_epi16(avg_hi, _MM_SHUFFLE(3, 3, 2, 0));
        avg_hi = _mm_shuffle_epi32(avg_hi, _MM_SHUFFLE(2, 0, 3, 3));
    }

    return simd_blend_epi8<flags>(_mm_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0, 0), avg_hi, avg_lo);
}

template <CpuFlags flags>
RGY_TARGET("ssse3") MT_FORCEINLINE static __m128i get_single_mask_value_422(const __m128i &row1_lo, const __m128i &row1_hi) {
  auto row1_lo_sh = _mm_srli_si128(row1_lo, 2);
  auto row1_hi_sh = _mm_srli_si128(row1_hi, 2);

  auto avg_lo = _mm_avg_epu16(row1_lo, row1_lo_sh);
  auto avg_hi = _mm_avg_epu16(row1_hi, row1_hi_sh);

  if (flags >= CPU_SSSE3) {
    avg_lo = _mm_shuffle_epi8(avg_lo, _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 13, 12, 9, 8, 5, 4, 1, 0));
    avg_hi = _mm_shuffle_epi8(avg_hi, _mm_set_epi8(13, 12, 9, 8, 5, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0));
  }
  else {
    avg_lo = _mm_shufflelo_epi16(avg_lo, _MM_SHUFFLE(3, 3, 2, 0));
    avg_lo = _mm_shufflehi_epi16(avg_lo, _MM_SHUFFLE(3, 3, 2, 0));
    avg_lo = _mm_shuffle_epi32(avg_lo, _MM_SHUFFLE(3, 3, 2, 0));

    avg_hi = _mm_shufflelo_epi16(avg_hi, _MM_SHUFFLE(3, 3, 2, 0));
    avg_hi = _mm_shufflehi_epi16(avg_hi, _MM_SHUFFLE(3, 3, 2, 0));
    avg_hi = _mm_shuffle_epi32(avg_hi, _MM_SHUFFLE(2, 0, 3, 3));
  }

  return simd_blend_epi8<flags>(_mm_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0, 0), avg_hi, avg_lo);
}

template<CpuFlags flags, int bits_per_pixel>
MT_FORCEINLINE static __m128i merge_core_simd(const __m128i &dst, const __m128i &src, const __m128i &mask, const __m128i &max_pixel_value, const __m128i &zero) {
  auto dst_lo = _mm_unpacklo_epi16(dst, zero);
  auto dst_hi = _mm_unpackhi_epi16(dst, zero);

  auto src_lo = _mm_unpacklo_epi16(src, zero);
  auto src_hi = _mm_unpackhi_epi16(src, zero);

  auto mask_lo = _mm_unpacklo_epi16(mask, zero);
  auto mask_hi = _mm_unpackhi_epi16(mask, zero);

  // 16 bits: return dst +(((src - dst) * (mask >> 1)) >> (bits_per_pixel - 1));
  // 10-14 bits: return dst +(((src - dst) * (mask)) >> (bits_per_pixel));

  auto diff_lo = _mm_sub_epi32(src_lo, dst_lo); // diff = src-dst
  auto diff_hi = _mm_sub_epi32(src_hi, dst_hi);

  __m128i lerp_lo, lerp_hi;

  if (bits_per_pixel == 16) {
    auto smask_lo = _mm_srai_epi32(mask_lo, 1); // smask = mask>>1
    auto smask_hi = _mm_srai_epi32(mask_hi, 1);

    lerp_lo = simd_mullo_epi32<flags>(diff_lo, smask_lo);
    lerp_hi = simd_mullo_epi32<flags>(diff_hi, smask_hi);

    lerp_lo = _mm_srai_epi32(lerp_lo, bits_per_pixel - 1); // for n bit, shr (n-1), scale back
    lerp_hi = _mm_srai_epi32(lerp_hi, bits_per_pixel - 1);
  }
  else {
    lerp_lo = simd_mullo_epi32<flags>(diff_lo, mask_lo);
    lerp_hi = simd_mullo_epi32<flags>(diff_hi, mask_hi);

    lerp_lo = _mm_srai_epi32(lerp_lo, bits_per_pixel);
    lerp_hi = _mm_srai_epi32(lerp_hi, bits_per_pixel);
  }

  auto result_lo = _mm_add_epi32(dst_lo, lerp_lo);
  auto result_hi = _mm_add_epi32(dst_hi, lerp_hi);

  auto result = simd_packus_epi32<flags>(result_lo, result_hi);
  if (bits_per_pixel < 16) // otherwise no clamp needed
    result = simd_min_epu16<flags>(result, max_pixel_value);

  auto mask_FFFF = _mm_cmpeq_epi16(mask, max_pixel_value); // mask == max ? FFFF : 0000
  result = simd_blend_epi8<flags>(mask_FFFF, src, result); // ensure that max mask value returns src

  auto mask_zero = _mm_cmpeq_epi16(mask, zero);
  result = simd_blend_epi8<flags>(mask_zero, dst, result); // ensure that zero mask value returns dst

  return result;
}

/* Stacked */

MT_FORCEINLINE static Word get_value_stacked_c(const Byte *pMsb, const Byte *pLsb, int x) {
    return (Word(pMsb[x]) << 8) + pLsb[x];
}

MT_FORCEINLINE static Word get_mask_420_stacked_c(const Byte *pMsb, const Byte *pLsb, ptrdiff_t pitch, int x) {
    x = x*2;
    return ((int((get_value_stacked_c(pMsb, pLsb, x)) + get_value_stacked_c(pMsb + pitch, pLsb + pitch, x) + 1) >> 1) +
        ((int(get_value_stacked_c(pMsb, pLsb, x+1)) + get_value_stacked_c(pMsb + pitch, pLsb + pitch, x+1) + 1) >> 1) + 1) >> 1;
}

MT_FORCEINLINE static Word get_mask_422_stacked_c(const Byte *pMsb, const Byte *pLsb, ptrdiff_t pitch, int x) {
  x = x * 2;
  return int((get_value_stacked_c(pMsb, pLsb, x)) + get_value_stacked_c(pMsb + pitch, pLsb + pitch, x) + 1) >> 1;
}

template<MaskMode mode>
void merge16_t_stacked_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                 const Byte *pMask, ptrdiff_t nMaskPitch, int nWidth, int nHeight, int nOrigHeight)
{
    auto pDstLsb = pDst + nDstPitch * nOrigHeight / 2;
    auto pSrc1Lsb = pSrc1 + nSrc1Pitch * nOrigHeight / 2;
    auto pMaskLsb = pMask + nMaskPitch * nOrigHeight / (mode == MASK420 ? 1 : 2);

    for ( int y = 0; y < nHeight / 2; ++y )
    {
        for ( int x = 0; x < nWidth; ++x ) {
            Word dst = get_value_stacked_c(pDst, pDstLsb, x);
            Word src = get_value_stacked_c(pSrc1, pSrc1Lsb, x);
            Word mask;

            if (mode == MASK420) {
              mask = get_mask_420_stacked_c(pMask, pMaskLsb, nMaskPitch, x);
            } else if (mode == MASK422) {
              mask = get_mask_422_stacked_c(pMask, pMaskLsb, nMaskPitch, x);
            } else {
              mask = get_value_stacked_c(pMask, pMaskLsb, x);
            }

            Word output = merge16_core_c<16>(dst, src, mask);

            pDst[x] = output >> 8;
            pDstLsb[x] = output & 0xFF;
        }
        pDst += nDstPitch;
        pDstLsb += nDstPitch;
        pSrc1 += nSrc1Pitch;
        pSrc1Lsb += nSrc1Pitch;

        if (mode == MASK420) {
            pMask += nMaskPitch * 2;
            pMaskLsb += nMaskPitch * 2;
        } else {
            pMask += nMaskPitch;
            pMaskLsb += nMaskPitch;
        }
    }
}

template <CpuFlags flags>
MT_FORCEINLINE static __m128i get_mask_420_stacked_simd(const Byte *pMsb, const Byte *pLsb, ptrdiff_t pitch, int x) {
    x = x*2;

    auto row1_lo = read_word_stacked_simd(pMsb, pLsb, x);
    auto row1_hi = read_word_stacked_simd(pMsb, pLsb, x+8);
    auto row2_lo = read_word_stacked_simd(pMsb+pitch, pLsb+pitch, x);
    auto row2_hi = read_word_stacked_simd(pMsb+pitch, pLsb+pitch, x+8);

    return get_single_mask_value_420<flags>(row1_lo, row1_hi, row2_lo, row2_hi);
}

template <CpuFlags flags>
MT_FORCEINLINE static __m128i get_mask_422_stacked_simd(const Byte *pMsb, const Byte *pLsb, int x) {
  x = x * 2;

  auto row1_lo = read_word_stacked_simd(pMsb, pLsb, x);
  auto row1_hi = read_word_stacked_simd(pMsb, pLsb, x + 8);

  return get_single_mask_value_422<flags>(row1_lo, row1_hi);
}

template <CpuFlags flags, MaskMode mode, Processor16 merge_c>
void merge16_t_stacked_simd(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                                                   const Byte *pMask, ptrdiff_t nMaskPitch, int nWidth, int nHeight, int nOrigHeight)
{
    int wMod8 = (nWidth / 8) * 8;
    auto pDst_s = pDst;
    auto pSrc1_s = pSrc1;
    auto pMask_s = pMask;

    auto pDstLsb = pDst + nDstPitch * nOrigHeight / 2;
    auto pSrc1Lsb = pSrc1 + nSrc1Pitch * nOrigHeight / 2;
    auto pMaskLsb = pMask + nMaskPitch * nOrigHeight / (mode == MASK420 ? 1 : 2);

    auto zero = _mm_setzero_si128();
    #pragma warning(disable: 4309)
    auto ffff = _mm_set1_epi16(0xFFFF); // max_pixel_value
    #pragma warning(default: 4309)
    auto ff = _mm_set1_epi16(0x00FF);

    for ( int j = 0; j < nHeight / 2; ++j ) {
        for ( int i = 0; i < wMod8; i+=8 ) {
            auto dst = read_word_stacked_simd(pDst, pDstLsb, i);
            auto src = read_word_stacked_simd(pSrc1, pSrc1Lsb, i);
            __m128i mask;

            if (mode == MASK420) {
                mask = get_mask_420_stacked_simd<flags>(pMask, pMaskLsb, nMaskPitch, i);
            } else if (mode == MASK422) {
                mask = get_mask_422_stacked_simd<flags>(pMask, pMaskLsb, i);
            } else {
                mask = read_word_stacked_simd(pMask, pMaskLsb, i);
            }

            auto result = merge_core_simd<flags, 16>(dst, src, mask, ffff, zero);

            write_word_stacked_simd(pDst, pDstLsb, i, result, ff, zero);
        }

        pDst += nDstPitch;
        pDstLsb += nDstPitch;
        pSrc1 += nSrc1Pitch;
        pSrc1Lsb += nSrc1Pitch;

        if (mode == MASK420) {
            pMask += nMaskPitch * 2;
            pMaskLsb += nMaskPitch * 2;
        } else {
            pMask += nMaskPitch;
            pMaskLsb += nMaskPitch;
        }
    }

    if (nWidth > wMod8) {
        merge_c(pDst_s + wMod8, nDstPitch, pSrc1_s + wMod8, nSrc1Pitch, pMask_s + wMod8, nMaskPitch, nWidth-wMod8, nHeight, nOrigHeight);
    }
}

/* Native */

MT_FORCEINLINE static Word get_mask_420_c(const Byte *ptr, ptrdiff_t pitch, int x) {
    x = x*2;

    return (((reinterpret_cast<const Word*>(ptr)[x] + reinterpret_cast<const Word*>(ptr+pitch)[x] + 1) >> 1) + 
        ((reinterpret_cast<const Word*>(ptr)[x+1] +  reinterpret_cast<const Word*>(ptr+pitch)[x+1] + 1) >> 1) + 1) >> 1;
}

MT_FORCEINLINE static Word get_mask_422_c(const Byte *ptr, int x) {
  x = x * 2;

  return (reinterpret_cast<const Word*>(ptr)[x] + reinterpret_cast<const Word*>(ptr)[x + 1] + 1) >> 1;
}

template<MaskMode mode, int bits_per_pixel>
void merge16_t_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                         const Byte *pMask, ptrdiff_t nMaskPitch, int nWidth, int nHeight, int /*nOrigHeight*/)
{
    for ( int y = 0; y < nHeight; ++y )
    {
        for ( int x = 0; x < nWidth; ++x ) { 
            Word dst = reinterpret_cast<const Word*>(pDst)[x];
            Word src = reinterpret_cast<const Word*>(pSrc1)[x];
            Word mask;

            if (mode == MASK420) {
                mask = get_mask_420_c(pMask, nMaskPitch, x);
            } else if(mode == MASK422)
                mask = get_mask_422_c(pMask, x);
            else {
                mask = reinterpret_cast<const Word*>(pMask)[x];
            }

            Word output = merge16_core_c<bits_per_pixel>(dst, src, mask);

            reinterpret_cast<Word*>(pDst)[x] = output;
        }
        pDst += nDstPitch;
        pSrc1 += nSrc1Pitch;

        if (mode == MASK420) {
            pMask += nMaskPitch * 2;
        } else {
            pMask += nMaskPitch; // 422, 444
        }
    }
}


template <CpuFlags flags>
MT_FORCEINLINE static __m128i get_mask_420_simd(const Byte *ptr, ptrdiff_t pitch, int x) {
    x = x*2;

    auto row1_lo = simd_load_si128<MemoryMode::SSE2_UNALIGNED>(reinterpret_cast<const __m128i*>(ptr+x));
    auto row1_hi = simd_load_si128<MemoryMode::SSE2_UNALIGNED>(reinterpret_cast<const __m128i*>(ptr+x+16));
    auto row2_lo = simd_load_si128<MemoryMode::SSE2_UNALIGNED>(reinterpret_cast<const __m128i*>(ptr+pitch+x));
    auto row2_hi = simd_load_si128<MemoryMode::SSE2_UNALIGNED>(reinterpret_cast<const __m128i*>(ptr+pitch+x+16));

    return get_single_mask_value_420<flags>(row1_lo, row1_hi, row2_lo, row2_hi);
}

template <CpuFlags flags>
MT_FORCEINLINE static __m128i get_mask_422_simd(const Byte *ptr, int x) {
  x = x * 2;

  auto row1_lo = simd_load_si128<MemoryMode::SSE2_UNALIGNED>(reinterpret_cast<const __m128i*>(ptr + x));
  auto row1_hi = simd_load_si128<MemoryMode::SSE2_UNALIGNED>(reinterpret_cast<const __m128i*>(ptr + x + 16));
  
  return get_single_mask_value_422<flags>(row1_lo, row1_hi);
}

template <CpuFlags flags, MaskMode mode, Processor16 merge_c, int bits_per_pixel>
void merge16_t_simd(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                            const Byte *pMask, ptrdiff_t nMaskPitch, int nWidth, int nHeight, int nOrigHeight)
{
    nWidth *= 2; // really rowsize: width * sizeof(uint16), see also division at C trailer

    int wMod16 = (nWidth / 16) * 16;
    auto pDst_s = pDst;
    auto pSrc1_s = pSrc1;
    auto pMask_s = pMask;

    int max = (1 << bits_per_pixel) - 1;

    auto zero = _mm_setzero_si128();
#pragma warning(disable: 4244)
    auto max_pixel_value = _mm_set1_epi16(max); // for checking max
#pragma warning(default: 4244)

    for ( int j = 0; j < nHeight; ++j ) {
        for ( int i = 0; i < wMod16; i+=16 ) {
            auto dst = simd_load_si128<MemoryMode::SSE2_UNALIGNED>(reinterpret_cast<const __m128i*>(pDst+i));
            auto src = simd_load_si128<MemoryMode::SSE2_UNALIGNED>(reinterpret_cast<const __m128i*>(pSrc1+i));
            __m128i mask;

            if (mode == MASK420) {
              mask = get_mask_420_simd<flags>(pMask, nMaskPitch, i);
            } else if (mode == MASK422) {
              mask = get_mask_422_simd<flags>(pMask, i);
            } else {
              mask = simd_load_si128<MemoryMode::SSE2_UNALIGNED>(reinterpret_cast<const __m128i*>(pMask + i));
            }

            __m128i result = merge_core_simd<flags, bits_per_pixel>(dst, src, mask, max_pixel_value, zero);

            simd_store_si128<MemoryMode::SSE2_UNALIGNED>(reinterpret_cast<__m128i*>(pDst+i), result);
        }

        pDst += nDstPitch;
        pSrc1 += nSrc1Pitch;

        if (mode == MASK420) {
            pMask += nMaskPitch * 2;
        } else {
            pMask += nMaskPitch; // 422, 444
        }
    }

    if (nWidth > wMod16) {
        const int maskShift = (mode == MASK420 || mode == MASK422) ? wMod16 * 2 : wMod16;
        merge_c(pDst_s + wMod16, nDstPitch, pSrc1_s + wMod16, nSrc1Pitch, pMask_s + maskShift, nMaskPitch, (nWidth-wMod16) / sizeof(uint16_t), nHeight, nOrigHeight);
    }
}

/* Stacked, always 16 bit */
Processor16 *merge16_c_stacked = &merge16_t_stacked_c<MASK444>;
Processor16 *merge16_luma_420_c_stacked = &merge16_t_stacked_c<MASK420>;
Processor16 *merge16_luma_422_c_stacked = &merge16_t_stacked_c<MASK422>;

Processor16 *merge16_sse2_stacked = merge16_t_stacked_simd<CPU_SSE2, MASK444, merge16_t_stacked_c<MASK444>>;
Processor16 *merge16_sse4_1_stacked = merge16_t_stacked_simd<CPU_SSE4_1, MASK444, merge16_t_stacked_c<MASK444>>;

Processor16 *merge16_luma_420_sse2_stacked = merge16_t_stacked_simd<CPU_SSE2, MASK420, merge16_t_stacked_c<MASK420>>;
Processor16 *merge16_luma_420_ssse3_stacked = merge16_t_stacked_simd<CPU_SSSE3, MASK420, merge16_t_stacked_c<MASK420>>;
Processor16 *merge16_luma_420_sse4_1_stacked = merge16_t_stacked_simd<CPU_SSE4_1, MASK420, merge16_t_stacked_c<MASK420>>;

Processor16 *merge16_luma_422_sse2_stacked = merge16_t_stacked_simd<CPU_SSE2, MASK422, merge16_t_stacked_c<MASK422>>;
Processor16 *merge16_luma_422_ssse3_stacked = merge16_t_stacked_simd<CPU_SSSE3, MASK422, merge16_t_stacked_c<MASK422>>;
Processor16 *merge16_luma_422_sse4_1_stacked = merge16_t_stacked_simd<CPU_SSE4_1, MASK422, merge16_t_stacked_c<MASK422>>;

/* Native */
// specialize them
// 4:1:1 is only in 8 bits
#define MAKE_TEMPLATES(bits_per_pixel) \
template void merge16_t_c<MASK444, bits_per_pixel>(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch, const Byte *pMask, ptrdiff_t nMaskPitch, int nWidth, int nHeight, int nOrigHeight); \
template void merge16_t_c<MASK420, bits_per_pixel>(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch, const Byte *pMask, ptrdiff_t nMaskPitch, int nWidth, int nHeight, int nOrigHeight); \
template void merge16_t_c<MASK422, bits_per_pixel>(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch, const Byte *pMask, ptrdiff_t nMaskPitch, int nWidth, int nHeight, int nOrigHeight); \
template void merge16_t_simd<CPU_SSE2, MASK444, merge16_t_c<MASK444, bits_per_pixel>, bits_per_pixel>(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch, const Byte *pMask, ptrdiff_t nMaskPitch, int nWidth, int nHeight, int nOrigHeight); \
template void merge16_t_simd<CPU_SSE4_1, MASK444, merge16_t_c<MASK444, bits_per_pixel>, bits_per_pixel>(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch, const Byte *pMask, ptrdiff_t nMaskPitch, int nWidth, int nHeight, int nOrigHeight); \
template void merge16_t_simd<CPU_SSE2, MASK420, merge16_t_c<MASK420, bits_per_pixel>, bits_per_pixel>(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch, const Byte *pMask, ptrdiff_t nMaskPitch, int nWidth, int nHeight, int nOrigHeight); \
template void merge16_t_simd<CPU_SSSE3, MASK420, merge16_t_c<MASK420, bits_per_pixel>, bits_per_pixel>(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch, const Byte *pMask, ptrdiff_t nMaskPitch, int nWidth, int nHeight, int nOrigHeight); \
template void merge16_t_simd<CPU_SSE4_1, MASK420, merge16_t_c<MASK420, bits_per_pixel>, bits_per_pixel>(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch, const Byte *pMask, ptrdiff_t nMaskPitch, int nWidth, int nHeight, int nOrigHeight); \
template void merge16_t_simd<CPU_SSE2, MASK422, merge16_t_c<MASK422, bits_per_pixel>, bits_per_pixel>(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch, const Byte *pMask, ptrdiff_t nMaskPitch, int nWidth, int nHeight, int nOrigHeight); \
template void merge16_t_simd<CPU_SSSE3, MASK422, merge16_t_c<MASK422, bits_per_pixel>, bits_per_pixel>(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch, const Byte *pMask, ptrdiff_t nMaskPitch, int nWidth, int nHeight, int nOrigHeight); \
template void merge16_t_simd<CPU_SSE4_1, MASK422, merge16_t_c<MASK422, bits_per_pixel>, bits_per_pixel>(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch, const Byte *pMask, ptrdiff_t nMaskPitch, int nWidth, int nHeight, int nOrigHeight);

MAKE_TEMPLATES(10)
MAKE_TEMPLATES(12)
MAKE_TEMPLATES(14)
MAKE_TEMPLATES(16)
#undef MAKE_TEMPLATES

#define MAKE_EXPORTS(bits_per_pixel) \
Processor16 *merge16_##bits_per_pixel##_c = &merge16_t_c<MASK444, bits_per_pixel>; \
Processor16 *merge16_luma_420_##bits_per_pixel##_c = &merge16_t_c<MASK420, bits_per_pixel>; \
Processor16 *merge16_luma_422_##bits_per_pixel##_c = &merge16_t_c<MASK422, bits_per_pixel>; \
Processor16 *merge16_##bits_per_pixel##_sse2 = merge16_t_simd<CPU_SSE2, MASK444, merge16_t_c<MASK444, bits_per_pixel>, bits_per_pixel>; \
Processor16 *merge16_##bits_per_pixel##_sse4_1 = merge16_t_simd<CPU_SSE4_1, MASK444, merge16_t_c<MASK444, bits_per_pixel>, bits_per_pixel>; \
Processor16 *merge16_luma_420_##bits_per_pixel##_sse2 = merge16_t_simd<CPU_SSE2, MASK420, merge16_t_c<MASK420, bits_per_pixel>, bits_per_pixel>; \
Processor16 *merge16_luma_420_##bits_per_pixel##_ssse3 = merge16_t_simd<CPU_SSSE3, MASK420, merge16_t_c<MASK420, bits_per_pixel>, bits_per_pixel>; \
Processor16 *merge16_luma_420_##bits_per_pixel##_sse4_1 = merge16_t_simd<CPU_SSE4_1, MASK420, merge16_t_c<MASK420, bits_per_pixel>, bits_per_pixel>; \
Processor16 *merge16_luma_422_##bits_per_pixel##_sse2 = merge16_t_simd<CPU_SSE2, MASK422, merge16_t_c<MASK422, bits_per_pixel>, bits_per_pixel>; \
Processor16 *merge16_luma_422_##bits_per_pixel##_ssse3 = merge16_t_simd<CPU_SSSE3, MASK422, merge16_t_c<MASK422, bits_per_pixel>, bits_per_pixel>; \
Processor16 *merge16_luma_422_##bits_per_pixel##_sse4_1 = merge16_t_simd<CPU_SSE4_1, MASK422, merge16_t_c<MASK422, bits_per_pixel>, bits_per_pixel>;

MAKE_EXPORTS(10)
MAKE_EXPORTS(12)
MAKE_EXPORTS(14)
MAKE_EXPORTS(16)
#undef MAKE_EXPORTS

} } } }