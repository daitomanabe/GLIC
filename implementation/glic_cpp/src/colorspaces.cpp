#include "colorspaces.hpp"
#include <cmath>
#include <algorithm>

namespace glic {

// Constants
constexpr float D65X = 0.950456f;
constexpr float D65Y = 1.0f;
constexpr float D65Z = 1.088754f;
constexpr float CIEEpsilon = 216.0f / 24389.0f;
constexpr float CIEK = 24389.0f / 27.0f;
constexpr float CIEK2epsilon = CIEK * CIEEpsilon;
constexpr float D65FX_4 = 4.0f * D65X / (D65X + 15.0f * D65Y + 3.0f * D65Z);
constexpr float D65FY_9 = 9.0f * D65Y / (D65X + 15.0f * D65Y + 3.0f * D65Z);
constexpr float RANGE_X = 100.0f * (0.4124f + 0.3576f + 0.1805f);
constexpr float RANGE_Y = 100.0f;
constexpr float RANGE_Z = 100.0f * (0.0193f + 0.1192f + 0.9505f);
constexpr float mepsilon = 1.0e-10f;
constexpr float corrratio = 1.0f / 2.4f;
constexpr float One_Third = 1.0f / 3.0f;
constexpr float one_hsixteen = 1.0f / 116.0f;
constexpr float Umax = 0.436f * 255.0f;
constexpr float Vmax = 0.615f * 255.0f;

inline float mapf(float value, float inMin, float inMax, float outMin, float outMax) {
    return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin);
}

// XYZ helper functions
inline float correctionxyz(float n) {
    return (n > 0.04045f ? std::pow((n + 0.055f) / 1.055f, 2.4f) : n / 12.92f) * 100.0f;
}

inline float recorrectionxyz(float n) {
    return n > 0.0031308f ? 1.055f * std::pow(n, corrratio) - 0.055f : 12.92f * n;
}

struct Vec3 {
    float x, y, z;
};

Vec3 _toXYZ(float rr, float gg, float bb) {
    float r = correctionxyz(rr);
    float g = correctionxyz(gg);
    float b = correctionxyz(bb);
    return {
        r * 0.4124f + g * 0.3576f + b * 0.1805f,
        r * 0.2126f + g * 0.7152f + b * 0.0722f,
        r * 0.0193f + g * 0.1192f + b * 0.9505f
    };
}

Color _fromXYZ(Color c, float xx, float yy, float zz) {
    float x = xx / 100.0f;
    float y = yy / 100.0f;
    float z = zz / 100.0f;

    int r = static_cast<int>(std::round(255.0f * recorrectionxyz(x * 3.2406f + y * -1.5372f + z * -0.4986f)));
    int g = static_cast<int>(std::round(255.0f * recorrectionxyz(x * -0.9689f + y * 1.8758f + z * 0.0415f)));
    int b = static_cast<int>(std::round(255.0f * recorrectionxyz(x * 0.0557f + y * -0.2040f + z * 1.0570f)));

    return blendRGB(c, r, g, b);
}

float PerceptibleReciprocal(float x) {
    float sgn = x < 0.0f ? -1.0f : 1.0f;
    if ((sgn * x) >= mepsilon) return 1.0f / x;
    return sgn / mepsilon;
}

// Color space name utilities
std::string colorSpaceName(ColorSpace cs) {
    switch (cs) {
        case ColorSpace::OHTA: return "OHTA";
        case ColorSpace::RGB: return "RGB";
        case ColorSpace::CMY: return "CMY";
        case ColorSpace::HSB: return "HSB";
        case ColorSpace::XYZ: return "XYZ";
        case ColorSpace::YXY: return "YXY";
        case ColorSpace::HCL: return "HCL";
        case ColorSpace::LUV: return "LUV";
        case ColorSpace::LAB: return "LAB";
        case ColorSpace::HWB: return "HWB";
        case ColorSpace::RGGBG: return "R-GGB-G";
        case ColorSpace::YPbPr: return "YPbPr";
        case ColorSpace::YCbCr: return "YCbCr";
        case ColorSpace::YDbDr: return "YDbDr";
        case ColorSpace::GS: return "Greyscale";
        case ColorSpace::YUV: return "YUV";
        default: return "RGB";
    }
}

ColorSpace colorSpaceFromName(const std::string& name) {
    if (name == "OHTA") return ColorSpace::OHTA;
    if (name == "RGB") return ColorSpace::RGB;
    if (name == "CMY") return ColorSpace::CMY;
    if (name == "HSB") return ColorSpace::HSB;
    if (name == "XYZ") return ColorSpace::XYZ;
    if (name == "YXY") return ColorSpace::YXY;
    if (name == "HCL") return ColorSpace::HCL;
    if (name == "LUV") return ColorSpace::LUV;
    if (name == "LAB") return ColorSpace::LAB;
    if (name == "HWB") return ColorSpace::HWB;
    if (name == "R-GGB-G" || name == "RGGBG") return ColorSpace::RGGBG;
    if (name == "YPbPr") return ColorSpace::YPbPr;
    if (name == "YCbCr") return ColorSpace::YCbCr;
    if (name == "YDbDr") return ColorSpace::YDbDr;
    if (name == "Greyscale" || name == "GS") return ColorSpace::GS;
    if (name == "YUV") return ColorSpace::YUV;
    return ColorSpace::RGB;
}

// Main conversion functions
Color toColorSpace(Color c, ColorSpace cs) {
    switch (cs) {
        case ColorSpace::OHTA: return toOHTA(c);
        case ColorSpace::CMY: return toCMY(c);
        case ColorSpace::HSB: return toHSB(c);
        case ColorSpace::XYZ: return toXYZ(c);
        case ColorSpace::YXY: return toYXY(c);
        case ColorSpace::HCL: return toHCL(c);
        case ColorSpace::LUV: return toLUV(c);
        case ColorSpace::LAB: return toLAB(c);
        case ColorSpace::HWB: return toHWB(c);
        case ColorSpace::RGGBG: return toRGGBG(c);
        case ColorSpace::YPbPr: return toYPbPr(c);
        case ColorSpace::YCbCr: return toYCbCr(c);
        case ColorSpace::YDbDr: return toYDbDr(c);
        case ColorSpace::GS: return toGS(c);
        case ColorSpace::YUV: return toYUV(c);
        default: return c;
    }
}

Color fromColorSpace(Color c, ColorSpace cs) {
    switch (cs) {
        case ColorSpace::OHTA: return fromOHTA(c);
        case ColorSpace::CMY: return fromCMY(c);
        case ColorSpace::HSB: return fromHSB(c);
        case ColorSpace::XYZ: return fromXYZ(c);
        case ColorSpace::YXY: return fromYXY(c);
        case ColorSpace::HCL: return fromHCL(c);
        case ColorSpace::LUV: return fromLUV(c);
        case ColorSpace::LAB: return fromLAB(c);
        case ColorSpace::HWB: return fromHWB(c);
        case ColorSpace::RGGBG: return fromRGGBG(c);
        case ColorSpace::YPbPr: return fromYPbPr(c);
        case ColorSpace::YCbCr: return fromYCbCr(c);
        case ColorSpace::YDbDr: return fromYDbDr(c);
        case ColorSpace::GS: return fromGS(c);
        case ColorSpace::YUV: return fromYUV(c);
        default: return c;
    }
}

// Greyscale
Color toGS(Color c) {
    int l = getLuma(c);
    return blendRGB(c, l, l, l);
}

Color fromGS(Color c) {
    return toGS(c);
}

// YUV
Color toYUV(Color c) {
    int R = getR(c);
    int G = getG(c);
    int B = getB(c);

    int Y = static_cast<int>(0.299f * R + 0.587f * G + 0.114f * B);
    int U = static_cast<int>(mapf(-0.14713f * R - 0.28886f * G + 0.436f * B, -Umax, Umax, 0, 255));
    int V = static_cast<int>(mapf(0.615f * R - 0.51499f * G - 0.10001f * B, -Vmax, Vmax, 0, 255));

    return blendRGB(c, Y, U, V);
}

Color fromYUV(Color c) {
    int Y = getR(c);
    float U = mapf(static_cast<float>(getG(c)), 0, 255, -Umax, Umax);
    float V = mapf(static_cast<float>(getB(c)), 0, 255, -Vmax, Vmax);

    int R = static_cast<int>(Y + 1.13983f * V);
    int G = static_cast<int>(Y - 0.39465f * U - 0.58060f * V);
    int B = static_cast<int>(Y + 2.03211f * U);

    return blendRGB(c, R, G, B);
}

// YDbDr
Color toYDbDr(Color c) {
    int R = getR(c);
    int G = getG(c);
    int B = getB(c);

    int Y = static_cast<int>(0.299f * R + 0.587f * G + 0.114f * B);
    int Db = static_cast<int>(127.5f + (-0.450f * R - 0.883f * G + 1.333f * B) / 2.666f);
    int Dr = static_cast<int>(127.5f + (-1.333f * R + 1.116f * G + 0.217f * B) / 2.666f);

    return blendRGB(c, Y, Db, Dr);
}

Color fromYDbDr(Color c) {
    int Y = getR(c);
    float Db = (getG(c) - 127.5f) * 2.666f;
    float Dr = (getB(c) - 127.5f) * 2.666f;

    int R = static_cast<int>(Y + 9.2303716147657e-05f * Db - 0.52591263066186533f * Dr);
    int G = static_cast<int>(Y - 0.12913289889050927f * Db + 0.26789932820759876f * Dr);
    int B = static_cast<int>(Y + 0.66467905997895482f * Db - 7.9202543533108e-05f * Dr);

    return blendRGB(c, R, G, B);
}

// YCbCr
Color toYCbCr(Color c) {
    int R = getR(c);
    int G = getG(c);
    int B = getB(c);

    int Y = static_cast<int>(0.2988390f * R + 0.5868110f * G + 0.1143500f * B);
    int Cb = static_cast<int>(-0.168736f * R - 0.3312640f * G + 0.5000000f * B + 127.5f);
    int Cr = static_cast<int>(0.5000000f * R - 0.4186880f * G - 0.0813120f * B + 127.5f);

    return blendRGB(c, Y, Cb, Cr);
}

Color fromYCbCr(Color c) {
    int Y = getR(c);
    float Cb = getG(c) - 127.5f;
    float Cr = getB(c) - 127.5f;

    int R = static_cast<int>(Y + 1.402f * Cr) + 1;
    int G = static_cast<int>(Y - 0.344136f * Cb - 0.714136f * Cr);
    int B = static_cast<int>(Y + 1.772000f * Cb) + 1;

    return blendRGB(c, R, G, B);
}

// YPbPr
Color toYPbPr(Color c) {
    int R = getR(c);
    int B = getB(c);

    int Y = getLuma(c);
    int Pb = B - Y;
    int Pr = R - Y;
    if (Pb < 0) Pb += 256;
    if (Pr < 0) Pr += 256;
    return blendRGB(c, Y, Pb, Pr);
}

Color fromYPbPr(Color c) {
    int Y = getR(c);
    int B = getG(c) + Y;
    int R = getB(c) + Y;
    if (R > 255) R -= 256;
    if (B > 255) B -= 256;

    int G = static_cast<int>((Y - 0.2126f * R - 0.0722f * B) / 0.7152f);

    return blendRGB(c, R, G, B);
}

// R-GGB-G
Color toRGGBG(Color c) {
    int G = getG(c);
    int R = getR(c) - G;
    int B = getB(c) - G;
    if (R < 0) R += 256;
    if (B < 0) B += 256;
    return blendRGB(c, R, G, B);
}

Color fromRGGBG(Color c) {
    int G = getG(c);
    int R = getR(c) + G;
    int B = getB(c) + G;
    if (R > 255) R -= 256;
    if (B > 255) B -= 256;
    return blendRGB(c, R, G, B);
}

// HSB
Color toHSB(Color c) {
    int R = getR(c);
    int G = getG(c);
    int B = getB(c);

    int _min = std::min({R, G, B});
    int _max = std::max({R, G, B});
    float delta = static_cast<float>(_max - _min);
    float saturation = _max > 0 ? delta / _max : 0;
    float brightness = r255[_max];

    if (delta == 0.0f) return blendRGB(c, 0, static_cast<int>(saturation * 255), static_cast<int>(brightness * 255));

    float hue = 0;
    if (R == _max) hue = static_cast<float>(G - B) / delta;
    else if (G == _max) hue = 2.0f + static_cast<float>(B - R) / delta;
    else hue = 4.0f + static_cast<float>(R - G) / delta;
    hue /= 6.0f;
    if (hue < 0.0f) hue += 1.0f;

    return blendRGB(c, static_cast<int>(hue * 255), static_cast<int>(saturation * 255), static_cast<int>(brightness * 255));
}

Color fromHSB(Color c) {
    float S = getNG(c);
    float B = getNB(c);
    if (S == 0.0f) {
        int v = static_cast<int>(B * 255);
        return blendRGB(c, v, v, v);
    }

    float h = 6.0f * getNR(c);
    float f = h - std::floor(h);
    float p = B * (1.0f - S);
    float q = B * (1.0f - S * f);
    float t = B * (1.0f - (S * (1.0f - f)));

    float r, g, b;
    switch (static_cast<int>(h)) {
        case 1: r = q; g = B; b = p; break;
        case 2: r = p; g = B; b = t; break;
        case 3: r = p; g = q; b = B; break;
        case 4: r = t; g = p; b = B; break;
        case 5: r = B; g = p; b = q; break;
        default: r = B; g = t; b = p; break;
    }
    return blendRGB(c, static_cast<int>(r * 255), static_cast<int>(g * 255), static_cast<int>(b * 255));
}

// HWB
Color toHWB(Color c) {
    int R = getR(c);
    int G = getG(c);
    int B = getB(c);

    int w = std::min({R, G, B});
    int v = std::max({R, G, B});

    int hue;
    if (v == w) {
        hue = 255;
    } else {
        float f = (R == w) ? static_cast<float>(G - B) : ((G == w) ? static_cast<float>(B - R) : static_cast<float>(R - G));
        float p = (R == w) ? 3.0f : ((G == w) ? 5.0f : 1.0f);
        hue = static_cast<int>(mapf((p - f / (v - w)) / 6.0f, 0, 1, 0, 254));
    }
    return blendRGB(c, hue, w, 255 - v);
}

Color fromHWB(Color c) {
    int H = getR(c);
    int B = 255 - getB(c);
    if (H == 255) {
        return blendRGB(c, B, B, B);
    }

    float hue = mapf(static_cast<float>(H), 0, 254, 0, 6);
    float v = r255[B];
    float whiteness = getNG(c);
    int i = static_cast<int>(std::floor(hue));
    float f = hue - i;
    if ((i & 0x01) != 0) f = 1.0f - f;
    float n = whiteness + f * (v - whiteness);

    float r, g, b;
    switch (i) {
        case 1: r = n; g = v; b = whiteness; break;
        case 2: r = whiteness; g = v; b = n; break;
        case 3: r = whiteness; g = n; b = v; break;
        case 4: r = n; g = whiteness; b = v; break;
        case 5: r = v; g = whiteness; b = n; break;
        default: r = v; g = n; b = whiteness; break;
    }
    return blendRGB(c, static_cast<int>(r * 255), static_cast<int>(g * 255), static_cast<int>(b * 255));
}

// LAB
Color toLAB(Color c) {
    Vec3 xyz = _toXYZ(getNR(c), getNG(c), getNB(c));
    xyz.x /= 100.0f;
    xyz.y /= 100.0f;
    xyz.z /= 100.0f;
    xyz.x /= D65X;
    xyz.y /= D65Y;
    xyz.z /= D65Z;

    float x, y, z;
    if (xyz.x > CIEEpsilon) x = std::pow(xyz.x, One_Third);
    else x = (CIEK * xyz.x + 16.0f) * one_hsixteen;

    if (xyz.y > CIEEpsilon) y = std::pow(xyz.y, One_Third);
    else y = (CIEK * xyz.y + 16.0f) * one_hsixteen;

    if (xyz.z > CIEEpsilon) z = std::pow(xyz.z, One_Third);
    else z = (CIEK * xyz.z + 16.0f) * one_hsixteen;

    float L = 255.0f * (((116.0f * y) - 16.0f) * 0.01f);
    float a = 255.0f * (0.5f * (x - y) + 0.5f);
    float b = 255.0f * (0.5f * (y - z) + 0.5f);

    return blendRGB(c, static_cast<int>(std::round(L)), static_cast<int>(std::round(a)), static_cast<int>(std::round(b)));
}

Color fromLAB(Color c) {
    float L = 100 * getNR(c);
    float a = getNG(c) - 0.5f;
    float b = getNB(c) - 0.5f;

    float y = (L + 16.0f) * one_hsixteen;
    float x = y + a;
    float z = y - b;

    float xxx = x * x * x;
    if (xxx > CIEEpsilon) x = xxx;
    else x = (116.0f * x - 16.0f) / CIEK;

    float yyy = y * y * y;
    if (yyy > CIEEpsilon) y = yyy;
    else y = L / CIEK;

    float zzz = z * z * z;
    if (zzz > CIEEpsilon) z = zzz;
    else z = (116.0f * z - 16.0f) / CIEK;

    return _fromXYZ(c, RANGE_X * x, RANGE_Y * y, RANGE_Z * z);
}

// LUV
Color toLUV(Color c) {
    Vec3 xyz = _toXYZ(getNR(c), getNG(c), getNB(c));
    xyz.x /= 100.0f;
    xyz.y /= 100.0f;
    xyz.z /= 100.0f;

    float d = xyz.y;
    float L;
    if (d > CIEEpsilon) L = 116.0f * std::pow(d, One_Third) - 16.0f;
    else L = CIEK * d;

    float alpha = PerceptibleReciprocal(xyz.x + 15.0f * xyz.y + 3.0f * xyz.z);
    float L13 = 13.0f * L;
    float u = L13 * ((4.0f * alpha * xyz.x) - D65FX_4);
    float v = L13 * ((9.0f * alpha * xyz.y) - D65FY_9);

    L /= 100.0f;
    u = (u + 134.0f) / 354.0f;
    v = (v + 140.0f) / 262.0f;

    return blendRGB(c, static_cast<int>(std::round(L * 255)), static_cast<int>(std::round(u * 255)), static_cast<int>(std::round(v * 255)));
}

Color fromLUV(Color c) {
    float L = 100.0f * getNR(c);
    float u = 354.0f * getNG(c) - 134.0f;
    float v = 262.0f * getNB(c) - 140.0f;

    float X, Y, Z;
    if (L > CIEK2epsilon) Y = std::pow((L + 16.0f) * one_hsixteen, 3.0f);
    else Y = L / CIEK;

    float L13 = 13.0f * L;
    float L52 = 52.0f * L;
    float Y5 = 5.0f * Y;
    float L13u = L52 / (u + L13 * D65FX_4);
    X = ((Y * ((39.0f * L / (v + L13 * D65FY_9)) - 5.0f)) + Y5) / ((((L13u) - 1.0f) / 3.0f) + One_Third);
    Z = (X * (((L13u) - 1.0f) / 3.0f)) - Y5;

    return _fromXYZ(c, 100 * X, 100 * Y, 100 * Z);
}

// HCL
Color toHCL(Color c) {
    float r = getNR(c);
    float g = getNG(c);
    float b = getNB(c);
    float maxVal = std::max({r, g, b});
    float chr = maxVal - std::min({r, g, b});

    float h = 0.0f;
    if (chr != 0) {
        if (r == maxVal) h = std::fmod((g - b) / chr + 6.0f, 6.0f);
        else if (g == maxVal) h = (b - r) / chr + 2.0f;
        else h = (r - g) / chr + 4.0f;
    }

    return blendRGB(c,
        static_cast<int>(std::round((h / 6.0f) * 255)),
        static_cast<int>(std::round(chr * 255)),
        static_cast<int>(std::round(255 * (0.298839f * r + 0.586811f * g + 0.114350f * b))));
}

Color fromHCL(Color c) {
    float h = 6.0f * getNR(c);
    float chr = getNG(c);
    float l = getNB(c);
    float x = chr * (1.0f - std::abs(std::fmod(h, 2.0f) - 1.0f));

    float r = 0.0f, g = 0.0f, b = 0.0f;
    if (h >= 0.0f && h < 1.0f) { r = chr; g = x; }
    else if (h >= 1.0f && h < 2.0f) { r = x; g = chr; }
    else if (h >= 2.0f && h < 3.0f) { g = chr; b = x; }
    else if (h >= 3.0f && h < 4.0f) { g = x; b = chr; }
    else if (h >= 4.0f && h < 5.0f) { r = x; b = chr; }
    else { r = chr; b = x; }

    float m = l - (0.298839f * r + 0.586811f * g + 0.114350f * b);
    return blendRGB(c,
        static_cast<int>(std::round(255 * (r + m))),
        static_cast<int>(std::round(255 * (g + m))),
        static_cast<int>(std::round(255 * (b + m))));
}

// YXY
Color toYXY(Color c) {
    Vec3 xyz = _toXYZ(getNR(c), getNG(c), getNB(c));
    float sum = xyz.x + xyz.y + xyz.z;
    float x = xyz.x > 0 ? xyz.x / sum : 0.0f;
    float y = xyz.y > 0 ? xyz.y / sum : 0.0f;

    return blendRGB(c,
        static_cast<int>(mapf(xyz.y, 0, RANGE_Y, 0, 255)),
        static_cast<int>(mapf(x, 0.0f, 1.0f, 0, 255)),
        static_cast<int>(mapf(y, 0.0f, 1.0f, 0, 255)));
}

Color fromYXY(Color c) {
    float Y = mapf(static_cast<float>(getR(c)), 0, 255, 0, RANGE_Y);
    float x = mapf(static_cast<float>(getG(c)), 0, 255, 0, 1.0f);
    float y = mapf(static_cast<float>(getB(c)), 0, 255, 0, 1.0f);
    float divy = Y / (y > 0 ? y : 1.0e-6f);

    return _fromXYZ(c, x * divy, Y, (1 - x - y) * divy);
}

// XYZ
Color toXYZ(Color c) {
    Vec3 xyz = _toXYZ(getNR(c), getNG(c), getNB(c));
    return blendRGB(c,
        static_cast<int>(mapf(xyz.x, 0, RANGE_X, 0, 255)),
        static_cast<int>(mapf(xyz.y, 0, RANGE_Y, 0, 255)),
        static_cast<int>(mapf(xyz.z, 0, RANGE_Z, 0, 255)));
}

Color fromXYZ(Color c) {
    float x = mapf(static_cast<float>(getR(c)), 0, 255, 0, RANGE_X);
    float y = mapf(static_cast<float>(getG(c)), 0, 255, 0, RANGE_Y);
    float z = mapf(static_cast<float>(getB(c)), 0, 255, 0, RANGE_Z);
    return _fromXYZ(c, x, y, z);
}

// CMY
Color toCMY(Color c) {
    return blendRGB(c, 255 - getR(c), 255 - getG(c), 255 - getB(c));
}

Color fromCMY(Color c) {
    return toCMY(c);
}

// OHTA
Color toOHTA(Color c) {
    int R = getR(c);
    int G = getG(c);
    int B = getB(c);

    int I1 = static_cast<int>(0.33333f * R + 0.33334f * G + 0.33333f * B);
    int I2 = static_cast<int>(mapf(0.5f * (R - B), -127.5f, 127.5f, 0, 255));
    int I3 = static_cast<int>(mapf(-0.25000f * R + 0.50000f * G - 0.25000f * B, -127.5f, 127.5f, 0, 255));

    return blendRGB(c, I1, I2, I3);
}

Color fromOHTA(Color c) {
    int I1 = getR(c);
    float I2 = mapf(static_cast<float>(getG(c)), 0, 255, -127.5f, 127.5f);
    float I3 = mapf(static_cast<float>(getB(c)), 0, 255, -127.5f, 127.5f);

    int R = static_cast<int>(I1 + 1.00000f * I2 - 0.66668f * I3);
    int G = static_cast<int>(I1 + 1.33333f * I3);
    int B = static_cast<int>(I1 - 1.00000f * I2 - 0.66668f * I3);

    return blendRGB(c, R, G, B);
}

// Utility function implementations
std::string predictionName(PredictionMethod pm) {
    switch (pm) {
        case PredictionMethod::SAD: return "SAD";
        case PredictionMethod::BSAD: return "BSAD";
        case PredictionMethod::RANDOM: return "RANDOM";
        case PredictionMethod::NONE: return "NONE";
        case PredictionMethod::CORNER: return "CORNER";
        case PredictionMethod::H: return "H";
        case PredictionMethod::V: return "V";
        case PredictionMethod::DC: return "DC";
        case PredictionMethod::DCMEDIAN: return "DCMEDIAN";
        case PredictionMethod::MEDIAN: return "MEDIAN";
        case PredictionMethod::AVG: return "AVG";
        case PredictionMethod::TRUEMOTION: return "TRUEMOTION";
        case PredictionMethod::PAETH: return "PAETH";
        case PredictionMethod::LDIAG: return "LDIAG";
        case PredictionMethod::HV: return "HV";
        case PredictionMethod::JPEGLS: return "JPEGLS";
        case PredictionMethod::DIFF: return "DIFF";
        case PredictionMethod::REF: return "REF";
        case PredictionMethod::ANGLE: return "ANGLE";
        case PredictionMethod::SPIRAL: return "SPIRAL";
        case PredictionMethod::NOISE: return "NOISE";
        case PredictionMethod::GRADIENT: return "GRADIENT";
        case PredictionMethod::MIRROR: return "MIRROR";
        case PredictionMethod::WAVE: return "WAVE";
        case PredictionMethod::CHECKERBOARD: return "CHECKERBOARD";
        case PredictionMethod::RADIAL: return "RADIAL";
        case PredictionMethod::EDGE: return "EDGE";
        default: return "NONE";
    }
}

PredictionMethod predictionFromName(const std::string& name) {
    if (name == "SAD") return PredictionMethod::SAD;
    if (name == "BSAD") return PredictionMethod::BSAD;
    if (name == "RANDOM") return PredictionMethod::RANDOM;
    if (name == "NONE") return PredictionMethod::NONE;
    if (name == "CORNER") return PredictionMethod::CORNER;
    if (name == "H") return PredictionMethod::H;
    if (name == "V") return PredictionMethod::V;
    if (name == "DC") return PredictionMethod::DC;
    if (name == "DCMEDIAN") return PredictionMethod::DCMEDIAN;
    if (name == "MEDIAN") return PredictionMethod::MEDIAN;
    if (name == "AVG") return PredictionMethod::AVG;
    if (name == "TRUEMOTION") return PredictionMethod::TRUEMOTION;
    if (name == "PAETH") return PredictionMethod::PAETH;
    if (name == "LDIAG") return PredictionMethod::LDIAG;
    if (name == "HV") return PredictionMethod::HV;
    if (name == "JPEGLS") return PredictionMethod::JPEGLS;
    if (name == "DIFF") return PredictionMethod::DIFF;
    if (name == "REF") return PredictionMethod::REF;
    if (name == "ANGLE") return PredictionMethod::ANGLE;
    if (name == "SPIRAL") return PredictionMethod::SPIRAL;
    if (name == "NOISE") return PredictionMethod::NOISE;
    if (name == "GRADIENT") return PredictionMethod::GRADIENT;
    if (name == "MIRROR") return PredictionMethod::MIRROR;
    if (name == "WAVE") return PredictionMethod::WAVE;
    if (name == "CHECKERBOARD") return PredictionMethod::CHECKERBOARD;
    if (name == "RADIAL") return PredictionMethod::RADIAL;
    if (name == "EDGE") return PredictionMethod::EDGE;
    return PredictionMethod::NONE;
}

std::string waveletName(WaveletType wt) {
    switch (wt) {
        case WaveletType::NONE: return "NONE";
        case WaveletType::HAAR: return "HAAR";
        case WaveletType::HAAR_ORTHOGONAL: return "HAAR_ORTHOGONAL";
        case WaveletType::DAUBECHIES2: return "DAUBECHIES2";
        case WaveletType::DAUBECHIES3: return "DAUBECHIES3";
        case WaveletType::DAUBECHIES4: return "DAUBECHIES4";
        case WaveletType::DAUBECHIES5: return "DAUBECHIES5";
        case WaveletType::DAUBECHIES6: return "DAUBECHIES6";
        case WaveletType::DAUBECHIES7: return "DAUBECHIES7";
        case WaveletType::DAUBECHIES8: return "DAUBECHIES8";
        case WaveletType::DAUBECHIES9: return "DAUBECHIES9";
        case WaveletType::DAUBECHIES10: return "DAUBECHIES10";
        case WaveletType::SYMLET2: return "SYMLET2";
        case WaveletType::SYMLET3: return "SYMLET3";
        case WaveletType::SYMLET4: return "SYMLET4";
        case WaveletType::SYMLET5: return "SYMLET5";
        case WaveletType::SYMLET6: return "SYMLET6";
        case WaveletType::SYMLET7: return "SYMLET7";
        case WaveletType::SYMLET8: return "SYMLET8";
        case WaveletType::SYMLET9: return "SYMLET9";
        case WaveletType::SYMLET10: return "SYMLET10";
        case WaveletType::COIFLET1: return "COIFLET1";
        case WaveletType::COIFLET2: return "COIFLET2";
        case WaveletType::COIFLET3: return "COIFLET3";
        case WaveletType::COIFLET4: return "COIFLET4";
        case WaveletType::COIFLET5: return "COIFLET5";
        default: return "NONE";
    }
}

WaveletType waveletFromName(const std::string& name) {
    if (name == "NONE") return WaveletType::NONE;
    if (name == "HAAR") return WaveletType::HAAR;
    if (name == "HAAR_ORTHOGONAL") return WaveletType::HAAR_ORTHOGONAL;
    if (name == "DAUBECHIES2" || name == "DB2") return WaveletType::DAUBECHIES2;
    if (name == "DAUBECHIES3" || name == "DB3") return WaveletType::DAUBECHIES3;
    if (name == "DAUBECHIES4" || name == "DB4") return WaveletType::DAUBECHIES4;
    if (name == "DAUBECHIES5" || name == "DB5") return WaveletType::DAUBECHIES5;
    if (name == "DAUBECHIES6" || name == "DB6") return WaveletType::DAUBECHIES6;
    if (name == "DAUBECHIES7" || name == "DB7") return WaveletType::DAUBECHIES7;
    if (name == "DAUBECHIES8" || name == "DB8") return WaveletType::DAUBECHIES8;
    if (name == "DAUBECHIES9" || name == "DB9") return WaveletType::DAUBECHIES9;
    if (name == "DAUBECHIES10" || name == "DB10") return WaveletType::DAUBECHIES10;
    if (name == "SYMLET2" || name == "SYM2") return WaveletType::SYMLET2;
    if (name == "SYMLET3" || name == "SYM3") return WaveletType::SYMLET3;
    if (name == "SYMLET4" || name == "SYM4") return WaveletType::SYMLET4;
    if (name == "SYMLET5" || name == "SYM5") return WaveletType::SYMLET5;
    if (name == "SYMLET6" || name == "SYM6") return WaveletType::SYMLET6;
    if (name == "SYMLET7" || name == "SYM7") return WaveletType::SYMLET7;
    if (name == "SYMLET8" || name == "SYM8") return WaveletType::SYMLET8;
    if (name == "SYMLET9" || name == "SYM9") return WaveletType::SYMLET9;
    if (name == "SYMLET10" || name == "SYM10") return WaveletType::SYMLET10;
    if (name == "COIFLET1" || name == "COIF1") return WaveletType::COIFLET1;
    if (name == "COIFLET2" || name == "COIF2") return WaveletType::COIFLET2;
    if (name == "COIFLET3" || name == "COIF3") return WaveletType::COIFLET3;
    if (name == "COIFLET4" || name == "COIF4") return WaveletType::COIFLET4;
    if (name == "COIFLET5" || name == "COIF5") return WaveletType::COIFLET5;
    return WaveletType::NONE;
}

std::string encodingName(EncodingMethod em) {
    switch (em) {
        case EncodingMethod::RAW: return "RAW";
        case EncodingMethod::PACKED: return "PACKED";
        case EncodingMethod::RLE: return "RLE";
        case EncodingMethod::DELTA: return "DELTA";
        case EncodingMethod::XOR: return "XOR";
        case EncodingMethod::ZIGZAG: return "ZIGZAG";
        default: return "RAW";
    }
}

EncodingMethod encodingFromName(const std::string& name) {
    if (name == "RAW") return EncodingMethod::RAW;
    if (name == "PACKED") return EncodingMethod::PACKED;
    if (name == "RLE") return EncodingMethod::RLE;
    if (name == "DELTA") return EncodingMethod::DELTA;
    if (name == "XOR") return EncodingMethod::XOR;
    if (name == "ZIGZAG") return EncodingMethod::ZIGZAG;
    return EncodingMethod::RAW;
}

} // namespace glic
