// SPDX-License-Identifier: Apache-2.0
#include "../attention_reference.h"
#include <cassert>
#include <cmath>
#include <vector>

int main() {
    std::vector<float> rope{1.0f, 0.0f, 0.0f, 1.0f};
    std::string error;
    std::vector<float> concatenated;
    assert(waqti::attention::concatenate_heads({1, 2, 3, 4}, 2, 2, concatenated, &error));
    assert((concatenated == std::vector<float>{1, 2, 3, 4}));
    std::vector<float> residual;
    assert(waqti::attention::add_residual({1, 2}, {3, 4}, residual, &error));
    assert((residual == std::vector<float>{4, 6}));
    assert(waqti::attention::apply_rope_interleaved(rope, 1, 4, 0, 1000000.0f, &error));
    assert(std::fabs(rope[0] - 1.0f) < 1e-6f && std::fabs(rope[1]) < 1e-6f);
    std::vector<float> position_one{1.0f, 0.0f, 0.0f, 0.0f};
    assert(waqti::attention::apply_rope_interleaved(position_one, 1, 4, 1, 1.0f, &error));
    assert(std::fabs(position_one[0] - std::cos(1.0f)) < 1e-6f);
    assert(std::fabs(position_one[1] - std::sin(1.0f)) < 1e-6f);

    // 4 query heads, 2 KV heads, head_dim=2 => KV heads repeat twice.
    std::vector<float> q(8, 1.0f), k{1, 0, 0, 1}, v{2, 3, 4, 5}, output;
    assert(waqti::attention::single_token_causal_attention(q, k, v, 4, 2, 2, output, &error));
    assert(output.size() == 8);
    assert(output[0] == 2 && output[1] == 3 && output[2] == 2 && output[3] == 3);
    assert(output[4] == 4 && output[5] == 5 && output[6] == 4 && output[7] == 5);
    assert(!waqti::attention::single_token_causal_attention(q, k, v, 3, 2, 2, output, &error));
    std::vector<float> multi;
    // One query head, two cached positions: both positions must contribute.
    assert(waqti::attention::causal_attention({1, 0}, {1, 0, 0, 1}, {2, 0, 0, 4}, 2, 1, 1, 2, multi, &error));
    const float p0 = std::exp(1.0f / std::sqrt(2.0f));
    const float p1 = 1.0f;
    assert(std::fabs(multi[0] - (p0 * 2.0f) / (p0 + p1)) < 1e-5f);
    assert(std::fabs(multi[1] - (p1 * 4.0f) / (p0 + p1)) < 1e-5f);
    return 0;
}
