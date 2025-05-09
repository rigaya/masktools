#ifndef __Mt_Lut_H__
#define __Mt_Lut_H__

#include "../../../common/base/filter.h"
#include "../../../../common/parser/parser.h"
#include "../lut_data.h"
#include "../lut_kernel.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Lut { namespace Single {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight, const Byte* lut);
typedef void(Processor16)(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight, const Word* lut, int mask);
typedef void(ProcessorCtx)(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight, Parser::Context &ctx);

Processor lut_c;

Processor16 lut16_c_native;
Processor16 lut16_c_stacked;

ProcessorCtx realtime8_c;
extern ProcessorCtx *realtime10_c;
extern ProcessorCtx *realtime12_c;
extern ProcessorCtx *realtime14_c;
extern ProcessorCtx *realtime16_c;
ProcessorCtx realtime32_c;

class Lut : public MaskTools::Filter
{
   std::unique_ptr<LutData> luts;
   int planeMap[4];

   Processor *processor;
   Processor16 *processor16;
   int bits_per_pixel;
   bool isStacked;
   bool realtime;

protected:
    virtual void process(int n, const Plane<Byte> &dst, int nPlane, const ::Filtering::Frame<const Byte> frames[4], const Constraint constraints[4], PNeoEnv env) override
    {
        UNUSED(n);
        UNUSED(constraints);
        UNUSED(frames);
        if (::IsCUDA(env)) {
           const uint8_t* const pSrc[1] = { frames[0].plane(nPlane).data() };
           lut_cuda(bits_per_pixel, 1, dst.data(), pSrc, (int)dst.pitch(), dst.width(), dst.height(),
              luts->GetTable(planeMap[nPlane], env), env);
        }
        else if (bits_per_pixel == 8)
           lut_c(dst.data(), dst.pitch(), dst.width(), dst.height(),
            (const uint8_t*)luts->GetTable(planeMap[nPlane], env));
        else if (bits_per_pixel <= 16)
           lut16_c_native(dst.data(), dst.pitch(), dst.width(), dst.height(),
            (const uint16_t*)luts->GetTable(planeMap[nPlane], env), (1 << bits_per_pixel) - 1);
    }

public:
   Lut(const Parameters &parameters, CpuFlags cpuFlags, PNeoEnv env)
      : MaskTools::Filter( parameters, FilterProcessingType::INPLACE, (CpuFlags)cpuFlags)
   {
      static const char *expr_strs[] = { "yExpr", "uExpr", "vExpr", "aExpr" };

      Parser::Parser parser = Parser::getDefaultParser().addSymbol(Parser::Symbol::X);

      isStacked = parameters["stacked"].toBool();
      bits_per_pixel = bit_depths[C];

      if (isStacked) {
         error = "Stacked is not supported";
         return;
      }

      realtime = parameters["realtime"].toBool();

      if (bits_per_pixel == 32) { // no lookup for float
        realtime = true;
      }

      if (realtime) {
         error = "realtime is not supported";
         return;
      }

      std::vector<std::unique_ptr<Parser::Context>> exprs;
      int firstGeneric = -1;

      /* compute the luts16 */
      for (int i = 0; i < 4; i++)
      {
         planeMap[i] = -1;

        if (operators[i] != PROCESS) {
          continue;
        }

        if (parameters[expr_strs[i]].undefinedOrEmptyString() && parameters["expr"].undefinedOrEmptyString()) {
          operators[i] = NONE; //inplace
          continue;
        }

        if (parameters[expr_strs[i]].is_defined()) {
          parser.parse(parameters[expr_strs[i]].toString(), " ");
        }
        else if (firstGeneric != -1) {
           planeMap[i] = firstGeneric;
           continue;
        }
        else {
           firstGeneric = (int)exprs.size();
           parser.parse(parameters["expr"].toString(), " ");
        }

        planeMap[i] = (int)exprs.size();
        exprs.emplace_back(new Parser::Context(parser.getExpression()));

        if (!exprs.back()->check())
        {
          error = "invalid expression in the lut";
          return;
        }
      }

      luts = make_lut_data(bits_per_pixel, 1, exprs, env);
   }

   InputConfiguration &input_configuration() const { return InPlaceOneFrame(); }
   InputConfiguration &input_configuration_cuda() const { return OneFrame(); }
   bool is_cuda_available() { return true; }

   static Signature filter_signature()
   {
      Signature signature("kmt_lut");

      signature.add(Parameter(TYPE_CLIP, "", false));
      signature.add(Parameter(String("x"), "expr", false));
      signature.add(Parameter(String("x"), "yExpr", false));
      signature.add(Parameter(String("x"), "uExpr", false));
      signature.add(Parameter(String("x"), "vExpr", false));
      signature.add(Parameter(String("x"), "aExpr", false));

      add_defaults( signature );

      signature.add(Parameter(false, "stacked", false));
      signature.add(Parameter(false, "realtime", false));
      return signature;
   }
};


} } } } }

#endif