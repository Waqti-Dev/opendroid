// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace waqti::attention {

bool concatenate_heads(const std::vector<float>& heads, size_t head_count, size_t head_dim,
                       std::vector<float>& output, std::string* error = nullptr);
bool add_residual(const std::vector<float>& residual_input, const std::vector<float>& projected,
                  std::vector<float>& output, std::string* error = nullptr);
bool apply_rope_interleaved(std::vector<float>& values, size_t heads, size_t head_dim,
                            size_t position, float theta, std::string* error = nullptr);
bool single_token_causal_attention(const std::vector<float>& q,
                                   const std::vector<float>& k,
                                   const std::vector<float>& v,
                                   size_t q_heads, size_t kv_heads, size_t head_dim,
                                   std::vector<float>& output, std::string* error = nullptr);
bool causal_attention(const std::vector<float>& q, const std::vector<float>& keys,
                      const std::vector<float>& values, size_t positions,
                      size_t q_heads, size_t kv_heads, size_t head_dim,
                      std::vector<float>& output, std::string* error = nullptr);

}  // namespace waqti::attention
