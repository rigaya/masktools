#ifndef __Mt_MorphologicFilter_H__
#define __Mt_MorphologicFilter_H__

#include "../../common/base/filter.h"
#include "../../../common/parser/parser.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic {

typedef void (Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nMaxDeviation, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight);
typedef void (StackedProcessor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nMaxDeviation, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight, int nOrigHeight);
typedef void (Processor16)(Word *pDst, ptrdiff_t nDstPitch, const Word *pSrc, ptrdiff_t nSrcPitch, int nMaxDeviation, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight, int nOrigHeight);

class MorphologicFilter : public MaskTools::Filter
{
    int nMaxDeviations[3];
    int *coordinates_list;
    int coordinates_count;

    MorphologicFilter(const MorphologicFilter &filter);

    int bits_per_pixel;
    bool isStacked;

protected:

    ProcessorList<Processor> processors;
    ProcessorList<StackedProcessor> stackedProcessors;
    ProcessorList<Processor16> processors16;

    virtual void process(int n, const Plane<Byte> &dst, int nPlane, const ::Filtering::Frame<const Byte> frames[3], const Constraint constraints[3]) override
    {
      UNUSED(n);
      if (bits_per_pixel == 8) {
        processors.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
          frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
          nMaxDeviations[nPlane], coordinates_list, coordinates_count, dst.width(), dst.height());
      }
      else if (isStacked) {
        stackedProcessors.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
          frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
          nMaxDeviations[nPlane], coordinates_list, coordinates_count, dst.width(), dst.height() / 2, dst.origheight()); // stacked: /2
      }
      else if(bits_per_pixel <= 16) {
        processors16.best_processor(constraints[nPlane])(reinterpret_cast<Word*>((Byte*)dst.data()), dst.pitch() / 2, /* /2: word sized */
          reinterpret_cast<const Word*>((const Byte*)(frames[0].plane(nPlane).data())), frames[0].plane(nPlane).pitch() / 2,
          nMaxDeviations[nPlane], coordinates_list, coordinates_count, dst.width(), dst.height(), dst.origheight());
      }
    }

    void FillCoordinates(const String &coordinates)
    {
        auto coeffs = Parser::getDefaultParser().parse(coordinates, " (),;.").getExpression();
        coordinates_count = coeffs.size();
        coordinates_list = new int[coordinates_count];
        int i = 0;

        while (!coeffs.empty())
        {
            coordinates_list[i++] = int(coeffs.front().getValue(0, 0, 0));
            coeffs.pop_front();
        }
    }

public:
    MorphologicFilter(const Parameters &parameters) : MaskTools::Filter(parameters, FilterProcessingType::CHILD), coordinates_list(NULL), coordinates_count(0)
    {
      isStacked = parameters["stacked"].toBool();
      bits_per_pixel = bit_depths[C];

      if (isStacked && bits_per_pixel != 8) {
        error = "Stacked specified for a non-8 bit clip";
        return;
      }
      
      if (isStacked)
        bits_per_pixel = 16;

      if (bits_per_pixel == 32) {
        error = "32 bit float clip is not supported yet";
        return;
      }

      int max_pixel_value = (1 << bits_per_pixel) - 1;

      nMaxDeviations[0] = clip<int, int>(parameters["thY"].is_defined() ? parameters["thY"].toInt() : max_pixel_value, 0, max_pixel_value);
      nMaxDeviations[1] = 
      nMaxDeviations[2] = clip<int, int>(parameters["thC"].is_defined() ? parameters["thC"].toInt() : max_pixel_value, 0, max_pixel_value);

    }

    ~MorphologicFilter()
    {
        delete[] coordinates_list;
        coordinates_list = NULL;
    }

    InputConfiguration &input_configuration() const { return OneFrame(); }
};

} } } } // namespace Morphologic, Filters, MaskTools, Filtering

#endif