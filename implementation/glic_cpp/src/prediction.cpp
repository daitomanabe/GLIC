#include "prediction.hpp"
#include "planes.hpp"
#include <cmath>
#include <algorithm>
#include <limits>
#include <random>

namespace glic {

namespace {

std::mt19937& getRng() {
    static std::mt19937 rng(12345);
    return rng;
}

int getMedian(int a, int b, int c) {
    return std::max(std::min(a, b), std::min(std::max(a, b), c));
}

int getDC(const Planes& p, int ch, const Segment& s) {
    int v = 0;
    for (int i = 0; i < s.size; i++) {
        v += p.get(ch, s.x - 1, s.y + i);
        v += p.get(ch, s.x + i, s.y - 1);
    }
    v += p.get(ch, s.x - 1, s.y - 1);
    v /= (s.size + s.size + 1);
    return v;
}

struct Vec2 {
    float x, y;
};

Vec2 getAngleRef(int i, int x, int y, float a, int w) {
    float xx = -1;
    float yy = -1;

    switch (i % 3) {
        case 0: {
            float v = (w - y - 1) + x * a;
            xx = (v - w) / a;
            yy = (w - 1 - a - v);
            break;
        }
        case 1: {
            float v = (w - x - 1) + y * a;
            yy = (v - w) / a;
            xx = (w - 1 - a - v);
            break;
        }
        case 2: {
            float v = x + y * a;
            yy = -1.0f;
            xx = v + a;
            break;
        }
    }

    if (xx > yy)
        return {std::round(xx), -1};
    else
        return {-1, std::round(yy)};
}

} // anonymous namespace

int getSAD(
    const std::vector<std::vector<int>>& pred,
    const Planes& planes,
    int channel,
    const Segment& segment
) {
    int sum = 0;
    for (int x = 0; x < segment.size; x++) {
        for (int y = 0; y < segment.size; y++) {
            sum += std::abs(planes.get(channel, segment.x + x, segment.y + y) - pred[x][y]);
        }
    }
    return sum;
}

std::vector<std::vector<int>> predict(
    PredictionMethod method,
    Planes& planes,
    int channel,
    Segment& segment
) {
    switch (method) {
        case PredictionMethod::CORNER:
            return predCorner(planes, channel, segment);
        case PredictionMethod::H:
            return predH(planes, channel, segment);
        case PredictionMethod::V:
            return predV(planes, channel, segment);
        case PredictionMethod::DC:
            return predDC(planes, channel, segment);
        case PredictionMethod::DCMEDIAN:
            return predDCMedian(planes, channel, segment);
        case PredictionMethod::MEDIAN:
            return predMedian(planes, channel, segment);
        case PredictionMethod::AVG:
            return predAvg(planes, channel, segment);
        case PredictionMethod::TRUEMOTION:
            return predTrueMotion(planes, channel, segment);
        case PredictionMethod::PAETH:
            return predPaeth(planes, channel, segment);
        case PredictionMethod::LDIAG:
            return predLDiag(planes, channel, segment);
        case PredictionMethod::HV:
            return predHV(planes, channel, segment);
        case PredictionMethod::JPEGLS:
            return predJpegLS(planes, channel, segment);
        case PredictionMethod::DIFF:
            return predDiff(planes, channel, segment);
        case PredictionMethod::REF:
            return predRef(planes, channel, segment);
        case PredictionMethod::ANGLE:
            return predAngle(planes, channel, segment);
        // New prediction methods
        case PredictionMethod::SPIRAL:
            return predSpiral(planes, channel, segment);
        case PredictionMethod::NOISE:
            return predNoise(planes, channel, segment);
        case PredictionMethod::GRADIENT:
            return predGradient(planes, channel, segment);
        case PredictionMethod::MIRROR:
            return predMirror(planes, channel, segment);
        case PredictionMethod::WAVE:
            return predWave(planes, channel, segment);
        case PredictionMethod::CHECKERBOARD:
            return predCheckerboard(planes, channel, segment);
        case PredictionMethod::RADIAL:
            return predRadial(planes, channel, segment);
        case PredictionMethod::EDGE:
            return predEdge(planes, channel, segment);
        case PredictionMethod::RANDOM: {
            auto& rng = getRng();
            std::uniform_int_distribution<int> dist(0, static_cast<int>(PredictionMethod::COUNT) - 1);
            return predict(static_cast<PredictionMethod>(dist(rng)), planes, channel, segment);
        }
        case PredictionMethod::SAD:
            return predSAD(planes, channel, segment, true);
        case PredictionMethod::BSAD:
            return predSAD(planes, channel, segment, false);
        default:
            return std::vector<std::vector<int>>(segment.size, std::vector<int>(segment.size, 0));
    }
}

std::vector<std::vector<int>> predCorner(const Planes& p, int ch, const Segment& s) {
    std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));
    int val = p.get(ch, s.x - 1, s.y - 1);
    for (int x = 0; x < s.size; x++) {
        for (int y = 0; y < s.size; y++) {
            res[x][y] = val;
        }
    }
    return res;
}

std::vector<std::vector<int>> predH(const Planes& p, int ch, const Segment& s) {
    std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));
    for (int x = 0; x < s.size; x++) {
        for (int y = 0; y < s.size; y++) {
            res[x][y] = p.get(ch, s.x - 1, s.y + y);
        }
    }
    return res;
}

std::vector<std::vector<int>> predV(const Planes& p, int ch, const Segment& s) {
    std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));
    for (int x = 0; x < s.size; x++) {
        for (int y = 0; y < s.size; y++) {
            res[x][y] = p.get(ch, s.x + x, s.y - 1);
        }
    }
    return res;
}

std::vector<std::vector<int>> predDC(const Planes& p, int ch, const Segment& s) {
    std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));
    int c = getDC(p, ch, s);
    for (int x = 0; x < s.size; x++) {
        for (int y = 0; y < s.size; y++) {
            res[x][y] = c;
        }
    }
    return res;
}

std::vector<std::vector<int>> predDCMedian(const Planes& p, int ch, const Segment& s) {
    std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));
    int c = getDC(p, ch, s);
    for (int x = 0; x < s.size; x++) {
        int v1 = p.get(ch, s.x + x, s.y - 1);
        for (int y = 0; y < s.size; y++) {
            int v2 = p.get(ch, s.x - 1, s.y + y);
            res[x][y] = getMedian(c, v1, v2);
        }
    }
    return res;
}

std::vector<std::vector<int>> predMedian(const Planes& p, int ch, const Segment& s) {
    std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));
    int c = p.get(ch, s.x - 1, s.y - 1);
    for (int x = 0; x < s.size; x++) {
        int v1 = p.get(ch, s.x + x, s.y - 1);
        for (int y = 0; y < s.size; y++) {
            int v2 = p.get(ch, s.x - 1, s.y + y);
            res[x][y] = getMedian(c, v1, v2);
        }
    }
    return res;
}

std::vector<std::vector<int>> predAvg(const Planes& p, int ch, const Segment& s) {
    std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));
    for (int x = 0; x < s.size; x++) {
        int v1 = p.get(ch, s.x + x, s.y - 1);
        for (int y = 0; y < s.size; y++) {
            int v2 = p.get(ch, s.x - 1, s.y + y);
            res[x][y] = (v1 + v2) >> 1;
        }
    }
    return res;
}

std::vector<std::vector<int>> predTrueMotion(const Planes& p, int ch, const Segment& s) {
    std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));
    int c = p.get(ch, s.x - 1, s.y - 1);
    for (int x = 0; x < s.size; x++) {
        int v1 = p.get(ch, s.x + x, s.y - 1);
        for (int y = 0; y < s.size; y++) {
            int v2 = p.get(ch, s.x - 1, s.y + y);
            res[x][y] = std::max(0, std::min(255, v1 + v2 - c));
        }
    }
    return res;
}

std::vector<std::vector<int>> predPaeth(const Planes& p, int ch, const Segment& s) {
    std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));
    int c = p.get(ch, s.x - 1, s.y - 1);
    for (int x = 0; x < s.size; x++) {
        int v1 = p.get(ch, s.x + x, s.y - 1);
        for (int y = 0; y < s.size; y++) {
            int v2 = p.get(ch, s.x - 1, s.y + y);
            int pp = v1 + v2 - c;
            int pa = std::abs(pp - v2);
            int pb = std::abs(pp - v1);
            int pc = std::abs(pp - c);
            int v = ((pa <= pb) && (pa <= pc)) ? v2 : (pb <= pc ? v1 : c);
            res[x][y] = std::max(0, std::min(255, v));
        }
    }
    return res;
}

std::vector<std::vector<int>> predLDiag(const Planes& p, int ch, const Segment& s) {
    std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));
    for (int x = 0; x < s.size; x++) {
        for (int y = 0; y < s.size; y++) {
            int ss = x + y;
            int xx = p.get(ch, s.x + (ss + 1 < s.size ? ss + 1 : s.size - 1), s.y - 1);
            int yy = p.get(ch, s.x - 1, s.y + (ss < s.size ? ss : s.size - 1));
            int c = ((x + 1) * xx + (y + 1) * yy) / (x + y + 2);
            res[x][y] = c;
        }
    }
    return res;
}

std::vector<std::vector<int>> predHV(const Planes& p, int ch, const Segment& s) {
    std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));
    for (int x = 0; x < s.size; x++) {
        for (int y = 0; y < s.size; y++) {
            int c;
            if (x > y) c = p.get(ch, s.x + x, s.y - 1);
            else if (y > x) c = p.get(ch, s.x - 1, s.y + y);
            else c = (p.get(ch, s.x + x, s.y - 1) + p.get(ch, s.x - 1, s.y + y)) >> 1;
            res[x][y] = c;
        }
    }
    return res;
}

std::vector<std::vector<int>> predJpegLS(const Planes& p, int ch, const Segment& s) {
    std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));
    for (int x = 0; x < s.size; x++) {
        int c = p.get(ch, s.x + x - 1, s.y - 1);
        int a = p.get(ch, s.x + x, s.y - 1);
        for (int y = 0; y < s.size; y++) {
            int b = p.get(ch, s.x - 1, s.y + y);
            int v;
            if (c >= std::max(a, b)) v = std::min(a, b);
            else if (c <= std::min(a, b)) v = std::max(a, b);
            else v = a + b - c;
            res[x][y] = v;
        }
    }
    return res;
}

std::vector<std::vector<int>> predDiff(const Planes& p, int ch, const Segment& s) {
    std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));
    for (int x = 0; x < s.size; x++) {
        int x1 = p.get(ch, s.x + x, s.y - 1);
        int x2 = p.get(ch, s.x + x, s.y - 2);
        for (int y = 0; y < s.size; y++) {
            int y1 = p.get(ch, s.x - 1, s.y + y);
            int y2 = p.get(ch, s.x - 2, s.y + y);
            int v = std::max(0, std::min(255, (y2 + y2 - y1 + x2 + x2 - x1) >> 1));
            res[x][y] = v;
        }
    }
    return res;
}

std::vector<std::vector<int>> predRef(Planes& p, int ch, Segment& s) {
    s.predType = PredictionMethod::REF;

    if (s.refX == std::numeric_limits<int16_t>::max() || s.refY == std::numeric_limits<int16_t>::max()) {
        // Find best reference
        auto& rng = getRng();
        int currSad = std::numeric_limits<int>::max();
        std::vector<std::vector<int>> currRes;

        for (int i = 0; i < 45; i++) {
            std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));
            std::uniform_int_distribution<int> distX(-s.size, s.x - 1);
            int xx = distX(rng);
            int yy;

            if (xx < s.x - s.size) {
                std::uniform_int_distribution<int> distY(-s.size, s.y - 1);
                yy = distY(rng);
            } else {
                std::uniform_int_distribution<int> distY(-s.size, s.y - s.size - 1);
                yy = distY(rng);
            }

            for (int x = 0; x < s.size; x++) {
                for (int y = 0; y < s.size; y++) {
                    res[x][y] = p.get(ch, xx + x, yy + y);
                }
            }

            int sad = getSAD(res, p, ch, s);
            if (sad < currSad) {
                currRes = res;
                currSad = sad;
                s.refX = static_cast<int16_t>(xx);
                s.refY = static_cast<int16_t>(yy);
            }
        }
        return currRes;
    } else {
        std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));
        for (int x = 0; x < s.size; x++) {
            for (int y = 0; y < s.size; y++) {
                res[x][y] = p.get(ch, s.refX + x, s.refY + y);
            }
        }
        return res;
    }
}

std::vector<std::vector<int>> predAngle(Planes& p, int ch, Segment& s) {
    s.predType = PredictionMethod::ANGLE;

    if (s.angle < 0 || s.refAngle < 0) {
        // Find best angle
        float stepA = 1.0f / std::min(16, s.size);
        std::vector<std::vector<int>> currRes;
        int currSad = std::numeric_limits<int>::max();

        for (int i = 0; i < 3; i++) {
            for (float a = 0; a < 1.0f; a += stepA) {
                float aa = static_cast<int>(a * 0x8000) / static_cast<float>(0x8000);
                std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));

                for (int x = 0; x < s.size; x++) {
                    for (int y = 0; y < s.size; y++) {
                        Vec2 angRef = getAngleRef(i, x, y, aa, s.size);
                        int xx = angRef.x >= s.size ? s.size - 1 : static_cast<int>(angRef.x);
                        res[x][y] = p.get(ch, xx + s.x, static_cast<int>(angRef.y) + s.y);
                    }
                }

                int sad = getSAD(res, p, ch, s);
                if (sad < currSad) {
                    currRes = res;
                    currSad = sad;
                    s.angle = a;
                    s.refAngle = i;
                }
            }
        }
        return currRes;
    } else {
        std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));
        for (int x = 0; x < s.size; x++) {
            for (int y = 0; y < s.size; y++) {
                Vec2 angRef = getAngleRef(s.refAngle, x, y, s.angle, s.size);
                int xx = angRef.x >= s.size ? s.size - 1 : static_cast<int>(angRef.x);
                res[x][y] = p.get(ch, xx + s.x, static_cast<int>(angRef.y) + s.y);
            }
        }
        return res;
    }
}

std::vector<std::vector<int>> predSAD(Planes& p, int ch, Segment& s, bool doSad) {
    std::vector<std::vector<int>> currRes;
    int currSad = doSad ? std::numeric_limits<int>::max() : std::numeric_limits<int>::min();
    PredictionMethod currType = PredictionMethod::NONE;

    for (int i = 0; i < static_cast<int>(PredictionMethod::COUNT); i++) {
        auto method = static_cast<PredictionMethod>(i);
        auto res = predict(method, p, ch, s);
        int sad = getSAD(res, p, ch, s);

        if ((doSad && sad < currSad) || (!doSad && sad > currSad)) {
            currSad = sad;
            currType = method;
            currRes = res;
        }
    }

    s.predType = currType;
    return currRes;
}

// ============================================================================
// New Prediction Methods
// ============================================================================

std::vector<std::vector<int>> predSpiral(const Planes& p, int ch, const Segment& s) {
    std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));
    int cx = s.size / 2;
    int cy = s.size / 2;

    for (int x = 0; x < s.size; x++) {
        for (int y = 0; y < s.size; y++) {
            int dx = x - cx;
            int dy = y - cy;
            int layer = std::max(std::abs(dx), std::abs(dy));

            if (layer == 0) {
                res[x][y] = p.get(ch, s.x - 1, s.y - 1);
            } else {
                float angle = std::atan2(static_cast<float>(dy), static_cast<float>(dx));
                float norm = (angle + static_cast<float>(M_PI)) / (2.0f * static_cast<float>(M_PI));
                int boundaryLen = s.size * 2;
                int idx = static_cast<int>(norm * boundaryLen) % boundaryLen;

                if (idx < s.size) {
                    res[x][y] = p.get(ch, s.x + idx, s.y - 1);
                } else {
                    res[x][y] = p.get(ch, s.x - 1, s.y + (idx - s.size));
                }
            }
        }
    }
    return res;
}

std::vector<std::vector<int>> predNoise(const Planes& p, int ch, const Segment& s) {
    std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));
    int base = p.get(ch, s.x - 1, s.y - 1);

    for (int x = 0; x < s.size; x++) {
        for (int y = 0; y < s.size; y++) {
            uint32_t hash = static_cast<uint32_t>(s.x + x) * 73856093u ^
                           static_cast<uint32_t>(s.y + y) * 19349663u;
            hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
            hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
            hash = (hash >> 16) ^ hash;

            int noise = static_cast<int>((hash & 0xFF) - 128) / 4;
            res[x][y] = std::max(0, std::min(255, base + noise));
        }
    }
    return res;
}

std::vector<std::vector<int>> predGradient(const Planes& p, int ch, const Segment& s) {
    std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));

    int tl = p.get(ch, s.x - 1, s.y - 1);
    int tr = p.get(ch, s.x + s.size - 1, s.y - 1);
    int bl = p.get(ch, s.x - 1, s.y + s.size - 1);
    int br = (tr + bl) / 2;

    for (int x = 0; x < s.size; x++) {
        for (int y = 0; y < s.size; y++) {
            float fx = (s.size > 1) ? static_cast<float>(x) / (s.size - 1) : 0.0f;
            float fy = (s.size > 1) ? static_cast<float>(y) / (s.size - 1) : 0.0f;

            float top = tl + (tr - tl) * fx;
            float bot = bl + (br - bl) * fx;
            res[x][y] = static_cast<int>(top + (bot - top) * fy);
        }
    }
    return res;
}

std::vector<std::vector<int>> predMirror(const Planes& p, int ch, const Segment& s) {
    std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));

    for (int x = 0; x < s.size; x++) {
        for (int y = 0; y < s.size; y++) {
            int mirrorY = s.size - 1 - y;
            res[x][y] = p.get(ch, s.x - 1, s.y + mirrorY);
        }
    }
    return res;
}

std::vector<std::vector<int>> predWave(const Planes& p, int ch, const Segment& s) {
    std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));

    float freq = static_cast<float>(M_PI) * 2.0f / s.size;

    for (int x = 0; x < s.size; x++) {
        for (int y = 0; y < s.size; y++) {
            float wave = std::sin(x * freq) + std::sin(y * freq);
            int offset = static_cast<int>(wave * 16);

            int base = (p.get(ch, s.x + x, s.y - 1) + p.get(ch, s.x - 1, s.y + y)) / 2;
            res[x][y] = std::max(0, std::min(255, base + offset));
        }
    }
    return res;
}

std::vector<std::vector<int>> predCheckerboard(const Planes& p, int ch, const Segment& s) {
    std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));

    for (int x = 0; x < s.size; x++) {
        for (int y = 0; y < s.size; y++) {
            if ((x + y) % 2 == 0) {
                res[x][y] = p.get(ch, s.x + x, s.y - 1);
            } else {
                res[x][y] = p.get(ch, s.x - 1, s.y + y);
            }
        }
    }
    return res;
}

std::vector<std::vector<int>> predRadial(const Planes& p, int ch, const Segment& s) {
    std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));

    int cx = s.size / 2;
    int cy = s.size / 2;
    float maxDist = std::sqrt(static_cast<float>(cx * cx + cy * cy));
    if (maxDist < 1.0f) maxDist = 1.0f;

    int center = p.get(ch, s.x - 1, s.y - 1);
    int edge = (p.get(ch, s.x + s.size - 1, s.y - 1) +
                p.get(ch, s.x - 1, s.y + s.size - 1)) / 2;

    for (int x = 0; x < s.size; x++) {
        for (int y = 0; y < s.size; y++) {
            float dist = std::sqrt(static_cast<float>((x - cx) * (x - cx) +
                                                       (y - cy) * (y - cy)));
            float t = dist / maxDist;
            res[x][y] = static_cast<int>(center + (edge - center) * t);
        }
    }
    return res;
}

std::vector<std::vector<int>> predEdge(const Planes& p, int ch, const Segment& s) {
    std::vector<std::vector<int>> res(s.size, std::vector<int>(s.size));

    for (int x = 0; x < s.size; x++) {
        for (int y = 0; y < s.size; y++) {
            int gx = p.get(ch, s.x + x, s.y - 1) - p.get(ch, s.x - 1, s.y + y);
            int gy = p.get(ch, s.x + x, s.y - 1) - p.get(ch, s.x - 1, s.y - 1);

            int base = (p.get(ch, s.x + x, s.y - 1) + p.get(ch, s.x - 1, s.y + y)) / 2;
            int edge = std::abs(gx) + std::abs(gy);

            res[x][y] = std::max(0, std::min(255, base + edge / 8));
        }
    }
    return res;
}

} // namespace glic
