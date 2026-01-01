#pragma once

#include "config.hpp"
#include <vector>
#include <memory>

namespace glic {

// Forward declaration
struct Segment;

// Reference color for out-of-bounds pixels
struct RefColor {
    int c[4];

    RefColor();
    RefColor(int r, int g, int b, int a = 255);
    RefColor(Color cc);
    RefColor(Color cc, ColorSpace cs);
};

// Image planes (3 channels)
class Planes {
public:
    Planes(int w, int h, ColorSpace cs);
    Planes(int w, int h, ColorSpace cs, const RefColor& ref);
    Planes(const Color* pixels, int w, int h, ColorSpace cs);
    Planes(const Color* pixels, int w, int h, ColorSpace cs, const RefColor& ref);

    // Create a clone with same dimensions but empty data
    std::unique_ptr<Planes> clone() const;

    // Convert to pixel array
    std::vector<Color> toPixels(const Color* originalPixels = nullptr) const;

    // Get/Set individual values
    int get(int channel, int x, int y) const;
    void set(int channel, int x, int y, int value);

    // Get segment data as 2D array
    std::vector<std::vector<double>> getSegment(int channel, const Segment& s) const;

    // Set segment data from 2D array
    void setSegment(int channel, const Segment& s, const std::vector<std::vector<double>>& values, ClampMethod method);

    // Arithmetic operations on segments
    void subtract(int channel, const Segment& s, const std::vector<std::vector<int>>& values, ClampMethod method);
    void add(int channel, const Segment& s, const std::vector<std::vector<int>>& values, ClampMethod method);

    // Dimensions
    int width() const { return w_; }
    int height() const { return h_; }
    int paddedWidth() const { return ww_; }
    int paddedHeight() const { return hh_; }
    ColorSpace colorSpace() const { return cs_; }
    const RefColor& refColor() const { return ref_; }

private:
    void extractPlanes(const Color* pixels);

    int w_, h_;      // Original dimensions
    int ww_, hh_;    // Power-of-2 padded dimensions
    ColorSpace cs_;
    RefColor ref_;
    std::vector<std::vector<std::vector<int>>> channels_; // [3][w][h]
};

// Clamping functions
int clampIn(ClampMethod method, int x);
int clampOut(ClampMethod method, int x);
int clamp(ClampMethod method, int x);

} // namespace glic
