#include "planes.hpp"
#include "colorspaces.hpp"
#include "segment.hpp"
#include <cmath>
#include <algorithm>

namespace glic {

constexpr float LOG2 = 0.693147180559945f;

// Clamping functions
int clampIn(ClampMethod method, int x) {
    switch (method) {
        case ClampMethod::MOD256:
            return x < 0 ? x + 256 : (x > 255 ? x - 256 : x);
        default:
            return x;
    }
}

int clampOut(ClampMethod method, int x) {
    switch (method) {
        case ClampMethod::MOD256:
            return x < 0 ? x + 256 : (x > 255 ? x - 256 : x);
        default:
            return std::max(0, std::min(255, x));
    }
}

int clamp(ClampMethod method, int x) {
    switch (method) {
        case ClampMethod::MOD256:
            return std::max(0, std::min(255, x));
        default:
            return std::max(-255, std::min(255, x));
    }
}

// RefColor implementation
RefColor::RefColor() : c{128, 128, 128, 255} {}

RefColor::RefColor(int r, int g, int b, int a) : c{r, g, b, a} {}

RefColor::RefColor(Color cc) {
    c[0] = getR(cc);
    c[1] = getG(cc);
    c[2] = getB(cc);
    c[3] = getA(cc);
}

RefColor::RefColor(Color cc, ColorSpace cs) : RefColor(toColorSpace(cc, cs)) {}

// Planes implementation
Planes::Planes(int w, int h, ColorSpace cs)
    : Planes(w, h, cs, RefColor(makeColor(128, 128, 128), cs)) {}

Planes::Planes(int w, int h, ColorSpace cs, const RefColor& ref)
    : w_(w), h_(h), cs_(cs), ref_(ref) {

    // Calculate power-of-2 padded dimensions
    ww_ = 1 << static_cast<int>(std::ceil(std::log(static_cast<float>(w)) / LOG2));
    hh_ = 1 << static_cast<int>(std::ceil(std::log(static_cast<float>(h)) / LOG2));

    // Initialize channels
    channels_.resize(3);
    for (int c = 0; c < 3; c++) {
        channels_[c].resize(w);
        for (int x = 0; x < w; x++) {
            channels_[c][x].resize(h);
            for (int y = 0; y < h; y++) {
                channels_[c][x][y] = ref_.c[c];
            }
        }
    }
}

Planes::Planes(const Color* pixels, int w, int h, ColorSpace cs)
    : Planes(pixels, w, h, cs, RefColor(makeColor(128, 128, 128), cs)) {}

Planes::Planes(const Color* pixels, int w, int h, ColorSpace cs, const RefColor& ref)
    : Planes(w, h, cs, ref) {
    extractPlanes(pixels);
}

void Planes::extractPlanes(const Color* pixels) {
    for (int x = 0; x < w_; x++) {
        for (int y = 0; y < h_; y++) {
            Color c = toColorSpace(pixels[y * w_ + x], cs_);
            channels_[0][x][y] = getR(c);
            channels_[1][x][y] = getG(c);
            channels_[2][x][y] = getB(c);
        }
    }
}

std::unique_ptr<Planes> Planes::clone() const {
    return std::make_unique<Planes>(w_, h_, cs_, ref_);
}

std::vector<Color> Planes::toPixels(const Color* originalPixels) const {
    std::vector<Color> pixels(w_ * h_);
    for (int x = 0; x < w_; x++) {
        for (int y = 0; y < h_; y++) {
            int off = y * w_ + x;
            uint8_t a = originalPixels ? getA(originalPixels[off]) : 255;
            Color c = makeColor(
                static_cast<uint8_t>(std::max(0, std::min(255, channels_[0][x][y]))),
                static_cast<uint8_t>(std::max(0, std::min(255, channels_[1][x][y]))),
                static_cast<uint8_t>(std::max(0, std::min(255, channels_[2][x][y]))),
                a
            );
            pixels[off] = fromColorSpace(c, cs_);
        }
    }
    return pixels;
}

int Planes::get(int channel, int x, int y) const {
    if (x < 0 || x >= w_ || y < 0 || y >= h_) {
        return ref_.c[channel];
    }
    return channels_[channel][x][y];
}

void Planes::set(int channel, int x, int y, int value) {
    if (x >= 0 && x < w_ && y >= 0 && y < h_) {
        channels_[channel][x][y] = value;
    }
}

std::vector<std::vector<double>> Planes::getSegment(int channel, const Segment& s) const {
    std::vector<std::vector<double>> res(s.size, std::vector<double>(s.size));
    for (int x = 0; x < s.size; x++) {
        for (int y = 0; y < s.size; y++) {
            res[x][y] = get(channel, x + s.x, y + s.y) / 255.0;
        }
    }
    return res;
}

void Planes::setSegment(int channel, const Segment& s, const std::vector<std::vector<double>>& values, ClampMethod method) {
    for (int x = 0; x < s.size; x++) {
        for (int y = 0; y < s.size; y++) {
            set(channel, x + s.x, y + s.y, clamp(method, static_cast<int>(std::round(values[x][y] * 255.0))));
        }
    }
}

void Planes::subtract(int channel, const Segment& s, const std::vector<std::vector<int>>& values, ClampMethod method) {
    for (int x = 0; x < s.size; x++) {
        for (int y = 0; y < s.size; y++) {
            int v = get(channel, x + s.x, y + s.y) - values[x][y];
            set(channel, x + s.x, y + s.y, clampIn(method, v));
        }
    }
}

void Planes::add(int channel, const Segment& s, const std::vector<std::vector<int>>& values, ClampMethod method) {
    for (int x = 0; x < s.size; x++) {
        for (int y = 0; y < s.size; y++) {
            int v = get(channel, x + s.x, y + s.y) + values[x][y];
            set(channel, x + s.x, y + s.y, clampOut(method, v));
        }
    }
}

} // namespace glic
