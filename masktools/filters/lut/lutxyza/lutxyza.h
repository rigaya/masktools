#ifndef __Mt_Lutxyza_H__
#define __Mt_Lutxyza_H__

#include "../../../common/base/filter.h"
#include "../../../../common/parser/parser.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Lut { namespace Quad {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch, const Byte *pSrc2, ptrdiff_t nSrc2Pitch, const Byte *pSrc3, ptrdiff_t nSrc3Pitch, int nWidth, int nHeight, const Byte *lut);
typedef void(ProcessorCtx)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, const Byte *pSrc2, ptrdiff_t nSrc2Pitch, const Byte *pSrc3, ptrdiff_t nSrc3Pitch, int nWidth, int nHeight, Parser::Context &ctx);

Processor lut_c;

ProcessorCtx realtime8_c;
extern ProcessorCtx *realtime10_c;
extern ProcessorCtx *realtime12_c;
extern ProcessorCtx *realtime14_c;
extern ProcessorCtx *realtime16_c;
ProcessorCtx realtime32_c;

class Lutxyza : public MaskTools::Filter
{
    struct Lut {
        bool used;
        Byte *ptr;
    };

   Lut luts[4+1];

   static Byte *calculateLut(const std::deque<Filtering::Parser::Symbol> &expr, int bits_per_pixel) {
       Parser::Context ctx(expr);
       size_t bufsize = ((size_t)1 << bits_per_pixel);
       bufsize = bufsize * bufsize*bufsize*bufsize;
       Byte *lut = new Byte[bufsize];
       // expr = "x y + z + a + 4 /" -> 2 min lut calculation time on i7-3770
       // When is it worth? LUT or realtime?
       // Lut calculation takes 256*256*256*256 expr.evaluation
       // This equals to 2071 frames in 1920x1080 (one plane)
       for ( int x = 0; x < 256; x++ ) {
           for ( int y = 0; y < 256; y++ ) {
               for ( int z = 0; z < 256; z++ ) {
                 for (int a = 0; a < 256; a++) {
                   lut[((size_t)x << 24) + (y << 16) + (z << 8) + a] = ctx.compute_byte_xyza(x, y, z, a);
                 }
               }
           }
       }
       return lut;
   }

   // for realtime
   std::deque<Filtering::Parser::Symbol> *parsed_expressions[4];

   Processor *processor;
   ProcessorCtx *processorCtx;
   ProcessorCtx *processorCtx16;
   ProcessorCtx *processorCtx32;
   int bits_per_pixel;
   bool realtime;

protected:
    virtual void process(int n, const Plane<Byte> &dst, int nPlane, const Filtering::Frame<const Byte> frames[4], const Constraint constraints[4], PNeoEnv env) override
    {
        UNUSED(n);
        UNUSED(constraints);
        UNUSED(env);
        if (realtime) {
          // thread safety
          Parser::Context ctx(*parsed_expressions[nPlane]);
          processorCtx(dst.data(), dst.pitch(), 
            frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(), 
            frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(),
            frames[2].plane(nPlane).data(), frames[2].plane(nPlane).pitch(),
            dst.width(), dst.height(), ctx);
        }
        else {
          // 4D lut! 8 bit only
          processor(dst.data(), dst.pitch(),
            frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
            frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(),
            frames[2].plane(nPlane).data(), frames[2].plane(nPlane).pitch(),
            dst.width(), dst.height(), luts[nPlane].ptr);
        }
    }

public:
   Lutxyza(const Parameters &parameters, CpuFlags cpuFlags, PNeoEnv env)
      : MaskTools::Filter( parameters, FilterProcessingType::INPLACE, (CpuFlags)cpuFlags)
   {
      UNUSED(env);
      for (int i = 0; i < 4; i++) {
        parsed_expressions[i] = nullptr;
      }

      for (int i = 0; i < 4+1; ++i) {
        luts[i].used = false;
        luts[i].ptr = nullptr;
      }

      bits_per_pixel = bit_depths[C];
      realtime = parameters["realtime"].toBool();
     
      // once, when a 4 GByte lut memory is nothing, allow 8 bit 4D real lut by default :)
      if(bits_per_pixel>8)
        realtime = true;
      else if ((uint64_t)std::numeric_limits<size_t>::max() <= 0xFFFFFFFFull) {
        realtime = true; // 4D lut is not possible on 32 bit environment
      }

      static const char *expr_strs[] = { "yExpr", "uExpr", "vExpr", "aExpr" };

      Parser::Parser parser = Parser::getDefaultParser().addSymbol(Parser::Symbol::X).addSymbol(Parser::Symbol::Y).addSymbol(Parser::Symbol::Z).addSymbol(Parser::Symbol::A);

      /* compute the luts */
      for ( int i = 0; i < 4; i++ )
      {
          if (operators[i] != PROCESS) {
              continue;
          }

          if (parameters[expr_strs[i]].undefinedOrEmptyString() && parameters["expr"].undefinedOrEmptyString()) {
              operators[i] = COPY_SECOND; //inplace
              continue;
          }

          bool customExpressionDefined = false;
          if (parameters[expr_strs[i]].is_defined()) {
            parser.parse(parameters[expr_strs[i]].toString(), " ");
            customExpressionDefined = true;
          }
          else
            parser.parse(parameters["expr"].toString(), " ");

          // for check:
          Parser::Context ctx(parser.getExpression());

          if (!ctx.check())
          {
            error = "invalid expression in the lut";
            return;
          }

          if (realtime) {
            parsed_expressions[i] = new std::deque<Parser::Symbol>(parser.getExpression());

            switch (bits_per_pixel) {
            case 8: processorCtx = realtime8_c; break;
            case 10: processorCtx = realtime10_c; break;
            case 12: processorCtx = realtime12_c; break;
            case 14: processorCtx = realtime14_c; break;
            case 16: processorCtx = realtime16_c; break;
            case 32: processorCtx = realtime32_c; break;
            }
            continue;
          }

          // 4D lut only 8 bits
          processor = lut_c;

          if (customExpressionDefined) {
              luts[i].used = true;
              luts[i].ptr = calculateLut(parser.getExpression(), 8);
          }
          else {
              if (luts[4].ptr == nullptr) {
                  luts[4].used = true;
                  luts[4].ptr = calculateLut(parser.getExpression(), 8);
              }
              luts[i].ptr = luts[4].ptr;
          }
      }
   }

   ~Lutxyza()
   {
       for (int i = 0; i < 4+1; ++i) {
           if (luts[i].used) {
               delete[] luts[i].ptr;
           }
       }
       for (int i = 0; i < 4; i++) {
         delete parsed_expressions[i];
       }
   }

   InputConfiguration &input_configuration() const { return InPlaceFourFrame(); }
	 InputConfiguration &input_configuration_cuda() const { return FourFrame(); }

   static Signature filter_signature()
   {
      Signature signature("kmt_lutxyza");

      signature.add(Parameter(TYPE_CLIP, "", false));
      signature.add(Parameter(TYPE_CLIP, "", false));
      signature.add(Parameter(TYPE_CLIP, "", false));
      signature.add(Parameter(TYPE_CLIP, "", false));
      signature.add(Parameter(String("x"), "expr", false));
      signature.add(Parameter(String("x"), "yExpr", false));
      signature.add(Parameter(String("x"), "uExpr", false));
      signature.add(Parameter(String("x"), "vExpr", false));

      add_defaults( signature );

      signature.add(Parameter(true, "realtime", false)); // 4D lut: default realtime calc.
      signature.add(Parameter(String("x"), "aExpr", false));
      return signature;
   }
};

} } } } }

#endif