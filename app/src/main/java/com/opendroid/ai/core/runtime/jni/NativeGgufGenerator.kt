package com.opendroid.ai.core.runtime.jni

import kotlinx.serialization.Serializable
import kotlinx.serialization.json.Json

@Serializable
data class NativeGgufGeneration(
    val ok: Boolean,
    val promptTokenIds: List<Long> = emptyList(),
    val generatedTokenIds: List<Long> = emptyList(),
    val generatedText: String = "",
    val generatedTokenCount: Int = 0,
    val error: String? = null
)

object NativeGgufGenerator {
    private val json = Json { ignoreUnknownKeys = true }

    init {
        System.loadLibrary("waqti_runtime")
    }

    @JvmStatic
    fun generate(modelPath: String, prompt: String, maxTokens: Int): NativeGgufGeneration =
        json.decodeFromString(nativeGenerate(modelPath, prompt, maxTokens))

    private external fun nativeGenerate(
        modelPath: String,
        prompt: String,
        maxTokens: Int
    ): String
}
