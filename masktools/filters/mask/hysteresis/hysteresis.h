#ifndef __Mt_Hysteresis_H__
#define __Mt_Hysteresis_H__

#include "../../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Mask { namespace Hysteresis {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                        const Byte *pSrc2, ptrdiff_t nSrc2Pitch, Byte *pTemp, int nWidth, int nHeight);

extern Processor *hysteresis_8_c;
extern Processor *hysteresis_10_c;
extern Processor *hysteresis_12_c;
extern Processor *hysteresis_14_c;
extern Processor *hysteresis_16_c;
extern Processor *hysteresis_32_c;

class Hysteresis : public MaskTools::Filter
{
    Byte *stack;
    Processor *processor;

protected:
    virtual void process(int n, const Plane<Byte> &dst, int nPlane, const Filtering::Frame<const Byte> frames[4], const Constraint constraints[4], PNeoEnv env) override
    {
        UNUSED(n);
        UNUSED(constraints);
        UNUSED(env);
        processor(dst.data(), dst.pitch(),
            frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
            frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(),
            stack, dst.width(), dst.height());
    }

public:
    Hysteresis(const Parameters &parameters, CpuFlags cpuFlags, PNeoEnv env)
       : MaskTools::Filter(parameters, FilterProcessingType::CHILD, (CpuFlags)cpuFlags), stack(nullptr)
    {
       UNUSED(env);
      int bits_per_pixel = bit_depths[C];
      int pixelsize = bits_per_pixel == 8 ? 1 : (bits_per_pixel == 32 ? 4 : 2);
      switch (bits_per_pixel) {
      case 8: processor = hysteresis_8_c; break;
      case 10: processor = hysteresis_10_c; break;
      case 12: processor = hysteresis_12_c; break;
      case 14: processor = hysteresis_14_c; break;
      case 16: processor = hysteresis_16_c; break;
      case 32: processor = hysteresis_32_c; break;
      }

      stack = reinterpret_cast<Byte*>(_aligned_malloc(nWidth*nHeight*pixelsize, 16));
    }

    ~Hysteresis() {
        _aligned_free(stack);
    }

    InputConfiguration &input_configuration() const { return TwoFrame(); }

    static Signature filter_signature()
    {
        Signature signature("kmt_hysteresis");

        signature.add(Parameter(TYPE_CLIP, "", false));
        signature.add(Parameter(TYPE_CLIP, "", false));

        return add_defaults(signature);
    }
};


} } } } }

#endif