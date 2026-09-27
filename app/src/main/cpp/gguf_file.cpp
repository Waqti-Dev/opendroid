// SPDX-License-Identifier: Apache-2.0
#include "gguf_file.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace waqti::gguf {

MappedFile::~MappedFile() { close(); }

bool MappedFile::open(const std::string& path, std::string* error) {
    close();
    fd_ = ::open(path.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd_ < 0) { if (error) *error = std::strerror(errno); return false; }
    struct stat st{};
    if (fstat(fd_, &st) != 0 || st.st_size <= 0) { if (error) *error = "cannot stat GGUF file"; close(); return false; }
    size_ = static_cast<uint64_t>(st.st_size);
    void* mapped = mmap(nullptr, static_cast<size_t>(size_), PROT_READ, MAP_PRIVATE, fd_, 0);
    if (mapped == MAP_FAILED) { if (error) *error = std::strerror(errno); close(); return false; }
    data_ = static_cast<uint8_t*>(mapped);
    return true;
}

void MappedFile::close() {
    if (data_ != nullptr) { munmap(data_, static_cast<size_t>(size_)); data_ = nullptr; }
    if (fd_ >= 0) { ::close(fd_); fd_ = -1; }
    size_ = 0;
}

bool MappedFile::tensor_bytes(const Metadata& metadata, size_t index,
                              const uint8_t*& data, uint64_t& size,
                              std::string* error) const {
    if (!is_open()) { if (error) *error = "GGUF file is not open"; return false; }
    if (index >= metadata.tensors.size()) { if (error) *error = "tensor index out of range"; return false; }
    const TensorInfo& tensor = metadata.tensors[index];
    uint64_t payload;
    if (!tensor_byte_size(tensor, payload)) { if (error) *error = "unsupported tensor shape or type"; return false; }
    if (metadata.tensor_data_offset > size_ || tensor.offset > size_ - metadata.tensor_data_offset ||
        payload > size_ - metadata.tensor_data_offset - tensor.offset) {
        if (error) *error = "tensor span exceeds mapped file";
        return false;
    }
    data = data_ + metadata.tensor_data_offset + tensor.offset;
    size = payload;
    return true;
}

bool MappedFile::view(const Metadata& metadata, std::string_view name,
                      TensorView& output, std::string* error) const {
    output = {};
    const TensorInfo* info = find_tensor(metadata, name);
    if (info == nullptr) { if (error) *error = "tensor not found: " + std::string(name); return false; }
    const size_t index = static_cast<size_t>(info - metadata.tensors.data());
    const uint8_t* data = nullptr; uint64_t bytes = 0;
    if (!tensor_bytes(metadata, index, data, bytes, error)) return false;
    output = TensorView{info, data, bytes};
    return true;
}

}  // namespace waqti::gguf
