// SPDX-License-Identifier: Apache-2.0
#include "generation_reference.h"
#include "transformer_layer.h"

#include <limits>

namespace waqti::generation {

bool greedy(const waqti::gguf::MappedFile& file, const waqti::gguf::Metadata& metadata,
            const waqti::qwen::Config& config, const std::vector<uint32_t>& prompt_tokens,
            size_t max_new_tokens, uint32_t eos_token, Result& result,
            std::string* error) {
    if (prompt_tokens.empty() || prompt_tokens.size() > config.context_length ||
        max_new_tokens > config.context_length - prompt_tokens.size()) {
        if (error) *error = "invalid prompt or generation length";
        return false;
    }
    waqti::gguf::TensorView embedding, norm, output_weight;
    if (!file.view(metadata, "token_embd.weight", embedding, error) ||
        !file.view(metadata, "output_norm.weight", norm, error) ||
        !file.view(metadata, "output.weight", output_weight, error)) return false;

    waqti::transformer::KVCache cache;
    if (!cache.initialize(config.block_count, config.kv_heads, config.head_dimension, error)) return false;
    result = {};
    result.generated_tokens.reserve(max_new_tokens);
    result.logits.reserve(max_new_tokens);

    std::vector<float> last_logits;
    const size_t total_prompt = prompt_tokens.size();
    for (size_t position = 0; position < total_prompt; ++position) {
        std::vector<float> hidden;
        if (!waqti::qwen::embedding_row(embedding, prompt_tokens[position], hidden, error)) return false;
        for (size_t layer = 0; layer < config.block_count; ++layer) {
            std::vector<float> next;
            if (!waqti::transformer::forward_layer_cached(file, metadata, config, layer,
                                                           hidden, position, cache, next,
                                                           nullptr, error)) return false;
            hidden.swap(next);
        }
        std::vector<float> final_hidden;
        if (!waqti::qwen::final_norm_and_logits(hidden, norm, output_weight,
                                                static_cast<float>(config.rms_epsilon),
                                                final_hidden, last_logits, error)) return false;
    }

    for (size_t step = 0; step < max_new_tokens; ++step) {
        size_t next_id = 0;
        float next_value = 0.0f;
        if (!waqti::qwen::argmax(last_logits, next_id, next_value, error)) return false;
        (void)next_value;
        const uint32_t token = static_cast<uint32_t>(next_id);
        result.generated_tokens.push_back(token);
        result.logits.push_back(last_logits);
        if (token == eos_token) break;
        const size_t position = total_prompt + step;
        std::vector<float> hidden;
        if (!waqti::qwen::embedding_row(embedding, token, hidden, error)) return false;
        for (size_t layer = 0; layer < config.block_count; ++layer) {
            std::vector<float> next;
            if (!waqti::transformer::forward_layer_cached(file, metadata, config, layer,
                                                           hidden, position, cache, next,
                                                           nullptr, error)) return false;
            hidden.swap(next);
        }
        std::vector<float> final_hidden;
        if (!waqti::qwen::final_norm_and_logits(hidden, norm, output_weight,
                                                static_cast<float>(config.rms_epsilon),
                                                final_hidden, last_logits, error)) return false;
    }
    return true;
}

}  // namespace waqti::generation
