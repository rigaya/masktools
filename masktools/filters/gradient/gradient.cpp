#include "gradient.h"
#include <type_traits>

using namespace Filtering;

//template<typename pixel_t, typename sad_t>
//using Distortion sad_t (*)(const pixel_t *pSrc, ptrdiff_t nSrcPitch, const pixel_t *pRef, ptrdiff_t nRefPitch, int nPrecision);
typedef int (Distorsion)(const Byte *pSrc, ptrdiff_t nSrcPitch, const Byte *pRef, ptrdiff_t nRefPitch, int nPrecision);
typedef int (Distorsion16)(const Word *pSrc, ptrdiff_t nSrcPitch, const Word *pRef, ptrdiff_t nRefPitch, int nPrecision);
typedef float (Distorsion_f)(const Float *pSrc, ptrdiff_t nSrcPitch, const Float *pRef, ptrdiff_t nRefPitch, int nPrecision);

template<int nBlockSizeX, int nBlockSizeY> 
int sad(const Byte *pSrc, ptrdiff_t nSrcPitch, const Byte *pRef, ptrdiff_t nRefPitch, int nPrecision)
{
  // pitches are already bit_depth corrected
   int nSad = 0;

   for ( int y = 0; y < nBlockSizeY; y++, pSrc += nSrcPitch * nPrecision, pRef += nRefPitch * nPrecision )
      for ( int x = 0; x < nBlockSizeX; x++ )
         nSad += abs<int>( pSrc[x * nPrecision] - pRef[x * nPrecision] );

   return nSad;
}

template<int nBlockSizeX, int nBlockSizeY>
int sad16(const Word *pSrc, ptrdiff_t nSrcPitch, const Word *pRef, ptrdiff_t nRefPitch, int nPrecision)
{
  // pitches are already bit_depth corrected
  int nSad = 0;

  for (int y = 0; y < nBlockSizeY; y++, pSrc += nSrcPitch * nPrecision, pRef += nRefPitch * nPrecision)
    for (int x = 0; x < nBlockSizeX; x++)
      nSad += abs<int>(pSrc[x * nPrecision] - pRef[x * nPrecision]);

  return nSad;
}

template<int nBlockSizeX, int nBlockSizeY>
float sad32(const float *pSrc, ptrdiff_t nSrcPitch, const float *pRef, ptrdiff_t nRefPitch, int nPrecision)
{
  // pitches are already bit_depth corrected
  float nSad = 0;

  for (int y = 0; y < nBlockSizeY; y++, pSrc += nSrcPitch * nPrecision, pRef += nRefPitch * nPrecision)
    for (int x = 0; x < nBlockSizeX; x++)
      nSad += abs<float>(pSrc[x * nPrecision] - pRef[x * nPrecision]);

  return nSad;
}

template<Distorsion distorsion, int nBlockSizeX, int nBlockSizeY> 
void generic_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, const Byte *pRef, ptrdiff_t nRefPitch, int nWidth, int nHeight, int nX, int nY, int nMinimum, int nMaximum, int nPrecision)
{
 
   int x, y;

   pSrc += nX + nY * nSrcPitch;

   const int max_pixel_value = 255;

   for ( y = 0; y < nHeight - nBlockSizeY * nPrecision + nPrecision; y++, pRef += nRefPitch, pDst += nDstPitch )
   {
      for ( x = 0; x < nWidth - nBlockSizeX * nPrecision + nPrecision; x++ )
         pDst[x] = clip<Byte, int>( (distorsion( pSrc, nSrcPitch, &pRef[x], nRefPitch, nPrecision ) - nMinimum) * max_pixel_value / (nMaximum - nMinimum) );

      for ( ; x < nWidth; x++ )
         pDst[x] = pDst[nWidth - nBlockSizeX * nPrecision + nPrecision - 1];
   }
   for ( y = 0; y < nBlockSizeY * nPrecision - nPrecision; y++ )
      memcpy( &pDst[y * nDstPitch], pDst - nDstPitch, nWidth );
}

template<Distorsion16 distorsion, int nBlockSizeX, int nBlockSizeY, int bits_per_pixel>
void generic16_c(Word *pDst, ptrdiff_t nDstPitch, const Word *pSrc, ptrdiff_t nSrcPitch, const Word *pRef, ptrdiff_t nRefPitch, int nWidth, int nHeight, int nX, int nY, int nMinimum, int nMaximum, int nPrecision)
{
  nSrcPitch /= sizeof(Word);
  nDstPitch /= sizeof(Word);
  nRefPitch /= sizeof(Word);

  int x, y;

  pSrc += nX + nY * nSrcPitch;

  const int max_pixel_value = (1 << bits_per_pixel) - 1;

  for (y = 0; y < nHeight - nBlockSizeY * nPrecision + nPrecision; y++, pRef += nRefPitch, pDst += nDstPitch)
  {
    for (x = 0; x < nWidth - nBlockSizeX * nPrecision + nPrecision; x++)
      pDst[x] = clip<Word, int>((distorsion(pSrc, nSrcPitch, &pRef[x], nRefPitch, nPrecision) - nMinimum) * max_pixel_value / (nMaximum - nMinimum));

    for (; x < nWidth; x++)
      pDst[x] = pDst[nWidth - nBlockSizeX * nPrecision + nPrecision - 1];
  }
  for (y = 0; y < nBlockSizeY * nPrecision - nPrecision; y++)
    memcpy(&pDst[y * nDstPitch], pDst - nDstPitch, nWidth * sizeof(Word));
}

template<Distorsion_f distorsion, int nBlockSizeX, int nBlockSizeY>
void generic_f_c(Float *pDst, ptrdiff_t nDstPitch, const Float *pSrc, ptrdiff_t nSrcPitch, const Float *pRef, ptrdiff_t nRefPitch, int nWidth, int nHeight, int nX, int nY, Float nMinimum, Float nMaximum, int nPrecision)
{
  nSrcPitch /= sizeof(Float);
  nDstPitch /= sizeof(Float);
  nRefPitch /= sizeof(Float);

  int x, y;

  pSrc += nX + nY * nSrcPitch;

  for (y = 0; y < nHeight - nBlockSizeY * nPrecision + nPrecision; y++, pRef += nRefPitch, pDst += nDstPitch)
  {
    for (x = 0; x < nWidth - nBlockSizeX * nPrecision + nPrecision; x++)
      pDst[x] = (distorsion(pSrc, nSrcPitch, &pRef[x], nRefPitch, nPrecision) - nMinimum) / (nMaximum - nMinimum);

    for (; x < nWidth; x++)
      pDst[x] = pDst[nWidth - nBlockSizeX * nPrecision + nPrecision - 1];
  }
  for (y = 0; y < nBlockSizeY * nPrecision - nPrecision; y++)
    memcpy(&pDst[y * nDstPitch], pDst - nDstPitch, nWidth * sizeof(Float));
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Gradient {

Processor *sad_c  = &generic_c<sad<16, 16>, 16, 16>;
Processor16 *sad_10_c = &generic16_c<sad16<16, 16>, 16, 16, 10>;
Processor16 *sad_12_c = &generic16_c<sad16<16, 16>, 16, 16, 12>;
Processor16 *sad_14_c = &generic16_c<sad16<16, 16>, 16, 16, 14>;
Processor16 *sad_16_c = &generic16_c<sad16<16, 16>, 16, 16, 16>;
Processor32 *sad_32_c = &generic_f_c<sad32<16, 16>, 16, 16>;

} } } }
