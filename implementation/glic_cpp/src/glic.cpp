#include "glic.hpp"
#include "colorspaces.hpp"
#include "planes.hpp"
#include "segment.hpp"
#include "prediction.hpp"
#include "quantization.hpp"
#include "wavelet.hpp"
#include "encoding.hpp"
#include "bitio.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <fstream>
#include <iostream>
#include <cstring>

namespace glic {

GlicCodec::GlicCodec() : config_() {}

GlicCodec::GlicCodec(const CodecConfig& config) : config_(config) {}

void GlicCodec::setConfig(const CodecConfig& config) {
    config_ = config;
}

void GlicCodec::setPostEffects(const PostEffectsConfig& effects) {
    postEffects_ = effects;
}

std::vector<uint8_t> GlicCodec::encodeToBuffer(const Color* pixels, int width, int height) {
    std::vector<uint8_t> buffer;

    try {
        std::cout << "Encoding started" << std::endl;
        std::cout << "Color space: " << colorSpaceName(config_.colorSpace) << std::endl;

        // Create planes from pixels
        RefColor ref(makeColor(config_.borderColorR, config_.borderColorG, config_.borderColorB), config_.colorSpace);
        Planes planes(pixels, width, height, config_.colorSpace, ref);

        // Arrays to store segmentation and data for each channel
        std::vector<Segment> segments[3];
        std::vector<uint8_t> segmentationData[3];
        std::vector<uint8_t> predictionData[3];
        std::vector<uint8_t> imageData[3];

        // Process each channel
        for (int p = 0; p < 3; p++) {
            const auto& chConfig = config_.channels[p];

            std::cout << "Channel " << p << " segmentation" << std::endl;

            // Create segmentation
            BitWriter segmWriter;
            segments[p] = makeSegmentation(
                segmWriter,
                planes,
                p,
                chConfig.minBlockSize,
                chConfig.maxBlockSize,
                chConfig.segmentationPrecision
            );
            segmWriter.align();
            segmentationData[p] = std::vector<uint8_t>(segmWriter.data().begin(), segmWriter.data().end());

            std::cout << "Created " << segments[p].size() << " segments" << std::endl;

            // Create wavelet transform if needed
            std::shared_ptr<Wavelet> wavelet = nullptr;
            std::unique_ptr<WaveletTransform> transform = nullptr;
            std::unique_ptr<MagnitudeCompressor> compressor = nullptr;

            if (chConfig.waveletType != WaveletType::NONE) {
                wavelet = createWavelet(chConfig.waveletType);
                transform = createTransform(chConfig.transformType, wavelet);
                if (chConfig.transformCompress > 0) {
                    compressor = std::make_unique<MagnitudeCompressor>(transCompressionValue(chConfig.transformCompress));
                }
            }

            std::cout << "Wavelet for plane " << p << " -> " << (wavelet ? wavelet->getName() : "NONE") << std::endl;
            std::cout << "Prediction for plane " << p << " -> " << predictionName(chConfig.predictionMethod) << std::endl;

            float pq = quantValue(chConfig.quantizationValue);

            // Create result planes for storing encoded values
            auto resultPlanes = planes.clone();

            // Process each segment
            for (auto& seg : segments[p]) {
                // Predict
                auto pred = predict(chConfig.predictionMethod, planes, p, seg);

                // Calculate residuals
                planes.subtract(p, seg, pred, chConfig.clampMethod);

                // Quantize
                if (pq > 0) {
                    quantize(planes, p, seg, pq, true);
                }

                // Apply wavelet transform if enabled
                if (transform) {
                    auto tr = planes.getSegment(p, seg);
                    tr = transform->forward(tr);

                    if (compressor) {
                        tr = compressor->compress(tr);
                    }

                    // Store result as ints
                    for (int x = 0; x < seg.size; x++) {
                        for (int y = 0; y < seg.size; y++) {
                            int val = static_cast<int>(std::round((tr[x][y] * chConfig.transformScale) / static_cast<float>(seg.size)));
                            planes.set(p, seg.x + x, seg.y + y, val);
                        }
                    }
                }

                // Store encoding value in result planes
                for (int x = 0; x < seg.size; x++) {
                    for (int y = 0; y < seg.size; y++) {
                        resultPlanes->set(p, seg.x + x, seg.y + y, planes.get(p, seg.x + x, seg.y + y));
                    }
                }

                // Decompress now for next prediction
                if (transform) {
                    std::vector<std::vector<double>> tr(seg.size, std::vector<double>(seg.size));
                    for (int x = 0; x < seg.size; x++) {
                        for (int y = 0; y < seg.size; y++) {
                            tr[x][y] = (seg.size * planes.get(p, seg.x + x, seg.y + y)) / static_cast<float>(chConfig.transformScale);
                        }
                    }
                    tr = transform->reverse(tr);
                    planes.setSegment(p, seg, tr, chConfig.clampMethod);
                }

                // Reverse quantization
                if (pq > 0) {
                    quantize(planes, p, seg, pq, false);
                }

                // Add back predictions
                pred = predict(seg.predType, planes, p, seg);
                planes.add(p, seg, pred, chConfig.clampMethod);
            }

            // Write prediction data
            BitWriter predWriter;
            for (const auto& seg : segments[p]) {
                predWriter.writeByte(static_cast<uint8_t>(seg.predType));
                predWriter.writeBits(static_cast<uint32_t>(static_cast<int16_t>(seg.refX)), 16);
                predWriter.writeBits(static_cast<uint32_t>(static_cast<int16_t>(seg.refY)), 16);
                predWriter.writeByte(static_cast<uint8_t>(seg.refAngle % 3));
                int16_t angleVal = static_cast<int16_t>(0x7000 * seg.angle);
                predWriter.writeBits(static_cast<uint32_t>(angleVal), 16);
            }
            predWriter.align();
            predictionData[p] = std::vector<uint8_t>(predWriter.data().begin(), predWriter.data().end());

            // Encode image data
            BitWriter dataWriter;
            encodeData(dataWriter, *resultPlanes, p, segments[p], chConfig.encodingMethod, chConfig);
            imageData[p] = std::vector<uint8_t>(dataWriter.data().begin(), dataWriter.data().end());
        }

        // Build output buffer

        // Magic + version
        buffer.push_back((GLIC_MAGIC >> 24) & 0xFF);
        buffer.push_back((GLIC_MAGIC >> 16) & 0xFF);
        buffer.push_back((GLIC_MAGIC >> 8) & 0xFF);
        buffer.push_back(GLIC_MAGIC & 0xFF);
        buffer.push_back((GLIC_VERSION >> 8) & 0xFF);
        buffer.push_back(GLIC_VERSION & 0xFF);

        // Width and height
        buffer.push_back((width >> 24) & 0xFF);
        buffer.push_back((width >> 16) & 0xFF);
        buffer.push_back((width >> 8) & 0xFF);
        buffer.push_back(width & 0xFF);
        buffer.push_back((height >> 24) & 0xFF);
        buffer.push_back((height >> 16) & 0xFF);
        buffer.push_back((height >> 8) & 0xFF);
        buffer.push_back(height & 0xFF);

        // Color space
        buffer.push_back(static_cast<uint8_t>(config_.colorSpace));

        // Border color
        buffer.push_back(config_.borderColorR);
        buffer.push_back(config_.borderColorG);
        buffer.push_back(config_.borderColorB);

        // Sizes for each channel (segmentation, prediction, image data)
        for (int p = 0; p < 3; p++) {
            uint32_t segSize = static_cast<uint32_t>(segmentationData[p].size());
            buffer.push_back((segSize >> 24) & 0xFF);
            buffer.push_back((segSize >> 16) & 0xFF);
            buffer.push_back((segSize >> 8) & 0xFF);
            buffer.push_back(segSize & 0xFF);
        }
        for (int p = 0; p < 3; p++) {
            uint32_t predSize = static_cast<uint32_t>(predictionData[p].size());
            buffer.push_back((predSize >> 24) & 0xFF);
            buffer.push_back((predSize >> 16) & 0xFF);
            buffer.push_back((predSize >> 8) & 0xFF);
            buffer.push_back(predSize & 0xFF);
        }
        for (int p = 0; p < 3; p++) {
            uint32_t dataSize = static_cast<uint32_t>(imageData[p].size());
            buffer.push_back((dataSize >> 24) & 0xFF);
            buffer.push_back((dataSize >> 16) & 0xFF);
            buffer.push_back((dataSize >> 8) & 0xFF);
            buffer.push_back(dataSize & 0xFF);
        }

        // Pad header to 64 bytes
        while (buffer.size() < GLIC_HEADER_SIZE) {
            buffer.push_back(0);
        }

        // Channel configs (32 bytes each)
        for (int p = 0; p < 3; p++) {
            const auto& ch = config_.channels[p];
            buffer.push_back(static_cast<uint8_t>(ch.predictionMethod));
            buffer.push_back(static_cast<uint8_t>(ch.quantizationValue));
            buffer.push_back(static_cast<uint8_t>(ch.clampMethod));
            buffer.push_back(static_cast<uint8_t>(ch.waveletType));
            buffer.push_back(static_cast<uint8_t>(ch.transformType));
            buffer.push_back((ch.transformScale >> 24) & 0xFF);
            buffer.push_back((ch.transformScale >> 16) & 0xFF);
            buffer.push_back((ch.transformScale >> 8) & 0xFF);
            buffer.push_back(ch.transformScale & 0xFF);
            buffer.push_back(static_cast<uint8_t>(ch.encodingMethod));

            // Pad to 32 bytes
            size_t start = buffer.size();
            while (buffer.size() < start + 32 - 10) {
                buffer.push_back(0);
            }
        }

        // Write segmentation data
        for (int p = 0; p < 3; p++) {
            buffer.insert(buffer.end(), segmentationData[p].begin(), segmentationData[p].end());
        }

        // Write prediction data
        for (int p = 0; p < 3; p++) {
            buffer.insert(buffer.end(), predictionData[p].begin(), predictionData[p].end());
        }

        // Write image data
        for (int p = 0; p < 3; p++) {
            buffer.insert(buffer.end(), imageData[p].begin(), imageData[p].end());
        }

        std::cout << "FINISHED" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Encoding failed: " << e.what() << std::endl;
        buffer.clear();
    }

    return buffer;
}

GlicResult GlicCodec::encode(const Color* pixels, int width, int height, const std::string& outputPath) {
    GlicResult result;
    result.width = width;
    result.height = height;

    auto buffer = encodeToBuffer(pixels, width, height);
    if (buffer.empty()) {
        result.success = false;
        result.error = "Encoding failed";
        return result;
    }

    std::ofstream file(outputPath, std::ios::binary);
    if (!file) {
        result.success = false;
        result.error = "Failed to open output file";
        return result;
    }

    file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    file.close();

    result.success = true;
    return result;
}

GlicResult GlicCodec::decodeFromBuffer(const std::vector<uint8_t>& buffer) {
    GlicResult result;

    try {
        std::cout << "Decoding started" << std::endl;

        if (buffer.size() < GLIC_HEADER_SIZE + 3 * GLIC_CHANNEL_HEADER_SIZE) {
            result.error = "Buffer too small";
            return result;
        }

        size_t pos = 0;

        // Read magic
        uint32_t magic = (static_cast<uint32_t>(buffer[pos]) << 24) |
                        (static_cast<uint32_t>(buffer[pos + 1]) << 16) |
                        (static_cast<uint32_t>(buffer[pos + 2]) << 8) |
                        static_cast<uint32_t>(buffer[pos + 3]);
        pos += 4;

        if (magic != GLIC_MAGIC) {
            result.error = "Invalid file format";
            return result;
        }

        // Skip version
        pos += 2;

        // Read dimensions
        int width = (static_cast<int>(buffer[pos]) << 24) |
                   (static_cast<int>(buffer[pos + 1]) << 16) |
                   (static_cast<int>(buffer[pos + 2]) << 8) |
                   static_cast<int>(buffer[pos + 3]);
        pos += 4;

        int height = (static_cast<int>(buffer[pos]) << 24) |
                    (static_cast<int>(buffer[pos + 1]) << 16) |
                    (static_cast<int>(buffer[pos + 2]) << 8) |
                    static_cast<int>(buffer[pos + 3]);
        pos += 4;

        result.width = width;
        result.height = height;

        // Read color space
        ColorSpace colorSpace = static_cast<ColorSpace>(buffer[pos++]);
        std::cout << "Color space: " << colorSpaceName(colorSpace) << std::endl;

        // Read border color
        uint8_t borderR = buffer[pos++];
        uint8_t borderG = buffer[pos++];
        uint8_t borderB = buffer[pos++];

        // Read sizes
        uint32_t segmentationSizes[3];
        uint32_t predictionSizes[3];
        uint32_t dataSizes[3];

        for (int p = 0; p < 3; p++) {
            segmentationSizes[p] = (static_cast<uint32_t>(buffer[pos]) << 24) |
                                   (static_cast<uint32_t>(buffer[pos + 1]) << 16) |
                                   (static_cast<uint32_t>(buffer[pos + 2]) << 8) |
                                   static_cast<uint32_t>(buffer[pos + 3]);
            pos += 4;
        }
        for (int p = 0; p < 3; p++) {
            predictionSizes[p] = (static_cast<uint32_t>(buffer[pos]) << 24) |
                                 (static_cast<uint32_t>(buffer[pos + 1]) << 16) |
                                 (static_cast<uint32_t>(buffer[pos + 2]) << 8) |
                                 static_cast<uint32_t>(buffer[pos + 3]);
            pos += 4;
        }
        for (int p = 0; p < 3; p++) {
            dataSizes[p] = (static_cast<uint32_t>(buffer[pos]) << 24) |
                          (static_cast<uint32_t>(buffer[pos + 1]) << 16) |
                          (static_cast<uint32_t>(buffer[pos + 2]) << 8) |
                          static_cast<uint32_t>(buffer[pos + 3]);
            pos += 4;
        }

        // Skip to end of header
        pos = GLIC_HEADER_SIZE;

        // Read channel configs
        ChannelConfig channelConfigs[3];
        for (int p = 0; p < 3; p++) {
            channelConfigs[p].predictionMethod = static_cast<PredictionMethod>(static_cast<int8_t>(buffer[pos++]));
            channelConfigs[p].quantizationValue = buffer[pos++];
            channelConfigs[p].clampMethod = static_cast<ClampMethod>(buffer[pos++]);
            channelConfigs[p].waveletType = static_cast<WaveletType>(buffer[pos++]);
            channelConfigs[p].transformType = static_cast<TransformType>(buffer[pos++]);
            channelConfigs[p].transformScale = (static_cast<int>(buffer[pos]) << 24) |
                                               (static_cast<int>(buffer[pos + 1]) << 16) |
                                               (static_cast<int>(buffer[pos + 2]) << 8) |
                                               static_cast<int>(buffer[pos + 3]);
            pos += 4;
            channelConfigs[p].encodingMethod = static_cast<EncodingMethod>(buffer[pos++]);

            // Skip padding
            pos += GLIC_CHANNEL_HEADER_SIZE - 10;
        }

        // Create planes
        RefColor ref(makeColor(borderR, borderG, borderB), colorSpace);
        Planes planes(width, height, colorSpace, ref);

        // Calculate padded dimensions
        int ww = 1;
        while (ww < width) ww *= 2;
        int hh = 1;
        while (hh < height) hh *= 2;

        // Read segmentation data and reconstruct segments
        std::vector<Segment> segments[3];
        for (int p = 0; p < 3; p++) {
            std::cout << "Channel " << p << " segmentation" << std::endl;
            BitReader segReader(buffer.data() + pos, segmentationSizes[p]);
            segments[p] = readSegmentation(segReader, ww, hh, width, height);
            pos += segmentationSizes[p];
        }

        // Read prediction data
        for (int p = 0; p < 3; p++) {
            BitReader predReader(buffer.data() + pos, predictionSizes[p]);
            for (auto& seg : segments[p]) {
                try {
                    seg.predType = static_cast<PredictionMethod>(predReader.readByte());
                    if (seg.predType == PredictionMethod::NONE) {
                        seg.predType = channelConfigs[p].predictionMethod;
                    }
                    seg.refX = static_cast<int16_t>(predReader.readBits(16));
                    seg.refY = static_cast<int16_t>(predReader.readBits(16));
                    seg.refAngle = predReader.readByte() % 3;
                    int16_t angleVal = static_cast<int16_t>(predReader.readBits(16));
                    seg.angle = static_cast<float>(angleVal) / 0x7000;
                } catch (...) {
                    break;
                }
            }
            pos += predictionSizes[p];
        }

        // Read and decode image data
        for (int p = 0; p < 3; p++) {
            BitReader dataReader(buffer.data() + pos, dataSizes[p]);
            decodeData(dataReader, planes, p, segments[p], channelConfigs[p].encodingMethod, channelConfigs[p]);
            pos += dataSizes[p];
        }

        // Reconstruct image
        for (int p = 0; p < 3; p++) {
            const auto& chConfig = channelConfigs[p];

            std::shared_ptr<Wavelet> wavelet = nullptr;
            std::unique_ptr<WaveletTransform> transform = nullptr;

            if (chConfig.waveletType != WaveletType::NONE) {
                wavelet = createWavelet(chConfig.waveletType);
                transform = createTransform(chConfig.transformType, wavelet);
            }

            std::cout << "Wavelet for plane " << p << " -> " << (wavelet ? wavelet->getName() : "NONE") << std::endl;
            std::cout << "Prediction for plane " << p << " -> " << predictionName(chConfig.predictionMethod) << std::endl;

            float pq = quantValue(chConfig.quantizationValue);

            for (auto& seg : segments[p]) {
                // Inverse wavelet transform
                if (transform) {
                    std::vector<std::vector<double>> tr(seg.size, std::vector<double>(seg.size));
                    for (int x = 0; x < seg.size; x++) {
                        for (int y = 0; y < seg.size; y++) {
                            tr[x][y] = (seg.size * planes.get(p, seg.x + x, seg.y + y)) / static_cast<float>(chConfig.transformScale);
                        }
                    }
                    tr = transform->reverse(tr);
                    planes.setSegment(p, seg, tr, chConfig.clampMethod);
                }

                // Inverse quantization
                if (pq > 0) {
                    quantize(planes, p, seg, pq, false);
                }

                // Add predictions
                auto pred = predict(seg.predType, planes, p, seg);
                planes.add(p, seg, pred, chConfig.clampMethod);
            }
        }

        // Convert to pixels
        result.pixels = planes.toPixels();

        // Apply post-processing effects
        if (postEffects_.enabled && !postEffects_.effects.empty()) {
            std::cout << "Applying " << postEffects_.effects.size() << " post effect(s)" << std::endl;
            applyEffects(result.pixels, width, height, postEffects_.effects);
        }

        result.success = true;

        std::cout << "FINISHED" << std::endl;
    } catch (const std::exception& e) {
        result.error = std::string("Decoding failed: ") + e.what();
        std::cerr << result.error << std::endl;
    }

    return result;
}

GlicResult GlicCodec::decode(const std::string& inputPath) {
    GlicResult result;

    std::ifstream file(inputPath, std::ios::binary | std::ios::ate);
    if (!file) {
        result.error = "Failed to open input file";
        return result;
    }

    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        result.error = "Failed to read file";
        return result;
    }

    return decodeFromBuffer(buffer);
}

bool loadImage(const std::string& path, std::vector<Color>& pixels, int& width, int& height) {
    int channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (!data) {
        std::cerr << "Failed to load image: " << path << std::endl;
        return false;
    }

    pixels.resize(width * height);
    for (int i = 0; i < width * height; i++) {
        pixels[i] = makeColor(data[i * 4], data[i * 4 + 1], data[i * 4 + 2], data[i * 4 + 3]);
    }

    stbi_image_free(data);
    return true;
}

bool saveImage(const std::string& path, const std::vector<Color>& pixels, int width, int height) {
    std::vector<unsigned char> data(width * height * 4);
    for (int i = 0; i < width * height; i++) {
        data[i * 4] = getR(pixels[i]);
        data[i * 4 + 1] = getG(pixels[i]);
        data[i * 4 + 2] = getB(pixels[i]);
        data[i * 4 + 3] = getA(pixels[i]);
    }

    if (!stbi_write_png(path.c_str(), width, height, 4, data.data(), width * 4)) {
        std::cerr << "Failed to save image: " << path << std::endl;
        return false;
    }

    return true;
}

} // namespace glic
