package com.opendroid.ai.core.llm

typealias OnDeviceModel = OnDeviceModelSpec

interface ModelManager {
    suspend fun download(model: OnDeviceModel)
    suspend fun delete(model: OnDeviceModel)
    suspend fun load(model: OnDeviceModel)
    suspend fun isDownloaded(model: OnDeviceModel): Boolean
    suspend fun currentModel(): OnDeviceModel?

    /**
     * Register models imported by the user.
     */
    suspend fun importModel(model: OnDeviceModel)

    /**
     * List available local models.
     */
    suspend fun availableModels(): List<OnDeviceModel>

    /**
     * Select the preferred local model for execution.
     */
    suspend fun selectModel(model: OnDeviceModel): Boolean
}
