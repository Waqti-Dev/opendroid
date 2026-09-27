// SPDX-License-Identifier: Apache-2.0
#include "attention_reference.h"

#include <cmath>
#include <limits>

namespace waqti::attention {

bool concatenate_heads(const std::vector<float>& heads, size_t head_count, size_t head_dim,
                       std::vector<float>& output, std::string* error) {
    if (head_count == 0 || head_dim == 0 || heads.size() != head_count * head_dim) {
        if (error) *error = "invalid head concatenation shape";
        return false;
    }
    output = heads;
    return true;
}

bool add_residual(const std::vector<float>& residual_input, const std::vector<float>& projected,
                  std::vector<float>& output, std::string* error) {
    if (residual_input.empty() || residual_input.size() != projected.size()) {
        if (error) *error = "residual dimensions do not match";
        return false;
    }
    output.resize(residual_input.size());
    for (size_t i = 0; i < output.size(); ++i) output[i] = residual_input[i] + projected[i];
    return true;
}

bool apply_rope_interleaved(std::vector<float>& values, size_t heads, size_t head_dim,
                            size_t position, float theta, std::string* error) {
    if (heads == 0 || head_dim == 0 || (head_dim % 2) != 0 || values.size() != heads * head_dim || theta <= 0.0f) {
        if (error) *error = "invalid RoPE shape or theta";
        return false;
    }
    for (size_t head = 0; head < heads; ++head) {
        const size_t base = head * head_dim;
        for (size_t pair = 0; pair < head_dim / 2; ++pair) {
            const float inv_freq = std::pow(theta, -static_cast<float>(2 * pair) / static_cast<float>(head_dim));
            const float angle = static_cast<float>(position) * inv_freq;
            const float c = std::cos(angle), s = std::sin(angle);
            const float x0 = values[base + 2 * pair];
            const float x1 = values[base + 2 * pair + 1];
            values[base + 2 * pair] = x0 * c - x1 * s;
            values[base + 2 * pair + 1] = x0 * s + x1 * c;
        }
    }
    return true;
}

bool single_token_causal_attention(const std::vector<float>& q,
                                   const std::vector<float>& k,
                                   const std::vector<float>& v,
                                   size_t q_heads, size_t kv_heads, size_t head_dim,
                                   std::vector<float>& output, std::string* error) {
    if (q_heads == 0 || kv_heads == 0 || head_dim == 0 || q_heads % kv_heads != 0 ||
        q.size() != q_heads * head_dim || k.size() != kv_heads * head_dim || v.size() != kv_heads * head_dim) {
        if (error) *error = "invalid GQA dimensions";
        return false;
    }
    const size_t repeat = q_heads / kv_heads;
    output.assign(q.size(), 0.0f);
    const float scale = 1.0f / std::sqrt(static_cast<float>(head_dim));
    // With one cached position, causal softmax has exactly one admissible score,
    // so its probability is one. This is intentionally the minimal correctness path.
    for (size_t q_head = 0; q_head < q_heads; ++q_head) {
        const size_t kv_head = q_head / repeat;
        float score = 0.0f;
        for (size_t i = 0; i < head_dim; ++i) score += q[q_head * head_dim + i] * k[kv_head * head_dim + i];
        score *= scale;
        if (!std::isfinite(score)) { if (error) *error = "non-finite attention score"; return false; }
        for (size_t i = 0; i < head_dim; ++i) output[q_head * head_dim + i] = v[kv_head * head_dim + i];
    }
    return true;
}

bool causal_attention(const std::vector<float>& q, const std::vector<float>& keys,
                      const std::vector<float>& values, size_t positions,
                      size_t q_heads, size_t kv_heads, size_t head_dim,
                      std::vector<float>& output, std::string* error) {
    if (positions == 0 || q_heads == 0 || kv_heads == 0 || head_dim == 0 ||
        q_heads % kv_heads != 0 || q.size() != q_heads * head_dim ||
        keys.size() != positions * kv_heads * head_dim ||
        values.size() != positions * kv_heads * head_dim) {
        if (error) *error = "invalid multi-position GQA dimensions";
        return false;
    }
    const size_t repeat = q_heads / kv_heads;
    const float scale = 1.0f / std::sqrt(static_cast<float>(head_dim));
    output.assign(q.size(), 0.0f);
    std::vector<float> scores(positions);
    for (size_t q_head = 0; q_head < q_heads; ++q_head) {
        const size_t kv_head = q_head / repeat;
        float maximum = -std::numeric_limits<float>::infinity();
        for (size_t position = 0; position < positions; ++position) {
            float score = 0.0f;
            const size_t base = position * kv_heads * head_dim + kv_head * head_dim;
            for (size_t i = 0; i < head_dim; ++i) score += q[q_head * head_dim + i] * keys[base + i];
            scores[position] = score * scale;
            if (!std::isfinite(scores[position])) { if (error) *error = "non-finite causal attention score"; return false; }
            maximum = std::max(maximum, scores[position]);
        }
        float denominator = 0.0f;
        for (size_t position = 0; position < positions; ++position) { scores[position] = std::exp(scores[position] - maximum); denominator += scores[position]; }
        if (!(denominator > 0.0f) || !std::isfinite(denominator)) { if (error) *error = "invalid causal attention softmax"; return false; }
        for (size_t position = 0; position < positions; ++position) {
            const float probability = scores[position] / denominator;
            const size_t base = position * kv_heads * head_dim + kv_head * head_dim;
            for (size_t i = 0; i < head_dim; ++i) output[q_head * head_dim + i] += probability * values[base + i];
        }
    }
    return true;
}

}  // namespace waqti::attention
