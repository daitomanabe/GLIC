#include "segment.hpp"
#include "planes.hpp"
#include <cmath>
#include <random>
#include <sstream>
#include <algorithm>

namespace glic {

namespace {

// Random number generator for standard deviation sampling
std::mt19937& getRng() {
    static std::mt19937 rng(42); // Fixed seed for reproducibility
    return rng;
}

void segmentRecursive(
    BitWriter& writer,
    std::vector<Segment>& segments,
    const Planes& planes,
    int channel,
    int x, int y, int size,
    int minSize, int maxSize,
    float threshold
) {
    if (x >= planes.width() || y >= planes.height()) return;

    float currStdDev = calcStdDev(planes, channel, x, y, size);

    if (size > maxSize || (size > minSize && currStdDev > threshold)) {
        writer.writeBoolean(true);
        int mid = size / 2;
        segmentRecursive(writer, segments, planes, channel, x, y, mid, minSize, maxSize, threshold);
        segmentRecursive(writer, segments, planes, channel, x + mid, y, mid, minSize, maxSize, threshold);
        segmentRecursive(writer, segments, planes, channel, x, y + mid, mid, minSize, maxSize, threshold);
        segmentRecursive(writer, segments, planes, channel, x + mid, y + mid, mid, minSize, maxSize, threshold);
    } else {
        writer.writeBoolean(false);
        Segment seg;
        seg.x = x;
        seg.y = y;
        seg.size = size;
        segments.push_back(seg);
    }
}

void readSegmentRecursive(
    BitReader& reader,
    std::vector<Segment>& segments,
    int x, int y, int size,
    int width, int height
) {
    if (x >= width || y >= height) return;

    bool decision;
    try {
        decision = reader.readBoolean();
    } catch (...) {
        decision = false;
    }

    if (decision && size > 2) {
        int mid = size / 2;
        readSegmentRecursive(reader, segments, x, y, mid, width, height);
        readSegmentRecursive(reader, segments, x + mid, y, mid, width, height);
        readSegmentRecursive(reader, segments, x, y + mid, mid, width, height);
        readSegmentRecursive(reader, segments, x + mid, y + mid, mid, width, height);
    } else {
        Segment seg;
        seg.x = x;
        seg.y = y;
        seg.size = size;
        segments.push_back(seg);
    }
}

} // anonymous namespace

std::string Segment::toString() const {
    std::ostringstream oss;
    oss << "x=" << x << ", y=" << y << ", size=" << size;
    return oss.str();
}

std::vector<Segment> makeSegmentation(
    BitWriter& writer,
    const Planes& planes,
    int channel,
    int minSize,
    int maxSize,
    float threshold
) {
    std::vector<Segment> segments;

    int startSize = std::max(planes.paddedWidth(), planes.paddedHeight());
    minSize = std::max(1, minSize);
    maxSize = std::min(512, maxSize);

    segmentRecursive(writer, segments, planes, channel, 0, 0, startSize, minSize, maxSize, threshold);

    return segments;
}

std::vector<Segment> readSegmentation(
    BitReader& reader,
    int paddedWidth,
    int paddedHeight,
    int width,
    int height
) {
    std::vector<Segment> segments;

    int startSize = std::max(paddedWidth, paddedHeight);
    readSegmentRecursive(reader, segments, 0, 0, startSize, width, height);

    return segments;
}

float calcStdDev(const Planes& planes, int channel, int x, int y, int size) {
    int limit = std::max(static_cast<int>(0.1f * size * size), 4);

    auto& rng = getRng();
    std::uniform_int_distribution<int> dist(0, size - 1);

    float A = 0;
    float Q = 0;

    for (int k = 1; k <= limit; k++) {
        int posx = dist(rng);
        int posy = dist(rng);

        int xk = planes.get(channel, x + posx, y + posy);

        float oldA = A;
        A += (xk - A) / k;
        Q += (xk - oldA) * (xk - A);
    }

    return std::sqrt(Q / (limit - 1));
}

} // namespace glic
