#include "expand.h"
#include "../functions16.h"

using namespace Filtering;

typedef Word (local_maximum_f)(Word a1, Word a2, Word a3, Word a4, Word a5, Word a6, Word a7, Word a8, Word a9);

static inline Word maximum_square(Word a1, Word a2, Word a3, Word a4, Word a5, Word a6, Word a7, Word a8, Word a9)
{
    Word nMax = a1;
    if ( a2 > nMax ) nMax = a2;
    if ( a3 > nMax ) nMax = a3;
    if ( a4 > nMax ) nMax = a4;
    if ( a5 > nMax ) nMax = a5;
    if ( a6 > nMax ) nMax = a6;
    if ( a7 > nMax ) nMax = a7;
    if ( a8 > nMax ) nMax = a8;
    if ( a9 > nMax ) nMax = a9;
    return nMax;
}

static inline Word maximum_horizontal(Word a1, Word a2, Word a3, Word a4, Word a5, Word a6, Word a7, Word a8, Word a9)
{
    Word nMax = a4;

    UNUSED(a1); UNUSED(a2); UNUSED(a3); UNUSED(a7); UNUSED(a8); UNUSED(a9); 

    if ( a5 > nMax ) nMax = a5;
    if ( a6 > nMax ) nMax = a6;
    return nMax;
}

static inline Word maximum_vertical(Word a1, Word a2, Word a3, Word a4, Word a5, Word a6, Word a7, Word a8, Word a9)
{
    Word nMax = a2;

    UNUSED(a1); UNUSED(a3); UNUSED(a4); UNUSED(a6); UNUSED(a7); UNUSED(a9); 

    if ( a5 > nMax ) nMax = a5;
    if ( a8 > nMax ) nMax = a8;
    return nMax;
}

static inline Word maximum_both(Word a1, Word a2, Word a3, Word a4, Word a5, Word a6, Word a7, Word a8, Word a9)
{
    Word nMax = a2;

    UNUSED(a1); UNUSED(a3); UNUSED(a7); UNUSED(a9); 

    if ( a4 > nMax ) nMax = a4;
    if ( a5 > nMax ) nMax = a5;
    if ( a6 > nMax ) nMax = a6;
    if ( a8 > nMax ) nMax = a8;
    return nMax;
}

template<local_maximum_f Maximum>
static inline Word maximumThresholded(Word a1, Word a2, Word a3, Word a4, Word a5, Word a6, Word a7, Word a8, Word a9, int nMaxDeviation)
{
    int nMaximum = Maximum(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    if ( nMaximum - a5 > nMaxDeviation ) nMaximum = a5 + nMaxDeviation;
    return static_cast<Word>(nMaximum);
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic { namespace Expand {

    class NewValue16 {
        int nMax;
        int nMaxDeviation;
        Word nValue;
    public:
        NewValue16(Word nValue, int nMaxDeviation) : nMax(-1), nMaxDeviation(nMaxDeviation), nValue(nValue) { }
        void add(Word _nValue) { if ( _nValue > nMax ) nMax = _nValue; }
        Word finalize() const { return static_cast<Word>(nMax < 0 ? nValue : (nMax - nValue > nMaxDeviation ? nValue + nMaxDeviation : nMax)); }
    };


StackedProcessor *expand_square_stacked_c = &MorphologicProcessor<Byte>::generic_16_c<
    process_line_morpho_stacked_c<Border::Left, maximumThresholded<::maximum_square>>,
    process_line_morpho_stacked_c<Border::None, maximumThresholded<::maximum_square>>,
    process_line_morpho_stacked_c<Border::Right, maximumThresholded<::maximum_square>>
    >;

StackedProcessor *expand_horizontal_stacked_c = &MorphologicProcessor<Byte>::generic_16_c<
    process_line_morpho_stacked_c<Border::Left, maximumThresholded<::maximum_horizontal>>,
    process_line_morpho_stacked_c<Border::None, maximumThresholded<::maximum_horizontal>>,
    process_line_morpho_stacked_c<Border::Right, maximumThresholded<::maximum_horizontal>>
    >;

StackedProcessor *expand_vertical_stacked_c = &MorphologicProcessor<Byte>::generic_16_c<
    process_line_morpho_stacked_c<Border::Left, maximumThresholded<::maximum_vertical>>,
    process_line_morpho_stacked_c<Border::None, maximumThresholded<::maximum_vertical>>,
    process_line_morpho_stacked_c<Border::Right, maximumThresholded<::maximum_vertical>>
    >;

StackedProcessor *expand_both_stacked_c = &MorphologicProcessor<Byte>::generic_16_c<
    process_line_morpho_stacked_c<Border::Left, maximumThresholded<::maximum_both>>,
    process_line_morpho_stacked_c<Border::None, maximumThresholded<::maximum_both>>,
    process_line_morpho_stacked_c<Border::Right, maximumThresholded<::maximum_both>>
    >;

Processor16 *expand_square_native_c = &MorphologicProcessor<Word>::generic_16_c<
    process_line_morpho_native_c<Border::Left, maximumThresholded<::maximum_square>>,
    process_line_morpho_native_c<Border::None, maximumThresholded<::maximum_square>>,
    process_line_morpho_native_c<Border::Right, maximumThresholded<::maximum_square>>
    >;

Processor16 *expand_horizontal_native_c = &MorphologicProcessor<Word>::generic_16_c<
    process_line_morpho_native_c<Border::Left, maximumThresholded<::maximum_horizontal>>,
    process_line_morpho_native_c<Border::None, maximumThresholded<::maximum_horizontal>>,
    process_line_morpho_native_c<Border::Right, maximumThresholded<::maximum_horizontal>>
    >;

Processor16 *expand_vertical_native_c = &MorphologicProcessor<Word>::generic_16_c<
    process_line_morpho_native_c<Border::Left, maximumThresholded<::maximum_vertical>>,
    process_line_morpho_native_c<Border::None, maximumThresholded<::maximum_vertical>>,
    process_line_morpho_native_c<Border::Right, maximumThresholded<::maximum_vertical>>
    >;

Processor16 *expand_both_native_c = &MorphologicProcessor<Word>::generic_16_c<
    process_line_morpho_native_c<Border::Left, maximumThresholded<::maximum_both>>,
    process_line_morpho_native_c<Border::None, maximumThresholded<::maximum_both>>,
    process_line_morpho_native_c<Border::Right, maximumThresholded<::maximum_both>>
    >;


StackedProcessor *expand_custom_stacked_c       = &generic_custom_stacked_c<NewValue16>;
Processor16 *expand_custom_native_c   = &generic_custom_native_c<NewValue16>;

} } } } }