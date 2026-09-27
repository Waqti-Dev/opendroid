// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "attention_reference.h"
#include "gguf_file.h"
#include "qwen_reference.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace waqti::transformer {

struct LayerTensors {
    waqti::gguf::TensorView attn_norm;
    waqti::gguf::TensorView attn_q;
    waqti::gguf::TensorView attn_k;
    waqti::gguf::TensorView attn_v;
    waqti::gguf::TensorView attn_output;
    waqti::gguf::TensorView ffn_norm;
    waqti::gguf::TensorView ffn_gate;
    waqti::gguf::TensorView ffn_up;
    waqti::gguf::TensorView ffn_down;
};

struct LayerDiagnostics {
    size_t layer_index = 0;
    size_t input_size = 0;
    size_t output_size = 0;
    float input_rms = 0.0f;
    float output_rms = 0.0f;
    float minimum = 0.0f;
    float maximum = 0.0f;
    float mean = 0.0f;
    bool finite = false;
};

struct KVCache {
    size_t layer_count = 0;
    size_t kv_heads = 0;
    size_t head_dim = 0;
    std::vector<std::vector<float>> keys;
    std::vector<std::vector<float>> values;

    bool initialize(size_t layers, size_t heads, size_t dimension, std::string* error = nullptr);
    bool append(size_t layer, const std::vector<float>& key, const std::vector<float>& value,
                std::string* error = nullptr);
    size_t positions(size_t layer) const;
};

bool lookup_layer_tensors(const waqti::gguf::Metadata& metadata,
                          const waqti::gguf::MappedFile& file, size_t layer_index,
                          LayerTensors& tensors, std::string* error = nullptr);

bool forward_layer(const waqti::gguf::MappedFile& file, const waqti::gguf::Metadata& metadata,
                   const waqti::qwen::Config& config, size_t layer_index,
                   const std::vector<float>& input, size_t position,
                   std::vector<float>& output, LayerDiagnostics* diagnostics = nullptr,
                   std::string* error = nullptr);

bool forward_layer_cached(const waqti::gguf::MappedFile& file, const waqti::gguf::Metadata& metadata,
                          const waqti::qwen::Config& config, size_t layer_index,
                          const std::vector<float>& input, size_t position, KVCache& cache,
                          std::vector<float>& output, LayerDiagnostics* diagnostics = nullptr,
                          std::string* error = nullptr);

}  // namespace waqti::transformer
