// videoacc.h - MinGW 用シム (mingw-w64 にはこのヘッダが無い)
// DirectShow SDK の IAMVideoAccelerator (DXVA 1.0) の宣言。
// メソッドの並びは SDK の vtable 順、構造体レイアウトも SDK 通り。
// IID の実体は strmiids 側にあるので、ここは extern 宣言のみ。
#pragma once
#include <strmif.h>
#include <ddraw.h>

typedef struct _AMVAInternalMemInfo {
    DWORD dwScratchMemAlloc;
} AMVAInternalMemInfo, *LPAMVAInternalMemInfo;

typedef struct _AMVAUncompDataInfo {
    DWORD dwUncompWidth;
    DWORD dwUncompHeight;
    DDPIXELFORMAT ddUncompPixelFormat;
} AMVAUncompDataInfo, *LPAMVAUncompDataInfo;

typedef struct _AMVACompBufferInfo {
    DWORD dwNumCompBuffers;
    DWORD dwWidthCompBuffers;
    DWORD dwHeightCompBuffers;
    DWORD dwBytesToAllocate;
    LONG lStrideCompBuffers;
    DDPIXELFORMAT ddCompPixelFormat;
    DWORD dwGeneralMiscFlags;
} AMVACompBufferInfo, *LPAMVACompBufferInfo;

typedef struct _AMVABeginFrameInfo {
    DWORD dwDestSurfaceIndex;
    LPBYTE pInputData;
    DWORD dwSizeInputData;
    LPBYTE pOutputData;
    DWORD dwSizeOutputData;
} AMVABeginFrameInfo, *LPAMVABeginFrameInfo;

typedef struct _AMVAEndFrameInfo {
    DWORD dwSizeMiscData;
    LPBYTE pMiscData;
} AMVAEndFrameInfo, *LPAMVAEndFrameInfo;

typedef struct _AMVAUncompBufferInfo {
    DWORD dwMinNumSurfaces;
    DWORD dwMaxNumSurfaces;
    DDPIXELFORMAT ddUncompPixelFormat;
} AMVAUncompBufferInfo, *LPAMVAUncompBufferInfo;

typedef struct _AMVABUFFERINFO {
    DWORD dwTypeIndex;
    DWORD dwBufferIndex;
    DWORD dwDataOffset;
    DWORD dwDataSize;
} AMVABUFFERINFO, *LPAMVABUFFERINFO;

EXTERN_C const IID IID_IAMVideoAcceleratorNotify;

struct __declspec(uuid("256a6a21-fbad-11d1-82bf-00a0c9696c8f")) IAMVideoAcceleratorNotify : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetUncompSurfacesInfo(const GUID *pGuid, LPAMVAUncompBufferInfo pUncompBufferInfo) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetUncompSurfacesInfo(DWORD dwActualUncompSurfacesAllocated) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCreateVideoAcceleratorData(const GUID *pGuid, LPDWORD pdwSizeMiscData, LPVOID *ppMiscData) = 0;
};

EXTERN_C const IID IID_IAMVideoAccelerator;

struct __declspec(uuid("256a6a22-fbad-11d1-82bf-00a0c9696c8f")) IAMVideoAccelerator : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetVideoAcceleratorGUIDs(LPDWORD pdwNumGuidsSupported, LPGUID pGuidsSupported) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetUncompFormatsSupported(const GUID *pGuid, LPDWORD pdwNumFormatsSupported, LPDDPIXELFORMAT pFormatsSupported) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetInternalMemInfo(const GUID *pGuid, const AMVAUncompDataInfo *pamvaUncompDataInfo, AMVAInternalMemInfo *pamvaInternalMemInfo) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCompBufferInfo(const GUID *pGuid, const AMVAUncompDataInfo *pamvaUncompDataInfo, LPDWORD pdwNumTypesCompBuffers, AMVACompBufferInfo *pamvaCompBufferInfo) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetInternalCompBufferInfo(LPDWORD pdwNumTypesCompBuffers, AMVACompBufferInfo *pamvaCompBufferInfo) = 0;
    virtual HRESULT STDMETHODCALLTYPE BeginFrame(const AMVABeginFrameInfo *amvaBeginFrameInfo) = 0;
    virtual HRESULT STDMETHODCALLTYPE EndFrame(const AMVAEndFrameInfo *pEndFrameInfo) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetBuffer(DWORD dwTypeIndex, DWORD dwBufferIndex, BOOL bReadOnly, LPVOID *ppBuffer, LONG *lpStride) = 0;
    virtual HRESULT STDMETHODCALLTYPE ReleaseBuffer(DWORD dwTypeIndex, DWORD dwBufferIndex) = 0;
    virtual HRESULT STDMETHODCALLTYPE Execute(DWORD dwFunction, LPVOID lpPrivateInputData, DWORD cbPrivateInputData, LPVOID lpPrivateOutputData, DWORD cbPrivateOutputData, DWORD dwNumBuffers, const AMVABUFFERINFO *pamvaBufferInfo) = 0;
    virtual HRESULT STDMETHODCALLTYPE QueryRenderStatus(DWORD dwTypeIndex, DWORD dwBufferIndex, DWORD dwFlags) = 0;
    virtual HRESULT STDMETHODCALLTYPE DisplayFrame(DWORD dwFlipToIndex, IMediaSample *pMediaSample) = 0;
};
