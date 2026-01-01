#pragma once

#include "config.hpp"
#include "bitio.hpp"
#include "planes.hpp"
#include "segment.hpp"
#include <vector>

namespace glic {

// Encode data using specified method
void encodeData(
    BitWriter& writer,
    const Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    EncodingMethod method,
    const ChannelConfig& config
);

// Decode data using specified method
void decodeData(
    BitReader& reader,
    Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    EncodingMethod method,
    const ChannelConfig& config
);

// Individual encoding methods
void encodeRaw(
    BitWriter& writer,
    const Planes& planes,
    int channel,
    const std::vector<Segment>& segments
);

void encodePacked(
    BitWriter& writer,
    const Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    const ChannelConfig& config
);

void encodeRLE(
    BitWriter& writer,
    const Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    const ChannelConfig& config
);

// Individual decoding methods
void decodeRaw(
    BitReader& reader,
    Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    size_t dataSize
);

void decodePacked(
    BitReader& reader,
    Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    const ChannelConfig& config
);

void decodeRLE(
    BitReader& reader,
    Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    const ChannelConfig& config
);

// New encoding methods
void encodeDelta(
    BitWriter& writer,
    const Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    const ChannelConfig& config
);

void encodeXOR(
    BitWriter& writer,
    const Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    const ChannelConfig& config
);

void encodeZigzag(
    BitWriter& writer,
    const Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    const ChannelConfig& config
);

void decodeDelta(
    BitReader& reader,
    Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    const ChannelConfig& config
);

void decodeXOR(
    BitReader& reader,
    Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    const ChannelConfig& config
);

void decodeZigzag(
    BitReader& reader,
    Planes& planes,
    int channel,
    const std::vector<Segment>& segments,
    const ChannelConfig& config
);

} // namespace glic
