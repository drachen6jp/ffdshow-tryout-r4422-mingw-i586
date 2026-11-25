// uuidof_shim.c - define the four __mingw_uuidof<T> specializations the
// consumer TUs reference, under their exact mangled names (taken from
// nm output of the referencing objects). The C++ functions return
// const GUID&; at the ABI level that is a pointer return, so plain C
// functions with asm labels satisfy them. __CRT_UUID_DECL plus an
// address-taking array in guids.cpp did not emit these under
// mingw-w64 14 / GCC 14, hence this direct approach.
#include <windows.h>

static const GUID guid_IAMVideoAccelerator =
    {0x256a6a22, 0xfbad, 0x11d1, {0x82, 0xbf, 0x00, 0xa0, 0xc9, 0x69, 0x6c, 0x8f}};
static const GUID guid_IAMVideoAcceleratorNotify =
    {0x256a6a21, 0xfbad, 0x11d1, {0x82, 0xbf, 0x00, 0xa0, 0xc9, 0x69, 0x6c, 0x8f}};
static const GUID guid_IMemAllocator =
    {0x56a8689c, 0x0ad4, 0x11ce, {0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}};
static const GUID guid_IMediaSample2 =
    {0x36b73884, 0xc2c8, 0x11cf, {0x8b, 0x46, 0x00, 0x80, 0x5f, 0x6c, 0xef, 0x60}};

const GUID *uuidof_IAMVideoAccelerator(void)
__asm__("__Z14__mingw_uuidofI19IAMVideoAcceleratorERK5_GUIDv");
const GUID *uuidof_IAMVideoAccelerator(void)
{
    return &guid_IAMVideoAccelerator;
}

const GUID *uuidof_IAMVideoAcceleratorNotify(void)
__asm__("__Z14__mingw_uuidofI25IAMVideoAcceleratorNotifyERK5_GUIDv");
const GUID *uuidof_IAMVideoAcceleratorNotify(void)
{
    return &guid_IAMVideoAcceleratorNotify;
}

const GUID *uuidof_IMemAllocator(void)
__asm__("__Z14__mingw_uuidofI13IMemAllocatorERK5_GUIDv");
const GUID *uuidof_IMemAllocator(void)
{
    return &guid_IMemAllocator;
}

const GUID *uuidof_IMediaSample2(void)
__asm__("__Z14__mingw_uuidofI13IMediaSample2ERK5_GUIDv");
const GUID *uuidof_IMediaSample2(void)
{
    return &guid_IMediaSample2;
}
