#include "glic.hpp"
#include "effects.hpp"
#include "colorspaces.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>

using namespace glic;

void printUsage(const char* programName) {
    std::cout << "GLIC - GLitch Image Codec (C++ Version)\n\n";
    std::cout << "Usage:\n";
    std::cout << "  " << programName << " encode <input.png> <output.glic> [options]\n";
    std::cout << "  " << programName << " decode <input.glic> <output.png> [options]\n\n";
    std::cout << "Encode Options:\n";
    std::cout << "  --colorspace <name>      Color space (default: HWB)\n";
    std::cout << "                           Options: RGB, HSB, HWB, OHTA, CMY, XYZ, YXY, LAB, LUV,\n";
    std::cout << "                                    HCL, YUV, YPbPr, YCbCr, YDbDr, GS, R-GGB-G\n";
    std::cout << "  --min-block <size>       Min block size (default: 2)\n";
    std::cout << "  --max-block <size>       Max block size (default: 256)\n";
    std::cout << "  --threshold <value>      Segmentation threshold (default: 15)\n";
    std::cout << "  --prediction <method>    Prediction method (default: PAETH)\n";
    std::cout << "                           Options: NONE, CORNER, H, V, DC, DCMEDIAN, MEDIAN, AVG,\n";
    std::cout << "                                    TRUEMOTION, PAETH, LDIAG, HV, JPEGLS, DIFF,\n";
    std::cout << "                                    REF, ANGLE, SAD, BSAD, RANDOM,\n";
    std::cout << "                                    SPIRAL, NOISE, GRADIENT, MIRROR, WAVE,\n";
    std::cout << "                                    CHECKERBOARD, RADIAL, EDGE\n";
    std::cout << "  --quantization <value>   Quantization value 0-255 (default: 110)\n";
    std::cout << "  --clamp <method>         Clamp method: none, mod256 (default: none)\n";
    std::cout << "  --wavelet <name>         Wavelet type (default: SYMLET8)\n";
    std::cout << "                           Options: NONE, HAAR, DB2-DB10, SYM2-SYM10, COIF1-COIF5\n";
    std::cout << "  --transform <type>       Transform type: fwt, wpt (default: fwt)\n";
    std::cout << "  --scale <value>          Transform scale (default: 20)\n";
    std::cout << "  --encoding <method>      Encoding method (default: packed)\n";
    std::cout << "                           Options: raw, packed, rle, delta, xor, zigzag\n";
    std::cout << "  --border <r,g,b>         Border color RGB (default: 128,128,128)\n";
    std::cout << "\nDecode Options (Post-Effects):\n";
    std::cout << "  --effect <name>          Apply post effect (can be used multiple times)\n";
    std::cout << "                           Options: pixelate, scanline, chromatic, dither,\n";
    std::cout << "                                    posterize, glitch\n";
    std::cout << "  --effect-intensity <n>   Effect intensity 0-100 (default: 50)\n";
    std::cout << "  --effect-blocksize <n>   Effect block size for pixelate/glitch (default: 8)\n";
    std::cout << "  --effect-offset <x,y>    Chromatic aberration offset (default: 2,0)\n";
    std::cout << "  --effect-levels <n>      Posterize levels (default: 4)\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << programName << " encode photo.png glitched.glic\n";
    std::cout << "  " << programName << " encode photo.png glitched.glic --colorspace HWB --prediction SPIRAL\n";
    std::cout << "  " << programName << " decode glitched.glic result.png --effect scanline --effect chromatic\n";
}

bool parseArgs(int argc, char* argv[], std::string& command, std::string& input,
               std::string& output, CodecConfig& config, PostEffectsConfig& postEffects) {
    if (argc < 4) {
        return false;
    }

    command = argv[1];
    input = argv[2];
    output = argv[3];

    // Default config
    config = CodecConfig();
    postEffects = PostEffectsConfig();

    // Default effect config for new effects
    EffectConfig currentEffect;
    currentEffect.intensity = 50;
    currentEffect.blockSize = 8;
    currentEffect.offsetX = 2;
    currentEffect.offsetY = 0;
    currentEffect.levels = 4;

    // Parse options
    for (int i = 4; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--colorspace" && i + 1 < argc) {
            config.colorSpace = colorSpaceFromName(argv[++i]);
        }
        else if (arg == "--min-block" && i + 1 < argc) {
            int val = std::stoi(argv[++i]);
            for (auto& ch : config.channels) ch.minBlockSize = val;
        }
        else if (arg == "--max-block" && i + 1 < argc) {
            int val = std::stoi(argv[++i]);
            for (auto& ch : config.channels) ch.maxBlockSize = val;
        }
        else if (arg == "--threshold" && i + 1 < argc) {
            float val = std::stof(argv[++i]);
            for (auto& ch : config.channels) ch.segmentationPrecision = val;
        }
        else if (arg == "--prediction" && i + 1 < argc) {
            auto val = predictionFromName(argv[++i]);
            for (auto& ch : config.channels) ch.predictionMethod = val;
        }
        else if (arg == "--quantization" && i + 1 < argc) {
            int val = std::stoi(argv[++i]);
            for (auto& ch : config.channels) ch.quantizationValue = val;
        }
        else if (arg == "--clamp" && i + 1 < argc) {
            std::string val = argv[++i];
            ClampMethod cm = (val == "mod256") ? ClampMethod::MOD256 : ClampMethod::NONE;
            for (auto& ch : config.channels) ch.clampMethod = cm;
        }
        else if (arg == "--wavelet" && i + 1 < argc) {
            auto val = waveletFromName(argv[++i]);
            for (auto& ch : config.channels) ch.waveletType = val;
        }
        else if (arg == "--transform" && i + 1 < argc) {
            std::string val = argv[++i];
            TransformType tt = (val == "wpt") ? TransformType::WPT : TransformType::FWT;
            for (auto& ch : config.channels) ch.transformType = tt;
        }
        else if (arg == "--scale" && i + 1 < argc) {
            int val = std::stoi(argv[++i]);
            for (auto& ch : config.channels) ch.transformScale = val;
        }
        else if (arg == "--encoding" && i + 1 < argc) {
            auto val = encodingFromName(argv[++i]);
            for (auto& ch : config.channels) ch.encodingMethod = val;
        }
        else if (arg == "--border" && i + 1 < argc) {
            std::string colorStr = argv[++i];
            int r = 128, g = 128, b = 128;
            if (sscanf(colorStr.c_str(), "%d,%d,%d", &r, &g, &b) == 3) {
                config.borderColorR = static_cast<uint8_t>(r);
                config.borderColorG = static_cast<uint8_t>(g);
                config.borderColorB = static_cast<uint8_t>(b);
            }
        }
        // Post-effect options
        else if (arg == "--effect" && i + 1 < argc) {
            std::string effectName = argv[++i];
            EffectConfig effect = currentEffect;
            effect.type = effectFromName(effectName);
            if (effect.type != EffectType::NONE) {
                postEffects.effects.push_back(effect);
                postEffects.enabled = true;
            }
        }
        else if (arg == "--effect-intensity" && i + 1 < argc) {
            currentEffect.intensity = std::stoi(argv[++i]);
        }
        else if (arg == "--effect-blocksize" && i + 1 < argc) {
            currentEffect.blockSize = std::stoi(argv[++i]);
        }
        else if (arg == "--effect-offset" && i + 1 < argc) {
            std::string offsetStr = argv[++i];
            int x = 2, y = 0;
            if (sscanf(offsetStr.c_str(), "%d,%d", &x, &y) >= 1) {
                currentEffect.offsetX = x;
                currentEffect.offsetY = y;
            }
        }
        else if (arg == "--effect-levels" && i + 1 < argc) {
            currentEffect.levels = std::stoi(argv[++i]);
        }
        else if (arg == "--help" || arg == "-h") {
            return false;
        }
    }

    return true;
}

int main(int argc, char* argv[]) {
    std::string command, input, output;
    CodecConfig config;
    PostEffectsConfig postEffects;

    if (!parseArgs(argc, argv, command, input, output, config, postEffects)) {
        printUsage(argv[0]);
        return 1;
    }

    std::transform(command.begin(), command.end(), command.begin(), ::tolower);

    GlicCodec codec(config);
    codec.setPostEffects(postEffects);

    if (command == "encode") {
        // Load input image
        std::vector<Color> pixels;
        int width, height;

        if (!loadImage(input, pixels, width, height)) {
            std::cerr << "Error: Failed to load image: " << input << std::endl;
            return 1;
        }

        std::cout << "Loaded image: " << width << "x" << height << std::endl;

        // Encode
        auto result = codec.encode(pixels.data(), width, height, output);

        if (!result.success) {
            std::cerr << "Error: " << result.error << std::endl;
            return 1;
        }

        std::cout << "Encoded to: " << output << std::endl;
    }
    else if (command == "decode") {
        // Decode
        auto result = codec.decode(input);

        if (!result.success) {
            std::cerr << "Error: " << result.error << std::endl;
            return 1;
        }

        // Save output image
        if (!saveImage(output, result.pixels, result.width, result.height)) {
            std::cerr << "Error: Failed to save image: " << output << std::endl;
            return 1;
        }

        std::cout << "Decoded to: " << output << " (" << result.width << "x" << result.height << ")" << std::endl;
    }
    else {
        std::cerr << "Error: Unknown command: " << command << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    return 0;
}
