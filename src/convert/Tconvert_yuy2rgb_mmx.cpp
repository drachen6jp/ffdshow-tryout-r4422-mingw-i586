#include "stdafx.h"
#include "Tconfig.h"
#include "Tconvert.h"
#include "convert_yuy2.h"
#include "Tconvert_simd.h"

TconvertYUY2toRGBFn TconvertYUY2toRGB_get(int uyvy, int rgb32, int pcRGB)
{
    if (uyvy) {
        if (rgb32) {
            return pcRGB ? Tmmx_ConvertYUY2toRGB<1,1,1>::mmx_ConvertYUY2toRGB : Tmmx_ConvertYUY2toRGB<1,1,0>::mmx_ConvertYUY2toRGB;
        } else {
            return pcRGB ? Tmmx_ConvertYUY2toRGB<1,0,1>::mmx_ConvertYUY2toRGB : Tmmx_ConvertYUY2toRGB<1,0,0>::mmx_ConvertYUY2toRGB;
        }
    } else {
        if (rgb32) {
            return pcRGB ? Tmmx_ConvertYUY2toRGB<0,1,1>::mmx_ConvertYUY2toRGB : Tmmx_ConvertYUY2toRGB<0,1,0>::mmx_ConvertYUY2toRGB;
        } else {
            return pcRGB ? Tmmx_ConvertYUY2toRGB<0,0,1>::mmx_ConvertYUY2toRGB : Tmmx_ConvertYUY2toRGB<0,0,0>::mmx_ConvertYUY2toRGB;
        }
    }
}

TconvertRGBtoYUY2Fn TconvertRGBtoYUY2_get(int rgb24)
{
    return rgb24 ? Tmmx_ConvertRGBtoYUY2<true,false>::mmx_ConvertRGBtoYUY2 : Tmmx_ConvertRGBtoYUY2<false,false>::mmx_ConvertRGBtoYUY2;
}
