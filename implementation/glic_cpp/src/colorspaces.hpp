#pragma once

#include "config.hpp"

namespace glic {

// Convert from RGB to specified color space
Color toColorSpace(Color c, ColorSpace cs);

// Convert from specified color space to RGB
Color fromColorSpace(Color c, ColorSpace cs);

// Individual color space conversions (to)
Color toOHTA(Color c);
Color toCMY(Color c);
Color toHSB(Color c);
Color toXYZ(Color c);
Color toYXY(Color c);
Color toHCL(Color c);
Color toLUV(Color c);
Color toLAB(Color c);
Color toHWB(Color c);
Color toRGGBG(Color c);
Color toYPbPr(Color c);
Color toYCbCr(Color c);
Color toYDbDr(Color c);
Color toGS(Color c);
Color toYUV(Color c);

// Individual color space conversions (from)
Color fromOHTA(Color c);
Color fromCMY(Color c);
Color fromHSB(Color c);
Color fromXYZ(Color c);
Color fromYXY(Color c);
Color fromHCL(Color c);
Color fromLUV(Color c);
Color fromLAB(Color c);
Color fromHWB(Color c);
Color fromRGGBG(Color c);
Color fromYPbPr(Color c);
Color fromYCbCr(Color c);
Color fromYDbDr(Color c);
Color fromGS(Color c);
Color fromYUV(Color c);

} // namespace glic
