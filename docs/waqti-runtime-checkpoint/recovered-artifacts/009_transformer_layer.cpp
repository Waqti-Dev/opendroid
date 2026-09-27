// SPDX-License-Identifier: Apache-2.0
#include "transformer_layer.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>

namespace waqti::transformer {
namespace {

bool view(const waqti::gguf::Metadata& metadata, const waqti::gguf::MappedFile& file,
          const std::string& name, waqti::gguf::TensorView& out, std::string* error) {
    if (!file.view(metadata, name, out, error)) {
        if (error && error->empty()) *error = "missing tensor: " + name;
        return false;
    }
    return true;
}

float rms(const std::vector<float>& values) {
    if (values.empty()) return 0.0f;
    double sum = 0.0;
    for (float value : values) sum += static_cast<double>(value) * value;
    return static_cast<float>(std::sqrt(sum / values.size()));
}

bool fill_diagnostics(size_t layer, const std::vector<float>& input,
                      const std::vector<float>& output, LayerDiagnostics* d,
                      std::string* error) {
    if (d == nullptr) return true;
    if (output.empty()) { if (error) *error = "empty transformer layer output"; return false; }
    d->layer_index = layer; d->input_size = input.size(); d->output_size = output.size();
    d->input_rms = rms(input); d->output_rms = rms(output);
    d->minimum = *std::min_element(output.begin(), output.end());
    d->maximum = *std::max_element(output.begin(), output.end());
    double sum = 0.0; d->finite = true;
    for (float value : output) { if (!std::isfinite(value)) d->finite = false; sum += value; }
    d->mean = static_cast<float>(sum / output.size());
    if (!d->finite) { if (error) *error = "non-finite transformer layer output"; return false; }
    return true;
}

}  // namespace

bool KVCache::initialize(size_t layers, size_t heads, size_t dimension, std::string* error) {
    if (layers == 0 || heads == 0 || dimension == 0) { if (error) *error = "invalid KV cache shape"; return false; }
    layer_count = layers; kv_heads = heads; head_dim = dimension;
    keys.assign(layers, {}); values.assign(layers, {}); return true;
}

bool KVCache::append(size_t layer, const std::vector<float>& key, const std::vector<float>& value,
                     std::string* error) {
    if (layer >= layer_count || key.size() != kv_heads * head_dim || value.size() != key.size()) {
        if (error) *error = "invalid KV cache append shape";
        return false;
    }
    keys[layer].insert(keys[layer].end(), key.begin(), key.end());
    values[layer].insert(values[layer].end(), value.begin(), value.end()); return true;
}

size_t KVCache::positions(size_t layer) const {
    if (layer >= layer_count || kv_heads == 0 || head_dim == 0) return 0;
    return keys[layer].size() / (kv_heads * head_dim);
}

bool lookup_layer_tensors(const waqti::gguf::Metadata& metadata,
                          const waqti::gguf::MappedFile& file, size_t layer_index,
                          LayerTensors& tensors, std::string* error) {
    if (layer_index >= metadata.block_count) { if (error) *error = "layer index outside GGUF block count"; return false; }
    char name[96];
#define LOOKUP_FIELD(field, suffix) \
    std::snprintf(name, sizeof(name), "blk.%zu.%s", layer_index, suffix); \
    if (!view(metadata, file, name, tensors.field, error)) return false
    LOOKUP_FIELD(attn_norm, "attn_norm.weight");
    LOOKUP_FIELD(attn_q, "attn_q.weight");
    LOOKUP_FIELD(attn_k, "attn_k.weight");
    LOOKUP_FIELD(attn_v, "attn_v.weight");
    LOOKUP_FIELD(attn_output, "attn_output.weight");
    LOOKUP_FIELD(ffn_norm, "ffn_norm.weight");
    LOOKUP_FIELD(ffn_gate, "ffn_gate.weight");
    LOOKUP_FIELD(ffn_up, "ffn_up.weight");
    LOOKUP_FIELD(ffn_down, "ffn_down.weight");
#undef LOOKUP_FIELD
    return true;
}

bool forward_layer_cached(const waqti::gguf::MappedFile& file, const waqti::gguf::Metadata& metadata,
                          const waqti::qwen::Config& config, size_t layer_index,
                          const std::vector<float>& input, size_t position, KVCache& cache,
                          std::vector<float>& output, LayerDiagnostics* diagnostics,
                          std::string* error) {
    if (input.size() != config.embedding_length || config.attention_heads == 0 || config.kv_heads == 0 || config.head_dimension == 0 ||
        cache.layer_count != config.block_count || cache.kv_heads != config.kv_heads || cache.head_dim != config.head_dimension ||
        cache.positions(layer_index) != position) { if (error) *error = "invalid cached layer input/config/position"; return false; }
    LayerTensors t; if (!lookup_layer_tensors(metadata, file, layer_index, t, error)) return false;
    std::vector<float> normalized, q, k, v;
    if (!waqti::qwen::rms_norm_reference(input, t.attn_norm, static_cast<float>(config.rms_epsilon), normalized, error) ||
        !waqti::qwen::matvec_reference(t.attn_q, normalized, q, error) ||
        !waqti::qwen::matvec_reference(t.attn_k, normalized, k, error) ||
        !waqti::qwen::matvec_reference(t.attn_v, normalized, v, error)) return false;
    if (!waqti::attention::apply_rope_interleaved(q, config.attention_heads, config.head_dimension, position, static_cast<float>(config.rope_freq_base), error) ||
        !waqti::attention::apply_rope_interleaved(k, config.kv_heads, config.head_dimension, position, static_cast<float>(config.rope_freq_base), error) ||
        !cache.append(layer_index, k, v, error)) return false;
    std::vector<float> attention_heads, attention_concat, projected, attention_residual;
    if (!waqti::attention::causal_attention(q, cache.keys[layer_index], cache.values[layer_index], position + 1,
                                             config.attention_heads, config.kv_heads, config.head_dimension, attention_heads, error) ||
        !waqti::attention::concatenate_heads(attention_heads, config.attention_heads, config.head_dimension, attention_concat, error) ||
        !waqti::qwen::matvec_reference(t.attn_output, attention_concat, projected, error) ||
        !waqti::attention::add_residual(input, projected, attention_residual, error)) return false;
    std::vector<float> ffn_input, gate, up, intermediate, down;
    if (!waqti::qwen::rms_norm_reference(attention_residual, t.ffn_norm, static_cast<float>(config.rms_epsilon), ffn_input, error) ||
        !waqti::qwen::matvec_reference(t.ffn_gate, ffn_input, gate, error) ||
        !waqti::qwen::matvec_reference(t.ffn_up, ffn_input, up, error) ||
        !waqti::qwen::swiglu(gate, up, intermediate, error) ||
        !waqti::qwen::matvec_reference(t.ffn_down, intermediate, down, error) ||
        !waqti::attention::add_residual(attention_residual, down, output, error)) return false;
    return fill_diagnostics(layer_index, input, output, diagnostics, error);
}

bool forward_layer(const waqti::gguf::MappedFile& file, const waqti::gguf::Metadata& metadata,
                   const waqti::qwen::Config& config, size_t layer_index,
                   const std::vector<float>& input, size_t position,
                   std::vector<float>& output, LayerDiagnostics* diagnostics,
                   std::string* error) {
    KVCache cache; if (!cache.initialize(config.block_count, config.kv_heads, config.head_dimension, error)) return false;
    return forward_layer_cached(file, metadata, config, layer_index, input, position, cache, output, diagnostics, error);
}

}  // namespace waqti::transformer
