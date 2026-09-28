// SPDX-License-Identifier: Apache-2.0
#include "tokenizer.h"

#include <algorithm>
#include <limits>
#include <string_view>
#include <vector>

namespace waqti::tokenizer {
namespace {

bool is_direct_byte(unsigned char byte) {
    return (byte >= 33 && byte <= 126) ||
           (byte >= 161 && byte <= 172) ||
           byte >= 174;
}

uint32_t byte_to_codepoint(unsigned char byte) {
    if (is_direct_byte(byte)) return byte;
    uint32_t codepoint = 256;
    for (unsigned int candidate = 0; candidate < byte; ++candidate) {
        if (!is_direct_byte(static_cast<unsigned char>(candidate))) ++codepoint;
    }
    return codepoint;
}

bool codepoint_to_byte(uint32_t codepoint, unsigned char& byte) {
    for (unsigned int candidate = 0; candidate <= 255; ++candidate) {
        if (byte_to_codepoint(static_cast<unsigned char>(candidate)) == codepoint) {
            byte = static_cast<unsigned char>(candidate);
            return true;
        }
    }
    return false;
}

void append_utf8(uint32_t codepoint, std::string& output) {
    if (codepoint <= 0x7F) {
        output.push_back(static_cast<char>(codepoint));
    } else if (codepoint <= 0x7FF) {
        output.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
        output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else if (codepoint <= 0xFFFF) {
        output.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
        output.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else {
        output.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
        output.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
}

bool next_codepoint(std::string_view input, size_t& offset, uint32_t& codepoint) {
    if (offset >= input.size()) return false;
    const unsigned char first = static_cast<unsigned char>(input[offset++]);
    if (first < 0x80) {
        codepoint = first;
        return true;
    }
    size_t continuation = 0;
    uint32_t value = 0;
    if ((first & 0xE0) == 0xC0) { continuation = 1; value = first & 0x1F; }
    else if ((first & 0xF0) == 0xE0) { continuation = 2; value = first & 0x0F; }
    else if ((first & 0xF8) == 0xF0) { continuation = 3; value = first & 0x07; }
    else return false;
    if (offset + continuation > input.size()) return false;
    for (size_t i = 0; i < continuation; ++i) {
        const unsigned char part = static_cast<unsigned char>(input[offset++]);
        if ((part & 0xC0) != 0x80) return false;
        value = (value << 6) | (part & 0x3F);
    }
    codepoint = value;
    return true;
}

std::string bytes_to_unicode(const std::string& utf8) {
    std::string mapped;
    mapped.reserve(utf8.size());
    for (unsigned char byte : utf8) append_utf8(byte_to_codepoint(byte), mapped);
    return mapped;
}

std::string unicode_to_bytes(const std::string& mapped) {
    std::string utf8;
    size_t offset = 0;
    uint32_t codepoint = 0;
    while (next_codepoint(mapped, offset, codepoint)) {
        unsigned char byte = 0;
        if (codepoint_to_byte(codepoint, byte)) utf8.push_back(static_cast<char>(byte));
        else append_utf8(codepoint, utf8);
    }
    return utf8;
}

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
    const std::string mapped = bytes_to_unicode(utf8);
    pieces.reserve(mapped.size());
    size_t offset = 0;
    uint32_t codepoint = 0;
    while (next_codepoint(mapped, offset, codepoint)) {
        std::string piece;
        append_utf8(codepoint, piece);
        pieces.push_back(std::move(piece));
    }

    // Qwen's pre-tokenizer keeps all but the final space of a repeated run
    // separate from the following word (e.g. "  punctuation" becomes " " +
    // " punctuation"). Preserve that boundary before applying BPE merges.
    std::vector<std::vector<std::string>> chunks;
    std::vector<std::string> chunk;
    for (size_t i = 0; i < pieces.size(); ++i) {
        chunk.push_back(pieces[i]);
        if (pieces[i] == u8"Ġ" && i + 2 < pieces.size() &&
            pieces[i + 1] == u8"Ġ" && pieces[i + 2] != u8"Ġ") {
            chunks.push_back(std::move(chunk));
            chunk.clear();
        }
    }
    if (!chunk.empty() || pieces.empty()) chunks.push_back(std::move(chunk));

    std::vector<int32_t> ids;
    ids.reserve(pieces.size());
    for (auto& current : chunks) {
        while (current.size() > 1) {
            size_t best = current.size();
            uint32_t best_rank = std::numeric_limits<uint32_t>::max();
            for (size_t i = 0; i + 1 < current.size(); ++i) {
                auto it = model_.merge_rank.find(pair_key(current[i], current[i + 1]));
                if (it != model_.merge_rank.end() && it->second < best_rank) {
                    best = i;
                    best_rank = it->second;
                }
            }
            if (best == current.size()) break;
            current[best] += current[best + 1];
            current.erase(current.begin() + static_cast<std::ptrdiff_t>(best + 1));
        }
        for (const auto& piece : current) {
            auto it = model_.vocab.find(piece);
            ids.push_back(it == model_.vocab.end() ? model_.unknown_id : it->second);
        }
    }
    return ids;
}

std::string ByteBpeTokenizer::decode(const std::vector<int32_t>& ids) const {
    std::string mapped;
    for (int32_t id : ids) {
        const auto it = model_.reverse_vocab.find(id);
        if (it != model_.reverse_vocab.end()) mapped += it->second;
    }
    return unicode_to_bytes(mapped);
}

}  // namespace waqti::tokenizer
