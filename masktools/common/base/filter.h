#ifndef __Mt_Base_H__
#define __Mt_Base_H__

#include "EnvCommon.h"
#include "../../../common/functions/functions.h"
#include "../../../common/params/params.h"
#include "../../../common/parser/parser.h"
#include "../../../common/utils/utils.h"
#include "../../common/params/params.h"
#include "../../common/clip/inputconfig.h"

namespace Filtering { namespace MaskTools { 

enum class FilterProcessingType {
    INPLACE,
    CHILD
};

class Filter {

protected:
    ClipArray childs;
    Parameters parameters;

    int nWidth, nHeight;
    Colorspace C;

    String error;
    CpuFlags flags;
    Operator operators[4];

    bool inPlace_;
    int nXOffset, nYOffset, nXOffsetUV, nYOffsetUV;
    int nCoreWidth, nCoreHeight, nCoreWidthUV, nCoreHeightUV;

   virtual void process(int n, const Plane<Byte> &dst, int nPlane, const Frame<const Byte> frames[4], const Constraint constraints[4], PNeoEnv env) = 0;

   static Signature &add_defaults(Signature &signature)
   {
      signature.add( Parameter( 3.0f, "Y", true) );
      signature.add( Parameter( 1.0f, "U", true) );
      signature.add( Parameter( 1.0f, "V", true) );
      signature.add( Parameter( Value( String( "" ) ), "chroma", false));
      signature.add( Parameter( 0, "offX", false) );
      signature.add( Parameter( 0, "offY", false) );
      signature.add( Parameter( -1, "w", false) );
      signature.add( Parameter( -1, "h", false) );
      signature.add( Parameter( true, "sse2", false) );
      signature.add( Parameter( true, "sse3", false) );
      signature.add( Parameter( true, "ssse3", false) );
      signature.add( Parameter( true, "sse4", false) );
      signature.add( Parameter( true, "avx", false));
      signature.add( Parameter( true, "avx2", false));
      signature.add( Parameter( 1.0f, "A", true)); // put it at the end, don't change original parameter order
      signature.add(Parameter(Value(String("")), "alpha", false)); // same function as "chroma", for alpha plane
      signature.add(Parameter(String("i8"), "paramscale", false)); // like in expressions + none

      return signature;
   }

   // general helper function
   static bool ScaleParam(String scalemode, float input, int clip_bits_per_pixel, float &scaled_f, int &scaled_i, bool fullscale, bool allowNegative)
   {
     int param_bits_per_pixel;
     int max_pixel_value = (1 << clip_bits_per_pixel) - 1;
     if (scalemode == "i8") param_bits_per_pixel = 8;
     else if (scalemode == "i10") param_bits_per_pixel = 10;
     else if (scalemode == "i12") param_bits_per_pixel = 12;
     else if (scalemode == "i14") param_bits_per_pixel = 14;
     else if (scalemode == "i16") param_bits_per_pixel = 16;
     else if (scalemode == "f32") param_bits_per_pixel = 32;
     else if ((scalemode == "none") || scalemode == "") {
       scaled_f = input;
       if (clip_bits_per_pixel != 32) {
         if(scaled_f >= 0 || !allowNegative)
           scaled_i = max(min(int(scaled_f), max_pixel_value), 0);
         else
           scaled_i = -max(min(int(-scaled_f), max_pixel_value), 0);
       }
       return true;
     }
     else {
       return false; // invalid parameter
     }

     // identical bit-depth, return original
     if (param_bits_per_pixel == clip_bits_per_pixel) {
       scaled_f = input;
       if (clip_bits_per_pixel != 32) {
         if(scaled_f >= 0 || !allowNegative)
           scaled_i = max(min(int(scaled_f), max_pixel_value), 0);
         else
           scaled_i = -max(min(int(-scaled_f), max_pixel_value), 0);
       }
       return true;
     }

     // param_bits_per_pixel -> clip_bits_per_pixel
     if (fullscale) {
       // stretch mode
       if (param_bits_per_pixel >= 8 && param_bits_per_pixel <= 16) {
         input = input / ((1 << param_bits_per_pixel) - 1); // convert to 0..1 range
       }
       // input is now scaled_f to 0..1
       if (clip_bits_per_pixel == 32)
         scaled_f = input; // as-is
       else
         scaled_f = input * ((1 << clip_bits_per_pixel) - 1); // scaling up to video bit depth
     }
     else {
       // bit shift mode, but preserves if max is specified
       if (clip_bits_per_pixel == 32) { // convert to float
         scaled_f = input / ((1 << param_bits_per_pixel) - 1);
       }
       else if (param_bits_per_pixel == 32) {
         scaled_f = input * ((1 << clip_bits_per_pixel) - 1);
       }
       else {
         bool isNegativeInput = input < 0;
         input = std::abs(input);
         float input_range_max = (float)((1 << param_bits_per_pixel) - 1);
         if (abs(input_range_max - input) < 0.000001) {
           scaled_f = (float)((1 << clip_bits_per_pixel) - 1); // keep max range
         }
         else {
           if (param_bits_per_pixel > clip_bits_per_pixel)
             scaled_f = input / ((1 << (param_bits_per_pixel - clip_bits_per_pixel))); // scale down by diff bits
           else
             scaled_f = input * ((1 << (clip_bits_per_pixel - param_bits_per_pixel))); // scale up by diff bits
         }
         if (isNegativeInput)
           scaled_f = -scaled_f;
       }
     }

     if (clip_bits_per_pixel != 32) {
       if (scaled_f >= 0 || !allowNegative)
         scaled_i = max(min(int(scaled_f), max_pixel_value), 0);
       else
         scaled_i = -max(min(int(-scaled_f), max_pixel_value), 0);
     }

     return true;
   }


public:

    Filter(const Parameters &parameters, FilterProcessingType processingType, CpuFlags _flags) :
        parameters(parameters),
        flags(_flags),
        inPlace_(processingType == FilterProcessingType::INPLACE),
        nXOffset(parameters["offx"].toInt()),
        nYOffset(parameters["offy"].toInt()),
        nCoreWidth(parameters["w"].toInt()),
        nCoreHeight((parameters["stacked"].is_defined() && parameters["stacked"].toBool() && parameters["h"].toInt()>=0) ? (2 * parameters["h"].toInt()) : parameters["h"].toInt())
    {
        for (auto &param: parameters) {
            if (param.getType() == TYPE_CLIP) {
                childs.push_back(param.getValue().toClip());
            }
        }

        assert(!childs.empty());

        nWidth = childs[0]->width();
        nHeight = childs[0]->height();
        C = childs[0]->colorspace();

        operators[0] = Operator((float)parameters["Y"].toFloat()); // prepare, float for memset
        operators[1] = Operator((float)parameters["U"].toFloat());
        operators[2] = Operator((float)parameters["V"].toFloat());
        operators[3] = Operator((float)parameters["A"].toFloat()); // new from 2.2.7: alpha

        if (nXOffset < 0 || nXOffset > nWidth) nXOffset = 0;
        if (nYOffset < 0 || nYOffset > nHeight) nYOffset = 0;
        if (nXOffset + nCoreWidth  > nWidth  || nCoreWidth  < 0) nCoreWidth = nWidth - nXOffset;
        if (nYOffset + nCoreHeight > nHeight || nCoreHeight < 0) nCoreHeight = nHeight - nYOffset;

        if (parameters["chroma"].is_defined())
        {
            /* overrides chroma channel operators according to the "chroma" string */
            String chroma = parameters["chroma"].toString();

            if (chroma == "process")
                operators[1] = operators[2] = PROCESS;
            else if (chroma == "copy")
                operators[1] = operators[2] = COPY;
            else if (chroma == "copy first")
                operators[1] = operators[2] = COPY;
            else if (chroma == "copy second")
                operators[1] = operators[2] = COPY_SECOND;
            else if (chroma == "copy third")
                operators[1] = operators[2] = COPY_THIRD;
            else if (chroma == "copy fourth")
                operators[1] = operators[2] = COPY_FOURTH;
            else if (chroma == "none") // 2.2.9
                operators[1] = operators[2] = NONE;
            else if (chroma == "ignore") // 2.2.9
                operators[1] = operators[2] = NONE;
            else {
              try {
                size_t i(0);
                float f = std::stof(chroma.c_str(), &i);
                if (f > 0) {
                  error = "filler value parameter should be <=0 for chroma";
                  return;
                }
                f = -f;
                operators[1] = operators[2] = Operator(MEMSET, f); // atoi(chroma.c_str())
              }
              catch (...)
              {
                error = "invalid parameter value for chroma";
                return;
              }
            }
        }

        if (parameters["alpha"].is_defined())
        {
          /* overrides alpha channel operators according to the "alpha" string v2.2.7- */
          String alpha = parameters["alpha"].toString();

          if (alpha == "process")
            operators[3] = PROCESS;
          else if (alpha == "copy")
            operators[3] = COPY;
          else if (alpha == "copy first")
            operators[3] = COPY;
          else if (alpha == "copy second")
            operators[3] = COPY_SECOND;
          else if (alpha == "copy third")
            operators[3] = COPY_THIRD;
          else if (alpha == "copy fourth")
            operators[3] = COPY_FOURTH;
          else if (alpha == "none") // 2.2.9
            operators[3] = NONE;
          else if (alpha == "ignore") // 2.2.9
            operators[3] = NONE;
          else {
            try {
              size_t i(0);
              float f = std::stof(alpha.c_str(), &i);
              if (f > 0) {
                error = "filler value parameter should be <=0 for alpha";
                return;
              }
              f = -f;
              operators[3] = Operator(MEMSET, f); // atoi(chroma.c_str())
            }
            catch (...)
            {
              error = "invalid parameter value for alpha";
              return;
            }
          }
        }

        for (int i = 0; i < 4; i++) {
          if (operators[i].getMode() != MEMSET) continue;
          float op_f = operators[i].value_f();
          int op;
          String scalemode = parameters["paramscale"].toString();
          bool fullscale = planes_isRGB[C];
          int bits_per_pixel = bit_depths[C];
          if (!ScaleParam(scalemode, op_f, bits_per_pixel, op_f, op, fullscale, false))
          {
            error = "invalid parameter: paramscale. Use i8, i10, i12, i14, i16, f32 for scale or none/empty to disable scaling";
            return;
          }
          operators[i] = -op_f; // for non 1-6: automatic MEMSET
        }

        /* checks the operators */
        for (int i = 0; i < 4; i++)
        {
            if (operators[i] == COPY_FOURTH && childs.size() < 4)
                operators[i] = COPY_THIRD;
            if (operators[i] == COPY_THIRD && childs.size() < 3)
                operators[i] = COPY_SECOND;
            if (operators[i] == COPY_SECOND && childs.size() < 2)
                operators[i] = COPY;
        }

        if (is_in_place())
        {
            /* in place filters copy differently */
            for (int i = 0; i < 4; i++)
            {
                switch (operators[i].getMode())
                {
                case COPY: operators[i] = NONE; break;
                case COPY_SECOND: operators[i] = COPY; break;
                case COPY_THIRD: operators[i] = COPY_SECOND; break;
                case COPY_FOURTH: operators[i] = COPY_THIRD; break;
                default: break;
                }
            }
        }

        /* effective modes */
        print(LOG_DEBUG, "modes : %i %i %i %i\n", operators[0].getMode(), operators[1].getMode(), operators[2].getMode(), operators[3].getMode());

        /* cpu flags */
        if (!parameters["sse2"].toBool()) flags &= ~CPU_SSE2;
        if (!parameters["sse3"].toBool()) flags &= ~CPU_SSE3;
        if (!parameters["ssse3"].toBool()) flags &= ~CPU_SSSE3;
        if (!parameters["sse4"].toBool()) {
            flags &= ~CPU_SSE4_1;
            flags &= ~CPU_SSE4_2;
        }
        if (!parameters["avx"].toBool()) flags &= ~CPU_AVX;
        if (!parameters["avx2"].toBool()) flags &= ~CPU_AVX2;

        print(LOG_DEBUG, "using cpu flags : 0x%x\n", flags);

        /* chroma offsets and box */
        if (C != COLORSPACE_Y8 && C != COLORSPACE_Y10 && C != COLORSPACE_Y12 && C != COLORSPACE_Y14 && C != COLORSPACE_Y16 && C != COLORSPACE_Y32 && C != COLORSPACE_NONE)
        {
          // also for planar RGB, naming is a bit confusing yet
            nXOffsetUV = nXOffset / width_ratios[1][C];
            nYOffsetUV = nYOffset / height_ratios[1][C];
            nCoreWidthUV = (nXOffset + nCoreWidth) / width_ratios[1][C] - nXOffsetUV;
            nCoreHeightUV = (nYOffset + nCoreHeight) / height_ratios[1][C] - nYOffsetUV;
        }

        /* effective offset */
        print(LOG_DEBUG, "offset : %i %i, width x height : %i x %i\n", nXOffset, nYOffset, nCoreWidth, nCoreHeight);

        /* check the colorspace */
        if (C == COLORSPACE_NONE)
            error = "masktools: unsupported colorspace, use Y8, YV12, YV16, YV24, YV411, greyscale, YUV(A)xxxP10-16/S, Planar RGB(A)";
    }

    void process_plane(int n, const Plane<Byte> &output_plane, int nPlane, const Constraint constraints[4], const Frame<const Byte> frames[4], PNeoEnv env)
    {
        bool isCUDA = ::IsCUDA(env);
        bool isStacked = parameters["stacked"].is_defined() && parameters["stacked"].toBool();

        auto copy_plane = isCUDA ? Functions::copy_plane_cuda : Functions::copy_plane;
        auto memset_plane = isCUDA ? Functions::memset_plane_cuda : Functions::memset_plane;
        auto memset_plane_16 = isCUDA ? Functions::memset_plane_16_cuda : Functions::memset_plane_16;
        auto memset_plane_32 = isCUDA ? Functions::memset_plane_32_cuda : Functions::memset_plane_32;

        switch (operators[nPlane].getMode())
        {
        case COPY:
          if (isStacked) {
            // in two parts, there may be sub-window
            // msb
            copy_plane(output_plane.data(), output_plane.pitch(),
              frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
              output_plane.width(), output_plane.height() / 2, env);
            // lsb
            copy_plane(output_plane.data() + output_plane.pitch() * output_plane.origheight() / 2, output_plane.pitch(),
              frames[0].plane(nPlane).data() + frames[0].plane(nPlane).pitch() * frames[0].plane(nPlane).origheight() / 2, frames[0].plane(nPlane).pitch(),
              output_plane.width(), output_plane.height() / 2, env);
          }
          else {
            copy_plane(output_plane.data(), output_plane.pitch(),
              frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
              output_plane.width()*output_plane.pixelsize(), output_plane.height(), env); // rowsize = width*pixelsize
          }
          break;
        case COPY_SECOND:
            if (isStacked) {
              // msb
              copy_plane(output_plane.data(), output_plane.pitch(),
                frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(),
                output_plane.width(), output_plane.height() / 2, env);
              // lsb
              copy_plane(output_plane.data() + output_plane.pitch() * output_plane.origheight() / 2, output_plane.pitch(),
                frames[1].plane(nPlane).data() + frames[1].plane(nPlane).pitch() * frames[1].plane(nPlane).origheight() / 2, frames[1].plane(nPlane).pitch(),
                output_plane.width(), output_plane.height() / 2, env);
            } else {
              copy_plane(output_plane.data(), output_plane.pitch(),
                frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(),
                output_plane.width()*output_plane.pixelsize(), output_plane.height(), env);
            }
            break;
        case COPY_THIRD:
            if (isStacked) {
              // msb
              copy_plane(output_plane.data(), output_plane.pitch(),
                frames[2].plane(nPlane).data(), frames[2].plane(nPlane).pitch(),
                output_plane.width(), output_plane.height() / 2, env);
              // lsb
              copy_plane(output_plane.data() + output_plane.pitch() * output_plane.origheight() / 2, output_plane.pitch(),
                frames[2].plane(nPlane).data() + frames[2].plane(nPlane).pitch() * frames[2].plane(nPlane).origheight() / 2, frames[2].plane(nPlane).pitch(),
                output_plane.width(), output_plane.height() / 2, env);
            }
            else {
              copy_plane(output_plane.data(), output_plane.pitch(),
                frames[2].plane(nPlane).data(), frames[2].plane(nPlane).pitch(),
                output_plane.width()*output_plane.pixelsize(), output_plane.height(), env);
            }
            break;
        case COPY_FOURTH:
          if (isStacked) {
            // msb
            copy_plane(output_plane.data(), output_plane.pitch(),
              frames[3].plane(nPlane).data(), frames[3].plane(nPlane).pitch(),
              output_plane.width(), output_plane.height() / 2, env);
            // lsb
            copy_plane(output_plane.data() + output_plane.pitch() * output_plane.origheight() / 2, output_plane.pitch(),
              frames[3].plane(nPlane).data() + frames[3].plane(nPlane).pitch() * frames[3].plane(nPlane).origheight() / 2, frames[3].plane(nPlane).pitch(),
              output_plane.width(), output_plane.height() / 2, env);
          }
          else {
            copy_plane(output_plane.data(), output_plane.pitch(),
              frames[3].plane(nPlane).data(), frames[3].plane(nPlane).pitch(),
              output_plane.width()*output_plane.pixelsize(), output_plane.height(), env);
          }
          break;
        case MEMSET:
            switch (output_plane.pixelsize()) {
            case 1:
              if (isStacked) {
                // in two parts, there may be sub-window
                Word val = static_cast<Word>(operators[nPlane].value());
                // msb
                memset_plane(output_plane.data(), output_plane.pitch(),
                  output_plane.width(), output_plane.height()/2,
                  static_cast<Byte>(val >> 8), env);
                // lsb
                memset_plane(output_plane.data() + output_plane.pitch() * output_plane.origheight() / 2, output_plane.pitch(),
                  output_plane.width(), output_plane.height() / 2,
                  static_cast<Byte>(val & 0xFF), env);
              } else {
                memset_plane(output_plane.data(), output_plane.pitch(),
                  output_plane.width(), output_plane.height(), // memset needs real width
                  static_cast<Byte>(operators[nPlane].value()), env);
              }
              break;
            case 2: // 16 bit
              memset_plane_16(output_plane.data(), output_plane.pitch(),
                output_plane.width(), output_plane.height(),
                static_cast<Word>(operators[nPlane].value()), env);
              break;
            case 4: // 32 bit/float
              memset_plane_32(output_plane.data(), output_plane.pitch(),
                output_plane.width(), output_plane.height(),
                static_cast<float>(operators[nPlane].value_f()), env);
              break;
            }
        break;
        case PROCESS:
            process(n, output_plane, nPlane, frames, constraints, env);
            break;
        case NONE:
        default: break;
        }
    }

    virtual Frame<Byte> get_frame(int n, const Frame<Byte> &output_frame, PNeoEnv env)
    {
        bool is_cuda = IsCUDA(env);
        auto& input_conf = is_cuda ? input_configuration_cuda() : input_configuration();
        Frame<Byte> output = output_frame.offset(nXOffset, nYOffset, nCoreWidth, nCoreHeight);

        Frame<const Byte> frames[4];
        Constraint constraints[4];

        int clipcount = int(input_conf.size());

        std::vector<PVideoFrame> tmp_videoframes(clipcount);

        for (int i = 0; i < clipcount; i++) {
            int childindex = input_conf[i].index();
            PClip &currentClip = childs[childindex];
            frames[i] = currentClip->get_const_frame(n + input_conf[i].offset(), tmp_videoframes[i], env)
                .offset(nXOffset, nYOffset, nCoreWidth, nCoreHeight);
        }

        for (int i = 0; i < plane_counts[C]; i++) {
            constraints[i] = Constraint(flags, output.plane(i));
        }

        for (int i = 0; i < clipcount; i++) {
          for (int j = 0; j < plane_counts[frames[i].colorspace()]; j++) {
                constraints[j] = Constraint(constraints[j], frames[i].plane(j));
            }
        }

        for (int i = 0; i < plane_counts[C]; i++) {
          process_plane(n, output.plane(i), i, constraints, frames, env);
        }

        return output_frame;
    }

    ClipArray &get_childs() { return childs; }

    String get_error() const { return error; }
    bool is_error() const { return !error.empty(); }
    bool is_in_place() const { return inPlace_; }
    bool is_in_place_cuda() const { return false; }

    virtual InputConfiguration &input_configuration() const = 0;
    virtual InputConfiguration &input_configuration_cuda() const { return input_configuration(); }
    virtual bool is_cuda_available() { return false; }

};

} } // namespace MaskTools, Filtering

#endif
