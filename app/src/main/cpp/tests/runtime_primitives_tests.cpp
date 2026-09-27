// SPDX-License-Identifier: Apache-2.0
#include "../tokenizer.h"
#include "../tensor_primitives.h"
#include <cassert>
#include <cmath>

int main() {
    waqti::tokenizer::Model model;
    model.unknown_id = 99;
    model.vocab = {{"a", 1}, {"b", 2}, {"ab", 3}, {"c", 4}};
    for (const auto& [token, id] : model.vocab) model.reverse_vocab[id] = token;
    model.merge_rank[std::string("a") + '\0' + "b"] = 0;
    waqti::tokenizer::ByteBpeTokenizer tokenizer(model);
    auto ids = tokenizer.encode("abc");
    assert((ids == std::vector<int32_t>{3, 4}));
    assert(tokenizer.decode(ids) == "abc");
    assert(tokenizer.encode("z")[0] == 99);

    using waqti::tensor::Matrix;
    Matrix a{2, 3, {1, 2, 3, 4, 5, 6}};
    Matrix b{3, 2, {7, 8, 9, 10, 11, 12}};
    Matrix c; std::string error;
    assert(waqti::tensor::matmul(a, b, c, &error));
    assert(c.rows == 2 && c.cols == 2 && std::fabs(c.values[0] - 58.0f) < 1e-5f && std::fabs(c.values[3] - 154.0f) < 1e-5f);
    Matrix bad{1, 1, {1}}; assert(!waqti::tensor::matmul(a, bad, c, &error));
    std::vector<float> x{3, 4}; assert(waqti::tensor::rms_norm(x, {1, 1}, 1e-5f, &error));
    assert(std::fabs(x[0] - 0.8485f) < 1e-3f && std::fabs(x[1] - 1.1314f) < 1e-3f);
    return 0;
}
