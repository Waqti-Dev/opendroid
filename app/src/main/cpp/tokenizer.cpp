// SPDX-License-Identifier: Apache-2.0
#include "tokenizer.h"

#include <algorithm>
#include <limits>

namespace waqti::tokenizer {
namespace {
std::string pair_key(const std::string& left, const std::string& right) {
    std::string key;
    key.reserve(left.size() + right.size() + 1);
    key.append(left).push_back('\0');
    key.append(right);
    return key;
}
}

Model model_from_gguf(const std::vector<std::string>& tokens,
                      const std::vector<std::string>& merges,
                      int32_t unknown_id) {
    Model model; model.unknown_id = unknown_id;
    for (size_t i = 0; i < tokens.size(); ++i) {
        const int32_t id = static_cast<int32_t>(i);
        model.vocab.emplace(tokens[i], id);
        model.reverse_vocab.emplace(id, tokens[i]);
    }
    for (size_t rank = 0; rank < merges.size(); ++rank) {
        const size_t separator = merges[rank].find(' ');
        if (separator == std::string::npos || separator == 0 || separator + 1 >= merges[rank].size()) continue;
        model.merge_rank.emplace(pair_key(merges[rank].substr(0, separator), merges[rank].substr(separator + 1)), static_cast<uint32_t>(rank));
    }
    return model;
}

ByteBpeTokenizer::ByteBpeTokenizer(Model model) : model_(std::move(model)) {}

std::vector<int32_t> ByteBpeTokenizer::encode(const std::string& utf8) const {
    std::vector<std::string> pieces;
    pieces.reserve(utf8.size());
    for (unsigned char byte : utf8) pieces.emplace_back(1, static_cast<char>(byte));

    while (pieces.size() > 1) {
        size_t best = pieces.size();
        uint32_t best_rank = std::numeric_limits<uint32_t>::max();
        for (size_t i = 0; i + 1 < pieces.size(); ++i) {
            auto it = model_.merge_rank.find(pair_key(pieces[i], pieces[i + 1]));
            if (it != model_.merge_rank.end() && it->second < best_rank) {
                best = i;
                best_rank = it->second;
            }
        }
        if (best == pieces.size()) break;
        pieces[best] += pieces[best + 1];
        pieces.erase(pieces.begin() + static_cast<std::ptrdiff_t>(best + 1));
    }

    std::vector<int32_t> ids;
    ids.reserve(pieces.size());
    for (const auto& piece : pieces) {
        auto it = model_.vocab.find(piece);
        ids.push_back(it == model_.vocab.end() ? model_.unknown_id : it->second);
    }
    return ids;
}

std::string ByteBpeTokenizer::decode(const std::vector<int32_t>& ids) const {
    std::string output;
    for (int32_t id : ids) {
        auto it = model_.reverse_vocab.find(id);
        if (it != model_.reverse_vocab.end()) output += it->second;
    }
    return output;
}

}  // namespace waqti::tokenizer
