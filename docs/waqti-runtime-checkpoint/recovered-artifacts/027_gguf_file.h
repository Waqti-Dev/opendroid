// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "gguf_reader.h"
#include <cstdint>
#include <string>
#include <vector>

namespace waqti::gguf {

struct TensorView {
    const TensorInfo* info = nullptr;
    const uint8_t* data = nullptr;
    uint64_t bytes = 0;
};

class MappedFile {
 public:
    MappedFile() = default;
    ~MappedFile();
    MappedFile(const MappedFile&) = delete;
    MappedFile& operator=(const MappedFile&) = delete;

    bool open(const std::string& path, std::string* error = nullptr);
    void close();
    bool is_open() const { return data_ != nullptr; }
    uint64_t size() const { return size_; }
    bool tensor_bytes(const Metadata& metadata, size_t index,
                      const uint8_t*& data, uint64_t& size,
                      std::string* error = nullptr) const;
    bool view(const Metadata& metadata, std::string_view name,
              TensorView& output, std::string* error = nullptr) const;

 private:
    int fd_ = -1;
    uint8_t* data_ = nullptr;
    uint64_t size_ = 0;
};

}  // namespace waqti::gguf
