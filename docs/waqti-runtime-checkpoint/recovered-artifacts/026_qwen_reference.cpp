// SPDX-License-Identifier: Apache-2.0
#include "qwen_reference.h"
#include "tensor_decode.h"

#include <cmath>
#include <limits>

namespace waqti::qwen {

bool config_from_metadata(const waqti::gguf::Metadata& metadata, Config& config, std::string* error) {
    if (metadata.architecture != "qwen2") { if (error) *error = "unsupported architecture: " + metadata.architecture; return false; }
    config.vocab_size = metadata.tokenizer_tokens.size();
    config.embedding_length = metadata.embedding_length;
    config.block_count = metadata.block_count;
    config.attention_heads = metadata.attention_head_count;
    config.kv_heads = metadata.attention_head_count_kv;
    config.feed_forward_length = metadata.feed_forward_length;
    config.context_length = metadata.context_length;
    config.rope_freq_base = metadata.rope_freq_base;
    config.rms_epsilon = metadata.rms_norm_epsilon;
    if (config.attention_heads == 0 || config.embedding_length == 0 || config.block_count == 0 ||
        config.vocab_size == 0 || config.kv_heads == 0 || config.embedding_length % config.attention_heads != 0 ||
        config.attention_heads % config.kv_heads != 0) {
        if (error) *error = "inconsistent or incomplete Qwen configuration";
        return false;
    }
    config.head_dimension = config.embedding_length / config.attention_heads;
    if (config.rope_freq_base <= 0.0 || config.rms_epsilon <= 0.0) { if (error) *error = "invalid Qwen RoPE or RMS epsilon"; return false; }
    return true;
}

bool embedding_row(const waqti::gguf::TensorView& embedding, uint32_t token_id,
                   std::vector<float>& output, std::string* error) {
    if (embedding.info == nullptr || embedding.data == nullptr || embedding.info->dimensions.size() != 2) { if (error) *error = "invalid embedding tensor view"; return false; }
    const uint64_t embedding_dim = embedding.info->dimensions[0];
    const uint64_t vocab = embedding.info->dimensions[1];
    if (token_id >= vocab || embedding_dim == 0 || embedding_dim > std::numeric_limits<size_t>::max()) { if (error) *error = "embedding token or dimensions out of range"; return false; }
    waqti::gguf::TensorInfo row = *embedding.info;
    row.dimensions = {embedding_dim};
    uint64_t row_bytes;
    if (!waqti::gguf::tensor_byte_size(row, row_bytes) || row_bytes > embedding.bytes || static_cast<uint64_t>(token_id) > (embedding.bytes - row_bytes) / row_bytes) { if (error) *error = "embedding row exceeds tensor payload"; return false; }
    const uint8_t* row_data = embedding.data + static_cast<uint64_t>(token_id) * row_bytes;
    return waqti::tensor::decode_to_f32(row, row_data, static_cast<size_t>(row_bytes), output, error);
}

bool rms_norm_reference(const std::vector<float>& input, const waqti::gguf::TensorView& weight,
                        float epsilon, std::vector<float>& output, std::string* error) {
    if (input.empty() || epsilon <= 0.0f || weight.info == nullptr || weight.info->dimensions.size() != 1 || weight.info->dimensions[0] != input.size()) { if (error) *error = "RMSNorm dimensions or epsilon invalid"; return false; }
    if (!waqti::tensor::decode_to_f32(*weight.info, weight.data, static_cast<size_t>(weight.bytes), output, error)) return false;
    if (output.size() != input.size()) return false;
    float mean_square = 0.0f;
    for (float value : input) mean_square += value * value;
    mean_square /= static_cast<float>(input.size());
    const float inv_rms = 1.0f / std::sqrt(mean_square + epsilon);
    for (size_t i = 0; i < input.size(); ++i) output[i] = input[i] * inv_rms * output[i];
    return true;
}

bool matvec_reference(const waqti::gguf::TensorView& matrix, const std::vector<float>& input,
                      std::vector<float>& output, std::string* error) {
    if (matrix.info == nullptr || matrix.data == nullptr || matrix.info->dimensions.size() != 2 ||
        matrix.info->dimensions[0] != input.size() || matrix.info->dimensions[1] == 0) {
        if (error) *error = "matrix-vector dimensions are invalid";
        return false;
    }
    const uint64_t input_dim = matrix.info->dimensions[0];
    const uint64_t output_dim = matrix.info->dimensions[1];
    waqti::gguf::TensorInfo row = *matrix.info; row.dimensions = {input_dim};
    uint64_t row_bytes;
    if (!waqti::gguf::tensor_byte_size(row, row_bytes) || row_bytes > matrix.bytes || output_dim > (matrix.bytes / row_bytes)) {
        if (error) *error = "matrix rows exceed tensor payload";
        return false;
    }
    output.assign(static_cast<size_t>(output_dim), 0.0f);
    std::vector<float> decoded;
    for (uint64_t out = 0; out < output_dim; ++out) {
        if (!waqti::tensor::decode_to_f32(row, matrix.data + out * row_bytes, static_cast<size_t>(row_bytes), decoded, error)) return false;
        float sum = 0.0f;
        for (size_t i = 0; i < input.size(); ++i) sum += input[i] * decoded[i];
        output[static_cast<size_t>(out)] = sum;
    }
    return true;
}

bool silu(const std::vector<float>& input, std::vector<float>& output, std::string* error) {
    output.resize(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        const float x = input[i];
        if (!std::isfinite(x)) { if (error) *error = "non-finite SiLU input"; return false; }
        output[i] = x >= 0.0f ? x / (1.0f + std::exp(-x)) : x * (std::exp(x) / (1.0f + std::exp(x)));
    }
    return true;
}

bool swiglu(const std::vector<float>& gate, const std::vector<float>& up,
            std::vector<float>& output, std::string* error) {
    if (gate.empty() || gate.size() != up.size()) { if (error) *error = "SwiGLU dimensions do not match"; return false; }
    std::vector<float> activated;
    if (!silu(gate, activated, error)) return false;
    output.resize(gate.size());
    for (size_t i = 0; i < output.size(); ++i) output[i] = activated[i] * up[i];
    return true;
}

bool argmax(const std::vector<float>& values, size_t& index, float& value, std::string* error) {
    if (values.empty()) { if (error) *error = "cannot argmax an empty vector"; return false; }
    index = 0; value = values[0];
    if (!std::isfinite(value)) { if (error) *error = "non-finite logit"; return false; }
    for (size_t i = 1; i < values.size(); ++i) {
        if (!std::isfinite(values[i])) { if (error) *error = "non-finite logit"; return false; }
        if (values[i] > value) { value = values[i]; index = i; }
    }
    return true;
}

bool final_norm_and_logits(const std::vector<float>& second_residual,
                           const waqti::gguf::TensorView& norm,
                           const waqti::gguf::TensorView& output_weight,
                           float epsilon, std::vector<float>& final_hidden,
                           std::vector<float>& logits, std::string* error) {
    if (second_residual.size() != 896 || norm.info == nullptr || output_weight.info == nullptr ||
        output_weight.info->dimensions.size() != 2 || output_weight.info->dimensions[0] != 896) {
        if (error) *error = "invalid final norm or LM head dimensions";
        return false;
    }
    if (!rms_norm_reference(second_residual, norm, epsilon, final_hidden, error)) return false;
    return matvec_reference(output_weight, final_hidden, logits, error);
}

}  // namespace waqti::qwen
