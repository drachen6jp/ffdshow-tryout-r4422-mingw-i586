#include "stdafx.h"
#include "Tconfig.h"
#include "Tconvert.h"
#include "convert_yuy2.h"

template<int uyvy,int rgb32,int pcRGB>
void Tmmx_ConvertYUY2toRGB<uyvy,rgb32,pcRGB>::sse2_ConvertYUY2toRGB(const BYTE* src,
                                                                     BYTE* dst,
                                                                     const BYTE* src_end,
                                                                     stride_t src_pitch,
                                                                     stride_t dst_pitch,
                                                                     int row_size,
                                                                     const unsigned char* Imatrix)
{
    for (;src!=src_end;src+=src_pitch,dst+=dst_pitch)
     {
      const unsigned char *srcLn=src,*srcLnEnd = srcLn + row_size - 16;
      unsigned char *dstLn = dst;
      while (YUV2RGB_INNER_LOOP_SSE2<0>(srcLn,srcLnEnd,dstLn,Imatrix))
       ;
      srcLn = srcLnEnd;
      dstLn = dst + row_size + (rgb32 ? row_size - 32: (row_size >>1) - 24);
      YUV2RGB_INNER_LOOP_SSE2<1>(srcLn,srcLnEnd,dstLn,Imatrix);
     }
}

template void Tmmx_ConvertYUY2toRGB<0,0,0>::sse2_ConvertYUY2toRGB(const BYTE*, BYTE*, const BYTE*, stride_t, stride_t, int, const unsigned char*);
template void Tmmx_ConvertYUY2toRGB<0,0,1>::sse2_ConvertYUY2toRGB(const BYTE*, BYTE*, const BYTE*, stride_t, stride_t, int, const unsigned char*);
template void Tmmx_ConvertYUY2toRGB<0,1,0>::sse2_ConvertYUY2toRGB(const BYTE*, BYTE*, const BYTE*, stride_t, stride_t, int, const unsigned char*);
template void Tmmx_ConvertYUY2toRGB<0,1,1>::sse2_ConvertYUY2toRGB(const BYTE*, BYTE*, const BYTE*, stride_t, stride_t, int, const unsigned char*);
template void Tmmx_ConvertYUY2toRGB<1,0,0>::sse2_ConvertYUY2toRGB(const BYTE*, BYTE*, const BYTE*, stride_t, stride_t, int, const unsigned char*);
template void Tmmx_ConvertYUY2toRGB<1,0,1>::sse2_ConvertYUY2toRGB(const BYTE*, BYTE*, const BYTE*, stride_t, stride_t, int, const unsigned char*);
template void Tmmx_ConvertYUY2toRGB<1,1,0>::sse2_ConvertYUY2toRGB(const BYTE*, BYTE*, const BYTE*, stride_t, stride_t, int, const unsigned char*);
template void Tmmx_ConvertYUY2toRGB<1,1,1>::sse2_ConvertYUY2toRGB(const BYTE*, BYTE*, const BYTE*, stride_t, stride_t, int, const unsigned char*);
