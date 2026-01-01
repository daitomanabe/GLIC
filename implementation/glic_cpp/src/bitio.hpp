#pragma once

#include <cstdint>
#include <vector>
#include <cstddef>

namespace glic {

class BitWriter {
public:
    BitWriter() : buffer_(), bitPos_(0), currentByte_(0) {}

    void writeBit(bool bit);
    void writeBits(uint32_t value, int numBits);
    void writeBoolean(bool value);
    void writeInt(int32_t value, bool isSigned, int numBits);
    void writeByte(uint8_t value);
    void writeBytes(const uint8_t* data, size_t size);

    void align();
    const std::vector<uint8_t>& data() const { return buffer_; }
    size_t size() const { return buffer_.size(); }
    void clear();

private:
    std::vector<uint8_t> buffer_;
    int bitPos_;
    uint8_t currentByte_;
};

class BitReader {
public:
    BitReader(const uint8_t* data, size_t size)
        : data_(data), size_(size), bytePos_(0), bitPos_(0) {}

    bool readBit();
    uint32_t readBits(int numBits);
    bool readBoolean();
    int32_t readInt(bool isSigned, int numBits);
    uint8_t readByte();
    void readBytes(uint8_t* buffer, size_t count);

    void align();
    bool eof() const { return bytePos_ >= size_ && bitPos_ == 0; }
    size_t bytesRemaining() const;

private:
    const uint8_t* data_;
    size_t size_;
    size_t bytePos_;
    int bitPos_;
};

} // namespace glic
