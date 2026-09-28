// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace waqti::tensor {

struct Matrix {
    size_t rows = 0;
    size_t cols = 0;
    std::vector<float> values;
};

bool matmul(const Matrix& lhs, const Matrix& rhs, Matrix& output, std::string* error = nullptr);
bool rms_norm(std::vector<float>& values, const std::vector<float>& weight,
              float epsilon = 1e-5f, std::string* error = nullptr);

}  // namespace waqti::tensor
