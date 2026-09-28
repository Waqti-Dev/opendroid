// SPDX-License-Identifier: Apache-2.0
#include "../toy_dense_model.h"
#include <cassert>

int main() {
    // Each embedding row maps deterministically to the next token through projection.
    waqti::tensor::Matrix embeddings{3, 2, {1, 0, 0, 1, 0, 2}};
    waqti::tensor::Matrix projection{2, 3, {0, 1, 0, 0, 0, 1}};
    waqti::model::ToyDenseModel model(embeddings, projection);
    std::vector<int32_t> output; std::string error;
    assert(model.generate({0}, 3, output, &error));
    assert((output == std::vector<int32_t>{0, 1, 2, 2}));
    assert(!model.generate({}, 1, output, &error));
    assert(!model.generate({9}, 1, output, &error));
    return 0;
}
