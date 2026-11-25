/*
 * Copyright (c) 2011 h.yamagata
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdafx.h"
#include <malloc.h>
#include "ffdshow_converters2.h"
#include "simd.h"
#include "Tconfig.h"  // CPU flags
class Tmmx;
class Tmmxext;
class Tsse2;

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#pragma warning (push)
#pragma warning (disable: 4799) // EMMS
#endif

void TffdshowConverters2::convert(
    uint64_t incsp,
    uint64_t outcsp,
    const uint8_t* srcY,
    const uint8_t* srcCb,
    const uint8_t* srcCr,
    uint8_t* dstY,
    uint8_t* dstCb,
    uint8_t* dstCr,
    int dx,
    int dy,
    stride_t stride_Y,
    stride_t stride_CbCr,
    stride_t stride_dstY,
    stride_t stride_dstCbCr)
{
    // clean up input args to helf alignment checking
    if (incsp == FF_CSP_NV12)
        srcCr = NULL;

    if (outcsp == FF_CSP_P016 || outcsp == FF_CSP_P010 || outcsp == FF_CSP_NV12 || outcsp == FF_CSP_P210 || outcsp == FF_CSP_P216)
        dstCr = NULL;

    if (outcsp == FF_CSP_AYUV || outcsp == FF_CSP_Y416 || outcsp == FF_CSP_RGB32)
        dstCb = dstCr = NULL;

#ifndef WIN64
    if (Tconfig::cpu_flags&FF_CPU_SSE2)
#endif
        convert_check_src_align<Tsse2>(incsp, outcsp, srcY, srcCb, srcCr, dstY, dstCb, dstCr, dx, dy, stride_Y, stride_CbCr, stride_dstY, stride_dstCbCr);
#ifndef WIN64
    else if (Tconfig::cpu_flags&FF_CPU_MMXEXT)
        convert_check_src_align<Tmmxext>(incsp, outcsp, srcY, srcCb, srcCr, dstY, dstCb, dstCr, dx, dy, stride_Y, stride_CbCr, stride_dstY, stride_dstCbCr);
    else
        convert_check_src_align<Tmmx>(incsp, outcsp, srcY, srcCb, srcCr, dstY, dstCb, dstCr, dx, dy, stride_Y, stride_CbCr, stride_dstY, stride_dstCbCr);
#endif
}

bool TffdshowConverters2::csp_sup_ffdshow_converter2(uint64_t incsp, uint64_t outcsp)
{
    switch (incsp) {
        case FF_CSP_420P:
            if (outcsp == FF_CSP_P016 || outcsp == FF_CSP_P010 || outcsp == FF_CSP_NV12)
                return true;
            break;
        case FF_CSP_420P10:
            if (outcsp == FF_CSP_P010 || outcsp == FF_CSP_P016)
                return true;
            break;
        case FF_CSP_444P10:
            if (outcsp == FF_CSP_Y416)
                return true;
            break;
        case FF_CSP_422P10:
            if (outcsp == FF_CSP_P210 || outcsp == FF_CSP_P216)
                return true;
            break;
        case FF_CSP_444P:
            if (outcsp == FF_CSP_AYUV)
                return true;
            break;
        case FF_CSP_NV12:
            if (outcsp == FF_CSP_P016 || outcsp == FF_CSP_P010 || outcsp == FF_CSP_420P)
                return true;
            break;
        case  FF_CSP_GBRP:
            if (outcsp == FF_CSP_RGB32)
                return true;
            break;
        default:
            return false;
    }
    return false;
}

