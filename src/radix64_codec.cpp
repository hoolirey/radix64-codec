#include "radix64_codec.hpp"
#include <array>

// Build a static inverse lookup table for ultra-fast, fail-closed symbol validation
static const std::array<int8_t, 256> INV_ALPHABET_MAP = []() {
    std::array<int8_t, 256> table;
    table.fill(-1); // Initialize all possible byte entries to invalid
    for (int i = 0; i < 64; ++i) {
        table[static_cast<uint8_t>(RADIX64_ALPHABET[i])] = static_cast<int8_t>(i);
    }
    return table;
}();

std::string Radix64Codec::encode(const uint8_t* data, size_t length) {
    std::string output;
    output.reserve(((length * 8) + 5) / 6);

    uint32_t accumulator = 0;
    int bits_collected = 0;

    for (size_t i = 0; i < length; ++i) {
        accumulator = (accumulator << 8) | data[i];
        bits_collected += 8;

        while (bits_collected >= 6) {
            bits_collected -= 6;
            int idx = (accumulator >> bits_collected) & 0x3F;
            output.push_back(RADIX64_ALPHABET[idx]);
        }
    }

    // Process leftover trailing fragments using strict zero-padding alignment
    if (bits_collected > 0) {
        accumulator <<= (6 - bits_collected);
        int idx = accumulator & 0x3F;
        output.push_back(RADIX64_ALPHABET[idx]);
    }

    return output;
}

std::vector<uint8_t> Radix64Codec::decode(const std::string& encoded) {
    if (encoded.empty()) return {};

    std::vector<uint8_t> output;
    output.reserve((encoded.length() * 6) / 8);

    uint32_t accumulator = 0;
    int bits_collected = 0;

    for (size_t i = 0; i < encoded.length(); ++i) {
        uint8_t current_char = static_cast<uint8_t>(encoded[i]);
        int8_t symbol_value = INV_ALPHABET_MAP[current_char];

        if (symbol_value == -1) {
            throw std::invalid_argument("[CODEC ERROR] Illegal character encountered in Radix64 stream.");
        }

        accumulator = (accumulator << 6) | static_cast<uint32_t>(symbol_value);
        bits_collected += 6;

        // Isolate the final symbol to evaluate trailing padding bit alignment
        if (i == encoded.length() - 1) {
            int leftover_bits = bits_collected % 8;
            if (leftover_bits > 0) {
                uint32_t mask = (1U << leftover_bits) - 1U;
                if ((accumulator & mask) != 0) {
                    throw std::runtime_error("[CANONICAL VIOLATION] Rejecting malformed stream: Non-zero trailing bits found in final block padding.");
                }
            }
        }

        while (bits_collected >= 8) {
            bits_collected -= 8;
            output.push_back(static_cast<uint8_t>((accumulator >> bits_collected) & 0xFF));
        }
    }

    // A structural single-symbol remainder (6 bits total) cannot make a single valid 8-bit byte frame
    if (bits_collected >= 6) {
        throw std::length_error("[CODEC ERROR] Rejecting malformed stream: Impossible structural byte format length.");
    }

    return output;
}
