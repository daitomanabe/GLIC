#include "bitio.hpp"
#include <stdexcept>

namespace glic {

void BitWriter::writeBit(bool bit) {
    if (bit) {
        currentByte_ |= (1 << (7 - bitPos_));
    }
    bitPos_++;
    if (bitPos_ == 8) {
        buffer_.push_back(currentByte_);
        currentByte_ = 0;
        bitPos_ = 0;
    }
}

void BitWriter::writeBits(uint32_t value, int numBits) {
    for (int i = numBits - 1; i >= 0; i--) {
        writeBit((value >> i) & 1);
    }
}

void BitWriter::writeBoolean(bool value) {
    writeBit(value);
}

void BitWriter::writeInt(int32_t value, bool isSigned, int numBits) {
    if (isSigned) {
        writeBits(static_cast<uint32_t>(value), numBits);
    } else {
        writeBits(static_cast<uint32_t>(value), numBits);
    }
}

void BitWriter::writeByte(uint8_t value) {
    if (bitPos_ == 0) {
        buffer_.push_back(value);
    } else {
        writeBits(value, 8);
    }
}

void BitWriter::writeBytes(const uint8_t* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        writeByte(data[i]);
    }
}

void BitWriter::align() {
    if (bitPos_ != 0) {
        buffer_.push_back(currentByte_);
        currentByte_ = 0;
        bitPos_ = 0;
    }
}

void BitWriter::clear() {
    buffer_.clear();
    bitPos_ = 0;
    currentByte_ = 0;
}

bool BitReader::readBit() {
    if (bytePos_ >= size_) {
        throw std::runtime_error("BitReader: End of data");
    }
    bool bit = (data_[bytePos_] >> (7 - bitPos_)) & 1;
    bitPos_++;
    if (bitPos_ == 8) {
        bytePos_++;
        bitPos_ = 0;
    }
    return bit;
}

uint32_t BitReader::readBits(int numBits) {
    uint32_t result = 0;
    for (int i = 0; i < numBits; i++) {
        result = (result << 1) | (readBit() ? 1 : 0);
    }
    return result;
}

bool BitReader::readBoolean() {
    return readBit();
}

int32_t BitReader::readInt(bool isSigned, int numBits) {
    uint32_t value = readBits(numBits);
    if (isSigned && (value & (1u << (numBits - 1)))) {
        // Sign extend
        value |= ~((1u << numBits) - 1);
    }
    return static_cast<int32_t>(value);
}

uint8_t BitReader::readByte() {
    if (bitPos_ == 0 && bytePos_ < size_) {
        return data_[bytePos_++];
    }
    return static_cast<uint8_t>(readBits(8));
}

void BitReader::readBytes(uint8_t* buffer, size_t count) {
    for (size_t i = 0; i < count; i++) {
        buffer[i] = readByte();
    }
}

void BitReader::align() {
    if (bitPos_ != 0) {
        bytePos_++;
        bitPos_ = 0;
    }
}

size_t BitReader::bytesRemaining() const {
    size_t remaining = size_ - bytePos_;
    if (bitPos_ > 0 && remaining > 0) {
        remaining--;
    }
    return remaining;
}

} // namespace glic
