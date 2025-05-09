#ifndef __Mt_Merge8_H__
#define __Mt_Merge8_H__

#include "../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Merge {

/* 8 bit */
typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                        const Byte *pSrc2, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight);
typedef void(Processor16)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
  const Byte *pSrc2, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight, int nOrigHeight);
typedef void(Processor32)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
  const Byte *pSrc2, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight);

Processor merge_c;
Processor merge_luma_420_c;
Processor merge_luma_422_c;
Processor merge_luma_411_c;

extern Processor *merge_sse2;
extern Processor *merge_asse2;
extern Processor *merge_luma_420_sse2;
extern Processor *merge_luma_420_asse2;
extern Processor *merge_luma_422_sse2;
extern Processor *merge_luma_422_asse2;
extern Processor *merge_luma_411_sse2;
extern Processor *merge_luma_411_asse2;

extern Processor *merge_sse4;
extern Processor *merge_asse4;
extern Processor *merge_luma_420_sse4;
extern Processor *merge_luma_420_asse4;
extern Processor *merge_luma_422_sse4;
extern Processor *merge_luma_422_asse4;
extern Processor *merge_luma_411_sse4;
extern Processor *merge_luma_411_asse4;

extern Processor *merge_avx2;
extern Processor *merge_luma_420_avx2;
extern Processor *merge_luma_422_avx2;
extern Processor *merge_luma_411_avx2;

/* 16 bit */

extern Processor16 *merge16_c_stacked;
extern Processor16 *merge16_luma_420_c_stacked;
extern Processor16 *merge16_luma_422_c_stacked;

extern Processor16 *merge16_sse2_stacked;
extern Processor16 *merge16_sse4_1_stacked;

extern Processor16 *merge16_luma_420_sse2_stacked;
extern Processor16 *merge16_luma_420_ssse3_stacked;
extern Processor16 *merge16_luma_420_sse4_1_stacked;

extern Processor16 *merge16_luma_422_sse2_stacked;
extern Processor16 *merge16_luma_422_ssse3_stacked;
extern Processor16 *merge16_luma_422_sse4_1_stacked;


#define MAKE_16BIT_EXTERNS(bits_per_pixel) \
extern Processor16 *merge16_##bits_per_pixel##_c; \
extern Processor16 *merge16_luma_420_##bits_per_pixel##_c; \
extern Processor16 *merge16_luma_422_##bits_per_pixel##_c; \
extern Processor16 *merge16_##bits_per_pixel##_sse2; \
extern Processor16 *merge16_##bits_per_pixel##_sse4_1; \
extern Processor16 *merge16_##bits_per_pixel##_avx2; \
extern Processor16 *merge16_luma_420_##bits_per_pixel##_sse2; \
extern Processor16 *merge16_luma_420_##bits_per_pixel##_ssse3; \
extern Processor16 *merge16_luma_420_##bits_per_pixel##_sse4_1; \
extern Processor16 *merge16_luma_420_##bits_per_pixel##_avx2; \
extern Processor16 *merge16_luma_422_##bits_per_pixel##_sse2; \
extern Processor16 *merge16_luma_422_##bits_per_pixel##_ssse3; \
extern Processor16 *merge16_luma_422_##bits_per_pixel##_sse4_1; \
extern Processor16 *merge16_luma_422_##bits_per_pixel##_avx2;

MAKE_16BIT_EXTERNS(10)
MAKE_16BIT_EXTERNS(12)
MAKE_16BIT_EXTERNS(14)
MAKE_16BIT_EXTERNS(16)
#undef MAKE_16BIT_EXTERNS

/* 32 bit */
Processor32 merge32_c;
Processor32 merge32_luma_420_c;
Processor32 merge32_luma_422_c;

extern Processor32 *merge32_sse2;
extern Processor32 *merge32_asse2;
extern Processor32 *merge32_luma_420_sse2;
extern Processor32 *merge32_luma_420_asse2;
extern Processor32 *merge32_luma_422_sse2;
extern Processor32 *merge32_luma_422_asse2;
extern Processor32 *merge32_avx2;
extern Processor32 *merge32_luma_420_avx2;
extern Processor32 *merge32_luma_422_avx2;

class Merge : public MaskTools::Filter
{

   bool use_luma;
   ProcessorList<Processor> processors;
   ProcessorList<Processor> chroma_processors;
   ProcessorList<Processor16> processors16;
   ProcessorList<Processor16> chroma_processors16;
   ProcessorList<Processor32> processors32;
   ProcessorList<Processor32> chroma_processors32;

protected:

  virtual void process(int n, const Plane<Byte> &dst, int nPlane, const Frame<const Byte> frames[4], const Constraint constraints[4], PNeoEnv env) override
  {
    UNUSED(n); UNUSED(env);
    int bits_per_pixel = bit_depths[C];
    if (parameters["stacked"].toBool())
      bits_per_pixel = 16;
    if (use_luma && ((nPlane == 1 || nPlane == 2))) {
      if (!(width_ratios[1][C] == 1 && height_ratios[1][C] == 1)) {
        // 420 or 422 or 411
        switch (bits_per_pixel) {
        case 8:
          chroma_processors.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
            frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
            frames[1].plane(0).data(), frames[1].plane(0).pitch(),
            dst.width(), dst.height());
          break;
        case 10: case 12: case 14: case 16:
          chroma_processors16.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
            frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
            frames[1].plane(0).data(), frames[1].plane(0).pitch(),
            dst.width(), dst.height(), dst.origheight());
          break;
        case 32:
          chroma_processors32.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
            frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
            frames[1].plane(0).data(), frames[1].plane(0).pitch(),
            dst.width(), dst.height());
          break;
        }
      }
      else {
        // 444
        switch (bits_per_pixel) {
        case 8:
          processors.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
            frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
            frames[1].plane(0).data(), frames[1].plane(0).pitch(),
            dst.width(), dst.height());
          break;
        case 10: case 12: case 14: case 16:
          processors16.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
            frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
            frames[1].plane(0).data(), frames[1].plane(0).pitch(),
            dst.width(), dst.height(), dst.origheight());
          break;
        case 32:
          processors32.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
            frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
            frames[1].plane(0).data(), frames[1].plane(0).pitch(),
            dst.width(), dst.height());
          break;
        }
      }
    }
    else { // Y or Alpha plane or luma==false
      switch (bits_per_pixel) {
      case 8:
        processors.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
          frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
          frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(),
          dst.width(), dst.height());
        break;
      case 10: case 12: case 14: case 16:
        processors16.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
          frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
          frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(),
          dst.width(), dst.height(), dst.origheight());
        break;
      case 32:
        processors32.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
          frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
          frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(),
          dst.width(), dst.height());
        break;
      }
    }
  }

public:
   Merge(const Parameters &parameters, CpuFlags cpuFlags, PNeoEnv env)
      : MaskTools::Filter( parameters, FilterProcessingType::INPLACE, (CpuFlags)cpuFlags)
   {
      UNUSED(env);
      bool isStacked = parameters["stacked"].toBool();
      int bits_per_pixel = bit_depths[C];

      bool is420 = width_ratios[1][C] == 2 && height_ratios[1][C] == 2;
      bool is422 = width_ratios[1][C] == 2 && height_ratios[1][C] == 1;
      bool is411 = width_ratios[1][C] == 4 && height_ratios[1][C] == 1;

      if (isStacked && bits_per_pixel != 8) {
        error = "Stacked specified for a non-8 bit clip";
        return;
      }
      if (isStacked && is411) {
        error = "Stacked 16 bit for 4:1:1 not supported";
        return;
      }

      use_luma = parameters["luma"].toBool();
      // PF: for greyscale: graceful fallback to chromaless operation
      if (plane_counts[C] == 1) {
        use_luma = false;
        operators[1] = operators[2] = operators[3] = NONE;
      }

      // todo: allow 8 bit masks for 10+ bits

      auto c1 = childs[0]->colorspace();
      auto c2 = childs[1]->colorspace();
      auto c3 = childs[2]->colorspace(); // mask

      if (bit_depths[c1] != bit_depths[c2] || bit_depths[c1] != bit_depths[c3]) {
        error = "clips should have identical bit depths";
        return;
      }

      if (use_luma) {
         // PF: implement use_luma for 422 and 411 clips
        if ((width_ratios[1][c1] != width_ratios[1][c2]) || (height_ratios[1][c1] != height_ratios[1][c2])) {
            error = "clips should have identical colorspace";
            return;
        }

        /* if "luma" is set, we force the chroma processing. Much more handy */
        operators[1] = operators[2] = PROCESS;
        /* no need to change U/V default processing, because of in place filter */
      }

      switch (bits_per_pixel) {
      case 8:
        if (isStacked) {
          /* add the processors16 */
          processors16.push_back(Filtering::Processor<Processor16>(merge16_c_stacked, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          processors16.push_back(Filtering::Processor<Processor16>(merge16_sse2_stacked, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
          processors16.push_back(Filtering::Processor<Processor16>(merge16_sse4_1_stacked, Constraint(CPU_SSE4_1, 1, 1, 1, 1), 2));

          /* add the chroma processors16 */
          if (is420) {
            chroma_processors16.push_back(Filtering::Processor<Processor16>(merge16_luma_420_c_stacked, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            chroma_processors16.push_back(Filtering::Processor<Processor16>(merge16_luma_420_sse2_stacked, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
            chroma_processors16.push_back(Filtering::Processor<Processor16>(merge16_luma_420_ssse3_stacked, Constraint(CPU_SSSE3, 1, 1, 1, 1), 2));
            chroma_processors16.push_back(Filtering::Processor<Processor16>(merge16_luma_420_sse4_1_stacked, Constraint(CPU_SSE4_1, 1, 1, 1, 1), 3));
          }
          else if (is422) { // 422
            chroma_processors16.push_back(Filtering::Processor<Processor16>(merge16_luma_422_c_stacked, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            chroma_processors16.push_back(Filtering::Processor<Processor16>(merge16_luma_422_sse2_stacked, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
            chroma_processors16.push_back(Filtering::Processor<Processor16>(merge16_luma_422_ssse3_stacked, Constraint(CPU_SSSE3, 1, 1, 1, 1), 2));
            chroma_processors16.push_back(Filtering::Processor<Processor16>(merge16_luma_422_sse4_1_stacked, Constraint(CPU_SSE4_1, 1, 1, 1, 1), 3));
          }
        }
        else {
          /* add the processors */
          processors.push_back(Filtering::Processor<Processor>(merge_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          processors.push_back(Filtering::Processor<Processor>(merge_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
          processors.push_back(Filtering::Processor<Processor>(merge_asse2, Constraint(CPU_SSE2, 1, 1, 16, 16), 2));
          processors.push_back(Filtering::Processor<Processor>(merge_sse4, Constraint(CPU_SSE4_1, 1, 1, 1, 1), 3));
          processors.push_back(Filtering::Processor<Processor>(merge_asse4, Constraint(CPU_SSE4_1, 1, 1, 16, 16), 4));
          processors.push_back(Filtering::Processor<Processor>(merge_avx2, Constraint(CPU_AVX2, 1, 1, 1, 1), 5));

          /* add the chroma processors */
          // they are used only for 420 and 422
          if (is420) {
            chroma_processors.push_back(Filtering::Processor<Processor>(merge_luma_420_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            chroma_processors.push_back(Filtering::Processor<Processor>(merge_luma_420_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
            chroma_processors.push_back(Filtering::Processor<Processor>(merge_luma_420_asse2, Constraint(CPU_SSE2, 1, 1, 16, 16), 2));
            chroma_processors.push_back(Filtering::Processor<Processor>(merge_luma_420_sse4, Constraint(CPU_SSE4_1, 1, 1, 1, 1), 3));
            chroma_processors.push_back(Filtering::Processor<Processor>(merge_luma_420_asse4, Constraint(CPU_SSE4_1, 1, 1, 16, 16), 4));
            chroma_processors.push_back(Filtering::Processor<Processor>(merge_luma_420_avx2, Constraint(CPU_AVX2, 1, 1, 1, 1), 5));
          }
          else if (is422) { // 422
            chroma_processors.push_back(Filtering::Processor<Processor>(merge_luma_422_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            chroma_processors.push_back(Filtering::Processor<Processor>(merge_luma_422_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
            chroma_processors.push_back(Filtering::Processor<Processor>(merge_luma_422_asse2, Constraint(CPU_SSE2, 1, 1, 16, 16), 2));
            chroma_processors.push_back(Filtering::Processor<Processor>(merge_luma_422_sse4, Constraint(CPU_SSE4_1, 1, 1, 1, 1), 3));
            chroma_processors.push_back(Filtering::Processor<Processor>(merge_luma_422_asse4, Constraint(CPU_SSE4_1, 1, 1, 16, 16), 4));
            chroma_processors.push_back(Filtering::Processor<Processor>(merge_luma_422_avx2, Constraint(CPU_AVX2, 1, 1, 1, 1), 5));
          }
          else {
            chroma_processors.push_back(Filtering::Processor<Processor>(merge_luma_411_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            chroma_processors.push_back(Filtering::Processor<Processor>(merge_luma_411_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
            chroma_processors.push_back(Filtering::Processor<Processor>(merge_luma_411_asse2, Constraint(CPU_SSE2, 1, 1, 16, 16), 2));
            chroma_processors.push_back(Filtering::Processor<Processor>(merge_luma_411_sse4, Constraint(CPU_SSE4_1, 1, 1, 1, 1), 3));
            chroma_processors.push_back(Filtering::Processor<Processor>(merge_luma_411_asse4, Constraint(CPU_SSE4_1, 1, 1, 16, 16), 4));
            chroma_processors.push_back(Filtering::Processor<Processor>(merge_luma_411_avx2, Constraint(CPU_AVX2, 1, 1, 1, 1), 5));
          }
        }
        break;
      case 10: case 12: case 14: case 16:
#define MAKE_16BIT_PROCESSORS(bits_per_pixel) \
          /* add the processors16 */ \
          processors16.push_back( Filtering::Processor<Processor16>( merge16_##bits_per_pixel##_c, Constraint( CPU_NONE, 1, 1, 1, 1 ), 0 ) ); \
          processors16.push_back( Filtering::Processor<Processor16>( merge16_##bits_per_pixel##_sse2, Constraint( CPU_SSE2, 1, 1, 1, 1 ), 1 ) ); \
          processors16.push_back( Filtering::Processor<Processor16>( merge16_##bits_per_pixel##_sse4_1, Constraint( CPU_SSE4_1, 1, 1, 1, 1 ), 2 ) ); \
          processors16.push_back( Filtering::Processor<Processor16>( merge16_##bits_per_pixel##_avx2, Constraint( CPU_AVX2, 1, 1, 1, 1 ), 3 ) ); \
          /* add the chroma processors16 */ \
          /* used only for 420 and 422  */ \
          if (is420) { \
            chroma_processors16.push_back(Filtering::Processor<Processor16>(merge16_luma_420_##bits_per_pixel##_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0)); \
            chroma_processors16.push_back(Filtering::Processor<Processor16>(merge16_luma_420_##bits_per_pixel##_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1)); \
            chroma_processors16.push_back(Filtering::Processor<Processor16>(merge16_luma_420_##bits_per_pixel##_ssse3, Constraint(CPU_SSSE3, 1, 1, 1, 1), 2)); \
            chroma_processors16.push_back(Filtering::Processor<Processor16>(merge16_luma_420_##bits_per_pixel##_sse4_1, Constraint(CPU_SSE4_1, 1, 1, 1, 1), 3)); \
            chroma_processors16.push_back(Filtering::Processor<Processor16>(merge16_luma_420_##bits_per_pixel##_avx2, Constraint(CPU_AVX2, 1, 1, 1, 1), 4)); \
          } \
          else { /* 422 */ \
            chroma_processors16.push_back(Filtering::Processor<Processor16>(merge16_luma_422_##bits_per_pixel##_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0)); \
            chroma_processors16.push_back(Filtering::Processor<Processor16>(merge16_luma_422_##bits_per_pixel##_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1)); \
            chroma_processors16.push_back(Filtering::Processor<Processor16>(merge16_luma_422_##bits_per_pixel##_ssse3, Constraint(CPU_SSSE3, 1, 1, 1, 1), 2)); \
            chroma_processors16.push_back(Filtering::Processor<Processor16>(merge16_luma_422_##bits_per_pixel##_sse4_1, Constraint(CPU_SSE4_1, 1, 1, 1, 1), 3)); \
            chroma_processors16.push_back(Filtering::Processor<Processor16>(merge16_luma_422_##bits_per_pixel##_avx2, Constraint(CPU_AVX2, 1, 1, 1, 1), 4)); \
          }

        switch (bit_depths[C]) {
        case 10: MAKE_16BIT_PROCESSORS(10);
          break;
        case 12: MAKE_16BIT_PROCESSORS(12);
          break;
        case 14: MAKE_16BIT_PROCESSORS(14);
          break;
        case 16: MAKE_16BIT_PROCESSORS(16);
          break;
        }
#undef MAKE_16BIT_PROCESSORS
        break;
      case 32:
        /* add the processors */
        processors32.push_back(Filtering::Processor<Processor32>(merge32_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
        processors32.push_back(Filtering::Processor<Processor32>(merge32_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
        processors32.push_back(Filtering::Processor<Processor32>(merge32_asse2, Constraint(CPU_SSE2, 1, 1, 16, 16), 2));
        processors32.push_back(Filtering::Processor<Processor32>(merge32_avx2, Constraint(CPU_AVX2, 1, 1, 1, 1), 3));

        /* add the chroma processors */
        // they are used only for 420 and 422
        if (is420) {
          chroma_processors32.push_back(Filtering::Processor<Processor32>(merge32_luma_420_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          chroma_processors32.push_back(Filtering::Processor<Processor32>(merge32_luma_420_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
          chroma_processors32.push_back(Filtering::Processor<Processor32>(merge32_luma_420_asse2, Constraint(CPU_SSE2, 1, 1, 16, 16), 2));
          chroma_processors32.push_back(Filtering::Processor<Processor32>(merge32_luma_420_avx2, Constraint(CPU_AVX2, 1, 1, 1, 1), 3));
        }
        else if (is422) { // 422
          chroma_processors32.push_back(Filtering::Processor<Processor32>(merge32_luma_422_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          chroma_processors32.push_back(Filtering::Processor<Processor32>(merge32_luma_422_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
          chroma_processors32.push_back(Filtering::Processor<Processor32>(merge32_luma_422_asse2, Constraint(CPU_SSE2, 1, 1, 16, 16), 2));
          chroma_processors32.push_back(Filtering::Processor<Processor32>(merge32_luma_422_avx2, Constraint(CPU_AVX2, 1, 1, 1, 1), 3));
        }
        break;
      }
   }

   InputConfiguration &input_configuration() const { return InPlaceThreeFrame(); }
	 InputConfiguration &input_configuration_cuda() const { return ThreeFrame(); }

   static Signature filter_signature()
   {
      Signature signature("kmt_merge");

      signature.add( Parameter( TYPE_CLIP, "", false) );
      signature.add( Parameter( TYPE_CLIP, "", false) );
      signature.add( Parameter( TYPE_CLIP, "", false) );
      signature.add( Parameter( false, "luma", false) );

      add_defaults( signature );

      signature.add(Parameter(false, "stacked", false));
      return signature;
   }
};

} } } } // namespace Merge, Filters, MaskTools, Filtering

#endif