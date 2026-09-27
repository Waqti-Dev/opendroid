// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "gguf_file.h"
#include "gguf_reader.h"
#include <cstdint>
#include <string>
#include <vector>

namespace waqti::qwen {

struct Config {
    uint64_t vocab_size = 0;
    uint64_t embedding_length = 0;
    uint64_t block_count = 0;
    uint64_t attention_heads = 0;
    uint64_t kv_heads = 0;
    uint64_t head_dimension = 0;
    uint64_t feed_forward_length = 0;
    uint64_t context_length = 0;
    double rope_freq_base = 0.0;
    double rms_epsilon = 0.0;
};

bool config_from_metadata(const waqti::gguf::Metadata& metadata, Config& config,
                          std::string* error = nullptr);
bool embedding_row(const waqti::gguf::TensorView& embedding, uint32_t token_id,
                   std::vector<float>& output, std::string* error = nullptr);
bool rms_norm_reference(const std::vector<float>& input, const waqti::gguf::TensorView& weight,
                        float epsilon, std::vector<float>& output,
                        std::string* error = nullptr);
bool matvec_reference(const waqti::gguf::TensorView& matrix, const std::vector<float>& input,
                      std::vector<float>& output, std::string* error = nullptr);
bool silu(const std::vector<float>& input, std::vector<float>& output,
          std::string* error = nullptr);
bool swiglu(const std::vector<float>& gate, const std::vector<float>& up,
            std::vector<float>& output, std::string* error = nullptr);
bool argmax(const std::vector<float>& values, size_t& index, float& value,
            std::string* error = nullptr);
bool final_norm_and_logits(const std::vector<float>& second_residual,
                           const waqti::gguf::TensorView& norm,
                           const waqti::gguf::TensorView& output_weight,
                           float epsilon, std::vector<float>& final_hidden,
                           std::vector<float>& logits, std::string* error = nullptr);

}  // namespace waqti::qwen
