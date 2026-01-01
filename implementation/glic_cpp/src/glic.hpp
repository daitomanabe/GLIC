#pragma once

#include "config.hpp"
#include "effects.hpp"
#include "planes.hpp"
#include "segment.hpp"
#include <vector>
#include <string>
#include <memory>

namespace glic {

// Result of encoding/decoding
struct GlicResult {
    std::vector<Color> pixels;
    int width = 0;
    int height = 0;
    bool success = false;
    std::string error;
};

// Main GLIC codec class
class GlicCodec {
public:
    GlicCodec();
    explicit GlicCodec(const CodecConfig& config);

    // Set configuration
    void setConfig(const CodecConfig& config);
    CodecConfig& config() { return config_; }
    const CodecConfig& config() const { return config_; }

    // Encode image to GLIC format
    GlicResult encode(const Color* pixels, int width, int height, const std::string& outputPath);

    // Decode GLIC file to image
    GlicResult decode(const std::string& inputPath);

    // Encode to memory buffer
    std::vector<uint8_t> encodeToBuffer(const Color* pixels, int width, int height);

    // Decode from memory buffer
    GlicResult decodeFromBuffer(const std::vector<uint8_t>& buffer);

    // Post-processing effects
    void setPostEffects(const PostEffectsConfig& effects);
    PostEffectsConfig& postEffects() { return postEffects_; }
    const PostEffectsConfig& postEffects() const { return postEffects_; }

private:
    CodecConfig config_;
    PostEffectsConfig postEffects_;
};

// File format constants
constexpr uint32_t GLIC_MAGIC = 0x474C4332; // "GLC2"
constexpr uint16_t GLIC_VERSION = 1;
constexpr size_t GLIC_HEADER_SIZE = 64;
constexpr size_t GLIC_CHANNEL_HEADER_SIZE = 32;

// Load image from file (PNG, JPG, BMP)
bool loadImage(const std::string& path, std::vector<Color>& pixels, int& width, int& height);

// Save image to file (PNG)
bool saveImage(const std::string& path, const std::vector<Color>& pixels, int width, int height);

} // namespace glic
