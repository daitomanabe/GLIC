#include "effects.hpp"
#include <cmath>
#include <algorithm>
#include <random>

namespace glic {

std::string effectName(EffectType et) {
    switch (et) {
        case EffectType::NONE: return "NONE";
        case EffectType::PIXELATE: return "PIXELATE";
        case EffectType::SCANLINE: return "SCANLINE";
        case EffectType::CHROMATIC_ABERRATION: return "CHROMATIC_ABERRATION";
        case EffectType::DITHER: return "DITHER";
        case EffectType::POSTERIZE: return "POSTERIZE";
        case EffectType::GLITCH_SHIFT: return "GLITCH_SHIFT";
        default: return "NONE";
    }
}

EffectType effectFromName(const std::string& name) {
    if (name == "PIXELATE") return EffectType::PIXELATE;
    if (name == "SCANLINE") return EffectType::SCANLINE;
    if (name == "CHROMATIC_ABERRATION" || name == "CHROMATIC") return EffectType::CHROMATIC_ABERRATION;
    if (name == "DITHER") return EffectType::DITHER;
    if (name == "POSTERIZE") return EffectType::POSTERIZE;
    if (name == "GLITCH_SHIFT" || name == "GLITCH") return EffectType::GLITCH_SHIFT;
    return EffectType::NONE;
}

void applyEffect(
    std::vector<Color>& pixels,
    int width,
    int height,
    const EffectConfig& config
) {
    switch (config.type) {
        case EffectType::PIXELATE:
            effectPixelate(pixels, width, height, config.blockSize);
            break;
        case EffectType::SCANLINE:
            effectScanline(pixels, width, height, config.intensity);
            break;
        case EffectType::CHROMATIC_ABERRATION:
            effectChromaticAberration(pixels, width, height, config.offsetX, config.offsetY);
            break;
        case EffectType::DITHER:
            effectDither(pixels, width, height, config.intensity);
            break;
        case EffectType::POSTERIZE:
            effectPosterize(pixels, width, height, config.levels);
            break;
        case EffectType::GLITCH_SHIFT:
            effectGlitchShift(pixels, width, height, config.blockSize, config.seed);
            break;
        default:
            break;
    }
}

void applyEffects(
    std::vector<Color>& pixels,
    int width,
    int height,
    const std::vector<EffectConfig>& effects
) {
    for (const auto& effect : effects) {
        applyEffect(pixels, width, height, effect);
    }
}

// PIXELATE - Reduce resolution in blocks
void effectPixelate(std::vector<Color>& pixels, int w, int h, int blockSize) {
    if (blockSize < 2) return;

    for (int by = 0; by < h; by += blockSize) {
        for (int bx = 0; bx < w; bx += blockSize) {
            // Calculate average color for block
            int sumR = 0, sumG = 0, sumB = 0, sumA = 0;
            int count = 0;

            for (int y = by; y < std::min(by + blockSize, h); y++) {
                for (int x = bx; x < std::min(bx + blockSize, w); x++) {
                    Color c = pixels[static_cast<size_t>(y * w + x)];
                    sumR += getR(c);
                    sumG += getG(c);
                    sumB += getB(c);
                    sumA += getA(c);
                    count++;
                }
            }

            Color avg = makeColor(
                static_cast<uint8_t>(sumR / count),
                static_cast<uint8_t>(sumG / count),
                static_cast<uint8_t>(sumB / count),
                static_cast<uint8_t>(sumA / count)
            );

            // Fill block with average
            for (int y = by; y < std::min(by + blockSize, h); y++) {
                for (int x = bx; x < std::min(bx + blockSize, w); x++) {
                    pixels[static_cast<size_t>(y * w + x)] = avg;
                }
            }
        }
    }
}

// SCANLINE - Add scanline effect
void effectScanline(std::vector<Color>& pixels, int w, int h, int intensity) {
    float factor = 1.0f - (static_cast<float>(intensity) / 100.0f) * 0.5f;

    for (int y = 0; y < h; y++) {
        if (y % 2 == 1) {  // Darken odd lines
            for (int x = 0; x < w; x++) {
                Color c = pixels[static_cast<size_t>(y * w + x)];
                pixels[static_cast<size_t>(y * w + x)] = makeColor(
                    static_cast<uint8_t>(getR(c) * factor),
                    static_cast<uint8_t>(getG(c) * factor),
                    static_cast<uint8_t>(getB(c) * factor),
                    getA(c)
                );
            }
        }
    }
}

// CHROMATIC_ABERRATION - Offset color channels
void effectChromaticAberration(std::vector<Color>& pixels, int w, int h, int offsetX, int offsetY) {
    std::vector<Color> result = pixels;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            // Red channel - offset in one direction
            int rxr = std::max(0, std::min(w - 1, x - offsetX));
            int ryr = std::max(0, std::min(h - 1, y - offsetY));
            uint8_t r = getR(pixels[static_cast<size_t>(ryr * w + rxr)]);

            // Green channel - no offset
            uint8_t g = getG(pixels[static_cast<size_t>(y * w + x)]);

            // Blue channel - offset in opposite direction
            int rxb = std::max(0, std::min(w - 1, x + offsetX));
            int ryb = std::max(0, std::min(h - 1, y + offsetY));
            uint8_t b = getB(pixels[static_cast<size_t>(ryb * w + rxb)]);

            result[static_cast<size_t>(y * w + x)] = makeColor(r, g, b, getA(pixels[static_cast<size_t>(y * w + x)]));
        }
    }

    pixels = std::move(result);
}

// DITHER - Add ordered dithering pattern
void effectDither(std::vector<Color>& pixels, int w, int h, int intensity) {
    // 4x4 Bayer matrix
    static const int bayer[4][4] = {
        {  0,  8,  2, 10 },
        { 12,  4, 14,  6 },
        {  3, 11,  1,  9 },
        { 15,  7, 13,  5 }
    };

    float scale = (static_cast<float>(intensity) / 100.0f) * 32.0f;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            Color c = pixels[static_cast<size_t>(y * w + x)];
            float threshold = static_cast<float>(bayer[y % 4][x % 4] - 8) * scale / 16.0f;

            int r = std::max(0, std::min(255, static_cast<int>(getR(c) + threshold)));
            int g = std::max(0, std::min(255, static_cast<int>(getG(c) + threshold)));
            int b = std::max(0, std::min(255, static_cast<int>(getB(c) + threshold)));

            pixels[static_cast<size_t>(y * w + x)] = makeColor(
                static_cast<uint8_t>(r),
                static_cast<uint8_t>(g),
                static_cast<uint8_t>(b),
                getA(c)
            );
        }
    }
}

// POSTERIZE - Reduce color levels
void effectPosterize(std::vector<Color>& pixels, int w, int h, int levels) {
    if (levels < 2) levels = 2;
    if (levels > 256) levels = 256;

    float step = 255.0f / static_cast<float>(levels - 1);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            Color c = pixels[static_cast<size_t>(y * w + x)];

            int r = static_cast<int>(std::round(getR(c) / step) * step);
            int g = static_cast<int>(std::round(getG(c) / step) * step);
            int b = static_cast<int>(std::round(getB(c) / step) * step);

            pixels[static_cast<size_t>(y * w + x)] = makeColor(
                static_cast<uint8_t>(std::max(0, std::min(255, r))),
                static_cast<uint8_t>(std::max(0, std::min(255, g))),
                static_cast<uint8_t>(std::max(0, std::min(255, b))),
                getA(c)
            );
        }
    }
}

// GLITCH_SHIFT - Random block displacement
void effectGlitchShift(std::vector<Color>& pixels, int w, int h, int blockSize, uint32_t seed) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> shiftDist(-blockSize * 2, blockSize * 2);
    std::uniform_int_distribution<int> probDist(0, 100);

    std::vector<Color> result = pixels;

    for (int by = 0; by < h; by += blockSize) {
        if (probDist(rng) < 30) {  // 30% chance to glitch this row
            int shift = shiftDist(rng);

            for (int y = by; y < std::min(by + blockSize, h); y++) {
                for (int x = 0; x < w; x++) {
                    int srcX = ((x - shift) % w + w) % w;
                    result[static_cast<size_t>(y * w + x)] = pixels[static_cast<size_t>(y * w + srcX)];
                }
            }
        }
    }

    pixels = std::move(result);
}

} // namespace glic
