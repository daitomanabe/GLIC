#pragma once

#include "config.hpp"
#include <vector>
#include <cstdint>
#include <string>

namespace glic {

// Post-processing effect types
enum class EffectType : uint8_t {
    NONE = 0,
    PIXELATE = 1,
    SCANLINE = 2,
    CHROMATIC_ABERRATION = 3,
    DITHER = 4,
    POSTERIZE = 5,
    GLITCH_SHIFT = 6,
    COUNT = 7
};

std::string effectName(EffectType et);
EffectType effectFromName(const std::string& name);

// Effect configuration
struct EffectConfig {
    EffectType type = EffectType::NONE;
    int intensity = 50;        // 0-100, effect strength
    int blockSize = 8;         // For pixelate, glitch_shift
    int offsetX = 2;           // For chromatic aberration
    int offsetY = 0;           // For chromatic aberration
    int levels = 4;            // For posterize
    uint32_t seed = 12345;     // For randomized effects
};

// Post-effects configuration
struct PostEffectsConfig {
    std::vector<EffectConfig> effects;
    bool enabled = false;
};

// Apply single effect to pixel buffer
void applyEffect(
    std::vector<Color>& pixels,
    int width,
    int height,
    const EffectConfig& config
);

// Apply multiple effects in sequence
void applyEffects(
    std::vector<Color>& pixels,
    int width,
    int height,
    const std::vector<EffectConfig>& effects
);

// Individual effect functions
void effectPixelate(std::vector<Color>& pixels, int w, int h, int blockSize);
void effectScanline(std::vector<Color>& pixels, int w, int h, int intensity);
void effectChromaticAberration(std::vector<Color>& pixels, int w, int h, int offsetX, int offsetY);
void effectDither(std::vector<Color>& pixels, int w, int h, int intensity);
void effectPosterize(std::vector<Color>& pixels, int w, int h, int levels);
void effectGlitchShift(std::vector<Color>& pixels, int w, int h, int blockSize, uint32_t seed);

} // namespace glic
