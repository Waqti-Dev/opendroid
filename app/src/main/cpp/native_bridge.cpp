// SPDX-License-Identifier: Apache-2.0
#include "gguf_reader.h"

#include <jni.h>
#include <string>
#include <vector>

namespace {
std::string json_escape(const std::string& value) {
    std::string out;
    out.reserve(value.size() + 2);
    for (char c : value) {
        if (c == '\\' || c == '"') { out.push_back('\\'); out.push_back(c); }
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else out.push_back(c);
    }
    return out;
}

jstring result_json(JNIEnv* env, const waqti::gguf::ParseResult& result) {
    std::string json;
    if (!result.ok) {
        json = "{\"ok\":false,\"error\":\"" + json_escape(result.error) + "\"}";
    } else {
        const auto& m = result.metadata;
        json = "{\"ok\":true,\"inferenceSupported\":false,\"version\":" + std::to_string(m.version) +
               ",\"tensorCount\":" + std::to_string(m.tensor_count) +
               ",\"metadataCount\":" + std::to_string(m.metadata_count) +
               ",\"architecture\":\"" + json_escape(m.architecture) + "\"" +
               ",\"contextLength\":" + std::to_string(m.context_length) +
               ",\"alignment\":" + std::to_string(m.alignment) +
               ",\"fileType\":" + std::to_string(m.file_type) +
               ",\"quantizationVersion\":" + std::to_string(m.quantization_version) + "}";
    }
    return env->NewStringUTF(json.c_str());
}
}  // namespace

extern "C" JNIEXPORT jstring JNICALL
Java_com_opendroid_ai_core_runtime_jni_NativeGgufInspector_nativeInspect(
    JNIEnv* env, jobject /* self */, jbyteArray input) {
    if (input == nullptr) {
        return env->NewStringUTF("{\"ok\":false,\"error\":\"null input\"}");
    }
    const jsize length = env->GetArrayLength(input);
    if (length < 0 || static_cast<size_t>(length) > 64ULL * 1024ULL * 1024ULL) {
        return env->NewStringUTF("{\"ok\":false,\"error\":\"input exceeds safety limit\"}");
    }
    std::vector<uint8_t> bytes(static_cast<size_t>(length));
    if (length > 0) {
        env->GetByteArrayRegion(input, 0, length, reinterpret_cast<jbyte*>(bytes.data()));
        if (env->ExceptionCheck()) return env->NewStringUTF("{\"ok\":false,\"error\":\"JNI read failed\"}");
    }
    try {
        return result_json(env, waqti::gguf::parse_metadata(bytes));
    } catch (...) {
        return env->NewStringUTF("{\"ok\":false,\"error\":\"native parser failure\"}");
    }
}
