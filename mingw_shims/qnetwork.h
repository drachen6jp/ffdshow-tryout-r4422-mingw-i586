// qnetwork.h - MinGW 用シム (mingw-w64 にはこのヘッダが無い)
// DirectShow SDK の IAMExtendedSeeking の宣言 (MSDN 上の所在は Qnetwork.h)。
// メソッドの並びは ffdshow の実装クラス TffdshowDec と一致する vtable 順。
// IID の実体は src/TffDecoder_reg.cpp の DEFINE_GUID 側にあるので、ここは extern 宣言のみ。
#pragma once
#include <oaidl.h>

typedef enum {
    AM_EXSEEK_CANSEEK = 0x01,
    AM_EXSEEK_CANSCAN = 0x02,
    AM_EXSEEK_MARKERSEEK = 0x04,
    AM_EXSEEK_SCANWITHOUTCLOCK = 0x08,
    AM_EXSEEK_NOSTANDARDREPAINT = 0x10,
    AM_EXSEEK_BUFFERING = 0x20,
    AM_EXSEEK_SENDS_VIDEOFRAMEREADY = 0x40
} AMExtendedSeekingCapabilities;

EXTERN_C const IID IID_IAMExtendedSeeking;

struct IAMExtendedSeeking : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE get_ExSeekCapabilities(long *pExCapabilities) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_MarkerCount(long *pMarkerCount) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_CurrentMarker(long *pCurrentMarker) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMarkerTime(long MarkerNum, double *pMarkerTime) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMarkerName(long MarkerNum, BSTR *pbstrMarkerName) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_PlaybackSpeed(double Speed) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_PlaybackSpeed(double *pSpeed) = 0;
};
