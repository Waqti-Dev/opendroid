// SPDX-License-Identifier: Apache-2.0
#include "generation_reference.h"
#include "gguf_reader.h"
#include "qwen_reference.h"
#include "tokenizer.h"

#include <jni.h>

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <new>
#include <string>
#include <utility>
#include <vector>

namespace {
constexpr size_t kMaxMetadataBytes = 64ULL * 1024ULL * 1024ULL;
constexpr size_t kMaxPromptBytes = 4ULL * 1024ULL * 1024ULL;

std::string json_escape(const std::string& value) {
    static constexpr char hex[] = "0123456789abcdef";
    std::string out;
    out.reserve(value.size() + 2);
    for (unsigned char c : value) {
        if (c == '\\' || c == '"') {
            out.push_back('\\');
            out.push_back(static_cast<char>(c));
        } else if (c == '\n') {
            out += "\\n";
        } else if (c == '\r') {
            out += "\\r";
        } else if (c == '\t') {
            out += "\\t";
        } else if (c < 0x20U) {
            out += "\\u00";
            out.push_back(hex[(c >> 4U) & 0x0fU]);
            out.push_back(hex[c & 0x0fU]);
        } else {
            out.push_back(static_cast<char>(c));
        }
    }
    return out;
}

jstring json_string(JNIEnv* env, const std::string& value) {
    return env->NewStringUTF(value.c_str());
}

std::string jstring_utf8(JNIEnv* env, jstring value, std::string* error) {
    if (value == nullptr) {
        if (error) *error = "null string argument";
        return {};
    }
    const char* chars = env->GetStringUTFChars(value, nullptr);
    if (chars == nullptr) {
        if (error) *error = "JNI string conversion failed";
        return {};
    }
    std::string result(chars);
    env->ReleaseStringUTFChars(value, chars);
    if (result.size() > kMaxPromptBytes) {
        if (error) *error = "prompt exceeds safety limit";
        result.clear();
    }
    return result;
}

bool read_prefix(const std::string& path, std::vector<uint8_t>& bytes, std::string* error) {
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input) {
        if (error) *error = "cannot open GGUF file";
        return false;
    }
    const std::streamoff size = input.tellg();
    if (size <= 0) {
        if (error) *error = "GGUF file is empty";
        return false;
    }
    const size_t prefix_size = static_cast<size_t>(std::min<std::streamoff>(size, kMaxMetadataBytes));
    bytes.resize(prefix_size);
    input.seekg(0, std::ios::beg);
    input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    if (!input && !input.eof()) {
        if (error) *error = "failed to read GGUF metadata prefix";
        return false;
    }
    return true;
}

bool parse_file(const std::string& path, waqti::gguf::ParseResult& parsed, std::string* error) {
    std::vector<uint8_t> prefix;
    if (!read_prefix(path, prefix, error)) return false;
    parsed = waqti::gguf::parse_metadata(prefix, kMaxMetadataBytes);
    if (!parsed.ok) {
        if (error) *error = parsed.error;
        return false;
    }
    return true;
}

bool has_required_tensors(const waqti::gguf::Metadata& metadata) {
    static constexpr const char* required[] = {
        "token_embd.weight", "output_norm.weight", "output.weight"
    };
    for (const char* name : required) {
        if (waqti::gguf::find_tensor(metadata, name) == nullptr) return false;
    }
    return true;
}

std::string metadata_json(const waqti::gguf::ParseResult& result, bool inference_supported) {
    if (!result.ok) {
        return "{\"ok\":false,\"error\":\"" + json_escape(result.error) + "\"}";
    }
    const auto& m = result.metadata;
    return "{\"ok\":true,\"inferenceSupported\":" +
           std::string(inference_supported ? "true" : "false") +
           ",\"version\":" + std::to_string(m.version) +
           ",\"tensorCount\":" + std::to_string(m.tensor_count) +
           ",\"metadataCount\":" + std::to_string(m.metadata_count) +
           ",\"architecture\":\"" + json_escape(m.architecture) + "\"" +
           ",\"contextLength\":" + std::to_string(m.context_length) +
           ",\"embeddingLength\":" + std::to_string(m.embedding_length) +
           ",\"layers\":" + std::to_string(m.block_count) +
           ",\"vocabSize\":" + std::to_string(m.tokenizer_tokens.size()) +
           ",\"alignment\":" + std::to_string(m.alignment) +
           ",\"fileType\":" + std::to_string(m.file_type) +
           ",\"quantizationVersion\":" + std::to_string(m.quantization_version) + "}";
}

std::string ids_json(const std::vector<uint32_t>& ids) {
    std::string out = "[";
    for (size_t i = 0; i < ids.size(); ++i) {
        if (i != 0) out += ",";
        out += std::to_string(ids[i]);
    }
    out += "]";
    return out;
}

std::string generation_json(const std::vector<int32_t>& prompt_ids,
                            const waqti::generation::Result& result,
                            const std::string& decoded) {
    std::vector<uint32_t> prompt;
    prompt.reserve(prompt_ids.size());
    for (int32_t id : prompt_ids) prompt.push_back(static_cast<uint32_t>(id));
    return "{\"ok\":true,\"promptTokenIds\":" + ids_json(prompt) +
           ",\"generatedTokenIds\":" + ids_json(result.generated_tokens) +
           ",\"generatedText\":\"" + json_escape(decoded) + "\"" +
           ",\"generatedTokenCount\":" + std::to_string(result.generated_tokens.size()) + "}";
}

}  // namespace

extern "C" JNIEXPORT jstring JNICALL
Java_com_opendroid_ai_core_runtime_jni_NativeGgufInspector_nativeInspect(
    JNIEnv* env, jobject /* self */, jbyteArray input) {
    if (input == nullptr) return json_string(env, "{\"ok\":false,\"error\":\"null input\"}");
    const jsize length = env->GetArrayLength(input);
    if (length < 0 || static_cast<size_t>(length) > kMaxMetadataBytes) {
        return json_string(env, "{\"ok\":false,\"error\":\"input exceeds safety limit\"}");
    }
    std::vector<uint8_t> bytes(static_cast<size_t>(length));
    if (length > 0) {
        env->GetByteArrayRegion(input, 0, length, reinterpret_cast<jbyte*>(bytes.data()));
        if (env->ExceptionCheck()) return json_string(env, "{\"ok\":false,\"error\":\"JNI read failed\"}");
    }
    try {
        const auto result = waqti::gguf::parse_metadata(bytes);
        return json_string(env, metadata_json(result, false));
    } catch (...) {
        return json_string(env, "{\"ok\":false,\"error\":\"native parser failure\"}");
    }
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_opendroid_ai_core_runtime_jni_NativeGgufInspector_nativeInspectFile(
    JNIEnv* env, jobject /* self */, jstring path_value) {
    std::string error;
    const std::string path = jstring_utf8(env, path_value, &error);
    if (path.empty()) return json_string(env, "{\"ok\":false,\"error\":\"" + json_escape(error) + "\"}");
    try {
        waqti::gguf::ParseResult result;
        if (!parse_file(path, result, &error)) {
            return json_string(env, "{\"ok\":false,\"error\":\"" + json_escape(error) + "\"}");
        }
        waqti::qwen::Config config;
        const bool supported = waqti::qwen::config_from_metadata(result.metadata, config, &error) &&
                               has_required_tensors(result.metadata);
        if (!supported && error.empty()) error = "GGUF does not contain a supported Qwen2 configuration";
        if (!supported) {
            return json_string(env, "{\"ok\":false,\"error\":\"" + json_escape(error) + "\"}");
        }
        return json_string(env, metadata_json(result, true));
    } catch (...) {
        return json_string(env, "{\"ok\":false,\"error\":\"native file inspection failure\"}");
    }
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_opendroid_ai_core_runtime_jni_NativeGgufGenerator_nativeGenerate(
    JNIEnv* env, jobject /* self */, jstring path_value, jstring prompt_value, jint max_tokens) {
    std::string error;
    const std::string path = jstring_utf8(env, path_value, &error);
    if (path.empty()) return json_string(env, "{\"ok\":false,\"error\":\"" + json_escape(error) + "\"}");
    const std::string prompt = jstring_utf8(env, prompt_value, &error);
    if (prompt.empty() && !error.empty()) return json_string(env, "{\"ok\":false,\"error\":\"" + json_escape(error) + "\"}");
    if (max_tokens <= 0 || max_tokens > 2048) {
        return json_string(env, "{\"ok\":false,\"error\":\"maxTokens must be between 1 and 2048\"}");
    }
    try {
        waqti::gguf::ParseResult parsed;
        if (!parse_file(path, parsed, &error)) {
            return json_string(env, "{\"ok\":false,\"error\":\"" + json_escape(error) + "\"}");
        }
        waqti::qwen::Config config;
        if (!waqti::qwen::config_from_metadata(parsed.metadata, config, &error)) {
            return json_string(env, "{\"ok\":false,\"error\":\"" + json_escape(error) + "\"}");
        }
        waqti::tokenizer::Model model = waqti::tokenizer::model_from_gguf(
            parsed.metadata.tokenizer_tokens, parsed.metadata.tokenizer_merges);
        waqti::tokenizer::ByteBpeTokenizer tokenizer(std::move(model));
        const std::vector<int32_t> prompt_ids = tokenizer.encode(prompt);
        if (prompt_ids.empty()) {
            return json_string(env, "{\"ok\":false,\"error\":\"prompt produced no tokens\"}");
        }
        if (prompt_ids.size() >= config.context_length ||
            static_cast<size_t>(max_tokens) > config.context_length - prompt_ids.size()) {
            return json_string(env, "{\"ok\":false,\"error\":\"prompt exceeds model context\"}");
        }
        std::vector<uint32_t> prompt_tokens;
        prompt_tokens.reserve(prompt_ids.size());
        for (int32_t id : prompt_ids) {
            if (id < 0) {
                return json_string(env, "{\"ok\":false,\"error\":\"invalid token ID\"}");
            }
            prompt_tokens.push_back(static_cast<uint32_t>(id));
        }
        waqti::gguf::MappedFile file;
        if (!file.open(path, &error)) {
            return json_string(env, "{\"ok\":false,\"error\":\"" + json_escape(error) + "\"}");
        }
        waqti::generation::Result result;
        if (!waqti::generation::greedy(file, parsed.metadata, config, prompt_tokens,
                                       static_cast<size_t>(max_tokens), 151645U, result, &error)) {
            return json_string(env, "{\"ok\":false,\"error\":\"" + json_escape(error) + "\"}");
        }
        std::vector<int32_t> generated_ids;
        generated_ids.reserve(result.generated_tokens.size());
        for (uint32_t id : result.generated_tokens) generated_ids.push_back(static_cast<int32_t>(id));
        return json_string(env, generation_json(prompt_ids, result, tokenizer.decode(generated_ids)));
    } catch (const std::bad_alloc&) {
        return json_string(env, "{\"ok\":false,\"error\":\"native allocation failure\"}");
    } catch (...) {
        return json_string(env, "{\"ok\":false,\"error\":\"native generation failure\"}");
    }
}
