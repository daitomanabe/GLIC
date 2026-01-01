# GLIC C++ - GLitch Image Codec

GLIC (GLitch Image Codec) の C++ コマンドライン実装版です。

**オリジナルの Processing (Java) 版から完全にポートされ、さらに新しいグリッチ効果が追加されています。**

## 特徴

- Processing版の全機能をC++17で再実装
- クロスプラットフォーム対応 (macOS, Linux, Windows)
- コマンドラインインターフェース
- 24種類の予測アルゴリズム（8種類追加）
- 6種類のエンコーディング方式（3種類追加）
- 6種類のポストプロセッシングエフェクト（新機能）

## ビルド方法

```bash
cd implementation/glic_cpp
mkdir build && cd build
cmake ..
cmake --build .
```

ビルドが完了すると `build/glic` が生成されます。

## 使用方法

### 基本コマンド

```bash
# エンコード（画像 → GLIC形式）
./glic encode input.png output.glic [options]

# デコード（GLIC形式 → 画像）
./glic decode input.glic output.png [options]
```

### エンコードオプション

| オプション | デフォルト | 説明 |
|-----------|----------|------|
| `--colorspace <name>` | HWB | 色空間 |
| `--min-block <size>` | 2 | 最小ブロックサイズ |
| `--max-block <size>` | 256 | 最大ブロックサイズ |
| `--threshold <value>` | 15 | セグメンテーション閾値 |
| `--prediction <method>` | PAETH | 予測方式 |
| `--quantization <value>` | 110 | 量子化値 (0-255) |
| `--clamp <method>` | none | クランプ方式 (none, mod256) |
| `--wavelet <name>` | SYMLET8 | ウェーブレット |
| `--transform <type>` | fwt | 変換タイプ (fwt, wpt) |
| `--scale <value>` | 20 | 変換スケール |
| `--encoding <method>` | packed | エンコード方式 |
| `--border <r,g,b>` | 128,128,128 | 境界色 (RGB) |

### デコードオプション（ポストエフェクト）

| オプション | デフォルト | 説明 |
|-----------|----------|------|
| `--effect <name>` | - | エフェクト適用（複数指定可） |
| `--effect-intensity <n>` | 50 | エフェクト強度 (0-100) |
| `--effect-blocksize <n>` | 8 | ブロックサイズ (pixelate, glitch用) |
| `--effect-offset <x,y>` | 2,0 | 色収差オフセット |
| `--effect-levels <n>` | 4 | ポスタライズレベル数 |

## 使用例

### 基本

```bash
# シンプルなエンコード・デコード
./glic encode photo.png glitched.glic
./glic decode glitched.glic result.png
```

### 予測方式を変えてグリッチ効果を調整

```bash
# スパイラル予測（渦巻き状のアーティファクト）
./glic encode photo.png out.glic --prediction SPIRAL

# 波形予測（波状のアーティファクト）
./glic encode photo.png out.glic --prediction WAVE

# ノイズ予測（ノイズ状のアーティファクト）
./glic encode photo.png out.glic --prediction NOISE

# チェッカーボード予測（市松模様のアーティファクト）
./glic encode photo.png out.glic --prediction CHECKERBOARD
```

### グリッチ効果を強める

```bash
# 閾値を下げて量子化を上げる
./glic encode photo.png out.glic --threshold 5 --quantization 200

# Haarウェーブレットでブロックノイズを強調
./glic encode photo.png out.glic --wavelet HAAR --quantization 180

# 複数の設定を組み合わせ
./glic encode photo.png out.glic --colorspace YUV --prediction RADIAL --quantization 150
```

### ポストエフェクトを適用

```bash
# スキャンライン効果
./glic decode out.glic result.png --effect scanline

# 色収差効果
./glic decode out.glic result.png --effect chromatic --effect-offset 5,2

# 複数エフェクトの組み合わせ
./glic decode out.glic result.png --effect posterize --effect scanline --effect chromatic

# ピクセル化 + グリッチシフト
./glic decode out.glic result.png --effect pixelate --effect-blocksize 16 --effect glitch
```

## 機能一覧

### 色空間 (16種類)

| 名前 | 説明 |
|------|------|
| RGB | 標準RGB |
| HSB | 色相・彩度・明度 |
| HWB | 色相・白さ・黒さ |
| OHTA | 最適色相変換 |
| CMY | シアン・マゼンタ・イエロー |
| XYZ | CIE XYZ |
| YXY | CIE Yxy |
| LAB | CIE L*a*b* |
| LUV | CIE L*u*v* |
| HCL | 色相・彩度・輝度 |
| YUV | アナログビデオ |
| YPbPr | HDビデオ |
| YCbCr | デジタルビデオ |
| YDbDr | SECAMビデオ |
| GS | グレースケール |
| R-GGB-G | ベイヤーパターン |

### 予測アルゴリズム (24種類)

#### 基本予測 (16種類 - Processing版互換)

| 名前 | 説明 |
|------|------|
| NONE | 予測なし |
| CORNER | 左上コーナー値 |
| H | 水平予測（左端から） |
| V | 垂直予測（上端から） |
| DC | 境界の平均値 |
| DCMEDIAN | DC + メディアン |
| MEDIAN | メディアン予測 |
| AVG | 平均値予測 |
| TRUEMOTION | 動き補償予測 |
| PAETH | PNG方式予測 |
| LDIAG | 左対角線予測 |
| HV | 水平/垂直選択 |
| JPEGLS | JPEG-LS方式予測 |
| DIFF | 二次差分予測 |
| REF | 参照ブロック探索 |
| ANGLE | 角度ベース予測 |

#### 新規追加予測 (8種類)

| 名前 | 説明 | グリッチ特性 |
|------|------|------------|
| SPIRAL | 中心からスパイラル状に予測 | 渦巻き状のアーティファクト |
| NOISE | 位置ハッシュベースのノイズ | ノイズ状のテクスチャ |
| GRADIENT | 4コーナーからバイリニア補間 | グラデーション状の歪み |
| MIRROR | ミラー/反転予測 | 反転パターン |
| WAVE | 正弦波ベースの変位 | 波状のうねり |
| CHECKERBOARD | 市松模様で交互予測 | チェッカーパターン |
| RADIAL | 中心からの放射状グラデーション | 放射状のアーティファクト |
| EDGE | エッジ検出ベースの予測 | エッジ強調効果 |

#### メタ予測

| 名前 | 説明 |
|------|------|
| SAD | 最適予測を自動選択（最小誤差） |
| BSAD | 最悪予測を自動選択（最大誤差 = 最大グリッチ） |
| RANDOM | ランダム予測選択 |

### エンコード方式 (6種類)

#### 基本方式 (3種類 - Processing版互換)

| 名前 | 説明 |
|------|------|
| raw | 直接保存 |
| packed | ビットパック圧縮 |
| rle | ランレングス符号化 |

#### 新規追加方式 (3種類)

| 名前 | 説明 |
|------|------|
| delta | 差分エンコーディング（連続値の差分を保存） |
| xor | XORエンコーディング（前の値とのXOR） |
| zigzag | Zigzagエンコーディング（符号付き値の効率的保存） |

### ウェーブレット変換

| ファミリー | バリエーション |
|-----------|--------------|
| Haar | HAAR |
| Daubechies | DB2, DB3, DB4, DB5, DB6, DB7, DB8, DB9, DB10 |
| Symlet | SYM2, SYM3, SYM4, SYM5, SYM6, SYM7, SYM8, SYM9, SYM10 |
| Coiflet | COIF1, COIF2, COIF3, COIF4, COIF5 |

### ポストプロセッシングエフェクト (6種類) - 新機能

デコード時に適用できる後処理エフェクトです。

| 名前 | 説明 | 関連オプション |
|------|------|--------------|
| pixelate | ピクセル化（モザイク効果） | `--effect-blocksize` |
| scanline | スキャンライン（CRTモニター風） | `--effect-intensity` |
| chromatic | 色収差（RGBチャンネルオフセット） | `--effect-offset` |
| dither | ディザリング（Bayerパターン） | `--effect-intensity` |
| posterize | ポスタライズ（色数削減） | `--effect-levels` |
| glitch | グリッチシフト（ランダムな行ずれ） | `--effect-blocksize` |

## プロジェクト構成

```
implementation/glic_cpp/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── main.cpp              # CLI エントリーポイント
│   ├── glic.hpp/cpp          # メインコーデック
│   ├── config.hpp            # 設定・定数定義
│   ├── planes.hpp/cpp        # 画像プレーン管理
│   ├── colorspaces.hpp/cpp   # 16種の色空間変換
│   ├── segment.hpp/cpp       # クワッドツリーセグメンテーション
│   ├── prediction.hpp/cpp    # 24種の予測アルゴリズム
│   ├── quantization.hpp/cpp  # 量子化
│   ├── wavelet.hpp/cpp       # ウェーブレット変換
│   ├── encoding.hpp/cpp      # 6種のエンコード方式
│   ├── effects.hpp/cpp       # 6種のポストエフェクト
│   └── bitio.hpp/cpp         # ビット単位I/O
└── external/
    ├── stb_image.h           # 画像読み込み (自動ダウンロード)
    └── stb_image_write.h     # 画像書き込み (自動ダウンロード)
```

## 依存関係

- C++17 以上
- CMake 3.14 以上
- stb_image / stb_image_write (ビルド時に自動ダウンロード)

## Processing版との違い

| 機能 | Processing版 | C++版 |
|------|-------------|-------|
| インターフェース | GUI | CLI |
| 予測アルゴリズム | 16種類 | 24種類（+8種類） |
| エンコード方式 | 3種類 | 6種類（+3種類） |
| ポストエフェクト | なし | 6種類（新機能） |
| プリセット | ファイル読み込み | CLIオプションで指定 |

## ライセンス

MIT License
