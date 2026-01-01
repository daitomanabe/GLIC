# GLIC - GLitch Image Codec

An experimental image codec designed to create controlled glitch effects through intentional manipulation of compression algorithms.

## Overview

GLIC transforms traditional image compression techniques into creative tools for generating unique visual artifacts. By exposing and manipulating prediction errors, quantization, and wavelet transforms, artists can achieve consistent, reproducible glitch aesthetics.

## Implementations

### Processing (GUI) - Original

The original implementation built with Processing provides an interactive visual interface for real-time experimentation.

- Real-time parameter adjustment
- Visual preset browser with 120+ presets
- Immediate preview
- Save/load configurations

Location: `implementation/` (Processing .pde files)

### C++ (CLI) - Ported Version

The Processing version has been fully ported to C++ as a command-line tool, with additional features added.

**Extended Features (C++ only):**
- 8 additional prediction methods (SPIRAL, NOISE, GRADIENT, MIRROR, WAVE, CHECKERBOARD, RADIAL, EDGE)
- 3 additional encoding methods (DELTA, XOR, ZIGZAG)
- 6 post-processing effects (pixelate, scanline, chromatic, dither, posterize, glitch)
- Batch processing support
- Cross-platform (macOS, Linux, Windows)

Location: `implementation/glic_cpp/`

## Features

| Category | Count | Examples |
|----------|-------|----------|
| Prediction Methods | 24 | PAETH, SPIRAL, WAVE, NOISE, RADIAL, EDGE |
| Encoding Methods | 6 | RAW, PACKED, RLE, DELTA, XOR, ZIGZAG |
| Color Spaces | 16 | RGB, HSB, HWB, LAB, YUV, XYZ, YCbCr |
| Wavelet Transforms | 25+ | Haar, Daubechies, Symlet, Coiflet |
| Post Effects | 6 | Pixelate, Scanline, Chromatic, Dither |

## Quick Start (C++ CLI)

### Build

```bash
cd implementation/glic_cpp
mkdir build && cd build
cmake ..
cmake --build .
```

### Usage

```bash
# Basic encode/decode
./glic encode photo.png glitched.glic
./glic decode glitched.glic result.png

# Creative examples
./glic encode photo.png out.glic --prediction SPIRAL --quantization 180
./glic encode photo.png out.glic --colorspace YUV --prediction WAVE
./glic decode out.glic result.png --effect scanline --effect chromatic
```

## Documentation

- [Project Website](webpage/index.html)
- [Google Docs](https://docs.google.com/document/d/1cdJvEmSKNAkzkU0dFUa-kb_QJB2ISQg-QfCqpHLFlck/edit)
- [C++ CLI Documentation](implementation/glic_cpp/README.md)

## License

MIT License
