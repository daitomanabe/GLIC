#include "wavelet.hpp"
#include <cmath>
#include <algorithm>
#include <stdexcept>

namespace glic {

// Haar wavelet coefficients
const std::vector<double> HaarWavelet::lpd_ = {0.7071067811865476, 0.7071067811865476};
const std::vector<double> HaarWavelet::hpd_ = {-0.7071067811865476, 0.7071067811865476};
const std::vector<double> HaarWavelet::lpr_ = {0.7071067811865476, 0.7071067811865476};
const std::vector<double> HaarWavelet::hpr_ = {0.7071067811865476, -0.7071067811865476};

// Daubechies 2 (db2) coefficients
const std::vector<double> Daubechies2::lpd_ = {
    -0.12940952255092145, 0.22414386804185735,
    0.836516303737469, 0.48296291314469025
};
const std::vector<double> Daubechies2::hpd_ = {
    -0.48296291314469025, 0.836516303737469,
    -0.22414386804185735, -0.12940952255092145
};
const std::vector<double> Daubechies2::lpr_ = {
    0.48296291314469025, 0.836516303737469,
    0.22414386804185735, -0.12940952255092145
};
const std::vector<double> Daubechies2::hpr_ = {
    -0.12940952255092145, -0.22414386804185735,
    0.836516303737469, -0.48296291314469025
};

// Daubechies 4 (db4) coefficients
const std::vector<double> Daubechies4::lpd_ = {
    -0.010597401784997278, 0.032883011666982945,
    0.030841381835986965, -0.18703481171888114,
    -0.02798376941698385, 0.6308807679295904,
    0.7148465705525415, 0.23037781330885523
};
const std::vector<double> Daubechies4::hpd_ = {
    -0.23037781330885523, 0.7148465705525415,
    -0.6308807679295904, -0.02798376941698385,
    0.18703481171888114, 0.030841381835986965,
    -0.032883011666982945, -0.010597401784997278
};
const std::vector<double> Daubechies4::lpr_ = {
    0.23037781330885523, 0.7148465705525415,
    0.6308807679295904, -0.02798376941698385,
    -0.18703481171888114, 0.030841381835986965,
    0.032883011666982945, -0.010597401784997278
};
const std::vector<double> Daubechies4::hpr_ = {
    -0.010597401784997278, -0.032883011666982945,
    0.030841381835986965, 0.18703481171888114,
    -0.02798376941698385, -0.6308807679295904,
    0.7148465705525415, -0.23037781330885523
};

// Symlet 4 (sym4) coefficients
const std::vector<double> Symlet4::lpd_ = {
    -0.07576571478927333, -0.02963552764599851,
    0.49761866763201545, 0.8037387518059161,
    0.29785779560527736, -0.09921954357684722,
    -0.012603967262037833, 0.032223100604042702
};
const std::vector<double> Symlet4::hpd_ = {
    -0.032223100604042702, -0.012603967262037833,
    0.09921954357684722, 0.29785779560527736,
    -0.8037387518059161, 0.49761866763201545,
    0.02963552764599851, -0.07576571478927333
};
const std::vector<double> Symlet4::lpr_ = {
    0.032223100604042702, -0.012603967262037833,
    -0.09921954357684722, 0.29785779560527736,
    0.8037387518059161, 0.49761866763201545,
    -0.02963552764599851, -0.07576571478927333
};
const std::vector<double> Symlet4::hpr_ = {
    -0.07576571478927333, 0.02963552764599851,
    0.49761866763201545, -0.8037387518059161,
    0.29785779560527736, 0.09921954357684722,
    -0.012603967262037833, -0.032223100604042702
};

// Symlet 8 (sym8) coefficients
const std::vector<double> Symlet8::lpd_ = {
    -0.0033824159510061256, -0.0005421323317911481,
    0.03169508781149298, 0.007607487324917605,
    -0.1432942383508097, -0.061273359067658524,
    0.4813596512583722, 0.7771857516997478,
    0.3644418948353314, -0.05194583810770904,
    -0.027219029917056003, 0.049137179673607506,
    0.003808752013890615, -0.01495225833704823,
    -0.0003029205147213668, 0.0018899503327594609
};
const std::vector<double> Symlet8::hpd_ = {
    -0.0018899503327594609, -0.0003029205147213668,
    0.01495225833704823, 0.003808752013890615,
    -0.049137179673607506, -0.027219029917056003,
    0.05194583810770904, 0.3644418948353314,
    -0.7771857516997478, 0.4813596512583722,
    0.061273359067658524, -0.1432942383508097,
    -0.007607487324917605, 0.03169508781149298,
    0.0005421323317911481, -0.0033824159510061256
};
const std::vector<double> Symlet8::lpr_ = {
    0.0018899503327594609, -0.0003029205147213668,
    -0.01495225833704823, 0.003808752013890615,
    0.049137179673607506, -0.027219029917056003,
    -0.05194583810770904, 0.3644418948353314,
    0.7771857516997478, 0.4813596512583722,
    -0.061273359067658524, -0.1432942383508097,
    0.007607487324917605, 0.03169508781149298,
    -0.0005421323317911481, -0.0033824159510061256
};
const std::vector<double> Symlet8::hpr_ = {
    -0.0033824159510061256, 0.0005421323317911481,
    0.03169508781149298, -0.007607487324917605,
    -0.1432942383508097, 0.061273359067658524,
    0.4813596512583722, -0.7771857516997478,
    0.3644418948353314, 0.05194583810770904,
    -0.027219029917056003, -0.049137179673607506,
    0.003808752013890615, 0.01495225833704823,
    -0.0003029205147213668, -0.0018899503327594609
};

// Coiflet 2 coefficients
const std::vector<double> Coiflet2::lpd_ = {
    0.0007205494453645122, -0.0018232088707029932,
    -0.0056114348193944995, 0.023680171946334084,
    0.0594344186464569, -0.0764885990783064,
    -0.41700518442169254, 0.8127236354455423,
    0.3861100668211622, -0.06737255472196302,
    -0.04146493678175915, 0.016387336463522112
};
const std::vector<double> Coiflet2::hpd_ = {
    -0.016387336463522112, -0.04146493678175915,
    0.06737255472196302, 0.3861100668211622,
    -0.8127236354455423, -0.41700518442169254,
    0.0764885990783064, 0.0594344186464569,
    -0.023680171946334084, -0.0056114348193944995,
    0.0018232088707029932, 0.0007205494453645122
};
const std::vector<double> Coiflet2::lpr_ = {
    0.016387336463522112, -0.04146493678175915,
    -0.06737255472196302, 0.3861100668211622,
    0.8127236354455423, -0.41700518442169254,
    -0.0764885990783064, 0.0594344186464569,
    0.023680171946334084, -0.0056114348193944995,
    -0.0018232088707029932, 0.0007205494453645122
};
const std::vector<double> Coiflet2::hpr_ = {
    0.0007205494453645122, 0.0018232088707029932,
    -0.0056114348193944995, -0.023680171946334084,
    0.0594344186464569, 0.0764885990783064,
    -0.41700518442169254, -0.8127236354455423,
    0.3861100668211622, 0.06737255472196302,
    -0.04146493678175915, -0.016387336463522112
};

// Factory function for wavelets
std::shared_ptr<Wavelet> createWavelet(WaveletType type) {
    switch (type) {
        case WaveletType::HAAR:
        case WaveletType::HAAR_ORTHOGONAL:
            return std::make_shared<HaarWavelet>();
        case WaveletType::DAUBECHIES2:
            return std::make_shared<Daubechies2>();
        case WaveletType::DAUBECHIES3:
        case WaveletType::DAUBECHIES4:
            return std::make_shared<Daubechies4>();
        case WaveletType::SYMLET2:
        case WaveletType::SYMLET3:
        case WaveletType::SYMLET4:
            return std::make_shared<Symlet4>();
        case WaveletType::SYMLET5:
        case WaveletType::SYMLET6:
        case WaveletType::SYMLET7:
        case WaveletType::SYMLET8:
        case WaveletType::SYMLET9:
        case WaveletType::SYMLET10:
            return std::make_shared<Symlet8>();
        case WaveletType::COIFLET1:
        case WaveletType::COIFLET2:
        case WaveletType::COIFLET3:
        case WaveletType::COIFLET4:
        case WaveletType::COIFLET5:
            return std::make_shared<Coiflet2>();
        default:
            return std::make_shared<HaarWavelet>();
    }
}

// Factory function for transforms
std::unique_ptr<WaveletTransform> createTransform(TransformType type, std::shared_ptr<Wavelet> wavelet) {
    switch (type) {
        case TransformType::WPT:
            return std::make_unique<WaveletPacketTransform>(wavelet);
        case TransformType::FWT:
        default:
            return std::make_unique<FastWaveletTransform>(wavelet);
    }
}

// FastWaveletTransform implementation
FastWaveletTransform::FastWaveletTransform(std::shared_ptr<Wavelet> wavelet)
    : wavelet_(wavelet) {}

std::vector<double> FastWaveletTransform::forward1D(const std::vector<double>& data) {
    size_t n = data.size();
    if (n < 2) return data;

    std::vector<double> result(n);
    const auto& lpd = wavelet_->getLowPassDecomposition();
    const auto& hpd = wavelet_->getHighPassDecomposition();
    size_t filterLen = lpd.size();

    size_t half = n / 2;
    for (size_t i = 0; i < half; i++) {
        double low = 0, high = 0;
        for (size_t j = 0; j < filterLen; j++) {
            size_t idx = (2 * i + j) % n;
            low += lpd[j] * data[idx];
            high += hpd[j] * data[idx];
        }
        result[i] = low;
        result[half + i] = high;
    }
    return result;
}

std::vector<double> FastWaveletTransform::reverse1D(const std::vector<double>& data) {
    size_t n = data.size();
    if (n < 2) return data;

    std::vector<double> result(n, 0);
    const auto& lpr = wavelet_->getLowPassReconstruction();
    const auto& hpr = wavelet_->getHighPassReconstruction();
    size_t filterLen = lpr.size();

    size_t half = n / 2;
    for (size_t i = 0; i < half; i++) {
        for (size_t j = 0; j < filterLen; j++) {
            size_t idx = (2 * i + j) % n;
            result[idx] += lpr[j] * data[i] + hpr[j] * data[half + i];
        }
    }
    return result;
}

std::vector<std::vector<double>> FastWaveletTransform::forward(const std::vector<std::vector<double>>& data) {
    size_t rows = data.size();
    if (rows == 0) return data;
    size_t cols = data[0].size();

    auto result = data;

    // Transform rows
    for (size_t i = 0; i < rows; i++) {
        size_t len = cols;
        while (len >= 2) {
            std::vector<double> temp(result[i].begin(), result[i].begin() + len);
            auto transformed = forward1D(temp);
            std::copy(transformed.begin(), transformed.end(), result[i].begin());
            len /= 2;
        }
    }

    // Transform columns
    for (size_t j = 0; j < cols; j++) {
        size_t len = rows;
        while (len >= 2) {
            std::vector<double> temp(len);
            for (size_t i = 0; i < len; i++) {
                temp[i] = result[i][j];
            }
            auto transformed = forward1D(temp);
            for (size_t i = 0; i < len; i++) {
                result[i][j] = transformed[i];
            }
            len /= 2;
        }
    }

    return result;
}

std::vector<std::vector<double>> FastWaveletTransform::reverse(const std::vector<std::vector<double>>& data) {
    size_t rows = data.size();
    if (rows == 0) return data;
    size_t cols = data[0].size();

    auto result = data;

    // Inverse transform columns
    for (size_t j = 0; j < cols; j++) {
        size_t len = 2;
        while (len <= rows) {
            std::vector<double> temp(len);
            for (size_t i = 0; i < len; i++) {
                temp[i] = result[i][j];
            }
            auto transformed = reverse1D(temp);
            for (size_t i = 0; i < len; i++) {
                result[i][j] = transformed[i];
            }
            len *= 2;
        }
    }

    // Inverse transform rows
    for (size_t i = 0; i < rows; i++) {
        size_t len = 2;
        while (len <= cols) {
            std::vector<double> temp(result[i].begin(), result[i].begin() + len);
            auto transformed = reverse1D(temp);
            std::copy(transformed.begin(), transformed.end(), result[i].begin());
            len *= 2;
        }
    }

    return result;
}

// WaveletPacketTransform implementation
WaveletPacketTransform::WaveletPacketTransform(std::shared_ptr<Wavelet> wavelet)
    : wavelet_(wavelet) {}

std::vector<double> WaveletPacketTransform::forward1D(const std::vector<double>& data, int level) {
    if (level <= 0 || data.size() < 2) return data;

    const auto& lpd = wavelet_->getLowPassDecomposition();
    const auto& hpd = wavelet_->getHighPassDecomposition();
    size_t n = data.size();
    size_t filterLen = lpd.size();

    std::vector<double> result(n);
    size_t half = n / 2;

    for (size_t i = 0; i < half; i++) {
        double low = 0, high = 0;
        for (size_t j = 0; j < filterLen; j++) {
            size_t idx = (2 * i + j) % n;
            low += lpd[j] * data[idx];
            high += hpd[j] * data[idx];
        }
        result[i] = low;
        result[half + i] = high;
    }

    // Recursively transform both halves
    std::vector<double> lowPart(result.begin(), result.begin() + half);
    std::vector<double> highPart(result.begin() + half, result.end());

    lowPart = forward1D(lowPart, level - 1);
    highPart = forward1D(highPart, level - 1);

    std::copy(lowPart.begin(), lowPart.end(), result.begin());
    std::copy(highPart.begin(), highPart.end(), result.begin() + half);

    return result;
}

std::vector<double> WaveletPacketTransform::reverse1D(const std::vector<double>& data, int level) {
    if (level <= 0 || data.size() < 2) return data;

    size_t n = data.size();
    size_t half = n / 2;

    // First reverse the recursive parts
    std::vector<double> lowPart(data.begin(), data.begin() + half);
    std::vector<double> highPart(data.begin() + half, data.end());

    lowPart = reverse1D(lowPart, level - 1);
    highPart = reverse1D(highPart, level - 1);

    const auto& lpr = wavelet_->getLowPassReconstruction();
    const auto& hpr = wavelet_->getHighPassReconstruction();
    size_t filterLen = lpr.size();

    std::vector<double> result(n, 0);

    for (size_t i = 0; i < half; i++) {
        for (size_t j = 0; j < filterLen; j++) {
            size_t idx = (2 * i + j) % n;
            result[idx] += lpr[j] * lowPart[i] + hpr[j] * highPart[i];
        }
    }

    return result;
}

std::vector<std::vector<double>> WaveletPacketTransform::forward(const std::vector<std::vector<double>>& data) {
    size_t rows = data.size();
    if (rows == 0) return data;
    size_t cols = data[0].size();

    int levels = static_cast<int>(std::log2(std::min(rows, cols)));

    auto result = data;

    // Transform rows
    for (size_t i = 0; i < rows; i++) {
        result[i] = forward1D(result[i], levels);
    }

    // Transform columns
    for (size_t j = 0; j < cols; j++) {
        std::vector<double> col(rows);
        for (size_t i = 0; i < rows; i++) {
            col[i] = result[i][j];
        }
        col = forward1D(col, levels);
        for (size_t i = 0; i < rows; i++) {
            result[i][j] = col[i];
        }
    }

    return result;
}

std::vector<std::vector<double>> WaveletPacketTransform::reverse(const std::vector<std::vector<double>>& data) {
    size_t rows = data.size();
    if (rows == 0) return data;
    size_t cols = data[0].size();

    int levels = static_cast<int>(std::log2(std::min(rows, cols)));

    auto result = data;

    // Inverse transform columns
    for (size_t j = 0; j < cols; j++) {
        std::vector<double> col(rows);
        for (size_t i = 0; i < rows; i++) {
            col[i] = result[i][j];
        }
        col = reverse1D(col, levels);
        for (size_t i = 0; i < rows; i++) {
            result[i][j] = col[i];
        }
    }

    // Inverse transform rows
    for (size_t i = 0; i < rows; i++) {
        result[i] = reverse1D(result[i], levels);
    }

    return result;
}

// MagnitudeCompressor implementation
MagnitudeCompressor::MagnitudeCompressor(double threshold)
    : threshold_(threshold) {}

std::vector<std::vector<double>> MagnitudeCompressor::compress(const std::vector<std::vector<double>>& data) {
    auto result = data;
    for (auto& row : result) {
        for (auto& val : row) {
            if (std::abs(val) < threshold_) {
                val = 0;
            }
        }
    }
    return result;
}

} // namespace glic
