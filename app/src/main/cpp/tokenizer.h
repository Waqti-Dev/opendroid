// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace waqti::tokenizer {

struct Model {
    std::unordered_map<std::string, int32_t> vocab;
    std::unordered_map<int32_t, std::string> reverse_vocab;
    std::unordered_map<std::string, uint32_t> merge_rank;
    int32_t unknown_id = -1;
};

Model model_from_gguf(const std::vector<std::string>& tokens,
                      const std::vector<std::string>& merges,
                      int32_t unknown_id = -1);

class ByteBpeTokenizer {
 public:
    explicit ByteBpeTokenizer(Model model);
    std::vector<int32_t> encode(const std::string& utf8) const;
    std::string decode(const std::vector<int32_t>& ids) const;

 private:
    Model model_;
};

}  // namespace waqti::tokenizer
