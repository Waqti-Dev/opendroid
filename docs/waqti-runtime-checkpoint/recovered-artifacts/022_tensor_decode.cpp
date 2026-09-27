// SPDX-License-Identifier: Apache-2.0
#include "tensor_decode.h"
#include "q4_k_dequant.h"

#include <cstring>
#include <limits>

namespace waqti::tensor {
namespace {
float half(uint16_t h) {
    const uint32_t sign = static_cast<uint32_t>(h & 0x8000) << 16;
    const uint32_t exponent = (h >> 10) & 0x1f;
    const uint32_t fraction = h & 0x3ff;
    uint32_t bits;
    if (exponent == 0) {
        if (fraction == 0) bits = sign;
        else { uint32_t f = fraction, e = 0; while ((f & 0x400) == 0) { f <<= 1; ++e; } bits = sign | ((113 - e) << 23) | ((f & 0x3ff) << 13); }
    } else if (exponent == 31) bits = sign | 0x7f800000u | (fraction << 13);
    else bits = sign | ((exponent + 112) << 23) | (fraction << 13);
    float out; std::memcpy(&out, &bits, sizeof(out)); return out;
}
uint16_t u16(const uint8_t* p) { return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8); }
bool count(const waqti::gguf::TensorInfo& tensor, size_t& out) {
    uint64_t n = 1;
    for (uint64_t d : tensor.dimensions) { if (d == 0 || n > std::numeric_limits<uint64_t>::max() / d) return false; n *= d; }
    if (n > std::numeric_limits<size_t>::max()) return false;
    out = static_cast<size_t>(n); return true;
}
}

bool decode_to_f32(const waqti::gguf::TensorInfo& tensor, const uint8_t* data,
                   size_t bytes, std::vector<float>& output, std::string* error) {
    size_t elements;
    if (data == nullptr || !count(tensor, elements)) { if (error) *error = "invalid tensor data or dimensions"; return false; }
    uint64_t expected;
    if (!waqti::gguf::tensor_byte_size(tensor, expected) || expected > bytes) { if (error) *error = "tensor payload is truncated or unsupported"; return false; }
    output.resize(elements);
    if (tensor.type == 0) { for (size_t i = 0; i < elements; ++i) std::memcpy(&output[i], data + i * 4, 4); return true; }
    if (tensor.type == 1) { for (size_t i = 0; i < elements; ++i) output[i] = half(u16(data + i * 2)); return true; }
    if (tensor.type == 8) {
        constexpr size_t kBlock = 32, kBytes = 34;
        for (size_t block = 0; block < elements / kBlock; ++block) {
            const uint8_t* src = data + block * kBytes; const float scale = half(u16(src));
            for (size_t i = 0; i < kBlock; ++i) output[block * kBlock + i] = scale * static_cast<float>(static_cast<int8_t>(src[2 + i]));
        }
        return true;
    }
    if (tensor.type == 6) {
        constexpr size_t kBlock = 32, kBytes = 22;
        for (size_t block = 0; block < elements / kBlock; ++block) {
            const uint8_t* src = data + block * kBytes;
            const float scale = half(u16(src));
            const uint32_t high = static_cast<uint32_t>(src[2]) |
                                  (static_cast<uint32_t>(src[3]) << 8) |
                                  (static_cast<uint32_t>(src[4]) << 16) |
                                  (static_cast<uint32_t>(src[5]) << 24);
            for (size_t i = 0; i < kBlock; ++i) {
                const uint8_t low = (src[6 + i / 2] >> (4 * (i % 2))) & 0x0f;
                const int value = static_cast<int>(low) | (((high >> i) & 1u) << 4);
                output[block * kBlock + i] = scale * static_cast<float>(value - 16);
            }
        }
        return true;
    }
    if (tensor.type == 14) {
        constexpr size_t kBlock = 256, kBytes = 210;
        for (size_t block = 0; block < elements / kBlock; ++block) {
            const uint8_t* src = data + block * kBytes;
            const uint8_t* ql = src;
            const uint8_t* qh = src + 128;
            const int8_t* scales = reinterpret_cast<const int8_t*>(src + 192);
            const float scale = half(u16(src + 208));
            for (size_t n = 0; n < kBlock; n += 128) {
                for (size_t l = 0; l < 32; ++l) {
                    const size_t is = l / 16;
                    const int q1 = static_cast<int>((ql[l] & 0x0f) | (((qh[l] >> 0) & 3) << 4)) - 32;
                    const int q2 = static_cast<int>((ql[l + 32] & 0x0f) | (((qh[l] >> 2) & 3) << 4)) - 32;
                    const int q3 = static_cast<int>(((ql[l] >> 4) & 0x0f) | (((qh[l] >> 4) & 3) << 4)) - 32;
                    const int q4 = static_cast<int>(((ql[l + 32] >> 4) & 0x0f) | (((qh[l] >> 6) & 3) << 4)) - 32;
                    output[block * kBlock + n + l] = scale * scales[is + 0] * q1;
                    output[block * kBlock + n + l + 32] = scale * scales[is + 2] * q2;
                    output[block * kBlock + n + l + 64] = scale * scales[is + 4] * q3;
                    output[block * kBlock + n + l + 96] = scale * scales[is + 6] * q4;
                }
                ql += 64; qh += 32; scales += 8;
            }
        }
        return true;
    }
    if (tensor.type == 12) {
        return waqti::quant::dequantize_q4_k(data, bytes, elements, output, error);
    }
    if (error) *error = "tensor type is not implemented by reference decoder";
    return false;
}

}  // namespace waqti::tensor
