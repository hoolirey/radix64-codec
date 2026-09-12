#ifndef RADIX64_CODEC_HPP
#define RADIX64_CODEC_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>

// Frozen RADIX64 Alphabet Definition (Strict Numeric-First Order per RADIX64-ACC-CCX-V1)
const char RADIX64_ALPHABET[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+-";

class Radix64Codec {
public:
    // Encodes arbitrary raw byte buffers into strict canonical RADIX64 strings
    static std::string encode(const uint8_t* data, size_t length);
    
    // Decodes canonical RADIX64 strings. Throws on non-zero trailing bits or bad padding lengths.
    static std::vector<uint8_t> decode(const std::string& encoded);
};

#endif // RADIX64_CODEC_HPP
