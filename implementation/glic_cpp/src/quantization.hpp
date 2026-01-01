#pragma once

#include "planes.hpp"
#include "segment.hpp"

namespace glic {

// Forward quantization (divide by val)
void quantize(Planes& planes, int channel, const Segment& segment, float val, bool forward);

// Helper to convert quantization value (0-255) to actual divisor
inline float quantValue(int v) {
    return v / 2.0f;
}

// Helper to convert transform compression value
inline float transCompressionValue(float v) {
    return 50.0f * (v / 255.0f) * (v / 255.0f);
}

} // namespace glic
