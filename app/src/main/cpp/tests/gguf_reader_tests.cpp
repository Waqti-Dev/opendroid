// SPDX-License-Identifier: Apache-2.0
#include "../gguf_reader.h"
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

static void u32(std::vector<uint8_t>& b, uint32_t x) { for (int i=0;i<4;++i) b.push_back(static_cast<uint8_t>(x >> (8*i))); }
static void u64(std::vector<uint8_t>& b, uint64_t x) { for (int i=0;i<8;++i) b.push_back(static_cast<uint8_t>(x >> (8*i))); }
static void str(std::vector<uint8_t>& b, const std::string& s) { u64(b, s.size()); b.insert(b.end(), s.begin(), s.end()); }
static std::vector<uint8_t> valid() {
  std::vector<uint8_t> b; u32(b, 0x46554747U); u32(b, 3); u64(b, 1); u64(b, 3);
  str(b, "general.architecture"); u32(b, 8); str(b, "qwen2");
  str(b, "general.alignment"); u32(b, 4); u32(b, 32);
  str(b, "qwen2.context_length"); u32(b, 4); u32(b, 32768);
  str(b, "blk.0.weight"); u32(b, 1); u64(b, 2); u32(b, 0); u64(b, 0);
  while (b.size() % 32 != 0) b.push_back(0);
  for (int i = 0; i < 8; ++i) b.push_back(0);
  return b;
}
int main() {
  auto r = waqti::gguf::parse_metadata(valid()); assert(r.ok); assert(r.metadata.architecture == "qwen2"); assert(r.metadata.context_length == 32768); assert(r.metadata.alignment == 32); assert(r.metadata.tensor_count == 1); assert(r.metadata.tensors.size() == 1);
  auto bad = valid(); bad[0] = 0; assert(!waqti::gguf::parse_metadata(bad).ok);
  auto truncated = valid(); truncated.pop_back(); assert(!waqti::gguf::parse_metadata(truncated).ok);
  auto huge = valid(); huge[24] = 0xff; assert(!waqti::gguf::parse_metadata(huge).ok);
  auto limited = waqti::gguf::parse_metadata(valid(), 24); assert(!limited.ok);
  return 0;
}
