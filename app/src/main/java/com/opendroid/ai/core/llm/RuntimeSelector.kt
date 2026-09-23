package com.opendroid.ai.core.llm

/**
 * Selects the most suitable runtime for an imported local model.
 * This keeps model management independent from the actual inference engine.
 */
class RuntimeSelector {

    fun select(model: LocalModelSpec): ModelRuntime {
        return when (model.format.lowercase()) {
            "gguf" -> ModelRuntime.LLAMA_CPP
            "tflite" -> ModelRuntime.LITERT
            else -> ModelRuntime.UNKNOWN
        }
    }
}

data class LocalModelSpec(
    val name: String,
    val format: String,
    val sizeGb: Double = 0.0,
    val contextSize: Int = 4096
)

enum class ModelRuntime {
    LLAMA_CPP,
    LITERT,
    UNKNOWN
}
