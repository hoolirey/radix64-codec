/**
 * RADIX64 Reference Implementation (JavaScript)
 */

const RADIX64_ALPHABET = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+-";

class RADIX64Codec {
    static encode(data) {
        if (typeof data === 'string') {
            data = new TextEncoder().encode(data);
        }
        
        const output = [];
        let accumulator = 0;
        let bits_collected = 0;
        
        for (let i = 0; i < data.length; i++) {
            accumulator = ((accumulator << 8) | data[i]) >>> 0; // Keep as uint32
            bits_collected += 8;
            
            while (bits_collected >= 6) {
                bits_collected -= 6;
                const idx = (accumulator >> bits_collected) & 0x3F;
                output.push(RADIX64_ALPHABET[idx]);
            }
        }
        
        // Process trailing bits
        if (bits_collected > 0) {
            accumulator = (accumulator << (6 - bits_collected)) >>> 0;
            const idx = accumulator & 0x3F;
            output.push(RADIX64_ALPHABET[idx]);
        }
        
        return output.join('');
    }
    
    static decode(encoded) {
        if (!encoded) return new Uint8Array(0);
        
        // Build inverse lookup
        const inv_map = {};
        for (let i = 0; i < RADIX64_ALPHABET.length; i++) {
            inv_map[RADIX64_ALPHABET[i]] = i;
        }
        
        const output = [];
        let accumulator = 0;
        let bits_collected = 0;
        
        for (let i = 0; i < encoded.length; i++) {
            const char = encoded[i];
            if (!(char in inv_map)) {
                throw new Error(`[CODEC ERROR] Illegal character '${char}' in Radix64 stream.`);
            }
            
            const symbol_value = inv_map[char];
            accumulator = ((accumulator << 6) | symbol_value) >>> 0;
            bits_collected += 6;
            
            // Check final symbol for trailing bits
            if (i === encoded.length - 1) {
                const leftover_bits = bits_collected % 8;
                if (leftover_bits > 0) {
                    const mask = (1 << leftover_bits) - 1;
                    if ((accumulator & mask) !== 0) {
                        throw new Error("[CANONICAL VIOLATION] Rejecting malformed stream: Non-zero trailing bits found in final block padding.");
                    }
                }
            }
            
            while (bits_collected >= 8) {
                bits_collected -= 8;
                output.push((accumulator >> bits_collected) & 0xFF);
            }
        }
        
        // Check for impossible remainder
        if (bits_collected >= 6) {
            throw new Error("[CODEC ERROR] Rejecting malformed stream: Impossible structural byte format length.");
        }
        
        return new Uint8Array(output);
    }
}

if (typeof module !== 'undefined' && module.exports) {
    module.exports = { RADIX64Codec, RADIX64_ALPHABET };
}
