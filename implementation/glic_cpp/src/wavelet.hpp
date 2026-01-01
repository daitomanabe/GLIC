#pragma once

#include "config.hpp"
#include <vector>
#include <memory>

namespace glic {

// Abstract wavelet base class
class Wavelet {
public:
    virtual ~Wavelet() = default;

    // Get filter coefficients
    virtual const std::vector<double>& getLowPassDecomposition() const = 0;
    virtual const std::vector<double>& getHighPassDecomposition() const = 0;
    virtual const std::vector<double>& getLowPassReconstruction() const = 0;
    virtual const std::vector<double>& getHighPassReconstruction() const = 0;

    virtual std::string getName() const = 0;
    virtual int getLength() const = 0;
};

// Wavelet transform base class
class WaveletTransform {
public:
    virtual ~WaveletTransform() = default;

    // 2D forward transform
    virtual std::vector<std::vector<double>> forward(const std::vector<std::vector<double>>& data) = 0;

    // 2D reverse transform
    virtual std::vector<std::vector<double>> reverse(const std::vector<std::vector<double>>& data) = 0;

    virtual std::string getName() const = 0;
};

// Fast Wavelet Transform
class FastWaveletTransform : public WaveletTransform {
public:
    explicit FastWaveletTransform(std::shared_ptr<Wavelet> wavelet);

    std::vector<std::vector<double>> forward(const std::vector<std::vector<double>>& data) override;
    std::vector<std::vector<double>> reverse(const std::vector<std::vector<double>>& data) override;
    std::string getName() const override { return "FWT"; }

private:
    std::vector<double> forward1D(const std::vector<double>& data);
    std::vector<double> reverse1D(const std::vector<double>& data);

    std::shared_ptr<Wavelet> wavelet_;
};

// Wavelet Packet Transform
class WaveletPacketTransform : public WaveletTransform {
public:
    explicit WaveletPacketTransform(std::shared_ptr<Wavelet> wavelet);

    std::vector<std::vector<double>> forward(const std::vector<std::vector<double>>& data) override;
    std::vector<std::vector<double>> reverse(const std::vector<std::vector<double>>& data) override;
    std::string getName() const override { return "WPT"; }

private:
    std::vector<double> forward1D(const std::vector<double>& data, int level);
    std::vector<double> reverse1D(const std::vector<double>& data, int level);

    std::shared_ptr<Wavelet> wavelet_;
};

// Magnitude compressor
class MagnitudeCompressor {
public:
    explicit MagnitudeCompressor(double threshold);

    std::vector<std::vector<double>> compress(const std::vector<std::vector<double>>& data);

private:
    double threshold_;
};

// Factory functions
std::shared_ptr<Wavelet> createWavelet(WaveletType type);
std::unique_ptr<WaveletTransform> createTransform(TransformType type, std::shared_ptr<Wavelet> wavelet);

// Specific wavelet implementations
class HaarWavelet : public Wavelet {
public:
    const std::vector<double>& getLowPassDecomposition() const override { return lpd_; }
    const std::vector<double>& getHighPassDecomposition() const override { return hpd_; }
    const std::vector<double>& getLowPassReconstruction() const override { return lpr_; }
    const std::vector<double>& getHighPassReconstruction() const override { return hpr_; }
    std::string getName() const override { return "Haar"; }
    int getLength() const override { return 2; }

private:
    static const std::vector<double> lpd_;
    static const std::vector<double> hpd_;
    static const std::vector<double> lpr_;
    static const std::vector<double> hpr_;
};

class Daubechies2 : public Wavelet {
public:
    const std::vector<double>& getLowPassDecomposition() const override { return lpd_; }
    const std::vector<double>& getHighPassDecomposition() const override { return hpd_; }
    const std::vector<double>& getLowPassReconstruction() const override { return lpr_; }
    const std::vector<double>& getHighPassReconstruction() const override { return hpr_; }
    std::string getName() const override { return "Daubechies2"; }
    int getLength() const override { return 4; }

private:
    static const std::vector<double> lpd_;
    static const std::vector<double> hpd_;
    static const std::vector<double> lpr_;
    static const std::vector<double> hpr_;
};

class Daubechies4 : public Wavelet {
public:
    const std::vector<double>& getLowPassDecomposition() const override { return lpd_; }
    const std::vector<double>& getHighPassDecomposition() const override { return hpd_; }
    const std::vector<double>& getLowPassReconstruction() const override { return lpr_; }
    const std::vector<double>& getHighPassReconstruction() const override { return hpr_; }
    std::string getName() const override { return "Daubechies4"; }
    int getLength() const override { return 8; }

private:
    static const std::vector<double> lpd_;
    static const std::vector<double> hpd_;
    static const std::vector<double> lpr_;
    static const std::vector<double> hpr_;
};

class Symlet4 : public Wavelet {
public:
    const std::vector<double>& getLowPassDecomposition() const override { return lpd_; }
    const std::vector<double>& getHighPassDecomposition() const override { return hpd_; }
    const std::vector<double>& getLowPassReconstruction() const override { return lpr_; }
    const std::vector<double>& getHighPassReconstruction() const override { return hpr_; }
    std::string getName() const override { return "Symlet4"; }
    int getLength() const override { return 8; }

private:
    static const std::vector<double> lpd_;
    static const std::vector<double> hpd_;
    static const std::vector<double> lpr_;
    static const std::vector<double> hpr_;
};

class Symlet8 : public Wavelet {
public:
    const std::vector<double>& getLowPassDecomposition() const override { return lpd_; }
    const std::vector<double>& getHighPassDecomposition() const override { return hpd_; }
    const std::vector<double>& getLowPassReconstruction() const override { return lpr_; }
    const std::vector<double>& getHighPassReconstruction() const override { return hpr_; }
    std::string getName() const override { return "Symlet8"; }
    int getLength() const override { return 16; }

private:
    static const std::vector<double> lpd_;
    static const std::vector<double> hpd_;
    static const std::vector<double> lpr_;
    static const std::vector<double> hpr_;
};

class Coiflet2 : public Wavelet {
public:
    const std::vector<double>& getLowPassDecomposition() const override { return lpd_; }
    const std::vector<double>& getHighPassDecomposition() const override { return hpd_; }
    const std::vector<double>& getLowPassReconstruction() const override { return lpr_; }
    const std::vector<double>& getHighPassReconstruction() const override { return hpr_; }
    std::string getName() const override { return "Coiflet2"; }
    int getLength() const override { return 12; }

private:
    static const std::vector<double> lpd_;
    static const std::vector<double> hpd_;
    static const std::vector<double> lpr_;
    static const std::vector<double> hpr_;
};

} // namespace glic
