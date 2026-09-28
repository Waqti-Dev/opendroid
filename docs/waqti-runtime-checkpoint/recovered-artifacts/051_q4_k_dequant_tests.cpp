// SPDX-License-Identifier: Apache-2.0
#include "../q4_k_dequant.h"
#include <cassert>
#include <cmath>
#include <vector>

int main() {
    std::vector<uint8_t> block(144, 0);
    block[0] = 0x00; block[1] = 0x3c; // d=1
    // dmin=0, scales[0..7]=1, qs=1 => first/second nibble decode to 1.
    for (size_t i = 4; i < 16; ++i) block[i] = 1;
    for (size_t i = 16; i < 144; ++i) block[i] = 0x11;
    std::vector<float> output; std::string error;
    assert(waqti::quant::dequantize_q4_k(block.data(), block.size(), 256, output, &error));
    assert(output.size() == 256);
    for (float value : output) assert(std::fabs(value - 1.0f) < 1e-5f);
    assert(!waqti::quant::dequantize_q4_k(block.data(), 143, 256, output, &error));
    assert(!waqti::quant::dequantize_q4_k(block.data(), block.size(), 255, output, &error));
    return 0;
}
