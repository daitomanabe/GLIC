#include "encoding.hpp"
#include <cmath>
#include <algorithm>

namespace glic {

namespace {

int calcBits(int scale) {
    return static_cast<int>(std::ceil(std::log(static_cast<float>(scale)) / std::log(2.0f)));
}

void emitPackedBits(BitWriter& writer, int channel, int bits, int val, const ChannelConfig& config) {
    if (config.waveletType == WaveletType::NONE) {
        if (config.clampMethod == ClampMethod::NONE) {
            writer.writeInt(val, false, 9);
        } else if (config.clampMethod == ClampMethod::MOD256) {
            writer.writeInt(val, true, 8);
        }
    } else {
        writer.writeInt(val, false, bits + 1);
    }
}

int readPackedBits(BitReader& reader, int channel, int bits, const ChannelConfig& config) {
    if (config.waveletType == WaveletType::NONE) {
        if (config.clampMethod == ClampMethod::NONE) {
            return reader.readInt(false, 9);
        } else if (config.clampMethod == ClampMethod::MOD256) {
            return reader.readInt(true, 8);
        }
    } else {
        return reader.readInt(false, bits + 1);
    }
    return 0;
}

} // anonymous namespace

void encodeData(
    BitWriter& writer,
    const Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    EncodingMethod method,
    const ChannelConfig& config
) {
    switch (method) {
        case EncodingMethod::PACKED:
            encodePacked(writer, planes, channel, segments, config);
            break;
        case EncodingMethod::RLE:
            encodeRLE(writer, planes, channel, segments, config);
            break;
        case EncodingMethod::DELTA:
            encodeDelta(writer, planes, channel, segments, config);
            break;
        case EncodingMethod::XOR:
            encodeXOR(writer, planes, channel, segments, config);
            break;
        case EncodingMethod::ZIGZAG:
            encodeZigzag(writer, planes, channel, segments, config);
            break;
        case EncodingMethod::RAW:
        default:
            encodeRaw(writer, planes, channel, segments);
            break;
    }
}

void decodeData(
    BitReader& reader,
    Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    EncodingMethod method,
    const ChannelConfig& config
) {
    switch (method) {
        case EncodingMethod::PACKED:
            decodePacked(reader, planes, channel, segments, config);
            break;
        case EncodingMethod::RLE:
            decodeRLE(reader, planes, channel, segments, config);
            break;
        case EncodingMethod::DELTA:
            decodeDelta(reader, planes, channel, segments, config);
            break;
        case EncodingMethod::XOR:
            decodeXOR(reader, planes, channel, segments, config);
            break;
        case EncodingMethod::ZIGZAG:
            decodeZigzag(reader, planes, channel, segments, config);
            break;
        case EncodingMethod::RAW:
        default:
            decodeRaw(reader, planes, channel, segments, 0);
            break;
    }
}

void encodeRaw(
    BitWriter& writer,
    const Planes& planes,
    int channel,
    const std::vector<Segment>& segments
) {
    for (const auto& seg : segments) {
        for (int x = 0; x < seg.size; x++) {
            for (int y = 0; y < seg.size; y++) {
                int val = planes.get(channel, seg.x + x, seg.y + y);
                // Write as 32-bit integer
                writer.writeBits(static_cast<uint32_t>(val), 32);
            }
        }
    }
    writer.align();
}

void encodePacked(
    BitWriter& writer,
    const Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    const ChannelConfig& config
) {
    int bits = calcBits(config.transformScale);

    for (const auto& seg : segments) {
        for (int x = 0; x < seg.size; x++) {
            for (int y = 0; y < seg.size; y++) {
                int val = planes.get(channel, seg.x + x, seg.y + y);
                emitPackedBits(writer, channel, bits, val, config);
            }
        }
    }
    writer.align();
}

void encodeRLE(
    BitWriter& writer,
    const Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    const ChannelConfig& config
) {
    int bits = calcBits(config.transformScale);
    int currentVal = 0;
    bool firstVal = true;
    int currentCnt = 0;

    for (const auto& seg : segments) {
        for (int x = 0; x < seg.size; x++) {
            for (int y = 0; y < seg.size; y++) {
                int val = planes.get(channel, seg.x + x, seg.y + y);

                if (firstVal) {
                    currentVal = val;
                    currentCnt = 1;
                    firstVal = false;
                } else {
                    if (currentVal != val || currentCnt == 129) {
                        if (currentCnt == 1) {
                            writer.writeBoolean(false);
                        } else {
                            writer.writeBoolean(true);
                            writer.writeInt(currentCnt - 2, true, 7);
                        }
                        emitPackedBits(writer, channel, bits, currentVal, config);
                        currentVal = val;
                        currentCnt = 1;
                    } else {
                        currentCnt++;
                    }
                }
            }
        }
    }

    // Write final run
    if (!firstVal) {
        if (currentCnt == 1) {
            writer.writeBoolean(false);
        } else {
            writer.writeBoolean(true);
            writer.writeInt(currentCnt - 2, true, 7);
        }
        emitPackedBits(writer, channel, bits, currentVal, config);
    }

    writer.align();
}

void decodeRaw(
    BitReader& reader,
    Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    size_t /*dataSize*/
) {
    for (const auto& seg : segments) {
        for (int x = 0; x < seg.size; x++) {
            for (int y = 0; y < seg.size; y++) {
                try {
                    int val = static_cast<int32_t>(reader.readBits(32));
                    planes.set(channel, seg.x + x, seg.y + y, val);
                } catch (...) {
                    // EOF reached
                    return;
                }
            }
        }
    }
    reader.align();
}

void decodePacked(
    BitReader& reader,
    Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    const ChannelConfig& config
) {
    int bits = calcBits(config.transformScale);

    for (const auto& seg : segments) {
        for (int x = 0; x < seg.size; x++) {
            for (int y = 0; y < seg.size; y++) {
                try {
                    int val = readPackedBits(reader, channel, bits, config);
                    planes.set(channel, seg.x + x, seg.y + y, val);
                } catch (...) {
                    // EOF reached
                    return;
                }
            }
        }
    }
    reader.align();
}

void decodeRLE(
    BitReader& reader,
    Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    const ChannelConfig& config
) {
    int bits = calcBits(config.transformScale);
    int currentVal = 0;
    bool doReadType = true;
    int currentCnt = 0;

    for (const auto& seg : segments) {
        for (int x = 0; x < seg.size; x++) {
            for (int y = 0; y < seg.size; y++) {
                try {
                    if (doReadType) {
                        if (reader.readBoolean()) {
                            currentCnt = reader.readInt(true, 7) + 2;
                            doReadType = false;
                        }
                        currentVal = readPackedBits(reader, channel, bits, config);
                    }
                    planes.set(channel, seg.x + x, seg.y + y, currentVal);
                    currentCnt--;
                    if (currentCnt <= 0) {
                        doReadType = true;
                    }
                } catch (...) {
                    // EOF reached
                    return;
                }
            }
        }
    }
    reader.align();
}

// ============================================================================
// New Encoding Methods
// ============================================================================

namespace {

// Zigzag encode signed to unsigned
inline uint32_t zigzagEncode(int32_t n) {
    return static_cast<uint32_t>((n << 1) ^ (n >> 31));
}

// Zigzag decode unsigned to signed
inline int32_t zigzagDecode(uint32_t n) {
    return static_cast<int32_t>((n >> 1) ^ -(static_cast<int32_t>(n) & 1));
}

} // anonymous namespace

void encodeDelta(
    BitWriter& writer,
    const Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    const ChannelConfig& config
) {
    int bits = calcBits(config.transformScale);
    int prevVal = 0;

    for (const auto& seg : segments) {
        for (int x = 0; x < seg.size; x++) {
            for (int y = 0; y < seg.size; y++) {
                int val = planes.get(channel, seg.x + x, seg.y + y);
                int delta = val - prevVal;
                uint32_t encoded = zigzagEncode(delta);
                writer.writeInt(static_cast<int>(encoded), false, bits + 2);
                prevVal = val;
            }
        }
    }
    writer.align();
}

void decodeDelta(
    BitReader& reader,
    Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    const ChannelConfig& config
) {
    int bits = calcBits(config.transformScale);
    int prevVal = 0;

    for (const auto& seg : segments) {
        for (int x = 0; x < seg.size; x++) {
            for (int y = 0; y < seg.size; y++) {
                try {
                    uint32_t encoded = static_cast<uint32_t>(reader.readInt(false, bits + 2));
                    int delta = zigzagDecode(encoded);
                    int val = prevVal + delta;
                    planes.set(channel, seg.x + x, seg.y + y, val);
                    prevVal = val;
                } catch (...) {
                    return;
                }
            }
        }
    }
    reader.align();
}

void encodeXOR(
    BitWriter& writer,
    const Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    const ChannelConfig& config
) {
    int bits = calcBits(config.transformScale);
    int prevVal = 0;

    for (const auto& seg : segments) {
        for (int x = 0; x < seg.size; x++) {
            for (int y = 0; y < seg.size; y++) {
                int val = planes.get(channel, seg.x + x, seg.y + y);
                int xorVal = val ^ prevVal;
                emitPackedBits(writer, channel, bits, xorVal, config);
                prevVal = val;
            }
        }
    }
    writer.align();
}

void decodeXOR(
    BitReader& reader,
    Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    const ChannelConfig& config
) {
    int bits = calcBits(config.transformScale);
    int prevVal = 0;

    for (const auto& seg : segments) {
        for (int x = 0; x < seg.size; x++) {
            for (int y = 0; y < seg.size; y++) {
                try {
                    int xorVal = readPackedBits(reader, channel, bits, config);
                    int val = xorVal ^ prevVal;
                    planes.set(channel, seg.x + x, seg.y + y, val);
                    prevVal = val;
                } catch (...) {
                    return;
                }
            }
        }
    }
    reader.align();
}

void encodeZigzag(
    BitWriter& writer,
    const Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    const ChannelConfig& config
) {
    int bits = calcBits(config.transformScale);

    for (const auto& seg : segments) {
        for (int x = 0; x < seg.size; x++) {
            for (int y = 0; y < seg.size; y++) {
                int val = planes.get(channel, seg.x + x, seg.y + y);
                uint32_t encoded = zigzagEncode(val);
                writer.writeInt(static_cast<int>(encoded), false, bits + 1);
            }
        }
    }
    writer.align();
}

void decodeZigzag(
    BitReader& reader,
    Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    const ChannelConfig& config
) {
    int bits = calcBits(config.transformScale);

    for (const auto& seg : segments) {
        for (int x = 0; x < seg.size; x++) {
            for (int y = 0; y < seg.size; y++) {
                try {
                    uint32_t encoded = static_cast<uint32_t>(reader.readInt(false, bits + 1));
                    int val = zigzagDecode(encoded);
                    planes.set(channel, seg.x + x, seg.y + y, val);
                } catch (...) {
                    return;
                }
            }
        }
    }
    reader.align();
}

} // namespace glic
