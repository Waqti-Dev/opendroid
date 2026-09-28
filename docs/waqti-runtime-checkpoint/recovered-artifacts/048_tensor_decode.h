// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "gguf_reader.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace waqti::tensor {

// Reference decode path. Optimized NEON kernels will replace this after correctness.
bool decode_to_f32(const waqti::gguf::TensorInfo& tensor, const uint8_t* data,
                   size_t bytes, std::vector<float>& output,
                   std::string* error = nullptr);

}  // namespace waqti::tensor
