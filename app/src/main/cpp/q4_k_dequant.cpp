// SPDX-License-Identifier: Apache-2.0
#include "q4_k_dequant.h"

#include <cmath>
#include <cstring>
#include <limits>

namespace waqti::quant {
namespace {
float fp16_to_float(uint16_t h) {
    const uint32_t sign = static_cast<uint32_t>(h & 0x8000) << 16;
    const uint32_t exponent = (h >> 10) & 0x1f;
    const uint32_t fraction = h & 0x3ff;
    uint32_t bits;
    if (exponent == 0) {
        if (fraction == 0) bits = sign;
        else {
            uint32_t f = fraction; uint32_t e = 0;
            while ((f & 0x400) == 0) { f <<= 1; ++e; }
            bits = sign | ((113 - e) << 23) | ((f & 0x3ff) << 13);
        }
    } else if (exponent == 31) bits = sign | 0x7f800000u | (fraction << 13);
    else bits = sign | ((exponent + 112) << 23) | (fraction << 13);
    float output; std::memcpy(&output, &bits, sizeof(output)); return output;
}
uint16_t read_u16(const uint8_t* p) { return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8); }
}

bool dequantize_q4_k(const uint8_t* data, size_t bytes, size_t elements,
                    std::vector<float>& output, std::string* error) {
    constexpr size_t kElementsPerBlock = 256;
    constexpr size_t kBytesPerBlock = 144;
    if (data == nullptr || elements == 0 || elements % kElementsPerBlock != 0) { if (error) *error = "Q4_K element count must be a positive multiple of 256"; return false; }
    const size_t blocks = elements / kElementsPerBlock;
    if (blocks > std::numeric_limits<size_t>::max() / kBytesPerBlock || bytes < blocks * kBytesPerBlock) { if (error) *error = "Q4_K payload is truncated"; return false; }
    output.resize(elements);
    for (size_t block = 0; block < blocks; ++block) {
        const uint8_t* src = data + block * kBytesPerBlock;
        const float d = fp16_to_float(read_u16(src));
        const float dmin = fp16_to_float(read_u16(src + 2));
        const uint8_t* scales = src + 4;
        const uint8_t* qs = src + 16;
        size_t out = block * kElementsPerBlock;
        size_t q = 0;
        for (size_t group = 0; group < 8; ++group) {
            uint8_t scale, minimum;
            if (group < 4) { scale = scales[group] & 0x3f; minimum = scales[group + 4] & 0x3f; }
            else { scale = static_cast<uint8_t>((scales[group + 4] & 0x0f) | ((scales[group - 4] >> 6) << 4)); minimum = static_cast<uint8_t>((scales[group + 4] >> 4) | ((scales[group] >> 6) << 4)); }
            const float ds = d * static_cast<float>(scale);
            const float dm = dmin * static_cast<float>(minimum);
            for (size_t l = 0; l < 16; ++l) {
                output[out++] = ds * static_cast<float>(qs[q] & 0x0f) - dm;
                output[out++] = ds * static_cast<float>(qs[q] >> 4) - dm;
                ++q;
            }
        }
    }
    return true;
}

}  // namespace waqti::quant
