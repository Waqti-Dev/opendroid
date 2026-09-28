// SPDX-License-Identifier: Apache-2.0
#include "toy_dense_model.h"

#include <algorithm>
#include <limits>

namespace waqti::model {
ToyDenseModel::ToyDenseModel(tensor::Matrix embeddings, tensor::Matrix projection)
    : embeddings_(std::move(embeddings)), projection_(std::move(projection)) {}

bool ToyDenseModel::generate(const std::vector<int32_t>& prompt, size_t max_new_tokens,
                             std::vector<int32_t>& output, std::string* error) const {
    if (prompt.empty()) { if (error) *error = "prompt cannot be empty"; return false; }
    if (embeddings_.values.size() != embeddings_.rows * embeddings_.cols ||
        projection_.values.size() != projection_.rows * projection_.cols ||
        projection_.rows != embeddings_.cols || projection_.cols != embeddings_.rows) {
        if (error) *error = "toy model dimensions are inconsistent";
        return false;
    }
    output = prompt;
    if (max_new_tokens == 0) return true;
    int32_t current = prompt.back();
    for (size_t step = 0; step < max_new_tokens; ++step) {
        if (current < 0 || static_cast<size_t>(current) >= embeddings_.rows) { if (error) *error = "token id outside vocabulary"; return false; }
        tensor::Matrix row{1, embeddings_.cols,
            std::vector<float>(embeddings_.values.begin() + current * embeddings_.cols,
                               embeddings_.values.begin() + (current + 1) * embeddings_.cols)};
        tensor::Matrix logits;
        if (!tensor::matmul(row, projection_, logits, error)) return false;
        auto best = std::max_element(logits.values.begin(), logits.values.end());
        if (best == logits.values.end()) { if (error) *error = "empty logits"; return false; }
        current = static_cast<int32_t>(std::distance(logits.values.begin(), best));
        output.push_back(current);
    }
    return true;
}
}  // namespace waqti::model
