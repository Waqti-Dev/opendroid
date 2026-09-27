// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace waqti::gguf {

struct TensorInfo {
    std::string name;
    std::vector<uint64_t> dimensions;
    uint32_t type = 0;
    uint64_t offset = 0;
};

struct Metadata {
    uint32_t version = 0;
    uint64_t tensor_count = 0;
    uint64_t metadata_count = 0;
    std::string architecture;
    uint64_t context_length = 0;
    uint64_t embedding_length = 0;
    uint64_t block_count = 0;
    uint64_t attention_head_count = 0;
    uint64_t attention_head_count_kv = 0;
    uint64_t feed_forward_length = 0;
    double rope_freq_base = 0.0;
    double rms_norm_epsilon = 0.0;
    uint32_t alignment = 0;
    uint32_t file_type = 0;
    uint32_t quantization_version = 0;
    std::vector<std::string> tokenizer_tokens;
    std::vector<std::string> tokenizer_merges;
    uint64_t tensor_data_offset = 0;
    std::vector<TensorInfo> tensors;
};

struct ParseResult {
    bool ok = false;
    Metadata metadata;
    std::string error;
};

bool tensor_byte_size(const TensorInfo& tensor, uint64_t& bytes, std::string* error = nullptr);
const TensorInfo* find_tensor(const Metadata& metadata, std::string_view name);

// Parses GGUF v3 header and metadata only. It never reads tensor payloads.
// Limits are intentional: this is an inspector boundary, not an inference loader.
ParseResult parse_metadata(const std::vector<uint8_t>& bytes,
                           uint64_t max_metadata_bytes = 64ULL * 1024ULL * 1024ULL);

}  // namespace waqti::gguf
