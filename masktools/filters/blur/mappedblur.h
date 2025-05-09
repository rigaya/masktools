#ifndef __Mt_Blur_H__
#define __Mt_Blur_H__

#include "../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Blur { 

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, const Byte *pMask, ptrdiff_t nMaskPitch, const Short *matrix, int nWidth, int nHeight);
typedef void(Processor16)(Word *pDst, ptrdiff_t nDstPitch, const Word *pSrc, ptrdiff_t nSrcPitch, const Word *pMask, ptrdiff_t nMaskPitch, const Short *matrix, int nWidth, int nHeight);
typedef void(Processor32)(Float *pDst, ptrdiff_t nDstPitch, const Float *pSrc, ptrdiff_t nSrcPitch, const Float *pMask, ptrdiff_t nMaskPitch, const Float *matrix, int nWidth, int nHeight);

extern Processor *mapped_below_c;
extern Processor *mapped_all_c;
extern Processor16 *mapped_below_10_c;
extern Processor16 *mapped_all_10_c;
extern Processor16 *mapped_below_12_c;
extern Processor16 *mapped_all_12_c;
extern Processor16 *mapped_below_14_c;
extern Processor16 *mapped_all_14_c;
extern Processor16 *mapped_below_16_c;
extern Processor16 *mapped_all_16_c;
extern Processor32 *mapped_below_32_c;
extern Processor32 *mapped_all_32_c;

class MappedBlur : public MaskTools::Filter
{
   Short matrix[10];
   Float matrix_f[10];

   int bits_per_pixel;

   ProcessorList<Processor> processors;
   ProcessorList<Processor16> processors16;
   ProcessorList<Processor32> processors32;

protected:
  virtual void process(int n, const Plane<Byte> &dst, int nPlane, const Frame<const Byte> frames[4], const Constraint constraints[4], PNeoEnv env) override
  {
    UNUSED(n); UNUSED(env);
    if (bits_per_pixel == 8) {
      processors.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
        frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
        frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(),
        matrix, dst.width(), dst.height());
    }
    else if (bits_per_pixel <= 16) {
      processors16.best_processor(constraints[nPlane])((Word *)dst.data(), dst.pitch(),
        (Word *)frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
        (Word *)frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(),
        matrix, dst.width(), dst.height());
    }
    else {
      processors32.best_processor(constraints[nPlane])((Float *)dst.data(), dst.pitch(),
        (Float *)frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
        (Float *)frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(),
        matrix_f, dst.width(), dst.height());
    }
  }

public:
   MappedBlur(const Parameters &parameters, CpuFlags cpuFlags, PNeoEnv env)
      : MaskTools::Filter( parameters, FilterProcessingType::CHILD, (CpuFlags)cpuFlags)
   {
      UNUSED(env);
      bits_per_pixel = bit_depths[C];
      bool isFloat = bits_per_pixel == 32;

      auto coeffs = Parser::getDefaultParser().parse(parameters["kernel"].toString(), " ").getExpression();

      if (isFloat) {
        memset(matrix_f, 0, sizeof(matrix_f));
        for (int i = 0; i < 9; i++)
        {
          if (!coeffs.size())
          {
            error = "invalid kernel";
            return;
          }

          matrix_f[9] += abs(matrix_f[i] = Float(coeffs.front().getValue(0, 0, 0)));
          coeffs.pop_front();
        }

        if (coeffs.size())
          matrix_f[9] = Float(coeffs.front().getValue(0, 0, 0));

        if (!matrix_f[9])
          matrix_f[9] = 1.0f;
      }
      else {
        memset(matrix, 0, sizeof(matrix));
        for (int i = 0; i < 9; i++)
        {
          if (!coeffs.size())
          {
            error = "invalid kernel";
            return;
          }

          matrix[9] += abs(matrix[i] = Short(coeffs.front().getValue(0, 0, 0)));
          coeffs.pop_front();
        }

        if (coeffs.size())
          matrix[9] = Short(coeffs.front().getValue(0, 0, 0));

        if (!matrix[9])
          matrix[9] = 1;
      }
      /* add the processors */
      if (parameters["mode"].toString() == "replace") {
        switch (bits_per_pixel) {
        case 8:
          processors.push_back(Filtering::Processor<Processor>(mapped_all_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 10:
          processors16.push_back(Filtering::Processor<Processor16>(mapped_all_10_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 12:
          processors16.push_back(Filtering::Processor<Processor16>(mapped_all_12_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 14:
          processors16.push_back(Filtering::Processor<Processor16>(mapped_all_14_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 16:
          processors16.push_back(Filtering::Processor<Processor16>(mapped_all_16_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 32:
          processors32.push_back(Filtering::Processor<Processor32>(mapped_all_32_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        }
      }
      else if (parameters["mode"].toString() == "dump") {
        switch (bits_per_pixel) {
        case 8:
          processors.push_back(Filtering::Processor<Processor>(mapped_below_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 10:
          processors16.push_back(Filtering::Processor<Processor16>(mapped_below_10_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 12:
          processors16.push_back(Filtering::Processor<Processor16>(mapped_below_12_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 14:
          processors16.push_back(Filtering::Processor<Processor16>(mapped_below_14_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 16:
          processors16.push_back(Filtering::Processor<Processor16>(mapped_below_16_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        case 32:
          processors32.push_back(Filtering::Processor<Processor32>(mapped_below_32_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
          break;
        }
      }
      else
      {
         error = "unknown mode";
         return;
      }
   }

   InputConfiguration &input_configuration() const { return TwoFrame(); }

   static Signature filter_signature()
   {
      Signature signature("kmt_mappedblur");

      signature.add(Parameter(TYPE_CLIP, "", false));
      signature.add(Parameter(TYPE_CLIP, "", false));
      signature.add(Parameter(String("1 1 1 1 1 1 1 1 1"), "kernel", false));
      signature.add(Parameter(String("replace"), "mode", false));

      return add_defaults( signature );
   }
};

} } } }

#endif