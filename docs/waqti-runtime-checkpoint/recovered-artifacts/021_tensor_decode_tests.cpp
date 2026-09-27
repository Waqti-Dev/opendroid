// SPDX-License-Identifier: Apache-2.0
#include "../tensor_decode.h"
#include <cassert>
#include <cmath>
#include <cstring>
#include <vector>

static waqti::gguf::TensorInfo tensor(uint32_t type) { return {"test", {32}, type, 0}; }
static void f32(std::vector<uint8_t>& b, float x) { uint32_t v; std::memcpy(&v, &x, 4); for (int i = 0; i < 4; ++i) b.push_back(static_cast<uint8_t>(v >> (8 * i))); }

int main() {
    std::vector<float> out; std::string error;
    std::vector<uint8_t> f32data; for (int i = 0; i < 32; ++i) f32(f32data, static_cast<float>(i));
    assert(waqti::tensor::decode_to_f32(tensor(0), f32data.data(), f32data.size(), out, &error));
    assert(out[7] == 7.0f);

    std::vector<uint8_t> f16data(64, 0); for (int i = 0; i < 32; ++i) { f16data[2*i] = 0x00; f16data[2*i+1] = 0x3c; }
    assert(waqti::tensor::decode_to_f32(tensor(1), f16data.data(), f16data.size(), out, &error));
    for (float v : out) assert(std::fabs(v - 1.0f) < 1e-6f);

    std::vector<uint8_t> q8(34, 0); q8[0] = 0x00; q8[1] = 0x3c; for (int i = 0; i < 32; ++i) q8[2+i] = static_cast<uint8_t>(i - 16);
    assert(waqti::tensor::decode_to_f32(tensor(8), q8.data(), q8.size(), out, &error));
    assert(out[0] == -16.0f && out[31] == 15.0f);
    assert(!waqti::tensor::decode_to_f32(tensor(8), q8.data(), 33, out, &error));
    std::vector<uint8_t> q5(22, 0); q5[0] = 0x00; q5[1] = 0x3c; q5[2] = q5[3] = q5[4] = q5[5] = 0xff;
    assert(waqti::tensor::decode_to_f32(tensor(6), q5.data(), q5.size(), out, &error));
    for (float v : out) assert(std::fabs(v) < 1e-6f);
    auto q6_info = tensor(14); q6_info.dimensions = {256};
    std::vector<uint8_t> q6(210, 0); q6[208] = 0x00; q6[209] = 0x3c;
    assert(waqti::tensor::decode_to_f32(q6_info, q6.data(), q6.size(), out, &error));
    assert(out.size() == 256); for (float v : out) assert(std::fabs(v) < 1e-6f);
    auto q4k_info = tensor(12); q4k_info.dimensions = {256};
    std::vector<uint8_t> q4k(144, 0); q4k[0] = q4k[2] = 0x00; q4k[1] = q4k[3] = 0x3c;
    assert(waqti::tensor::decode_to_f32(q4k_info, q4k.data(), q4k.size(), out, &error));
    assert(out.size() == 256); for (float v : out) assert(std::fabs(v) < 1e-6f);
    return 0;
}
