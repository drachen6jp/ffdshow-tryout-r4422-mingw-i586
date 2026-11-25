// IQuickSyncDecoder.h - MinGW 用シム
// 本来は別プロジェクト (Eric Gur 氏の IntelQuickSyncDecoder) を
// src/codecs/IntelQuickSyncDecoder/ に取り込むものだが、r4422 の
// スナップショットには vcxproj だけでソースが含まれていない。
// ffdshow.ax は DLL を実行時にロードするだけ (Tdll + createQuickSync /
// destroyQuickSync) なので、ここでは TvideoCodecQuickSync.cpp が使う
// 宣言だけを再現する。対象機 (Win2000 + Pentium/K6) では QuickSync の
// ハード自体が存在せず、check() の SSE4.1 ゲートでも弾かれる。
#pragma once

#define QS_DEC_DLL_NAME "IntelQuickSyncDecoder.dll"

class CMediaType;
struct IDirect3DDeviceManager9;
struct IMediaSample;

struct CQsConfig {
    bool bEnableSwEmulation;
};

struct QsFrameData {
    enum { I = 0, P = 1, B = 2 };
    unsigned char *y;
    unsigned char *u;
    DWORD dwStride;
    DWORD frameType;
    DWORD dwInterlaceFlags;
    RECT rcClip;
    BOOL bReadOnly;
    REFERENCE_TIME rtStart;
    REFERENCE_TIME rtStop;
    DWORD dwPictAspectRatioX;
    DWORD dwPictAspectRatioY;
};

struct IQuickSyncDecoder {
    virtual bool getOK() = 0;
    virtual HRESULT InitDecoder(const CMediaType *pMediaType, FOURCC fourCC) = 0;
    virtual HRESULT Decode(IMediaSample *pSample) = 0;
    virtual HRESULT TestMediaType(const CMediaType *pMediaType, FOURCC fourCC) = 0;
    virtual HRESULT BeginFlush() = 0;
    virtual HRESULT EndFlush() = 0;
    virtual HRESULT Flush(bool bDeliverSamples) = 0;
    virtual HRESULT OnSeek(REFERENCE_TIME rtSeek) = 0;
    virtual void GetConfig(CQsConfig *pConfig) = 0;
    virtual void SetConfig(CQsConfig *pConfig) = 0;
    virtual HRESULT SetD3DDeviceManager(IDirect3DDeviceManager9 *pDeviceManager) = 0;
    virtual void SetDeliverSurfaceCallback(void *obj, HRESULT (*callback)(void *obj, QsFrameData *frameData)) = 0;
};
