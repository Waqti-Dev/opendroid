// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace waqti::quant {

// Reference-only Q4_K block dequantization. It prioritizes correctness over speed.
bool dequantize_q4_k(const uint8_t* data, size_t bytes, size_t elements,
                    std::vector<float>& output, std::string* error = nullptr);

}  // namespace waqti::quant
