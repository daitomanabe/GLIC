#pragma once

#include "config.hpp"
#include "segment.hpp"
#include <vector>

namespace glic {

class Planes;

// Main prediction function
std::vector<std::vector<int>> predict(
    PredictionMethod method,
    Planes& planes,
    int channel,
    Segment& segment
);

// Calculate Sum of Absolute Differences
int getSAD(
    const std::vector<std::vector<int>>& pred,
    const Planes& planes,
    int channel,
    const Segment& segment
);

// Individual prediction methods
std::vector<std::vector<int>> predCorner(const Planes& p, int ch, const Segment& s);
std::vector<std::vector<int>> predH(const Planes& p, int ch, const Segment& s);
std::vector<std::vector<int>> predV(const Planes& p, int ch, const Segment& s);
std::vector<std::vector<int>> predDC(const Planes& p, int ch, const Segment& s);
std::vector<std::vector<int>> predDCMedian(const Planes& p, int ch, const Segment& s);
std::vector<std::vector<int>> predMedian(const Planes& p, int ch, const Segment& s);
std::vector<std::vector<int>> predAvg(const Planes& p, int ch, const Segment& s);
std::vector<std::vector<int>> predTrueMotion(const Planes& p, int ch, const Segment& s);
std::vector<std::vector<int>> predPaeth(const Planes& p, int ch, const Segment& s);
std::vector<std::vector<int>> predLDiag(const Planes& p, int ch, const Segment& s);
std::vector<std::vector<int>> predHV(const Planes& p, int ch, const Segment& s);
std::vector<std::vector<int>> predJpegLS(const Planes& p, int ch, const Segment& s);
std::vector<std::vector<int>> predDiff(const Planes& p, int ch, const Segment& s);
std::vector<std::vector<int>> predRef(Planes& p, int ch, Segment& s);
std::vector<std::vector<int>> predAngle(Planes& p, int ch, Segment& s);
std::vector<std::vector<int>> predSAD(Planes& p, int ch, Segment& s, bool doSad);

// New prediction methods
std::vector<std::vector<int>> predSpiral(const Planes& p, int ch, const Segment& s);
std::vector<std::vector<int>> predNoise(const Planes& p, int ch, const Segment& s);
std::vector<std::vector<int>> predGradient(const Planes& p, int ch, const Segment& s);
std::vector<std::vector<int>> predMirror(const Planes& p, int ch, const Segment& s);
std::vector<std::vector<int>> predWave(const Planes& p, int ch, const Segment& s);
std::vector<std::vector<int>> predCheckerboard(const Planes& p, int ch, const Segment& s);
std::vector<std::vector<int>> predRadial(const Planes& p, int ch, const Segment& s);
std::vector<std::vector<int>> predEdge(const Planes& p, int ch, const Segment& s);

} // namespace glic
