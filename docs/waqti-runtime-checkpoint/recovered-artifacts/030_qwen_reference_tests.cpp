// SPDX-License-Identifier: Apache-2.0
#include "../qwen_reference.h"
#include <cassert>
#include <cmath>
#include <vector>

int main() {
    waqti::gguf::Metadata metadata;
    metadata.architecture = "qwen2"; metadata.embedding_length = 32; metadata.block_count = 1;
    metadata.attention_head_count = 4; metadata.attention_head_count_kv = 2;
    metadata.feed_forward_length = 64; metadata.context_length = 128;
    metadata.rope_freq_base = 1000000.0; metadata.rms_norm_epsilon = 1e-6;
    metadata.tokenizer_tokens.resize(128);
    waqti::qwen::Config config; std::string error;
    assert(waqti::qwen::config_from_metadata(metadata, config, &error));
    assert(config.head_dimension == 8 && config.vocab_size == 128);

    // One Q5_0 row, with all decoded values equal to zero (q=16, scale=1).
    std::vector<uint8_t> bytes(22, 0); bytes[0] = 0x00; bytes[1] = 0x3c;
    bytes[2] = bytes[3] = bytes[4] = bytes[5] = 0xff;
    waqti::gguf::TensorInfo info{"token_embd.weight", {32, 1}, 6, 0};
    waqti::gguf::TensorView view{&info, bytes.data(), bytes.size()};
    std::vector<float> row;
    assert(waqti::qwen::embedding_row(view, 0, row, &error));
    assert(row.size() == 32); for (float value : row) assert(std::fabs(value) < 1e-6f);

    waqti::gguf::TensorInfo norm_info{"norm", {32}, 0, 0};
    std::vector<uint8_t> norm_bytes(32 * 4, 0); for (int i = 0; i < 32; ++i) { norm_bytes[i*4] = 0x00; norm_bytes[i*4+1] = 0x00; norm_bytes[i*4+2] = 0x80; norm_bytes[i*4+3] = 0x3f; }
    waqti::gguf::TensorView norm_view{&norm_info, norm_bytes.data(), norm_bytes.size()};
    std::vector<float> input(32, 2.0f), normalized;
    assert(waqti::qwen::rms_norm_reference(input, norm_view, 1e-6f, normalized, &error));
    for (float value : normalized) assert(std::fabs(value - 1.0f) < 1e-5f);

    std::vector<uint8_t> matrix_bytes(68, 0);
    matrix_bytes[0] = 0x00; matrix_bytes[1] = 0x3c;
    for (int i = 0; i < 32; ++i) matrix_bytes[2 + i] = 1;
    matrix_bytes[34] = 0x00; matrix_bytes[35] = 0x3c;
    for (int i = 0; i < 32; ++i) matrix_bytes[36 + i] = 2;
    waqti::gguf::TensorInfo matrix_info{"mat", {32, 2}, 8, 0};
    waqti::gguf::TensorView matrix_view{&matrix_info, matrix_bytes.data(), matrix_bytes.size()};
    std::vector<float> projected;
    assert(waqti::qwen::matvec_reference(matrix_view, input, projected, &error));
    assert(projected.size() == 2 && std::fabs(projected[0] - 64.0f) < 1e-5f && std::fabs(projected[1] - 128.0f) < 1e-5f);
    std::vector<float> activated, gated;
    assert(waqti::qwen::silu({0.0f, 1.0f, -1.0f}, activated, &error));
    assert(std::fabs(activated[0]) < 1e-6f && std::fabs(activated[1] - 0.7310586f) < 1e-5f);
    assert(waqti::qwen::swiglu({1.0f, 2.0f}, {3.0f, 4.0f}, gated, &error));
    assert(std::fabs(gated[0] - 2.1931758f) < 1e-5f);
    size_t best = 0; float best_value = 0.0f;
    assert(waqti::qwen::argmax({-1.0f, 3.0f, 2.0f}, best, best_value, &error));
    assert(best == 1 && best_value == 3.0f);
    return 0;
}
