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

    // Qwen/GPT-style byte BPE stores non-ASCII UTF-8 bytes as mapped Unicode
    // code points in the GGUF vocabulary. Verify Arabic and mixed text round-trip
    // through that representation rather than falling back to unknown tokens.
    waqti::tokenizer::Model qwen_model;
    qwen_model.unknown_id = 999;
    const std::vector<std::string> qwen_pieces = {
        u8"Ù", u8"ħ", u8"Ø", u8"±", u8"Ń", u8"¨", u8"§",
        "H", "e", "l", "o", u8"Ġ"
    };
    int32_t next_id = 10;
    for (const auto& piece : qwen_pieces) {
        qwen_model.vocab[piece] = next_id;
        qwen_model.reverse_vocab[next_id] = piece;
        ++next_id;
    }
    waqti::tokenizer::ByteBpeTokenizer qwen_tokenizer(qwen_model);
    const auto arabic_ids = qwen_tokenizer.encode(u8"مرحبا");
    assert(arabic_ids.size() == 10);
    assert(qwen_tokenizer.decode(arabic_ids) == u8"مرحبا");
    const auto mixed_ids = qwen_tokenizer.encode(u8"Hello مرحبا");
    assert(qwen_tokenizer.decode(mixed_ids) == u8"Hello مرحبا");

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
