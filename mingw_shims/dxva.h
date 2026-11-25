// dxva.h - MinGW 用シム
// mingw-w64 の dxva.h に無い DXVA 1.0 の型・定数・マクロだけを足す。
// 構造体レイアウトは SDK の dxva.h (MS Learn の各構造体ページと VC-1 仕様書) で確認した。
#pragma once
#include_next <dxva.h>

typedef DWORD DXVA_ConfigQueryOrReplyFunc, *LPDXVA_ConfigQueryOrReplyFunc;
#define DXVA_QUERYORREPLYFUNCFLAG_DECODER_PROBE_QUERY     0xFFFFF1
#define DXVA_QUERYORREPLYFUNCFLAG_DECODER_LOCK_QUERY      0xFFFFF5
#define writeDXVA_QueryOrReplyFunc(ptr, flg, fnc) ((*(ptr)) = ((flg) << 8) | (fnc))

typedef struct _DXVA_ConfigPictureDecode {
    DXVA_ConfigQueryOrReplyFunc dwFunction;
    DWORD dwReservedBits[3];
    GUID guidConfigBitstreamEncryption;
    GUID guidConfigMBcontrolEncryption;
    GUID guidConfigResidDiffEncryption;
    BYTE bConfigBitstreamRaw;
    BYTE bConfigMBcontrolRasterOrder;
    BYTE bConfigResidDiffHost;
    BYTE bConfigSpatialResid8;
    BYTE bConfigResid8Subtraction;
    BYTE bConfigSpatialHost8or9Clipping;
    BYTE bConfigSpatialResidInterleaved;
    BYTE bConfigIntraResidUnsigned;
    BYTE bConfigResidDiffAccelerator;
    BYTE bConfigHostInverseScan;
    BYTE bConfigSpecificIDCT;
    BYTE bConfig4GroupedCoefs;
} DXVA_ConfigPictureDecode, *LPDXVA_ConfigPictureDecode;

typedef struct _DXVA_BufferDescription {
    DWORD dwTypeIndex;
    DWORD dwBufferIndex;
    DWORD dwDataOffset;
    DWORD dwDataSize;
    DWORD dwFirstMBaddress;
    DWORD dwNumMBsInBuffer;
    DWORD dwWidth;
    DWORD dwHeight;
    DWORD dwStride;
    DWORD dwReservedBits;
} DXVA_BufferDescription, *LPDXVA_BufferDescription;

typedef struct _DXVA_Status_VC1 {
    USHORT StatusReportFeedbackNumber;
    WORD wDecodedPictureIndex;
    WORD wDeblockedPictureIndex;
    UCHAR bPicStructure;
    UCHAR bBufType;
    UCHAR bStatus;
    UCHAR bReserved8Bits;
    USHORT wNumMbsAffected;
} DXVA_Status_VC1, *LPDXVA_Status_VC1;

typedef struct _DXVA_ConnectMode {
    GUID guidMode;
    WORD wRestrictedMode;
} DXVA_ConnectMode, *LPDXVA_ConnectMode;

#define DXVA_RESTRICTED_MODE_UNRESTRICTED 0xFFFF
#define DXVA_RESTRICTED_MODE_MPEG2_A      0x0A
#define DXVA_RESTRICTED_MODE_MPEG2_C      0x0C
#define DXVA_RESTRICTED_MODE_H264_E       0x68
#define DXVA_RESTRICTED_MODE_VC1_D        0xA3

#define DXVA_COMPBUFFER_TYPE_THAT_IS_NOT_USED 0
#define DXVA_MOTION_VECTOR_BUFFER             16
#define DXVA_FILM_GRAIN_BUFFER                17
