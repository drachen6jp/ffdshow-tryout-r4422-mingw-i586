#ifndef _TCONVERT_SIMD_H_
#define _TCONVERT_SIMD_H_

// Function pointer types matching the avisynth_yv12_to_yuy2 /
// avisynth_yuy2_to_yv12 members of Tconvert (see Tconvert.h).
// Include this header after Tconvert.h (BYTE and stride_t must
// already be defined).

typedef void (*TconvertYV12toYUY2Fn)(const BYTE *srcY, const BYTE *srcU, const BYTE *srcV, int src_rowsize, stride_t src_pitch, stride_t src_pitch_uv,
                                     BYTE *dst, stride_t dst_pitch,
                                     int height);
typedef void (*TconvertYUY2toYV12Fn)(const BYTE *src, int src_rowsize, stride_t src_pitch,
                                     BYTE *dstY, BYTE *dstU, BYTE *dstV, stride_t dst_pitch, stride_t dst_pitchUV,
                                     int height);

TconvertYV12toYUY2Fn TconvertYV12_sse2_yv12_to_yuy2(int interlaced);
TconvertYUY2toYV12Fn TconvertYV12_sse2_yuy2_to_yv12(int interlaced);
TconvertYV12toYUY2Fn TconvertYV12_mmxext_yv12_to_yuy2(int interlaced);
TconvertYUY2toYV12Fn TconvertYV12_mmxext_yuy2_to_yv12(int interlaced);
TconvertYV12toYUY2Fn TconvertYV12_mmx_yv12_to_yuy2(int interlaced);
TconvertYUY2toYV12Fn TconvertYV12_mmx_yuy2_to_yv12(int interlaced);
typedef void (*TconvertYUY2toRGBFn)(const BYTE *src, BYTE *dst, const BYTE *src_end, stride_t src_pitch, stride_t dst_pitch, int row_size, const unsigned char *Imatrix);
TconvertYUY2toRGBFn TconvertYUY2toRGB_get(int uyvy, int rgb32, int pcRGB);
typedef void (*TconvertRGBtoYUY2Fn)(const unsigned char *src, unsigned char *dst, stride_t src_pitch, stride_t dst_pitch, int w, int h, const Tmmx_ConvertRGBtoYUY2matrix *matrix);
TconvertRGBtoYUY2Fn TconvertRGBtoYUY2_get(int rgb24);
#endif
