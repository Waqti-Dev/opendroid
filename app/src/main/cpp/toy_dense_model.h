// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "tensor_primitives.h"
#include <cstdint>
#include <string>
#include <vector>

namespace waqti::model {

// Verification harness only: this is not a GGUF transformer implementation.
class ToyDenseModel {
 public:
    ToyDenseModel(tensor::Matrix embeddings, tensor::Matrix projection);
    bool generate(const std::vector<int32_t>& prompt, size_t max_new_tokens,
                  std::vector<int32_t>& output, std::string* error = nullptr) const;

 private:
    tensor::Matrix embeddings_;
    tensor::Matrix projection_;
};

}  // namespace waqti::model
