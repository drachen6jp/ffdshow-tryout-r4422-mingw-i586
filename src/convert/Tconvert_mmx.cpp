#include "stdafx.h"
#include "Tconvert.h"
#include "convert_yv12.h"
#include "Tconvert_simd.h"

TconvertYV12toYUY2Fn TconvertYV12_mmx_yv12_to_yuy2(int interlaced)
{
    return interlaced ? TconvertYV12<Tmmx>::yv12_i_to_yuy2 : TconvertYV12<Tmmx>::yv12_to_yuy2;
}

TconvertYUY2toYV12Fn TconvertYV12_mmx_yuy2_to_yv12(int interlaced)
{
    return interlaced ? TconvertYV12<Tmmx>::yuy2_i_to_yv12 : TconvertYV12<Tmmx>::yuy2_to_yv12;
}
