package com.opendroid.ai.core.llm

import java.io.File

/**
 * Registry for imported local model metadata.
 * LocalModelConfig is the canonical type defined by ModelImporter.
 */
class LocalModelRegistry {
    private val models = mutableMapOf<String, LocalModelConfig>()

    fun register(model: LocalModelConfig): Boolean {
        val file = File(model.path)
        if (!file.exists()) return false
        models[model.id] = model
        return true
    }

    fun remove(id: String) { models.remove(id) }
    fun get(id: String): LocalModelConfig? = models[id]
    fun list(): List<LocalModelConfig> = models.values.toList()
    fun selectForCoding(): LocalModelConfig? = models.values.firstOrNull { it.preferred }
}
