#ifndef __Mt_MakeDiff_H__
#define __Mt_MakeDiff_H__

#include "../../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Support { namespace MakeDiff {

/* 8 bit */
typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight);
typedef void(Processor16)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight, int nOrigHeight);
typedef void(Processor32)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight);

Processor makediff_c;
extern Processor *makediff_sse2;
extern Processor *makediff_asse2;

/* 16 bit */

Processor16 makediff16_stacked_c;
Processor16 makediff16_stacked_sse2;

extern Processor16 *makediff16_native_10_c;
extern Processor16 *makediff16_native_12_c;
extern Processor16 *makediff16_native_14_c;
extern Processor16 *makediff16_native_16_c;
extern Processor16 *makediff16_native_10_sse2;
extern Processor16 *makediff16_native_12_sse2;
extern Processor16 *makediff16_native_14_sse2;
extern Processor16 *makediff16_native_16_sse2;
extern Processor16 *makediff16_native_10_sse4_1;
extern Processor16 *makediff16_native_12_sse4_1;
extern Processor16 *makediff16_native_14_sse4_1;
extern Processor16 *makediff16_native_16_sse4_1;

/* 32 bit */
Processor32 makediff32_c;
extern Processor32 *makediff32_sse2;
extern Processor32 *makediff32_asse2;

class MakeDiff : public MaskTools::Filter
{

    ProcessorList<Processor> processors;
    ProcessorList<Processor16> processors16;
    ProcessorList<Processor32> processors32;
    int bits_per_pixel;

protected:

    virtual void process(int n, const Plane<Byte> &dst, int nPlane, const Filtering::Frame<const Byte> frames[4], const Constraint constraints[4], PNeoEnv env) override
    {
        UNUSED(n); UNUSED(env);
        if (bits_per_pixel == 8)
          processors.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
            frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
            dst.width(), dst.height());
        else if (bits_per_pixel <= 16)
          processors16.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
            frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
            dst.width(), dst.height(), dst.origheight());
        else
          processors32.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
            frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
            dst.width(), dst.height());
    }

public:
    MakeDiff(const Parameters &parameters, CpuFlags cpuFlags, PNeoEnv env
    ) : MaskTools::Filter(parameters, FilterProcessingType::INPLACE, (CpuFlags)cpuFlags)
    {
       UNUSED(env);
      bool isStacked = parameters["stacked"].toBool();
      bits_per_pixel = bit_depths[C];

      if (isStacked && bits_per_pixel != 8) {
        error = "Stacked specified for a non-8 bit clip";
        return;
      }

      if (isStacked)
        bits_per_pixel = 16;

      /* add the processors */
      if (isStacked) {
        processors16.push_back(Filtering::Processor<Processor16>(makediff16_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
        processors16.push_back(Filtering::Processor<Processor16>(makediff16_stacked_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
      }
      else {
        switch (bits_per_pixel) {
        case 8:
          processors.push_back(Filtering::Processor<Processor>(&makediff_c, Constraint(CPU_NONE, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 1), 0));
          processors.push_back(Filtering::Processor<Processor>(makediff_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 1), 1));
          processors.push_back(Filtering::Processor<Processor>(makediff_asse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_16, 16), 2));
          break;
        case 10:
          processors16.push_back(Filtering::Processor<Processor16>(makediff16_native_10_c, Constraint(CPU_NONE, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 1), 0));
          processors16.push_back(Filtering::Processor<Processor16>(makediff16_native_10_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 1), 1));
          processors16.push_back(Filtering::Processor<Processor16>(makediff16_native_10_sse4_1, Constraint(CPU_SSE4_1, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 1), 2));
          break;
        case 12:
          processors16.push_back(Filtering::Processor<Processor16>(makediff16_native_12_c, Constraint(CPU_NONE, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 1), 0));
          processors16.push_back(Filtering::Processor<Processor16>(makediff16_native_12_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 1), 1));
          processors16.push_back(Filtering::Processor<Processor16>(makediff16_native_12_sse4_1, Constraint(CPU_SSE4_1, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 1), 2));
          break;
        case 14:
          processors16.push_back(Filtering::Processor<Processor16>(makediff16_native_14_c, Constraint(CPU_NONE, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 1), 0));
          processors16.push_back(Filtering::Processor<Processor16>(makediff16_native_14_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 1), 1));
          processors16.push_back(Filtering::Processor<Processor16>(makediff16_native_14_sse4_1, Constraint(CPU_SSE4_1, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 1), 2));
          break;
        case 16:
          processors16.push_back(Filtering::Processor<Processor16>(makediff16_native_16_c, Constraint(CPU_NONE, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 1), 0));
          processors16.push_back(Filtering::Processor<Processor16>(makediff16_native_16_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 1), 1));
          processors16.push_back(Filtering::Processor<Processor16>(makediff16_native_16_sse4_1, Constraint(CPU_SSE4_1, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 1), 2));
          break;
        case 32:
          processors32.push_back(Filtering::Processor<Processor32>(&makediff32_c, Constraint(CPU_NONE, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 1), 0));
          processors32.push_back(Filtering::Processor<Processor32>(makediff32_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 1), 1));
          processors32.push_back(Filtering::Processor<Processor32>(makediff32_asse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_16, 16), 2));
          break;
        }
      }
    }

    InputConfiguration &input_configuration() const { return InPlaceTwoFrame(); }
		InputConfiguration &input_configuration_cuda() const { return TwoFrame(); }

    static Signature filter_signature()
    {
        Signature signature("kmt_makediff");

        signature.add(Parameter(TYPE_CLIP, "", false));
        signature.add(Parameter(TYPE_CLIP, "", false));

        add_defaults(signature);

        signature.add(Parameter(false, "stacked", false));
        return signature;
    }

};

} } } } }

#endif