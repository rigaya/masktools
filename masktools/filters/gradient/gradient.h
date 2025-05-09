#ifndef __Mt_Gradient_H__
#define __Mt_Gradient_H__

#include "../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Gradient {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, const Byte *pRef, ptrdiff_t nRefPitch, int nWidth, int nHeight, int nX, int nY, int nMinimum, int nMaximum, int nPrecision);
typedef void(Processor16)(Word *pDst, ptrdiff_t nDstPitch, const Word *pSrc, ptrdiff_t nSrcPitch, const Word *pRef, ptrdiff_t nRefPitch, int nWidth, int nHeight, int nX, int nY, int nMinimum, int nMaximum, int nPrecision);
typedef void(Processor32)(Float *pDst, ptrdiff_t nDstPitch, const Float *pSrc, ptrdiff_t nSrcPitch, const Float *pRef, ptrdiff_t nRefPitch, int nWidth, int nHeight, int nX, int nY, Float nMinimum, Float nMaximum, int nPrecision);

extern Processor *sad_c;
extern Processor16 *sad_10_c;
extern Processor16 *sad_12_c;
extern Processor16 *sad_14_c;
extern Processor16 *sad_16_c;
extern Processor32 *sad_32_c;

class Gradient : public MaskTools::Filter
{

   ProcessorList<Processor> processors;
   ProcessorList<Processor16> processors16;
   ProcessorList<Processor32> processors32;
   int nX[4];
   int nY[4];
   int nMinimum;
   int nMaximum;
   float nMinimum_f;
   float nMaximum_f;
   int nPrecision;

   int bits_per_pixel;

protected:

  virtual void process(int n, const Plane<Byte> &dst, int nPlane, const Frame<const Byte> frames[4], const Constraint constraints[4], PNeoEnv env) override
  {
    UNUSED(n); UNUSED(env);
    if (bits_per_pixel == 8)
      processors.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
        frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
        frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(),
        dst.width(), dst.height(), nX[nPlane], nY[nPlane], nMinimum, nMaximum, nPrecision);
    else if (bits_per_pixel <= 16)
      processors16.best_processor(constraints[nPlane])((Word *)dst.data(), dst.pitch(),
      (Word *)frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
        (Word *)frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(),
        dst.width(), dst.height(), nX[nPlane], nY[nPlane], nMinimum, nMaximum, nPrecision);
    else // 32
      processors32.best_processor(constraints[nPlane])((Float *)dst.data(), dst.pitch(),
      (Float *)frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
        (Float *)frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(),
        dst.width(), dst.height(), nX[nPlane], nY[nPlane], nMinimum_f, nMaximum_f, nPrecision);
  }

public:
   Gradient(const Parameters &parameters, CpuFlags cpuFlags, PNeoEnv env)
      : MaskTools::Filter( parameters, FilterProcessingType::CHILD, (CpuFlags)cpuFlags)
   {
      UNUSED(env);
      bits_per_pixel = bit_depths[C];
     
      /* add the processors */
      if (parameters["distorsion"].toString() == "sad")
      {
        switch (bits_per_pixel) {
        case 8: processors.push_back(Filtering::Processor<Processor>(sad_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0)); break;
        case 10: processors16.push_back(Filtering::Processor<Processor16>(sad_10_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0)); break;
        case 12: processors16.push_back(Filtering::Processor<Processor16>(sad_12_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0)); break;
        case 14: processors16.push_back(Filtering::Processor<Processor16>(sad_14_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0)); break;
        case 16: processors16.push_back(Filtering::Processor<Processor16>(sad_16_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0)); break;
        case 32: processors32.push_back(Filtering::Processor<Processor32>(sad_32_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0)); break;
        }
        if (bits_per_pixel <= 16) {
          nMinimum = parameters["min"].is_defined() ? parameters["min"].toInt() : 0;
          nMaximum = parameters["max"].is_defined() ? parameters["max"].toInt() : 16 * 16 * (1 << bits_per_pixel); // max sad of 16x16 block
        }
        else {
          nMinimum_f = parameters["min"].is_defined() ? (Float)parameters["min"].toFloat() : 0.0f;
          nMaximum_f = parameters["max"].is_defined() ? (Float)parameters["max"].toFloat() : 16 * 16 * 1.0f; // max sad of block size 16x16 
        }
      }
      else 
      {
         error = "unknown distorsion";
         return;
      }

      nX[0] = parameters["size_x"].is_defined() ? clip<int, int>(parameters["size_x"].toInt(), 0, nCoreWidth - 1) : nCoreWidth / 2;
      nY[0] = parameters["size_y"].is_defined() ? clip<int, int>(parameters["size_y"].toInt(), 0, nCoreHeight - 1) : nCoreHeight / 2;
      nPrecision = parameters["precision"].toInt();

      for ( int i = 1; i < plane_counts[C]; i++ )
      {
         nX[i] = nX[0] / width_ratios[i][C];
         nY[i] = nY[0] / height_ratios[i][C];
      }
   }

   InputConfiguration &input_configuration() const { return TwoFrame(); }

   static Signature filter_signature()
   {
      Signature signature("kmt_gradient");

      signature.add(Parameter(TYPE_CLIP, "", false));
      signature.add(Parameter(TYPE_CLIP, "", false));
      signature.add(Parameter(String("sad"), "distorsion", false));
      signature.add(Parameter(0, "size_x", false));
      signature.add(Parameter(0, "size_y", false));
      signature.add(Parameter(0.0f, "min", true));
      signature.add(Parameter(65535.0f, "max", true)); // bit depth adaptive 
      signature.add(Parameter(1, "precision", false));

      return add_defaults( signature );
   }

};


} } } }

#endif