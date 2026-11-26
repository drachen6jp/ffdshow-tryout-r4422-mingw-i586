# ffdshow-tryout r4422 — MinGW / i586 / Windows 2000 build

Unofficial build of **ffdshow-tryout r4422** (`ffdshow.ax`) and its bundled
FFmpeg snapshot (`ffmpeg.dll`), cross-compiled with MinGW-w64 GCC for
**i586 (Pentium / K6 class CPUs)** and plain **Windows 2000** — no extended
kernel, no XP-only APIs, no Pentium Pro+ instructions (CMOV-free) on the
execution paths used.

The upstream MSVC binaries cannot run in this environment (the official
`ffdshow.ax` imports XP-era APIs; the official `ffmpeg.dll` contains
thousands of CMOV instructions). This build replaces both files as a
matched pair.

* 日本語の説明は後半にあります。

## Verified environment (2026-11-25)

- PC-9821V166, Pentium MMX 166 MHz, Windows 2000 SP4 (NEC98 edition)
- GeForce 8500 GT PCI (Kuroutoshikou GF8500GT-P256H), ForceWare **163.75**
- mplayerc (classic Media Player Classic) + this ffdshow build:
  **720p H.264 with DXVA plays correctly** (VMR9 and Overlay renderers)

Notes from testing:

- 1080p H.264 DXVA does **not** work in this environment (decoder creation
  fails during DXVA negotiation). Software decoding works.
- ForceWare 169.21 and 175.16 fail DXVA negotiation even at 720p in this
  environment. 163.75 is the confirmed driver.
- Playback smoothness can also depend on disk transfer speed. In one
  test environment a PIO-mode HDD could not feed 720p fast enough
  (reading from USB 1.1 storage worked better); in another environment
  the same kind of PIO-mode HDD played 720p without starving. It
  depends on the environment.

## Installation

You need the official ffdshow-tryout r4422 package as the base
(`ffdshow_rev4422_20120409.exe`); this release only supplies the two
rebuilt files, `ffdshow.ax` and `ffmpeg.dll`. The procedure differs
by OS and involves replacing files by hand.

### Windows XP

The official installer works on XP, including registration.

1. Install ffdshow-tryout r4422 with the official installer.
2. Replace `ffmpeg.dll` in the install folder with the one from
   this release.

That is all. The official `ffdshow.ax` runs fine on XP; on
Pentium/K6-class CPUs the blocker is the official `ffmpeg.dll`,
which is full of CMOV instructions. (You may also replace
`ffdshow.ax` with the one from this release — the two files here
are a matched pair.)

### Windows 2000

The official `ffdshow.ax` cannot be registered on Windows 2000 (it
imports XP-era APIs), so installing the official package alone gets
you nowhere. You first need the official package's files *without*
installing them:

- either install r4422 on an XP machine and copy the installed
  folder over, or
- extract the official installer — it is an Inno Setup package —
  with an extraction tool such as innounp.

Then:

1. Replace both `ffdshow.ax` and `ffmpeg.dll` with the ones from
   this release. On Windows 2000 the pair must be replaced
   together.
2. Register the filter: `regsvr32 ffdshow.ax`

Extended kernels for Windows 2000 also exist. This build is made
for Windows 2000 without an extended kernel, but if you try it in
combination with one, a report on how it went would be appreciated.

### Using DXVA

DXVA (hardware-accelerated decoding) is **disabled by default**
after registration. To use it, enable it in the ffdshow settings:

- Windows XP: the configuration can be opened directly from the
  Start menu (ffdshow → Video decoder configuration).
- Windows 2000: no installer was run, so there is no configuration
  shortcut. Play a file in a player (e.g. mplayerc), then open the
  filter's property page from the player's menu: right-click →
  Filters → ffdshow Video Decoder.

### Player

Use a player that itself runs on your CPU (the classic `mplayerc.exe`
works on Pentium-class machines; MPC-HC 1.3.x contains CMOV
instructions and does not).

## Changes from upstream r4422 (GPL §2(a) notice)

This is a modified version of ffdshow-tryout r4422. The modifications,
made in November 2026, are:

- New MinGW build system (`src/Makefile.mingw`) replacing the MSVC projects
- Compatibility shims (`mingw_shims/`): ATL subset, `__uuidof`, interlocked
  intrinsics, Vista/XP API replacements
- Windows 2000 compatibility layer (`win2k_compat.c`): `GetThreadId` and
  condition variables implemented with Windows 2000-era APIs only
- i586 code generation throughout (`-march=i586`, CMOV disabled,
  `HAVE_CMOV` off); SIMD code split into per-variant translation units so
  SSE2 instructions cannot leak into generic code paths
- The single SEH (`__try/__except`) use in `TDXVADecoderH264.cpp` replaced
  with a plain copy (MinGW GCC cannot compile MSVC SEH)
- Boost 1.49 `tss_pe.cpp` adjusted for MinGW

Full build documentation (in Japanese) is in `ffdshow_mingw_build.md` in
this repository, including the GCC patch (`gcc-14-mno-cmov.patch`) needed
to make GCC 14 honour `-mno-cmov`.

## A request

This build uses MinGW because that is the toolchain at hand. Upstream
ffdshow is an MSVC project, and an MSVC build would be the more natural
artifact. If you have MSVC and can build r4422 so that it registers and
runs on plain Windows 2000 — ideally also usable on Pentium/K6, i.e.
without CMOV — please do, and consider publishing it. A report that
such a build exists would already be welcome.

## License

This work is distributed under the **GNU General Public License, version 2
or (at your option) any later version** — see `licenses/GPL-2.0.txt`.

Component licenses:

- ffdshow-tryout and FFmpeg (incl. libpostproc): GPL v2 or later
- Boost 1.49 (thread, statically linked): Boost Software License 1.0 —
  see `licenses/Boost_Software_License_1.0.txt`
- MinGW-w64 runtime (statically linked): see
  `licenses/COPYING.MinGW-w64-runtime.txt`
- zlib: zlib License (no notice required for binary distribution)

The complete corresponding source code for the distributed binaries is
this repository, including all patches and the build documentation.

**No warranty.** These binaries are provided "as is", without warranty of
any kind, as stated in the GPL. Use at your own risk.

Codec patents (H.264 etc.) are a separate matter from copyright licensing;
this distribution grants no patent licenses.

Built and tested by Ryu (drachen6jp), with AI assistance (Muse).

---

# ffdshow-tryout r4422 — MinGW / i586 / Windows 2000 ビルド (日本語)

ffdshow-tryout **r4422** (`ffdshow.ax`) と、その内蔵 FFmpeg スナップ
ショット (`ffmpeg.dll`) を、MinGW-w64 GCC で **i586 (Pentium / K6 級
CPU)** 向けにビルドし直した非公式ビルドです。**素の Windows 2000**
(拡張カーネルなし) で動作します。XP 世代の API は使わず、実行経路に
CMOV 等の Pentium Pro 以降の命令は含まれません。

公式の MSVC バイナリはこの環境では動きません (公式 `ffdshow.ax` は
XP 世代の API を輸入し、公式 `ffmpeg.dll` には CMOV が数千件含まれ
ます)。本ビルドはこの2ファイルを対で置き換えるものです。

## 実機確認環境 (2026-11-25)

- PC-9821V166、Pentium MMX 166MHz、Windows 2000 SP4 (NEC98 版)
- GeForce 8500 GT PCI (玄人志向 GF8500GT-P256H)、ForceWare **163.75**
- 古典 mplayerc.exe + 本ビルドで **720p H.264 が DXVA で再生可能**
  (VMR9 / Overlay 両対応)

実測で分かった注意点:

- 1080p の H.264 DXVA はこの環境では動作しません (DXVA 交渉の
  デコーダ生成段で失敗します)。ソフトデコードは可能です
- ForceWare 169.21 と 175.16 はこの環境では 720p でも DXVA 交渉に
  失敗します。動作確認済みは 163.75 です
- 再生の滑らかさはディスク転送速度に依存することもあります。
  ある環境では PIO モードの HDD で 720p の供給が追いつきません
  でしたが (USB 1.1 ストレージからの読み込みで改善)、別の環境
  では PIO の HDD でも詰まらずに再生できました。環境によります

## インストール

土台として公式の ffdshow-tryout r4422 パッケージ
(`ffdshow_rev4422_20120409.exe`) が必要です。本リリースが供給する
のは再ビルドした `ffdshow.ax` と `ffmpeg.dll` の2ファイルだけです。
手順は OS で異なり、ファイルの入れ替え作業を行います。

### Windows XP の場合

公式インストーラは XP では正常に動作し、登録まで済みます。

1. 公式インストーラで ffdshow-tryout r4422 をインストールする
2. インストール先フォルダの `ffmpeg.dll` を、本リリースのものに
   入れ替える

これだけです。公式 `ffdshow.ax` は XP では問題なく動きます。
Pentium / K6 級 CPU での障害は、CMOV を大量に含む公式
`ffmpeg.dll` の方です (本リリースの `ffdshow.ax` に入れ替えても
構いません。こちらの2ファイルは対になっています)。

### Windows 2000 の場合

公式 `ffdshow.ax` は XP 世代の API を輸入しているため
Windows 2000 では登録できません。公式パッケージをインストール
するだけでは進みません。まず、インストールせずに公式パッケージ
の中身のファイルを取り出す必要があります:

- XP 機で r4422 をインストールし、インストール先フォルダを
  そのまま持ってくる、または
- 公式インストーラ (Inno Setup 形式) を、innounp 等の解凍
  ソフトで解凍する

そのうえで:

1. `ffdshow.ax` と `ffmpeg.dll` の両方を本リリースのものに
   入れ替える (Windows 2000 では対で入れ替えが必要です)
2. `regsvr32 ffdshow.ax` で登録する

なお、Windows 2000 用の拡張カーネルも存在します。本ビルドは
拡張カーネルなしの Windows 2000 向けですが、拡張カーネルと
併用した場合の動作報告があれば助かります。

### DXVA を使う場合

DXVA (ハードウェア再生支援) は登録直後は無効です。使う場合は
ffdshow の設定で有効にしてください。

- Windows XP: スタートメニューの ffdshow 設定
  (Video decoder configuration) から直接開けます
- Windows 2000: インストーラを使っていないため設定の
  ショートカットがありません。プレイヤー (mplayerc 等) で
  ファイルを再生し、右クリック→「フィルタ」→
  「ffdshow Video Decoder」からプロパティを開いてください

### プレイヤーについて

プレイヤー自体も CPU で動くものを使ってください (古典
mplayerc.exe は Pentium 級で動作します。MPC-HC 1.3.x は CMOV を
含むため Pentium では動作しません)

## 上流 r4422 からの変更点 (GPL §2(a) の明示)

本リポジトリは ffdshow-tryout r4422 の改変版です (2026年11月)。
変更内容は上記英語セクションおよび同梱のビルド手順書
`ffdshow_mingw_build.md` に記載しています。要点は MinGW ビルド系の
新設、互換シム群、Windows 2000 互換層、i586 化 (CMOV 無効化と
SIMD の翻訳単位分離)、SEH の置換、Boost の MinGW 調整です。

## お願い

本ビルドは、手元のツールチェインの都合で MinGW 製です。上流の
ffdshow は MSVC のプロジェクトであり、MSVC 製ビルドの方が本来の
形だと思います。MSVC をお持ちで、r4422 を素の Windows 2000 で
登録・動作する形にビルドできる方がいれば、ぜひお願いします
(できれば Pentium / K6 でも使える、CMOV を含まない形だと
なお助かります)。公開していただければ幸いですし、できたという
報告だけでも歓迎します。

## ライセンス

**GPL version 2 or later** で配布します (`licenses/GPL-2.0.txt`)。
Boost 1.49 と MinGW-w64 ランタイムのライセンス文書も `licenses/`
に同梱しています。配布バイナリに対応するソース一式は本リポジトリ
そのものです。無保証です。コーデック特許は著作権ライセンスとは
別問題であり、本配布に特許の実施権は含まれません。

製作・実機検証: Ryu (drachen6jp)。開発には AI (Muse) の支援を
受けました。
