// mpconfig.h - MinGW 用シム (mingw-w64 にはこのヘッダが無い)
// DirectShow (Platform SDK) の IMixerPinConfig / IMixerPinConfig2 の宣言。
// メソッドの並びは SDK の vtable 順 (ffdshow の実装クラスと一致)。
// COLORKEY は baseclasses の strmif.h 側で定義済みなのでここには置かない。
// IID の実体は src/baseclasses/strmiids.cpp に既にある。
#pragma once
#include <strmif.h>
#include <ddraw.h>

typedef enum tagAM_ASPECT_RATIO_MODE {
    AM_ARMODE_STRETCHED = 0,
    AM_ARMODE_LETTER_BOX = 1,
    AM_ARMODE_CROP = 2,
    AM_ARMODE_STRETCHED_AS_PRIMARY = 3
} AM_ASPECT_RATIO_MODE;

EXTERN_C const IID IID_IMixerPinConfig;
EXTERN_C const IID IID_IMixerPinConfig2;

struct IMixerPinConfig : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE SetRelativePosition(DWORD dwLeft, DWORD dwTop, DWORD dwRight, DWORD dwBottom) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetRelativePosition(DWORD *pdwLeft, DWORD *pdwTop, DWORD *pdwRight, DWORD *pdwBottom) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetZOrder(DWORD dwZOrder) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetZOrder(DWORD *pdwZOrder) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetColorKey(COLORKEY *pColorKey) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetColorKey(COLORKEY *pColorKey, DWORD *pColor) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetBlendingParameter(DWORD dwBlendingParameter) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetBlendingParameter(DWORD *pdwBlendingParameter) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetAspectRatioMode(AM_ASPECT_RATIO_MODE amAspectRatioMode) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetAspectRatioMode(AM_ASPECT_RATIO_MODE *pamAspectRatioMode) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetStreamTransparent(BOOL bStreamTransparent) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetStreamTransparent(BOOL *pbStreamTransparent) = 0;
};

struct IMixerPinConfig2 : public IMixerPinConfig {
    virtual HRESULT STDMETHODCALLTYPE GetOverlaySurfaceColorControls(DDCOLORCONTROL *pColorControl) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetOverlaySurfaceColorControls(DDCOLORCONTROL *pColorControl) = 0;
};
