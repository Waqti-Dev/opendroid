// SPDX-License-Identifier: Apache-2.0
#include "sequence_reference.h"
#include "transformer_layer.h"

namespace waqti::sequence {

bool forward(const waqti::gguf::MappedFile& file, const waqti::gguf::Metadata& metadata,
             const waqti::qwen::Config& config, const std::vector<uint32_t>& token_ids,
             Result& result, std::string* error) {
    if (token_ids.empty() || token_ids.size() > config.context_length) { if (error) *error = "invalid sequence length"; return false; }
    waqti::gguf::TensorView embedding, norm, output_weight;
    if (!file.view(metadata, "token_embd.weight", embedding, error) ||
        !file.view(metadata, "output_norm.weight", norm, error) ||
        !file.view(metadata, "output.weight", output_weight, error)) return false;
    waqti::transformer::KVCache cache;
    if (!cache.initialize(config.block_count, config.kv_heads, config.head_dimension, error)) return false;
    result = {};
    result.hidden.reserve(token_ids.size()); result.final_hidden.reserve(token_ids.size()); result.logits.reserve(token_ids.size());
    for (size_t position = 0; position < token_ids.size(); ++position) {
        std::vector<float> hidden;
        if (!waqti::qwen::embedding_row(embedding, token_ids[position], hidden, error)) return false;
        for (size_t layer = 0; layer < config.block_count; ++layer) {
            std::vector<float> next;
            if (!waqti::transformer::forward_layer_cached(file, metadata, config, layer, hidden, position, cache, next, nullptr, error)) return false;
            hidden.swap(next);
        }
        std::vector<float> final_hidden, logits;
        if (!waqti::qwen::final_norm_and_logits(hidden, norm, output_weight, static_cast<float>(config.rms_epsilon), final_hidden, logits, error)) return false;
        result.hidden.push_back(std::move(hidden)); result.final_hidden.push_back(std::move(final_hidden)); result.logits.push_back(std::move(logits));
    }
    return true;
}

}  // namespace waqti::sequence
