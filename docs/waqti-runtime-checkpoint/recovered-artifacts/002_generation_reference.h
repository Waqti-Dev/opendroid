// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "gguf_file.h"
#include "qwen_reference.h"
#include <cstdint>
#include <string>
#include <vector>

namespace waqti::generation {

struct Result {
    std::vector<uint32_t> generated_tokens;
    std::vector<std::vector<float>> logits;
};

// Reference autoregressive generation. The KV cache is kept across prompt and
// generated positions; this is greedy sampling only and intentionally has no
// external runtime dependency.
bool greedy(const waqti::gguf::MappedFile& file, const waqti::gguf::Metadata& metadata,
            const waqti::qwen::Config& config, const std::vector<uint32_t>& prompt_tokens,
            size_t max_new_tokens, uint32_t eos_token, Result& result,
            std::string* error = nullptr);

}  // namespace waqti::generation
