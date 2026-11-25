#include "stdafx.h"
#include <malloc.h>
#include "ffdshow_converters2.h"
#include "simd.h"
#include "Tconfig.h"
#include "ffdshow_converters2_templates.h"

template void TffdshowConverters2::convert_check_src_align<Tmmxext>(uint64_t, uint64_t, const uint8_t*, const uint8_t*, const uint8_t*, uint8_t*, uint8_t*, uint8_t*, int, int, stride_t, stride_t, stride_t, stride_t);
