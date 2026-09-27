// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "gguf_file.h"
#include "qwen_reference.h"
#include <cstddef>
#include <string>
#include <vector>

namespace waqti::sequence {

struct Result {
    std::vector<std::vector<float>> hidden;
    std::vector<std::vector<float>> final_hidden;
    std::vector<std::vector<float>> logits;
};

bool forward(const waqti::gguf::MappedFile& file, const waqti::gguf::Metadata& metadata,
             const waqti::qwen::Config& config, const std::vector<uint32_t>& token_ids,
             Result& result, std::string* error = nullptr);

}  // namespace waqti::sequence
