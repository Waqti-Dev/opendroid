// SPDX-License-Identifier: Apache-2.0
#include "gguf_reader.h"

#include <cstring>
#include <limits>

namespace waqti::gguf {
namespace {

constexpr uint32_t kMagic = 0x46554747U;  // "GGUF" as little-endian bytes
constexpr uint32_t kVersion = 3;
constexpr uint64_t kMaxString = 1ULL << 20;
constexpr uint64_t kMaxArray = 1ULL << 24;

enum class Type : uint32_t { U8 = 0, I8, U16, I16, U32, I32, F32, Bool, String, Array, U64, I64, F64 };

class Reader {
 public:
  explicit Reader(const std::vector<uint8_t>& b, uint64_t limit) : b_(b), limit_(limit) {}
  size_t pos() const { return p_; }
  bool bytes(size_t n) const { return n <= b_.size() - p_ && p_ + n <= limit_; }
  bool u8(uint8_t& v) { if (!bytes(1)) return false; v = b_[p_++]; return true; }
  bool u16(uint16_t& v) { return integral(v, 2); }
  bool u32(uint32_t& v) { return integral(v, 4); }
  bool u64(uint64_t& v) { return integral(v, 8); }
  bool i8(int8_t& v) { uint8_t x; if (!u8(x)) return false; v = static_cast<int8_t>(x); return true; }
  bool i16(int16_t& v) { uint16_t x; if (!u16(x)) return false; v = static_cast<int16_t>(x); return true; }
  bool i32(int32_t& v) { uint32_t x; if (!u32(x)) return false; v = static_cast<int32_t>(x); return true; }
  bool i64(int64_t& v) { uint64_t x; if (!u64(x)) return false; v = static_cast<int64_t>(x); return true; }
  bool f32(float& v) { uint32_t x; if (!u32(x)) return false; std::memcpy(&v, &x, 4); return true; }
  bool f64(double& v) { uint64_t x; if (!u64(x)) return false; std::memcpy(&v, &x, 8); return true; }
  bool align(uint64_t alignment) {
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) return false;
    const uint64_t remainder = p_ % alignment;
    const uint64_t padding = remainder == 0 ? 0 : alignment - remainder;
    if (padding > std::numeric_limits<size_t>::max() - p_ || !bytes(static_cast<size_t>(padding))) return false;
    p_ += static_cast<size_t>(padding); return true;
  }
  bool string(std::string& out) { uint64_t n; if (!u64(n) || n > kMaxString || n > b_.size() - p_ || !bytes(static_cast<size_t>(n))) return false; out.assign(reinterpret_cast<const char*>(b_.data() + p_), static_cast<size_t>(n)); p_ += static_cast<size_t>(n); return true; }
  bool string_array(std::vector<std::string>& output) {
    uint32_t element_type; uint64_t count;
    if (!u32(element_type) || element_type != static_cast<uint32_t>(Type::String) || !u64(count) || count > kMaxArray) return false;
    output.clear(); output.reserve(static_cast<size_t>(count));
    for (uint64_t i = 0; i < count; ++i) { std::string value; if (!string(value)) return false; output.push_back(std::move(value)); }
    return true;
  }
  bool skip(Type t, unsigned depth = 0) {
    if (depth > 32) return false;
    switch (t) {
      case Type::U8: uint8_t u8; return this->u8(u8);
      case Type::I8: int8_t i8; return this->i8(i8);
      case Type::U16: uint16_t u16; return this->u16(u16);
      case Type::I16: int16_t i16; return this->i16(i16);
      case Type::U32: uint32_t u32; return this->u32(u32);
      case Type::I32: int32_t i32; return this->i32(i32);
      case Type::F32: float f32; return this->f32(f32);
      case Type::Bool: { uint8_t x; return this->u8(x) && (x == 0 || x == 1); }
      case Type::String: { std::string s; return string(s); }
      case Type::U64: uint64_t u64; return this->u64(u64);
      case Type::I64: int64_t i64; return this->i64(i64);
      case Type::F64: double f64; return this->f64(f64);
      case Type::Array: {
        uint32_t element_type; uint64_t count;
        if (!this->u32(element_type) || element_type > static_cast<uint32_t>(Type::F64) || !this->u64(count) || count > kMaxArray) return false;
        for (uint64_t i = 0; i < count; ++i) if (!skip(static_cast<Type>(element_type), depth + 1)) return false;
        return true;
      }
    }
    return false;
  }
 private:
  template <typename T> bool integral(T& out, size_t width) {
    if (!bytes(width)) return false;
    uint64_t x = 0; for (size_t i = 0; i < width; ++i) x |= static_cast<uint64_t>(b_[p_++]) << (8 * i);
    out = static_cast<T>(x); return true;
  }
  const std::vector<uint8_t>& b_; uint64_t limit_; size_t p_ = 0;
};

bool type_u64(Reader& r, Type t, uint64_t& out) {
  if (t == Type::U32) { uint32_t x; if (!r.u32(x)) return false; out = x; return true; }
  if (t == Type::U64) return r.u64(out);
  return false;
}

bool type_double(Reader& r, Type t, double& out) {
  if (t == Type::F32) { float x; if (!r.f32(x)) return false; out = x; return true; }
  if (t == Type::F64) return r.f64(out);
  return false;
}

}  // namespace

bool tensor_byte_size(const TensorInfo& tensor, uint64_t& bytes, std::string* error) {
  uint64_t elements = 1;
  for (uint64_t dimension : tensor.dimensions) {
    if (dimension == 0 || elements > std::numeric_limits<uint64_t>::max() / dimension) { if (error) *error = "tensor element count overflow"; return false; }
    elements *= dimension;
  }
  uint64_t block = 1; uint64_t block_bytes = 0;
  switch (tensor.type) {
    case 0: block_bytes = 4; break;       // F32
    case 1: block_bytes = 2; break;       // F16
    case 2: block = 32; block_bytes = 18; break; // Q4_0
    case 3: block = 32; block_bytes = 20; break; // Q4_1
    case 6: block = 32; block_bytes = 22; break; // Q5_0
    case 7: block = 32; block_bytes = 24; break; // Q5_1
    case 8: block = 32; block_bytes = 34; break; // Q8_0
    case 9: block = 32; block_bytes = 36; break; // Q8_1
    case 10: block = 256; block_bytes = 84; break; // Q2_K
    case 11: block = 256; block_bytes = 110; break; // Q3_K
    case 12: block = 256; block_bytes = 144; break; // Q4_K
    case 13: block = 256; block_bytes = 176; break; // Q5_K
    case 14: block = 256; block_bytes = 210; break; // Q6_K
    case 15: block = 256; block_bytes = 292; break; // Q8_K
    default: if (error) *error = "unsupported GGUF tensor type"; return false;
  }
  if (elements % block != 0 || elements / block > std::numeric_limits<uint64_t>::max() / block_bytes) { if (error) *error = "tensor dimensions are incompatible with block type"; return false; }
  bytes = (elements / block) * block_bytes; return true;
}

const TensorInfo* find_tensor(const Metadata& metadata, std::string_view name) {
  for (const TensorInfo& tensor : metadata.tensors) {
    if (tensor.name == name) return &tensor;
  }
  return nullptr;
}

ParseResult parse_metadata(const std::vector<uint8_t>& bytes, uint64_t max_metadata_bytes) {
  ParseResult result;
  if (bytes.size() < 24) { result.error = "GGUF header is truncated"; return result; }
  const uint64_t limit = std::min<uint64_t>(bytes.size(), max_metadata_bytes);
  Reader r(bytes, limit);
  uint32_t magic, version; uint64_t tensors, metadata;
  if (!r.u32(magic) || magic != kMagic) { result.error = "invalid GGUF magic"; return result; }
  if (!r.u32(version) || version != kVersion || !r.u64(tensors) || !r.u64(metadata)) { result.error = "unsupported or truncated GGUF header"; return result; }
  if (metadata > kMaxArray) { result.error = "metadata count exceeds safety limit"; return result; }
  result.metadata.version = version; result.metadata.tensor_count = tensors; result.metadata.metadata_count = metadata;
  for (uint64_t i = 0; i < metadata; ++i) {
    std::string key; uint32_t raw_type;
    if (!r.string(key) || !r.u32(raw_type) || raw_type > static_cast<uint32_t>(Type::F64)) { result.error = "invalid metadata entry"; return result; }
    Type type = static_cast<Type>(raw_type); uint64_t number;
    if (key == "general.architecture" && type == Type::String) { if (!r.string(result.metadata.architecture)) { result.error = "invalid architecture"; return result; } }
    else if (key == "general.alignment" && type_u64(r, type, number)) result.metadata.alignment = static_cast<uint32_t>(number);
    else if (key == "general.file_type" && type_u64(r, type, number)) result.metadata.file_type = static_cast<uint32_t>(number);
    else if (key == "general.quantization_version" && type_u64(r, type, number)) result.metadata.quantization_version = static_cast<uint32_t>(number);
    else if (result.metadata.architecture.size() && key == result.metadata.architecture + ".context_length" && type_u64(r, type, number)) result.metadata.context_length = number;
    else if (result.metadata.architecture.size() && key == result.metadata.architecture + ".embedding_length" && type_u64(r, type, number)) result.metadata.embedding_length = number;
    else if (result.metadata.architecture.size() && key == result.metadata.architecture + ".block_count" && type_u64(r, type, number)) result.metadata.block_count = number;
    else if (result.metadata.architecture.size() && key == result.metadata.architecture + ".attention.head_count" && type_u64(r, type, number)) result.metadata.attention_head_count = number;
    else if (result.metadata.architecture.size() && key == result.metadata.architecture + ".attention.head_count_kv" && type_u64(r, type, number)) result.metadata.attention_head_count_kv = number;
    else if (result.metadata.architecture.size() && key == result.metadata.architecture + ".feed_forward_length" && type_u64(r, type, number)) result.metadata.feed_forward_length = number;
    else if (result.metadata.architecture.size() && key == result.metadata.architecture + ".rope.freq_base" && type_double(r, type, result.metadata.rope_freq_base)) {}
    else if (result.metadata.architecture.size() && key == result.metadata.architecture + ".attention.layer_norm_rms_epsilon" && type_double(r, type, result.metadata.rms_norm_epsilon)) {}
    else if (key == "tokenizer.ggml.tokens" && type == Type::Array) { if (!r.string_array(result.metadata.tokenizer_tokens)) { result.error = "invalid tokenizer tokens"; return result; } }
    else if (key == "tokenizer.ggml.merges" && type == Type::Array) { if (!r.string_array(result.metadata.tokenizer_merges)) { result.error = "invalid tokenizer merges"; return result; } }
    else if (!r.skip(type)) { result.error = "invalid metadata value"; return result; }
  }
  if (result.metadata.architecture.empty()) { result.error = "missing general.architecture"; return result; }
  if (tensors > (1ULL << 20)) { result.error = "tensor count exceeds safety limit"; return result; }
  result.metadata.tensors.reserve(static_cast<size_t>(tensors));
  for (uint64_t i = 0; i < tensors; ++i) {
    TensorInfo info; uint32_t dimensions;
    if (!r.string(info.name) || info.name.empty() || !r.u32(dimensions) || dimensions > 8) {
      result.error = "invalid tensor descriptor"; return result;
    }
    info.dimensions.resize(dimensions);
    for (uint64_t& dimension : info.dimensions) {
      if (!r.u64(dimension) || dimension == 0) { result.error = "invalid tensor dimensions"; return result; }
    }
    if (!r.u32(info.type) || info.type > 39 || !r.u64(info.offset)) {
      result.error = "invalid tensor type or offset"; return result;
    }
    result.metadata.tensors.push_back(std::move(info));
  }
  const uint64_t alignment = result.metadata.alignment == 0 ? 32 : result.metadata.alignment;
  if (!r.align(alignment) || r.pos() > bytes.size()) { result.error = "invalid tensor data alignment"; return result; }
  result.metadata.tensor_data_offset = r.pos();
  for (const TensorInfo& tensor : result.metadata.tensors) {
    if (tensor.offset > bytes.size() - result.metadata.tensor_data_offset) {
      result.error = "tensor offset exceeds file bounds"; return result;
    }
    uint64_t payload_size;
    if (!tensor_byte_size(tensor, payload_size) || payload_size > bytes.size() - result.metadata.tensor_data_offset - tensor.offset) {
      result.error = "tensor payload exceeds file bounds or has unsupported shape"; return result;
    }
  }
  result.ok = true; return result;
}

}  // namespace waqti::gguf
