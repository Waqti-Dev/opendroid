package com.opendroid.ai.core.llm

import java.io.File

/**
 * Registry for user imported local LLM artifacts.
 * Keeps model metadata separate from runtime implementation.
 */
data class LocalModelConfig(
    val id: String,
    val name: String,
    val path: String,
    val format: String = "GGUF",
    val quantization: String? = null,
    val contextSize: Int = 8192,
    val preferredForCoding: Boolean = false
)

class LocalModelRegistry {
    private val models = mutableMapOf<String, LocalModelConfig>()

    fun register(model: LocalModelConfig): Boolean {
        val file = File(model.path)
        if (!file.exists()) return false
        models[model.id] = model
        return true
    }

    fun remove(id: String) {
        models.remove(id)
    }

    fun get(id: String): LocalModelConfig? = models[id]

    fun list(): List<LocalModelConfig> = models.values.toList()

    fun selectForCoding(): LocalModelConfig? =
        models.values.firstOrNull { it.preferredForCoding }
}
