package com.opendroid.ai.core.models

class ModelRegistry {
    private val models = mutableListOf<ModelInfo>()

    fun register(model: ModelInfo) {
        models.removeAll { it.name == model.name }
        models.add(model)
    }

    fun getModels(): List<ModelInfo> = models.toList()

    fun find(name: String): ModelInfo? {
        return models.firstOrNull { it.name == name }
    }
}
