// SPDX-License-Identifier: Apache-2.0
#include "tensor_primitives.h"

#include <cmath>

namespace waqti::tensor {
namespace {
void fail(std::string* error, const char* message) { if (error) *error = message; }
}

bool matmul(const Matrix& lhs, const Matrix& rhs, Matrix& output, std::string* error) {
    if (lhs.values.size() != lhs.rows * lhs.cols || rhs.values.size() != rhs.rows * rhs.cols) {
        fail(error, "matrix storage size mismatch"); return false;
    }
    if (lhs.cols != rhs.rows) { fail(error, "matrix dimensions are incompatible"); return false; }
    Matrix result{lhs.rows, rhs.cols, std::vector<float>(lhs.rows * rhs.cols, 0.0f)};
    for (size_t i = 0; i < lhs.rows; ++i) {
        for (size_t k = 0; k < lhs.cols; ++k) {
            const float a = lhs.values[i * lhs.cols + k];
            for (size_t j = 0; j < rhs.cols; ++j) result.values[i * rhs.cols + j] += a * rhs.values[k * rhs.cols + j];
        }
    }
    output = std::move(result); return true;
}

bool rms_norm(std::vector<float>& values, const std::vector<float>& weight, float epsilon, std::string* error) {
    if (values.empty() || values.size() != weight.size()) { fail(error, "RMSNorm vector sizes differ"); return false; }
    if (!(epsilon > 0.0f) || !std::isfinite(epsilon)) { fail(error, "invalid RMSNorm epsilon"); return false; }
    double sum = 0.0;
    for (float value : values) { if (!std::isfinite(value)) { fail(error, "non-finite RMSNorm input"); return false; } sum += static_cast<double>(value) * value; }
    const float scale = 1.0f / std::sqrt(static_cast<float>(sum / values.size()) + epsilon);
    for (size_t i = 0; i < values.size(); ++i) values[i] = values[i] * scale * weight[i];
    return true;
}

}  // namespace waqti::tensor
