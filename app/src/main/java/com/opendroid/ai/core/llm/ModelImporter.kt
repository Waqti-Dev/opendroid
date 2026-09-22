package com.opendroid.ai.core.llm

import kotlinx.serialization.Serializable

/**
 * Local model import configuration.
 *
 * Allows users to add their own local models without changing the app code.
 * The runtime backend decides how to load the model (llama.cpp, MLC, etc.).
 */
@Serializable
data class LocalModelConfig(
    val id: String,
    val name: String,
    val path: String,
    val format: ModelFormat,
    val quantization: String? = null,
    val contextLength: Int = 4096,
    val preferred: Boolean = false
)

@Serializable
enum class ModelFormat {
    GGUF,
    MLC,
    ONNX,
    OTHER
}

interface LocalModelImporter {
    suspend fun importModel(config: LocalModelConfig): Result<LocalModelConfig>
    suspend fun validateModel(config: LocalModelConfig): Boolean
}

class DefaultLocalModelImporter : LocalModelImporter {
    override suspend fun importModel(config: LocalModelConfig): Result<LocalModelConfig> {
        return if (validateModel(config)) {
            Result.success(config)
        } else {
            Result.failure(IllegalArgumentException("Invalid local model"))
        }
    }

    override suspend fun validateModel(config: LocalModelConfig): Boolean {
        return config.id.isNotBlank() &&
            config.name.isNotBlank() &&
            config.path.isNotBlank()
    }
}
