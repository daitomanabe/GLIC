#pragma once

#include <cstdint>
#include <array>
#include <string>

namespace glic {

// Color spaces
enum class ColorSpace : uint8_t {
    OHTA = 0,
    RGB = 1,
    CMY = 2,
    HSB = 3,
    XYZ = 4,
    YXY = 5,
    HCL = 6,
    LUV = 7,
    LAB = 8,
    HWB = 9,
    RGGBG = 10,
    YPbPr = 11,
    YCbCr = 12,
    YDbDr = 13,
    GS = 14,
    YUV = 15,
    COUNT = 16
};

std::string colorSpaceName(ColorSpace cs);
ColorSpace colorSpaceFromName(const std::string& name);

// Prediction methods
enum class PredictionMethod : int8_t {
    SAD = -1,
    BSAD = -2,
    RANDOM = -3,
    NONE = 0,
    CORNER = 1,
    H = 2,
    V = 3,
    DC = 4,
    DCMEDIAN = 5,
    MEDIAN = 6,
    AVG = 7,
    TRUEMOTION = 8,
    PAETH = 9,
    LDIAG = 10,
    HV = 11,
    JPEGLS = 12,
    DIFF = 13,
    REF = 14,
    ANGLE = 15,
    // New prediction methods
    SPIRAL = 16,
    NOISE = 17,
    GRADIENT = 18,
    MIRROR = 19,
    WAVE = 20,
    CHECKERBOARD = 21,
    RADIAL = 22,
    EDGE = 23,
    COUNT = 24
};

std::string predictionName(PredictionMethod pm);
PredictionMethod predictionFromName(const std::string& name);

// Clamp methods
enum class ClampMethod : uint8_t {
    NONE = 0,
    MOD256 = 1
};

// Transform types
enum class TransformType : uint8_t {
    RANDOM = 255,
    FWT = 0,
    WPT = 1,
    COUNT = 2
};

// Wavelet types
enum class WaveletType : uint8_t {
    RANDOM = 255,
    NONE = 0,
    HAAR_ORTHOGONAL = 1,
    BIORTHOGONAL11 = 2,
    BIORTHOGONAL13 = 3,
    BIORTHOGONAL15 = 4,
    BIORTHOGONAL22 = 5,
    BIORTHOGONAL24 = 6,
    BIORTHOGONAL26 = 7,
    BIORTHOGONAL28 = 8,
    BIORTHOGONAL31 = 9,
    BIORTHOGONAL33 = 10,
    BIORTHOGONAL35 = 11,
    BIORTHOGONAL37 = 12,
    BIORTHOGONAL39 = 13,
    BIORTHOGONAL44 = 14,
    BIORTHOGONAL55 = 15,
    BIORTHOGONAL68 = 16,
    COIFLET1 = 17,
    COIFLET2 = 18,
    COIFLET3 = 19,
    COIFLET4 = 20,
    COIFLET5 = 21,
    SYMLET2 = 22,
    SYMLET3 = 23,
    SYMLET4 = 24,
    SYMLET5 = 25,
    SYMLET6 = 26,
    SYMLET7 = 27,
    SYMLET8 = 28,
    SYMLET9 = 29,
    SYMLET10 = 30,
    DAUBECHIES2 = 31,
    DAUBECHIES3 = 32,
    DAUBECHIES4 = 33,
    DAUBECHIES5 = 34,
    DAUBECHIES6 = 35,
    DAUBECHIES7 = 36,
    DAUBECHIES8 = 37,
    DAUBECHIES9 = 38,
    DAUBECHIES10 = 39,
    HAAR = 40,
    COUNT = 41
};

std::string waveletName(WaveletType wt);
WaveletType waveletFromName(const std::string& name);

// Encoding methods
enum class EncodingMethod : uint8_t {
    RAW = 0,
    PACKED = 1,
    RLE = 2,
    // New encoding methods
    DELTA = 3,
    XOR = 4,
    ZIGZAG = 5,
    COUNT = 6
};

std::string encodingName(EncodingMethod em);
EncodingMethod encodingFromName(const std::string& name);

// Codec configuration for a single channel
struct ChannelConfig {
    int minBlockSize = 2;
    int maxBlockSize = 256;
    float segmentationPrecision = 15.0f;
    PredictionMethod predictionMethod = PredictionMethod::PAETH;
    int quantizationValue = 110;
    ClampMethod clampMethod = ClampMethod::NONE;
    TransformType transformType = TransformType::FWT;
    WaveletType waveletType = WaveletType::SYMLET8;
    float transformCompress = 0.0f;
    int transformScale = 20;
    EncodingMethod encodingMethod = EncodingMethod::PACKED;
};

// Full codec configuration
struct CodecConfig {
    ColorSpace colorSpace = ColorSpace::HWB;
    uint8_t borderColorR = 128;
    uint8_t borderColorG = 128;
    uint8_t borderColorB = 128;
    std::array<ChannelConfig, 3> channels;

    CodecConfig() {
        for (auto& ch : channels) {
            ch = ChannelConfig{};
        }
    }
};

// Color type (ARGB packed)
using Color = uint32_t;

inline uint8_t getA(Color c) { return (c >> 24) & 0xFF; }
inline uint8_t getR(Color c) { return (c >> 16) & 0xFF; }
inline uint8_t getG(Color c) { return (c >> 8) & 0xFF; }
inline uint8_t getB(Color c) { return c & 0xFF; }

inline Color makeColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    return (static_cast<uint32_t>(a) << 24) |
           (static_cast<uint32_t>(r) << 16) |
           (static_cast<uint32_t>(g) << 8) |
           static_cast<uint32_t>(b);
}

inline Color blendRGB(Color c, int r, int g, int b) {
    r = std::max(0, std::min(255, r));
    g = std::max(0, std::min(255, g));
    b = std::max(0, std::min(255, b));
    return (c & 0xFF000000) | (r << 16) | (g << 8) | b;
}

// Helper for normalized color values (0-1)
constexpr float r255[256] = {
    0.0f, 0.003921569f, 0.007843138f, 0.011764706f, 0.015686275f, 0.019607844f, 0.023529412f, 0.02745098f,
    0.03137255f, 0.03529412f, 0.039215688f, 0.043137256f, 0.047058824f, 0.050980393f, 0.05490196f, 0.05882353f,
    0.0627451f, 0.06666667f, 0.07058824f, 0.07450981f, 0.078431375f, 0.08235294f, 0.08627451f, 0.09019608f,
    0.09411765f, 0.09803922f, 0.101960786f, 0.105882354f, 0.10980392f, 0.11372549f, 0.11764706f, 0.12156863f,
    0.1254902f, 0.12941177f, 0.13333334f, 0.13725491f, 0.14117648f, 0.14509805f, 0.14901961f, 0.15294118f,
    0.15686275f, 0.16078432f, 0.16470589f, 0.16862746f, 0.17254902f, 0.1764706f, 0.18039216f, 0.18431373f,
    0.1882353f, 0.19215687f, 0.19607843f, 0.2f, 0.20392157f, 0.20784314f, 0.21176471f, 0.21568628f,
    0.21960784f, 0.22352941f, 0.22745098f, 0.23137255f, 0.23529412f, 0.23921569f, 0.24313726f, 0.24705882f,
    0.2509804f, 0.25490198f, 0.25882354f, 0.2627451f, 0.26666668f, 0.27058825f, 0.27450982f, 0.2784314f,
    0.28235295f, 0.28627452f, 0.2901961f, 0.29411766f, 0.29803923f, 0.3019608f, 0.30588236f, 0.30980393f,
    0.3137255f, 0.31764707f, 0.32156864f, 0.3254902f, 0.32941177f, 0.33333334f, 0.3372549f, 0.34117648f,
    0.34509805f, 0.34901962f, 0.3529412f, 0.35686275f, 0.36078432f, 0.3647059f, 0.36862746f, 0.37254903f,
    0.3764706f, 0.38039216f, 0.38431373f, 0.3882353f, 0.39215687f, 0.39607844f, 0.4f, 0.40392157f,
    0.40784314f, 0.4117647f, 0.41568628f, 0.41960785f, 0.42352942f, 0.42745098f, 0.43137255f, 0.43529412f,
    0.4392157f, 0.44313726f, 0.44705883f, 0.4509804f, 0.45490196f, 0.45882353f, 0.4627451f, 0.46666667f,
    0.47058824f, 0.4745098f, 0.47843137f, 0.48235294f, 0.4862745f, 0.49019608f, 0.49411765f, 0.49803922f,
    0.5019608f, 0.5058824f, 0.50980395f, 0.5137255f, 0.5176471f, 0.52156866f, 0.5254902f, 0.5294118f,
    0.53333336f, 0.5372549f, 0.5411765f, 0.54509807f, 0.54901963f, 0.5529412f, 0.5568628f, 0.56078434f,
    0.5647059f, 0.5686275f, 0.57254905f, 0.5764706f, 0.5803922f, 0.58431375f, 0.5882353f, 0.5921569f,
    0.59607846f, 0.6f, 0.6039216f, 0.60784316f, 0.6117647f, 0.6156863f, 0.61960787f, 0.62352943f,
    0.627451f, 0.6313726f, 0.63529414f, 0.6392157f, 0.6431373f, 0.64705884f, 0.6509804f, 0.654902f,
    0.65882355f, 0.6627451f, 0.6666667f, 0.67058825f, 0.6745098f, 0.6784314f, 0.68235296f, 0.6862745f,
    0.6901961f, 0.69411767f, 0.69803923f, 0.7019608f, 0.7058824f, 0.70980394f, 0.7137255f, 0.7176471f,
    0.72156864f, 0.7254902f, 0.7294118f, 0.73333335f, 0.7372549f, 0.7411765f, 0.74509805f, 0.7490196f,
    0.7529412f, 0.75686276f, 0.7607843f, 0.7647059f, 0.76862746f, 0.77254903f, 0.7764706f, 0.78039217f,
    0.78431374f, 0.7882353f, 0.7921569f, 0.79607844f, 0.8f, 0.8039216f, 0.80784315f, 0.8117647f,
    0.8156863f, 0.81960785f, 0.8235294f, 0.827451f, 0.83137256f, 0.8352941f, 0.8392157f, 0.84313726f,
    0.84705883f, 0.8509804f, 0.85490197f, 0.85882354f, 0.8627451f, 0.8666667f, 0.87058824f, 0.8745098f,
    0.8784314f, 0.88235295f, 0.8862745f, 0.8901961f, 0.89411765f, 0.8980392f, 0.9019608f, 0.90588236f,
    0.9098039f, 0.9137255f, 0.91764706f, 0.92156863f, 0.9254902f, 0.92941177f, 0.93333334f, 0.9372549f,
    0.9411765f, 0.94509804f, 0.9490196f, 0.9529412f, 0.95686275f, 0.9607843f, 0.9647059f, 0.96862745f,
    0.972549f, 0.9764706f, 0.98039216f, 0.9843137f, 0.9882353f, 0.99215686f, 0.99607843f, 1.0f
};

inline float getNR(Color c) { return r255[getR(c)]; }
inline float getNG(Color c) { return r255[getG(c)]; }
inline float getNB(Color c) { return r255[getB(c)]; }

inline int getLuma(Color c) {
    return std::max(0, std::min(255, static_cast<int>(0.2126f * getR(c) + 0.7152f * getG(c) + 0.0722f * getB(c))));
}

} // namespace glic
