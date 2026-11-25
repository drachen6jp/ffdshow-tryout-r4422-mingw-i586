# ffdshow.ax (r4422) MinGW ビルド手順

## 結論と根拠

公式 ffdshow.ax の輸入を全数実測したところ、Windows 2000 に無い API は **7 個だけ** で、
全て VS2010 の静的リンク CRT 由来だった (EncodePointer / DecodePointer /
InitializeSListHead / InterlockedPushEntrySList / InterlockedPopEntrySList /
InterlockedFlushSList / QueryDepthSList)。
ffdshow 自身のコードはこの 7 個を一切呼んでいない (ツリー全域 grep で 0 件)。
つまり殻のコード自体は Windows 2000 で動く資格があり、CRT が msvcrt.dll 系になる
MinGW ビルドにすれば、この 7 個は構造的に消える。

移植ブロッカーの実測 (r4422 ツリー全域):

- MFC: 無し。ATL: `<atlbase.h>` が DXVA 系ヘッダ 2 枚のみで、中身は CComPtr / CComQIPtr
  (mingw-w64 の atlbase.h で足りる)
- SEH `__try/__except`: 2 箇所。うち ffglobals.cpp のスレッド名付けは `#ifdef DEBUG`
  の内側なので Release では消える。実質のパッチは 1 箇所だけ (下記 P4)
- MSVC インライン asm: 2 箇所とも `#ifndef __GNUC__` でガード済みで C フォールバックあり。無修正
- `#include <ppl.h>`: 使用は Tconvert.cpp の `Concurrency::parallel_for_each` 1 箇所のみ。
  直列版シム (mingw_shims/ppl.h) で代替
- `stdext::hash_map`: 7 ファイルで使用。stdafx.h に __GNUC__ 用シムを追加 (下記 P5)
- boost: ツリー外依存。stdafx.h 経由で boost::thread / mutex / foreach を使用。
  1.49.0 を別途用意し、win32 版 thread のソース 2 本を Makefile 内で一緒に組む
- 衛星 DLL (ff_libmad.dll 等) は ffdshow.ax にリンクされておらず実行時ロード。
  本 Makefile の対象は ffdshow.ax 本体のみ (衛星も全て同じ XP API を輸入しているので、
  必要なものは同じ手つきで別途組む。libavcodec で代替できるコーデックはそちら優先でよい)

## 前提

1. boost 1.49.0 を展開する (SourceForge の boost プロジェクト、boost_1_49_0.tar.bz2)。
   既定では trunk の隣 (`../boost_1_49_0`) を見る。場所が違う場合は
   `make -f Makefile.mingw BOOST_DIR=/path/to/boost_1_49_0`
2. `Makefile.mingw` を trunk ルート (ffdshow_2010.sln と同じ場所) に置く。
   `mingw_shims/` フォルダも trunk ルートに置く。

## ソースパッチ (2 件)

### P4: src/codecs/TDXVADecoderH264.cpp (115 行目付近)

次の 3 行を、

```
				__try {
					memcpy (pDXVABuffer+3, Nalu.GetDataBuffer(), Nalu.GetDataLength());
				} __except (EXCEPTION_EXECUTE_HANDLER) { break; }
```

次の 1 行にする (長さの負値チェックは直前に既にあるので、この SEH は保険だった)。

```
				memcpy (pDXVABuffer+3, Nalu.GetDataBuffer(), Nalu.GetDataLength());
```

### P5: src/stdafx.h

` #include <hash_map>` の行の直後に追加する。

```
#ifdef __GNUC__
#include <ext/hash_map>
namespace stdext {
    using __gnu_cxx::hash_map;
    using __gnu_cxx::hash_multimap;
    using __gnu_cxx::hash;
}
#endif
```

## ビルド

```
make -f Makefile.mingw -j14
```

出力は `bin/ffdshow.ax` (ffmpeg.dll と同じ bin/)。初回はエラーが出たファイルを
教えてもらえれば順に潰す。止まらずに最後まで走らせたいときは `make -k` を付けると
コンパイルできる分は全部進む。

生成物について:

- `src/svn_version.h` は Makefile が自動生成する (SVN_REVISION 4422、ビルド日付)。
  version.h はツリー側にあるので生成は svn_version.h だけ
- マニフェスト (comctl32 v6) はリソース ID 2 で埋め込む (公式は外部 .manifest ファイル
  だが、埋め込みでも同じ効果)
- リンクは `-static-libgcc -static-libstdc++` で GCC ランタイムを静的にし、
  輸入がシステム DLL だけになるようにしてある
- サブシステムは 5.0 を明示 (ffmpeg.dll のときの dllwrap 既定 4.0 と違い、今回は
  リンクを自分で握るので 2000 の正規値にした)

## 検証の門

```
make -f Makefile.mingw check
```

- 輸入 DLL がシステム DLL だけ (KERNEL32 / USER32 / GDI32 / COMDLG32 / ADVAPI32 /
  SHELL32 / ole32 / OLEAUT32 / WINMM / COMCTL32 / DINPUT / SHLWAPI / d3d9 の範囲)
- XP API チェックが OK (何も引っかからない)
- 輸出が公式の 13 件と一致:
  DllCanUnloadNow, DllGetClassObject, DllRegisterServer, DllUnregisterServer,
  configure, configureAudio, configureAudioRaw, configureDXVA, configureDec,
  configureEnc, configureRaw, configureSubtitles, ffacm2creator
- CMOV センサス (手書き asm は実行時ディスパッチ前提なので、数が出ても場所の確認が要る)

## 注意

- 対象は Release 相当のみ (NDEBUG)。DEBUG 定義の経路は追っていない
- `Concurrency::parallel_for_each` は直列実行になる (色変換の一部が単スレッド化する。
  意味は変わらない)
- ソースの文字コード変換でエラーが出るファイルがあったら教えてほしい
  (-finput-charset の指定で対処する)
- ff_kernelDeint.dll は従来どおり対象外

## 第2弾パッチ (2026-11-22、初回ビルドのエラーを受けて)

方針変更: `-std=gnu++14` → **`-std=gnu++98`** (Makefile.mingw 内の 3 箇所すべて変更済み:
CXXFLAGS 1 箇所と、boost thread 2 ルール内の直書き 2 箇所。boost 側は CXXFLAGS を使わず
直書きのため、CXXFLAGS だけの変更では gnu++14 のまま残るので注意)。
理由: src/tuple.h (C++11 以前の独自 tuple) が libstdc++ の std::tuple 等と衝突するため。
コード側の C++11 使用は実測で Tconvert.cpp の mt_copy 1 関数のみ (nullptr 0、override は
コメント内のみ、for each 構文なし)。boost 1.49 も gnu++98 が本籍で相性が良くなる。

- **P6a**: src/stdint.h と src/inttypes.h (同梱の MSVC 専用版がシステム側を覆い隠す) —
  両ファイルとも先頭に `#ifdef __GNUC__` / `#include_next <stdint.h>` (inttypes は
  `<inttypes.h>`) / `#else` を入れ、末尾に `#endif` を追加。本体は #else 側に残る。
  これで boost の INT64_C リテラルエラーも消える。
- **P6b**: src/stdafx.h の ` #include <array>` 行を
  `#if !defined(__GNUC__) || __cplusplus >= 201103L` … `#endif` で囲む
  (std::array の使用は mt_copy のみで、P7 で消える)。
- **P7**: src/convert/Tconvert.cpp の mt_copy — std::array とラムダを C 配列 + for ループに
  書き換え (ppl シムは元々直列なので意味は同一)。
- **P8**: src/ffglobals.h の append 2 箇所 — `insert(this->end(),` → `this->insert(this->end(),`
  (依存基底クラスの two-phase lookup 対策)。
- **P9**: src/dwstring.h の stdext::hash_value 特殊化宣言2行を、__GNUC__ 時は
  __gnu_cxx::hash<DwString<char>> / <DwString<wchar_t>> の定義 (自前の文字列ハッシュ) に
  置き換え。src/dwstring.cpp 側の hash_value 定義2つは `#ifndef __GNUC__` で囲む。
  ハッシュ値自体は実行時のみのデータ構造用なので MSVC と一致しなくてよい。
- **P10**: src/Crect.h は差し替え版を用意 (CPointT/CSizeT/CRectT 内の基底メンバ参照
  x/y/cx/cy/left/top/right/bottom に this-> を付けただけの機械的変更、108 箇所、
  CRLF 保持、strip 検証済み)。

## 第3弾パッチ (2026-11-22、TvideoCodecQuickSync.cpp)

- **P11**: IntelQuickSyncDecoder のソースは r4422 スナップショットに無い (externals
  取り込み前提で vcxproj のみ)。DLL は実行時ロードなので、使用分だけの宣言シム
  `mingw_shims/IntelQuickSyncDecoder/src/IQuickSyncDecoder.h` を用意。
- **P12**: TvideoCodecQuickSync.cpp の `__cpuid` を __GNUC__ 時は `<cpuid.h>` の
  `__get_cpuid` に置き換え。
- **P13**: 同ファイルの `__super::onDiscontinuity()` を __GNUC__ 時は
  `TvideoCodecDec::onDiscontinuity()` に置き換え。

## 第4弾 (2026-11-22、差し替えファイルで納品)

- **P14**: src/imgFilters/TffPict.h — `enum AVColorRange;` の前方宣言は MSVC 拡張で
  GCC は不可。実定義は src/ffmpeg/libavcodec/avcodec.h (142/154行) にあるので、
  __GNUC__ 時はそちらを include する差し替え版を用意 (AVColorSpace の使用箇所も同時解決)。
- **P15**: src/ffglobals.h — vectorEx の push_back 9 箇所に this-> を追加。
  P8 の insert 2 箇所も含めた差し替え版で納品 (strip 検証済み)。

## 第5弾 (2026-11-22、SIMD 変換 TU)

- **P16**: Makefile.mingw に対象限定ルールを追加 —
  `src/convert/ffdshow_converters2.o` と `src/convert/Tconvert.o` だけ
  `CXXFLAGS += -mmmx -msse2`。simd.h の Tsse2 は `#ifdef __SSE2__` ガード内で、
  呼び出し側は Tconfig::cpu_flags の実行時ディスパッチなので、この2ファイルに
  SSE2 を解禁するのが設計通り。全体の -march=i586 は不変。
- **P17**: src/convert/ffdshow_converters2.cpp — `_mm::__m` の宣言11箇所に
  typename を付けた差し替え版 (strip 検証済み)。

## 第6弾 (2026-11-22、大文字小文字シムの一括追加)

ツリー全域の #include を走査し、大文字小文字だけ違う名前で include されている
箇所を先回りで全部シム化 (mingw_shims/ に追加、実体名へ橋渡しする1行ファイル):
TSubtitleMixedProps.h (実体 TsubtitleMixedProps.h、今回のエラー)、
CRect.h (実体 Crect.h、Tfont.h が使用 — 同じ TU で次に当たるはずだった)、
StdAfx.h、TOSDSettings.h、TffdecoderVideo.h、TimgFilterGradFun.h、
Vmr9.h、D3d9.h、MMDeviceAPI.h、WinUser.h の計10枚。
mingw_shims.tar.gz に一式を固めて納品。

## 第7弾 (2026-11-22、qnetwork.h シム + comptr.h)

- **P18**: mingw_shims/qnetwork.h を新規作成。IAMExtendedSeeking は DirectShow SDK の
  qnetwork.h の宣言で mingw-w64 には無い (stdafx.h:65 が全 TU で include するので同名シムで解決)。
  メソッドは実装クラス TffdshowDec の7つ (get_ExSeekCapabilities / get_MarkerCount /
  get_CurrentMarker / GetMarkerTime / GetMarkerName / put_PlaybackSpeed / get_PlaybackSpeed)、
  AM_EXSEEK_* 定数付き。IID の実体は TffDecoder_reg.cpp の DEFINE_GUID 側にあるので extern 宣言のみ。
- **P19**: src/comptr.h — comptrQ::operator= の ptrAssign 1 箇所に this-> を付けた
  差し替え版 (strip 検証済み)。
- 備考: ffglobals.h:581 のエラーが第4弾と同一で再発 — 差し替え未適用の可能性を
  ユーザーに確認依頼 (grep で this->push_back が 9 件あるはず)。
  → 次の ffglobals.cpp のログで ffglobals.h 側のエラーは消えており、適用済みと判明。

## 第8弾 (2026-11-22、ffglobals.cpp の default)

- **P20**: src/ffglobals.cpp — `_get_bits1` の引数名 `default` は C++ キーワードで
  MSVC が通していただけ。`defval` に改名した差し替え版 (宣言と return の2箇所のみ、
  strip 検証済み)。「引数が足りない」の大量エラーは `=0` が読めなかった副作用で、
  根本原因はこの1件だけ。

## 第9弾 (2026-11-22、字幕 SIMD ヘッダ)

- **P21**: Makefile.mingw の SSE2 対象を拡張 — 字幕系4 TU (CPolygon / Tfont /
  TsubtitleText / TrenderedTextSubtitleWord) と TimgFilterNoise / TnoiseSettings を追加。
  TrenderedTextSubtitleWord.h と TimgFilterNoise.h は __m128i/__m64 を直接使うため。
- **P22**: src/subtitles/TrenderedTextSubtitleWord.h — __GNUC__ 時に
  <mmintrin.h>/<emmintrin.h> を読む3行+ガードを追加した差し替え版 (strip 検証済み)。
  このヘッダは simd.h より前に読まれる TU があるため、型の手当てが別途必要だった。

## 第10弾 (2026-11-22、TsubtitleFormat.cpp の & 漏れ)

- **P23**: src/subtitles/TsubtitleFormat.cpp:1184 — `writeProps(contents,props)` は
  他の7箇所すべて `&props` / `&lineProps` で、ここだけ & が漏れている (MSVC が通していた)。
  & を足した差し替え版 (strip 検証済み)。

## 第11弾 (2026-11-22、TsubtitleMixedProps.cpp の _mm_empty)

- **P24**: src/subtitles/TsubtitleMixedProps.cpp:550 — 素の `_mm_empty()` が1箇所だけあり
  intrinsic ヘッダを読んでいない。__GNUC__ 時は `__asm__ volatile ("emms")` に置き換えた
  差し替え版 (ガードは #ifdef/#else/#endif の1組、strip 検証済み)。
  この TU 自体は MMX 演算をしないのでフラグ追加はせず、ファイル内で完結させた。

## 第12弾 (2026-11-22、atlbase.h シム)

- **P25**: mingw_shims/atlbase.h を新規作成。事前調査で「mingw-w64 の atlbase.h で足りる」
  と判断したのは見込み違いで、mingw-w64 に ATL は同梱されていなかった。
  ffdshow の使用は DXVA 系ヘッダ3枚の CComPtr / CComQIPtr だけなので、
  ATL 本家と同じ意味論でその2クラスを実装したシムを用意。

## 第13弾 (2026-11-22、videoacc.h シム)

- **P26**: mingw_shims/videoacc.h を新規作成。IAMVideoAccelerator (DXVA 1.0) も
  mingw-w64 に無い SDK ヘッダだった。メソッド12個の vtable 順と AMVA 系構造体6種の
  レイアウトは SDK / Wine の videoacc.idl で確認し、ffdshow 側のメンバ参照とも照合。
  IID は strmiids 側にあるので extern 宣言のみ。IAMVideoAcceleratorNotify は未使用のため入れない。

## 第14弾 (2026-11-22、Crect / TffPict 再配置、videoacc 拡張、Tlibavcodec、avisynth)

- **P27**: ユーザー側で Crect.h (mingw_shims 配置分) と TffPict.h が元の状態に戻っていた。
  Crect.h は今回から tar 内の mingw_shims/Crect.h にも同梱 (CRect.h ブリッジと併せて)。
  念のため src/Crect.h への上書きも依頼。TffPict.h は再送して再配置を依頼。
- **P28**: videoacc.h に IAMVideoAcceleratorNotify (メソッド3個、Wine IDL 順) と
  AMVAUncompBufferInfo を追加 (TffdshowDecVideoOutputPin が実装していた。当初の grep で見落とし)。
- **P29**: src/ffmpeg/Tlibavcodec.h:99 — `enum AVPacketSideDataType` の前方使用 (MSVC 拡張)。
  本物の avcodec.h と共存する TU が多いため enum 定義は持ち込まず、__GNUC__ 時は
  引数を int に替えた差し替え版 (ABI 同一、呼び出し側の列挙子は暗黙変換で通る。strip 検証済み)。
- **P30**: src/imgFilters/avisynth/avisynth.h:549 — IClip のデストラクタの __stdcall が
  派生側の暗黙デストラクタと属性競合。ヘッダ自身の Intel コンパイラ用分岐
  (`#if defined(__INTEL_COMPILER)` で __stdcall 無し版) に __GNUC__ も加えた差し替え版 (strip 検証済み)。

## 第15弾 (2026-11-22、TrgbPrimaries.h のバックスラッシュ include)

- **P31**: src/convert/TrgbPrimaries.h:7 — `#include "libswscale\swscale.h"` と
  パス区切りがバックスラッシュ (MSVC は通す)。ツリー全域でこの形はこの1件だけ。
  `/` に直した差し替え版 (strip 検証済み)。

## 第16弾 (2026-11-22、MEDIASUBTYPE_DTS)

- **P32**: MEDIASUBTYPE_DTS は SDK の ksuuids.h 由来で mingw-w64 の uuids.h には無い。
  値はこちらも WAVE_FORMAT 形式ではなく {e06d8033-db46-11cf-b4d1-00805f6cbbea} (ksuuids.h で確認)。
  使用する TU は全て ffdshow_mediaguids.h を読んでいるので、その DTS_HD の直前に
  DEFINE_GUID を1行足した差し替え版を納品 (strip 検証済み)。
  実体は <initguid.h> を読む TffDecoder_reg.cpp 側に立つ。

## 第17弾 (2026-11-22、TffDecoderDXVA.cpp のバックスラッシュ include)

- **P33**: src/TffDecoderDXVA.cpp:53-54 — `#include "codecs\TvideoCodecLibavcodec.h"` と
  `#include "codecs\TvideoCodecLibavcodecDxva.h"` の2件 (同じファイルに2行続いていた。
  初版の差し替えはエラーの出た53行目しか直しておらず、54行目を取りこぼした)。
  前回の走査はダウンロード済みファイルだけだったため漏れていた。今回は Makefile の
  SRCS 一覧と突き合わせて未取得の14ファイルを補完走査し、頑健な再走査でも
  この形は他に無いことを確認。2件とも `/` に直した差し替え版 (ファイル内に残り0件を検証)。

## 第18弾 (2026-11-22、DXVA 1.0 の型と uuidof)

- **P34**: mingw_shims/dxva.h を新規作成 (#include_next で本体を読み、不足分だけ追加)。
  mingw-w64 の dxva.h に無い DXVA 1.0 の3構造体 (DXVA_ConfigPictureDecode /
  DXVA_BufferDescription / DXVA_Status_VC1) と、DXVA_ConfigQueryOrReplyFunc typedef、
  フラグ定数 (PROBE_QUERY=0xFFFFF1、LOCK_QUERY=0xFFFFF5)、writeDXVA_QueryOrReplyFunc
  マクロ (これも dxva.h 側のマクロだった)。レイアウトと値は MS Learn の構造体ページ、
  VC-1 仕様書 §3.8.1、実在の dxva.h 複製で照合 (フラグ値は MS Learn の DDI 文書でも裏取り)。
- **P35**: videoacc.h の両インターフェイスに uuid を持たせる。
  CComQIPtr の既定テンプレート引数 &__uuidof(T) が、uuid 属性の無い自作宣言では
  定数式にならずエラーになっていたため。当初 __CRT_UUID_DECL で直そうとしたが効かず、
  mingw の正規の書き方 (MIDL_INTERFACE と同じ struct への __declspec(uuid("..."))) に変更。
- **P36**: src/codecs/TDXVADecoder.h:108 — 純粋仮想の `= NULL;` は MSVC 拡張。
  `= 0;` に直した差し替え版 (ツリー全域でこの形はこの1件だけ。strip 検証済み)。

## 第19弾 (2026-11-22、CComQIPtr のテンプレート引数)

- **P37**: atlbase.h の CComQIPtr から第2テンプレート引数 (const IID *piid) を撤去。
  MinGW の __uuidof は __mingw_uuidof<T>() という関数呼び出しに展開され、その戻り値
  (関数内 static への参照) のアドレスはテンプレート引数にならないのが根本原因で、
  uuid 属性の有無とは無関係だった。IID はコンストラクタ/代入の中で __uuidof(T) を
  直接使って実行時に取る形に変更 (意味論は本家と同じ)。ffdshow 側の使用は全て
  型引数1つだけなので互換。videoacc.h の __declspec(uuid) は式の中の __uuidof 用に残す。

## 第20弾 (2026-11-22、ソースの文字コード CP1252)

- **P38**: ツリーは文字コード混在 (大半は ASCII、12ファイルは UTF-8 BOM 付き、
  9ファイルは CP1252)。一括で -finput-charset を付けると BOM 側が壊れるため、
  CP1252 のバイトが文字列リテラルの中にある4ファイルだけ、該当バイトを
  8進エスケープに置き換えた差し替え版を納品 (MSVC が読んでいたコードポイントと同一。
  8進にしたのは後続文字が16進数字でも食い込まないため)。
  TffdshowDec.cpp の n° ×2、TglobalSettings.h の tÜrk、TlevelsSettings.cpp の Didée、
  TsubtitlesSettings.cpp の Español / Bokmòl ×2 / Provenšal ×2。
  コメント内だけの CP1252 (tuple.h の Järvi 等) は GCC が通すので無変更。

## 第21弾 (2026-11-22、IEC61937 の GUID)

- **P39**: mingw_shims/ksmedia.h を新規作成。ツリー全域で使う KSDATAFORMAT 名のうち
  mingw-w64 の ksmedia.h に無いのは2つだけだった:
  KSDATAFORMAT_SUBTYPE_IEC61937_AAC (00000006-0cea-0010-8000-00aa00389b71) と
  KSDATAFORMAT_SUBTYPE_IEC61937_WMA_PRO (00000164-0000-0010-8000-00aa00389b71)。
  値は MS Learn の IEC 61937 対応表で確認し、mingw 本体と同じ DEFINE_GUIDSTRUCT 形式で追加。

## 第22弾 (2026-11-22、__super の全数処理)

- **P40**: `__super::` は QuickSync の1件だけではなかった。ツリー全域で7ファイル11箇所。
  残り6ファイル分を基底クラス名に置き換えた差し替え版を納品:
  TffdshowDecAudioInputPin.cpp (→TinputPin、4箇所)、TffdshowDecVideoOutputPin.cpp
  (→CTransformOutputPin、2箇所)、TffdshowDecVideoAllocatorDXVA.cpp (→CBaseAllocator、1箇所)、
  TDXVADecoderVC1.cpp / TDXVADecoderH264.cpp (→TDXVADecoder、各1箇所)、
  TrealAudioInfo.cpp (→TrealAudioInfo、2箇所)。これで __super はツリーから消えた。

## 第23弾 (2026-11-22、バックスラッシュ include の完結)

- **P41**: 走査用のダウンロードに静かな失敗が混ざっていて census に穴が開いていた。
  SRCS 全359ファイルを hidden_files/srccorpus/ に取り直して (永続化) 全数走査した結果、
  バックスラッシュ include は3ファイルだけだった: TffDecoderDXVA.cpp (修正済み)、
  TffdshowDecVideo.cpp (1件)、TffdshowDecVideoOutputPin.cpp (2件)。
  後の2つの差し替え版を納品。TffdshowDecVideoOutputPin.cpp は第22弾の __super 修正と
  合体済み (差し替えは1回で済む)。

## 第24弾の注記 (2026-11-22、Crect.h の配置)

- Crect.h の修正版は mingw_shims/ に置いても、src/ 直下のファイルからの
  quoted include では届かない (include 元と同じディレクトリが優先されるため、
  src/TffdshowDecVideoAllocator.h は src/Crect.h の未修正版を読んでいた)。
  他ディレクトリの includer は shims 側で通っていた。**src/Crect.h 自体を
  修正版で上書きする**のが正しい配置。

## 第25弾 (P42、2026-11-22)

- mingw_shims/dxva.h に DXVA_ConnectMode 構造体と不足定数を追加:
  DXVA_RESTRICTED_MODE_UNRESTRICTED=0xFFFF / MPEG2_A=0x0A / MPEG2_C=0x0C /
  H264_E=0x68 / VC1_D=0xA3、DXVA_COMPBUFFER_TYPE_THAT_IS_NOT_USED=0 /
  DXVA_MOTION_VECTOR_BUFFER=16 / DXVA_FILM_GRAIN_BUFFER=17。
- H264_E=0x68 は公式 ffdshow.ax の DXVA_PARAMS テーブル実測で確定
  (MPEG2 0x0A/0x0C と VC1_D 0xA3 も同テーブルで照合済み。VC1_D は
  VC-1 仕様書 §4 でも 0xA3 と一致)。
- 教訓: DXVA トークンの census は codecs 系だけでなく全域でやること。
  今回は全 SRCS コーパスで再実施し、残りは ffdshow 内部名のみと確認。

## 第26弾 (P43、2026-11-22)

- text<T> から ffstring への暗黙変換は「変換演算子 + DwString コンストラクタ」の
  二段で、標準 C++ では不可 (MSVC 拡張で通っていた)。該当は2箇所のみ
  (全域 grep で確認): TtextInputPin.cpp:98 と TinputPin.cpp:101。
  両方とも (const char_t*) キャストを明示した差し替え版を納品。
  const char_t* を引数に取る関数への text<> 渡しは一段変換なので無事。

## 第27弾 (P44、2026-11-22)

- Rasterizer.cpp の MSVC 記法2種: 関数形式キャスト (__int64(x) は GCC では
  long long の2語型名になり不可) と i64 サフィックス。全域 census で
  このファイルだけ (キャスト2件・リテラル2件)。(long long)(...) と LL に
  直した差し替え版を納品 (配置先 src/subtitles/)。

## 第28弾 (P45、2026-11-22)

- TrenderedTextSubtitleWord.cpp の _mm::__m 宣言3行に typename を追加
  (P17 と同種。converters2 は修正済みで、残りはこのファイルだけだった)。
  配置先 src/subtitles/。

## 第29弾 (P46/P47、2026-11-22)

- P46: TrgbPrimaries.h の enum AVColorSpace 前方使用 (P14 と同種) →
  __GNUC__ 時に libavcodec/avcodec.h を include する行を files/TrgbPrimaries.h
  に追加 (src/convert/ へ再配置)。
- P47: TspuImage.cpp の template struct TspuImageSimd<Tsse2> が、simd.h の
  Tsse2 が #ifdef __SSE2__ ガードのため見えない → Makefile.mingw の
  SSE2 解禁リスト (字幕系の行) に TspuImage.o を追加。

## 第30弾 (P48、2026-11-22)

- Tsse2 をテンプレート引数に使う TU は全て __SSE2__ が要る (simd.h の
  ガード)。全域 census で該当 TU を洗い出し、Makefile.mingw の SSE2 解禁
  リストへ一括追加: TsubtitleDVD、imgFilters 10本 (Bitmap/Blur/Chroma/
  Colorize/Deinterlace/GradualDenoise/Luma/Postproc/Sharpen/Warpsharp)、
  TaudioFilter。最終検証では、これらの TU の SSE2 命令が Tsse2 テンプレート
  関数内に閉じていることを objdump で確認すること。

## 第31弾 (P49、2026-11-22)

- __gnu_cxx::hash_map の erase は戻り値 void (MSVC の stdext 版は iterator)。
  全域の `x = cont.erase(x)` 12箇所を容器型で仕分けし、実際に壊れるのは
  hash_map の TsubtitlePGSParser.cpp:94 の1箇所だけと確定 (vector/list 系は
  C++98 でも iterator を返す。TsubtitlesTextpinPGS の1件はコメント内)。
  `m_palettes.erase(p++);` に直した差し替え版を納品 (src/subtitles/)。

## 第32弾 (P50、2026-11-22)

- MMX 版の問題: simd.h の Tmmx 補助関数や _mm_empty を使う TU は -mmmx が
  要る (i586 だけでは MMX intrinsic が target mismatch)。全域 census で
  Tmmx/_mm_empty 使用 TU を洗い、未解禁の13本に -mmmx のみの解禁行を
  Makefile.mingw に追加 (2xsai、SimpleResize、TimgFilterDCT/Denoise3d/
  Perspective/ShowMV、asharp、nic_postProcess、TrgbPrimaries、image、
  ffdshow_converters、TffdshowEnc、TvideoCodecLibavcodec)。

## 第33弾 (P51、2026-11-22)

- SimpleResize.cpp は simd.h の命令マクロを __m128i で使う (SSE2 版
  オーバーロードが要る) ため、-mmmx のみでは不足。SSE2 ブロックは
  SSE2enabled (Tconfig::cpu_flags の FF_CPU_SSE2) の実行時判定の内側に
  あることをソースで確認し、SSE2 解禁側へ移動。

## 第34弾 (P52、2026-11-22)

- _mm_shuffle_pi16 (PSHUFW) 等の SSE1 (MMXEXT) intrinsic を使う TU は
  -msse が要る。全域 census の該当は SimpleResize/Deinterlace/Postproc
  (SSE2 解禁済み) のほか TimgFilterDenoise3d と nic_postProcess。
  後2者を -mmmx -msse の行へ分離。

## 第35弾 (P53、2026-11-22)

- ffalign(width, 8) の width が long で、int / unsigned int の2オーバー
  ロードが曖昧 (MSVC では long==int で同一視)。ffglobals.h に __GNUC__ 時
  の long 版オーバーロードを追加 (files/ffglobals.h を src/ へ再配置)。

## 第36弾 (P54、2026-11-22)

- Tmmxext (simd.h の MMXEXT 層、_mm_max_pu8 等) を使う TU は -msse が
  要る。全域 census の Tmmxext 使用 TU のうち未解禁だった asharp と
  convert/image を -mmmx -msse の行へ移動。

## 第37弾 (P55、2026-11-22)

- Cavisynth.cpp の #include "imgfilters/avisynth/..." は小文字の
  imgfilters で実ディレクトリ imgFilters と大小が違う (全域でこの1件)。
  mingw_shims/imgfilters/avisynth/TimgFilterAvisynth.h の橋渡しシムを
  追加し tar を再生成。

## 第38弾 (P56、2026-11-22)

- src/tuple.h は先頭で _MSC_VER==1600 のときだけ本物の <tuple> を使い、
  それ以外は 1始まりの旧 boost コピー (namespace std) に落ちる。呼び出し
  側 (Cgeneric 等) は 0始まりの std::get 前提なので GCC では壊れる。
  差し替え版 tuple.h (src/ へ配置): __GNUC__ 時は <tr1/tuple> を読み
  std へ using する分岐を先頭に追加 (strip 検証済み)。

## 第39弾 (P57、2026-11-22)

- Twindow.h と TpresetSettings.cpp は _MSC_VER==1600 で「モダンな
  std::get / 全要素コンストラクタ」と「旧コピーのメンバ get」の分岐を
  持つ。GCC でもモダン側を取るよう条件に defined(__GNUC__) || を追加
  した差し替え版2枚 (Twindow.h は src/dialog/、TpresetSettings.cpp は
  src/settings/ へ配置。strip 検証済み)。

## 第40弾 (P58、2026-11-22)

- ffmpeg 側 C ヘッダ (dsputil.h 等) の restrict を C++ で読むため
  CXXFLAGS に -Drestrict=__restrict__ を追加 (C 側には付けない)。

## 第41弾 (P59、2026-11-22)

- TDXVADecoder.h の CHECK_HR マクロの余分な ## を除去 (files 側の
  既納品版に追記。src/codecs/ へ再配置)。
- TDXVADecoder.cpp の pInputData / pMiscData への int* / DWORD* 代入に
  (LPBYTE) キャストを追加した差し替え版 (src/codecs/ へ配置)。

## 第42弾 (P60、2026-11-22)

- TDXVADecoderH264.cpp の __try/__except (P4 で除去済みだったもの) が
  P40 の差し替え版作成時に原典から復活していた (私の合体漏れ)。
  files 側の現行版 (__super 修正込み) に SEH 除去を再適用して再送。

## 第43弾 (P61、2026-11-22)

- ffdshow_converters.cpp は SSE2 intrinsic を直接使う。init() の SSE2 も
  Tconvert のモード選択 (FF_CPU_SSE2 のときだけ MODE_ffdshow_converters)
  の内側でのみ呼ばれることを確認し、SSE2 解禁行へ移動。

## 第44弾 (P62/P63、2026-11-22)

- (P62) image.cpp:631 の2語型名の関数形式キャスト unsigned int(...) を
  通常キャストへ (全域でこの1行だけ)。差し替え版を src/convert/ へ。
- (P63) firfilter.cpp は Tsse (SSE 浮動小数点) を使う唯一の TU。
  -mmmx -msse の行へ追加。

## 第45弾 (P64、2026-11-22)

- TtrayIcon.h にも _MSC_VER==1600 分岐が1件 (先の census 時点で
  コーパス未収録だった)。同方式で条件を広げた差し替え版 (src/ へ配置)。

## 第46弾 (P65、2026-11-22) — 全 .cpp 通過、リソース段へ

- src/version.h は UTF-16LE (BOM 付き) で windres が読めない。
  中身は同一のまま CP1252 (©=A9h) + CRLF に変換した version.h を
  files に用意 (src/ へ配置)。

## 第47弾 (P66、2026-11-22)

- svn_version.h の BUILD_YEAR 等は文字列リテラルでないと rc の文字列
  連結が壊れる。生成ルールを引用符付きに修正。既に生成済みの
  src/svn_version.h は消してから再生成すること。

## 第48弾 (P67、2026-11-22) — リンク段到達

- LDLIBS から -loldnames (この環境に無い) と -lstrmiids (同梱の
  baseclasses/strmiids.cpp と GUID が二重定義になる) を除去。

## 第49弾 (P68、2026-11-22) — リンクの未解決シンボル対策

- 未解決は5系統: (1) _imp___Interlocked* 1189件 — intrin.h を読まない
  TU では dllimport 宣言だけが見えるため。CXXFLAGS に -include intrin.h
  を追加 (要・全オブジェクト再ビルド)。(2) boost::thread の _imp_ —
  消費者側に -DBOOST_THREAD_USE_LIB が無かった。DEFS に追加。
  (3) IEC61937 GUID 5種、(4) MR_* サービス GUID 3種と CLSID_AviSplitter、
  (5) __mingw_uuidof 特殊化4種 — mingw_shims/guids.cpp を新設して定義し
  リンクに追加。GUID 値は mingw-w64 / Wine の原典で確認。

## 第50弾 (P69、2026-11-22)

- boost の tss_pe.cpp が MinGW の世代判定に失敗し、古代 MinGW 向けの
  _tls_used 定義を出してランタイム (tlssup.o) と二重定義になった。
  判定条件に defined(__MINGW64_VERSION_MAJOR) を足した差し替え版を
  ../boost_1_49_0/libs/thread/src/win32/tss_pe.cpp へ配置。
  これで boost 側は .CRT$XLB へのコールバック登録だけになる。

## 第51弾 (P70、2026-11-22)

- 作業ミスではなく make の性質: フラグだけの変更ではオブジェクトが
  再ビルドされない。Interlocked 対策 (-include intrin.h) は未検証のまま
  だった。obj を消して全再ビルドが必要。
- __mingw_uuidof 特殊化は inline のため、使う側で宣言しか見えないと
  実体が出ない。guids.cpp に参照配列を足して実体化を強制。
- なお GUID 実体・boost _imp_・_tls_used はこのログで解消済み。

## 第52弾 (P71、2026-11-22)

- -include intrin.h は無効と実証 (再ビルド後も __imp_ 参照が不変)。
  原因: mingw-w64 の libkernel32.a は stdcall 装飾付き Interlocked の
  __imp_ を意図的に提供しない (kernel32.def に FIXME コメントあり)。
  対策を interlocked_shim.c に切替: 5関数の実装 (GCC ビルトイン使用)
  と __imp_ ポインタセルを自前で定義。Makefile から intrin 指定を除去。
- uuidof は GUID アドレスの配列ではコンパイラに畳み込まれて関数の
  実体が出なかった。関数アドレスの配列に変更。

## 第53弾 (P72、2026-11-22)

- Interlocked は interlocked_shim で全解消。残りは uuidof 5件のみ。
- 関数アドレス配列が static だったため未使用として捨てられていた。
  外部リンケージに変更して実体化を強制。

## 第54弾 (P73、2026-11-22)

- guids.cpp のソースは正しい (fns は 43 行目) のに実体が出ないことを
  nm で確認。__CRT_UUID_DECL + アドレス配列の方式は GCC 14 では
  実体化しないと判断し、uuidof_shim.c に切替: 参照側オブジェクトの
  nm で得た正確なマングル名で4関数を直接定義 (戻り値 const GUID& は
  ABI 上ポインタ返しと同一)。guids.cpp は GUID データのみに整理。

## 第55弾 (P74、2026-11-22) — CMOV 対策の方針転換

- リンク成功 (bin/ffdshow.ax、20.3MB)。検証の門: 輸入は Win2000 世代の
  DLL のみ、XP API ゼロ、輸出 13 件一致、サブシステム 5.0。
- CMOV は 491 件。GCC 14 の i386.md では整数 cmov が TARGET_CMOVE
  (= TARGET_CMOV || TARGET_SSE) でゲートされており、SSE 解禁と CMOV は
  一体。-mno-cmov は存在しない。混在 TU では _c フォールバック側が
  汚染される (asharp_run_c に 60 件、image の _c 変換群に各 1 件)。
- 方針: 混在 TU はコマンドライン解禁をやめ、SIMD テンプレートの
  定義区間だけ #pragma GCC target で解禁する。第一弾は asharp
  (Tasharp テンプレート 202-419 行を push_options/target("sse")/
  pop_options で囲み、Makefile の -msse 行から asharp.o を外した)。

## 第56弾 (P75、2026-11-22) — GCC に -mno-cmov を追加

- ユーザーの希望で GCC 本体に -mno-cmov を実装 (gcc-14-mno-cmov.patch:
  i386.opt に Var(flag_cmov) の mcmov オプションを追加し、i386.h の
  TARGET_CMOV / TARGET_CMOVE に && flag_cmov を入れた)。
  /etc/portage/patches/cross-i686-w64-mingw32/gcc/ に置いて再 emerge。
  検証済み: -msse2 では cmov 1件、-mno-cmov 併用で 0件。
  (最初は patches 直下に置いて届かなかった。サブディレクトリ必須)
- ffdshow 側は解禁行 (SSE2 5行 + SSE1 1行) に -mno-cmov を追加し、
  asharp.o を SSE1 行へ復帰。asharp の pragma 実験は不要になった。

## 第57弾 (2026-11-22) — 検証の門、全通過

- -mno-cmov 付き全再ビルド後: 輸入は Win2000 世代 DLL のみ、XP API
  ゼロ、輸出 13 件一致、サブシステム 5.0、CMOV はリンク後イメージの
  線形デコードで 1 件見えたが、オブジェクト単位では全て 0 =
  データの誤読で実体なし。ffdshow.ax の MinGW ビルドは完了。
- 残件: Win2000 実機での差し替え確認 (公式 r4422 導入機で自作
  ffmpeg.dll + ffdshow.ax に差し替え、通常の H.264 ソフトデコード
  → DXVA の順。カーネルデインターレースは ff_kernelDeint.dll が
  未処理なので選ばない)。

## 第58弾 (P76、2026-11-22) — 実機ロード失敗と win2k_compat.c

- Win2000 実機で regsvr32 が「指定されたプロシージャが見つかりません」。
  公式 .ax との輸入差分で特定: 確実に Win2000 に無いのは kernel32 の
  条件変数4つ (Vista) と GetThreadId (XP)。参照元は libgcc.a の
  gthr-win32-cond.o / gthr-win32.o (nm で実証)。ほかに msvcrt の
  _s 系8個が容疑者。
- 対策は mingw_shims/win2k_compat.c: GetThreadId は ntdll の
  NtQueryInformationThread で実装、条件変数はセマフォ+完了イベントの
  古典設計 (遅延生成で CONDITION_VARIABLE_INIT に対応)、_s 系は
  通常実装。__imp_ セルは綴り2通りを両方定義。check の検査リストも
  これらの名前まで拡張した。

## 第59弾 (2026-11-22) — DrWatson 解析: NULL 関数ポインタ呼び出し

- 実機 (Pentium 系、Win2000 SP4) で MPC-HC 1269 + 外部フィルタ
  ffdshow (DXVA 有効 + H.264 は libavcodec) で H.264 再生時に落下。
- drwtsn32.log: 落下スレッド (ID 0x13c) は eip=00000000、ebp=0、
  eax=0。NULL の関数ポインタを call した形。.ax のロード範囲は
  024B0000–0345C000 (サイズ FAC000)。即時フレームは壊れているが、
  スタック上に .ax 内の戻りアドレス候補が残る (イメージ内オフセット
  2F3D32 / 2F6AAC / 2F96C5 / 2F978A / 304DF5)。
- 次手: nm で各オフセットを含む関数を特定 + DXVA 無効での再現確認
  (切り分け) + 公式 .ax × 自作 ffmpeg.dll の組み合わせ確認。

## 第60弾 (2026-11-22) — DrWatson ログ全文の精査と切り分けの追加

- ログ全文 (450行) を読み直し、第59弾に無かった事実を追加:
  CPU は x86 Family 5 Model 4 Stepping 4 (Pentium MMX 相当)、1 CPU。
  落下は 2026/11/22 14:11:37、pid 348、Win2000 SP4 build 2195。
  落下スレッド (ID 0x13c) は eax=0、esi=0、edi=0、ebp=0、edx=02095448、
  ecx=01618138、ebx=023e34d0。例外は c0000005 (アクセス違反) で
  #UD ではない。
- 5つの戻りアドレス (2F3D32 / 2F6AAC / 2F96C5 / 2F978A / 304DF5) は
  raw スタックにも全て確認済み。スタック上の並びは TryMediaTypes →
  AttemptConnection → SetMediaType (→ CTransformOutputPin::SetMediaType)
  の呼び出し順と整合する。即時フレームは壊れており (ebp=0)、
  DrWatson のバックトレース自体も1フレーム目がヒープ番地
  (023E3434) を指す壊れたものなので、このログからさらに
  絞れる情報は残っていない。ログの読み直しは不要。
- 切り分けに1手追加: 再現時は外部フィルタを ffdshow video だけに
  してから始める (当日は video / dxva / decode の3つ同時登録だった)。
  詳細は win2000_swap_test.md の「2026-11-22 実機経過」に移した。
  ユーザーは外出中で、実機側の調査は戻ってから行う。

## 第61弾 (2026-11-22 夕) — 切り分け結果と WMP の名指し

- DXVA ON/OFF どちらも同じ落下。external から DXVA を外しても、
  レンダラーを Overlay Mixer にしても落下 → プレイヤー固有でも
  レンダラー固有でも DXVA 固有でもない。
- WMP9 + AVI (自作クリップ) では **ffmpeg.dll の中、オフセット
  0x137C87** で落下 (WMP のエラー署名が名指し)。命令は
  cmpb $0x1,0x2(%ecx,%edi,1) で、オブジェクト照合で
  libavcodec/h264.o の **decode_nal_units のスタートコード探索**
  (h264.c:4190、buf[buf_index+2] の読み) と特定。ソース上の
  ループ条件は buf_index+3 < next_avc で有界のはずなので、
  渡された buf_size と実バッファの不一致が疑わしい。
- MPC-HC の eip=0 (MP4/AVCC 経路) と WMP の範囲外読み
  (AVI/Annex B 経路) は別件の可能性あり。
- 次: WMP 分の DrWatson ログ (レジスタ ecx/edi とスタック) で
  実際に読もうとしたアドレスと長さを確認する。

## 第62弾 (2026-11-23 朝) — Wine 打ち切りと QEMU への移行、h264test 結果の先読み

- Wine の落下はローダ相性と結論し深追い打ち切り (2026-11-22 夜)。
  自作 dll は Win2000 実機では正常にロード・動作する。次は QEMU 上の
  Win2000 で h264test.exe を回す (ユーザー決定)。再開パックは
  2026-11-23 06:49 JST に送付済み (h264test.exe / 自作・公式
  ffmpeg.dll / stream.h264、判定は fault address − ECX と buf_size
  の比較)。
- WMP 分の DrWatson ログは結局残らなかった (WMP9 は独自のエラー報告
  で、drwtsn32.log の 2 本目に WMP の落下は入っていない)。第61弾の
  「次: WMP 分のログで確認」はこのため実施不能で、h264test の結果が
  その代わりになる。
- 今朝の原典確認: FFmpeg 0.10.16 の libavcodec/h264.c の
  decode_nal_units を読んだ。Annex B 経路のスタートコード探索は
  `for(; buf_index + 3 < next_avc; buf_index++)` で
  `buf[buf_index]==0 && buf[buf_index+1]==0 && buf[buf_index+2]==1`
  を見る (next_avc は Annex B では buf_size)。つまりこの読みは
  buf_index+2 <= buf_size-2 の範囲にしか届かず、渡された buf_size
  の内側からはみ出せない。AVC 経路も nalsize に
  `nalsize > buf_size - buf_index` のガードがあり同じく buf_size
  を超えない。
- したがって WMP でこの読みが fault したという事実だけで、渡された
  buf_size が実バッファより大きかったことが決まる。デコーダ内部の
  バグではなく、ffdshow 側が avcodec_decode_video2 へ渡す
  (data, size) の組み立てが容疑者になる。
- h264test の結果の読み方 (先読み):
  - DONE ならデコーダは無罪確定で、容疑は ffdshow のパケット渡し
    (size の過大申告) に絞れる。次は trunk の
    src/codecs/TvideoCodecLibavcodec.cpp で、avcodec_decode_video2
    へ渡すバッファと size の出所を見る。
  - 同じ探索で fault すれば、h264test は size=fsize の正確な値を
    渡しているので、ffdshow 側の説明は崩れ、デコーダ側 (同梱スナッ
    プショットの当該ループ) が容疑者になる。
- 留意: ffdshow 同梱のスナップショットの行番号は 0.10.16 と違う
  (当該箇所は h264.c:4190)。上の有界性の議論はループの形が同じで
  あることに依存するので、trunk 側で `buf_index + 3 < next_avc`
  の条件を 1 行確認するのが最終の裏付けになる。
- 別経路の注意 (上とは無関係): ff_h264_decode_nal の高速読み
  (AV_RN64A) は length より最大 7 バイト先まで読むため、パケット
  バッファには FF_INPUT_BUFFER_PADDING_SIZE (64 バイト) のパディ
  ングが要る。h264test は PAD=64 でこれを満たしている。WMP の
  fault は探索ループの命令だったので、このパディングの話では説明
  できない (区別しておくこと)。

## 第63弾 (2026-11-23 朝) — 同梱スナップショット r4422 実物のループ確認で裏付け完了

- 第62弾で残っていた「trunk 側で `buf_index + 3 < next_avc` を
  1 行確認する」件、同梱スナップショットそのもの (r4422) で確認済み。
  SourceForge のプロジェクト名は複数形の ffdshow-tryouts ではなく
  単数形の **ffdshow-tryout** (それが 404 の原因だった)。
  r4422 は「Bump version」(clsid2、2012-04-08)、h264.c 自体は
  そのリビジョンでは無変更で r4421「Updated Libav」と同一。
- trunk/src/ffmpeg/libavcodec/h264.c (r4422、4619行):
  decode_nal_units は 4139行目。Annex B のスタートコード探索は
  **4190行目** `for (; buf_index + 3 < next_avc; buf_index++)` で、
  4192〜4194行目が buf[buf_index] / buf[buf_index+1] /
  buf[buf_index+2] の判定。ユーザーの objdump 照合の h264.c:4190
  と行番号まで一致する。ループの直後は 4197行目
  `if (buf_index + 3 >= buf_size)`、4198行目 `break;`。
  HEAD (r4534) 版も同じトークン列で、行番号だけが違う (3677行目)。
- したがって「渡された buf_size の内側からはみ出せない」という
  第62弾の議論は、0.10.16 での確認ではなく同梱実物での確認になった。
  h264test の読み方はこれで確定: DONE なら ffdshow 側の size 過大
  申告が決まり。同じ探索で fault した場合は、正しい size を渡して
  いるのにループがはみ出すことはあり得ないので、容疑はループ自体
  ではなく buf ポインタ側か別の読みに移る (第62弾の二択の後半は
  この意味で読み替えること)。

## 第64弾 (P77): TU 解禁の漏れ — Tconvert の変種分離

- QEMU 上の Win2000 + WMP で out1 (H.264 320x180、音声なし) を再生
  すると ffdshow.ax 内オフセット 0x349518 で落下。VA は
  0x67149518 (ImageBase 0x66E00000 + 0x349518。加算の繰り上がりを
  落として 0x66E349518 と誤読した回があり、訂正済み)。
  nm で TconvertYV12<Tmmx>::yv12_to_yuy2 の +0x78 と特定され、
  命令は movd %xmm3 = SSE2 だった。
- 機構: Tconvert.cpp は Tmmx / Tmmxext / Tsse2 の3変種を同一 TU で
  実体化し、Makefile の TU 単位 -msse2 解禁により GCC が MMX 版の
  通常コード (convert_yv12.h の movd ラッパー) にも xmm を使った。
  実行時ディスパッチ (Tconfig::cpu_flags の FF_CPU_SSE2 /
  FF_CPU_MMXEXT) は正しく Tmmx を選んでいた。CMOV の -mno-cmov と
  同型の漏れで、検証の門は CMOV しか見ていなかったのが穴。
- 対策 (変種分離): Tsse2 / Tmmxext 変種へのポインタ取得を専用 TU
  に切り出し、Tconvert.cpp 本体はベースライン (-mmmx のみ) に戻す。
  新規ファイルは src/convert/Tconvert_simd.h (関数ポインタ型と
  getter 宣言)、Tconvert_sse2.cpp、Tconvert_mmxext.cpp。
  Tconvert.cpp は include 1 行追加と、ディスパッチ 2 ブロック
  (yv12_to_yuy2 / yuy2_to_yv12) の差し替え。Makefile は SRCS_CPP に
  2 ファイル追加、Tconvert.o を SSE2 行から MMX 行へ移動、新 TU に
  各自のフラグ行を追加。
- 既知の不完全さ: Tconvert_mmxext.o は -msse で組むため内部に xmm
  中継が残る可能性があるが、Pentium MMX / K6 では FF_CPU_MMXEXT
  自体が立たず Tmmxext は選択されないため、対象範囲では実害なし。
- 検証: 全再ビルド後、Tconvert.o の objdump で xmm が 0 件である
  こと、従来の check (輸入・輸出・CMOV) の全通過、QEMU の WMP で
  out1 が変換を通過すること。残りの解禁 TU 群 (字幕、imgFilters、
  TaudioFilter、ffdshow_converters 等) は census の上で同型の分離を
  順に適用する (次弾以降)。

## 第65弾 (P78): P77 の訂正 — convert_yuy2.h は SSE2 なしでは通らない

- P77 を適用して Tconvert.o を -mmmx のみに落とすとコンパイルが
  失敗した。理由は私の見落とし: Tconvert.cpp は convert_yuy2.h
  (YUY2→RGB) も include しており、Tmmx_ConvertYUY2toRGB の SSE2
  メンバ (GET_Y_SSE2、YUV2RGB_INNER_LOOP_SSE2 等) は __m128i 版の
  ラッパー (simd.h の SSE2 区画) なしではコンパイル自体が通らない。
  このクラスは入口関数 mmx_ConvertYUY2toRGB の内側で実行時分岐
  する設計で、TU を落とすと SSE2 側が消えるのではなく全体が死ぬ。
- 訂正した方針: Tconvert.o は SSE2 解禁に戻す。その代わり
  TconvertYV12<Tmmx> も getter 方式に統一し、Tconvert.cpp 内から
  TconvertYV12 の全変種への直接参照を消す。Tmmx の実体化は新規
  Tconvert_mmx.cpp (-mmmx) だけで起きる。P77 の sse2 / mmxext の
  getter TU はそのまま使う。Tconvert_simd.h の getter 宣言は 6 本
  (mmx / mmxext / sse2 × 2方向) になった。
- 残る未確認: Tconvert.o 内の Tmmx_ConvertYUY2toRGB の MMX ループ
  と Tmmx_ConvertRGBtoYUY2 にも xmm 混入があるか。旧 bin/ffdshow.ax
  への関数単位 census (xmm を含む関数名の一覧から Tmmx 系を抽出)
  で確認し、混入があれば convert_yuy2.h 側の分離を次弾で扱う。

## 第66弾: xmm census の結果 — 漏れは全解禁 TU に及ぶ

- 旧 bin/ffdshow.ax に対して「xmm を含む関数」のうち名前に Tmmx を
  含むもの (Tmmxext を除く) を抽出すると 60 関数が該当した。
  TU 単位の SSE/SSE2 解禁が、その TU で実体化される Tmmx 変種の
  通常コードへ xmm を漏らす現象は、Tconvert 固有ではなく全解禁 TU
  共通の系統的なものと確定した。MMX のみの行 (476) の TU は
  1 件も出てこないことも整合する。
- 内訳: TconvertYV12<Tmmx> 4 (P78 で対処中)、
  TffdshowConverters2 の convert_simd<Tmmx> 系 17 (既定の変換経路、
  次の本命)、Tmmx_ConvertRGBtoYUY2 の入口 2 (MMX 専用クラスなので
  混入確定)、Tmmx_ConvertYUY2toRGB の入口 8 (SSE2 ループが
  forceinline で同じ関数に同居するため、この census では判定不能)、
  yv12_to_yv12_simd<Tmmx> 1、フィルタ群 22 (Blur / Luma / Noise /
  Bitmap / Chroma / Xsharp / Unsharp / Colorize / Warpsharp /
  GradualDenoise / FramerateDoubler / asharp / nic_dering /
  NoiseMplayer、いずれも有効化時のみ通る)、字幕群 3
  (TspuImageSimd 2、TrenderedTextSubtitleWord 1、字幕表示時のみ)。
- 対処の優先順位: (1) P78 完走と同 census での消滅確認、
  (2) ffdshow_converters2、(3) convert_yuy2 の入口分割、
  (4) フィルタ・字幕群を同じ型で一括。
- 留意: この census は名前に Tmmx を含むものだけが対象。変種名が
  名前に出ない素の関数にも同種の漏れがあり得るため、最終的には
  xmm を含む全関数の棚卸しが要る (P78 後の .ax で行う方が
  正規の SSE 関数との見分けが付けやすい)。

## 第67弾: uuidof_shim.o がリンク時に無い — Makefile の旧事故が露出

- P78 の全再ビルド (rm -rf obj) でリンクが失敗した。ld が
  obj/MinGW32/ffdshow/uuidof_shim.o を見つけられない。
- 原因は第58弾 (win2k_compat 追加) のときに起きた Makefile 編集
  事故の残骸だった。重複除去の復旧が不完全で、uuidof_shim.o の
  ルールはレシピを失って空になり、win2k_compat.o のレシピは
  二重化していた。今まで露出しなかったのは、事故以前に作られた
  uuidof_shim.o が残っていて make が作り直す必要がなかったため。
  rm -rf obj で初めて顕在化した。
- 修正: uuidof_shim.o ルールにレシピ (mkdir + CC の2行) を復旧し、
  win2k_compat.o の二重化したレシピを1組に戻した。こちら側の
  Makefile.mingw は修正済み。他の shim ルール (guids、
  interlocked_shim) は正常なことを確認した。
- 教訓: ルールの重複除去をした後は、対象ルール全部について
  「ヘッダの直後がタブ行か」を機械的に確認すること。
