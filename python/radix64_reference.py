#!/usr/bin/env python3
"""RADIX64 Reference Implementation (Python)"""

import struct

RADIX64_ALPHABET = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+-"

class RADIX64Codec:
    @staticmethod
    def encode(data):
        """Encode bytes to RADIX64 string."""
        if isinstance(data, str):
            data = data.encode('utf-8')
        
        output = []
        accumulator = 0
        bits_collected = 0
        
        for byte in data:
            accumulator = (accumulator << 8) | byte
            bits_collected += 8
            
            while bits_collected >= 6:
                bits_collected -= 6
                idx = (accumulator >> bits_collected) & 0x3F
                output.append(RADIX64_ALPHABET[idx])
        
        # Process trailing bits
        if bits_collected > 0:
            accumulator <<= (6 - bits_collected)
            idx = accumulator & 0x3F
            output.append(RADIX64_ALPHABET[idx])
        
        return ''.join(output)
    
    @staticmethod
    def decode(encoded):
        """Decode RADIX64 string to bytes."""
        if not encoded:
            return b''
        
        # Build inverse lookup
        inv_map = {c: i for i, c in enumerate(RADIX64_ALPHABET)}
        
        output = []
        accumulator = 0
        bits_collected = 0
        
        for i, char in enumerate(encoded):
            if char not in inv_map:
                raise ValueError(f"[CODEC ERROR] Illegal character '{char}' in Radix64 stream.")
            
            symbol_value = inv_map[char]
            accumulator = (accumulator << 6) | symbol_value
            bits_collected += 6
            
            # Check final symbol for trailing bits
            if i == len(encoded) - 1:
                leftover_bits = bits_collected % 8
                if leftover_bits > 0:
                    mask = (1 << leftover_bits) - 1
                    if (accumulator & mask) != 0:
                        raise RuntimeError("[CANONICAL VIOLATION] Rejecting malformed stream: Non-zero trailing bits found in final block padding.")
            
            while bits_collected >= 8:
                bits_collected -= 8
                output.append((accumulator >> bits_collected) & 0xFF)
        
        # Check for impossible remainder
        if bits_collected >= 6:
            raise ValueError("[CODEC ERROR] Rejecting malformed stream: Impossible structural byte format length.")
        
        return bytes(output)
