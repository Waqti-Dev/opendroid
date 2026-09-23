package com.opendroid.ai.core.llm

/**
 * Monitors device resources before selecting or loading local models.
 * This is a lightweight policy layer; platform-specific sensors can be plugged in later.
 */
class ModelResourceMonitor {

    data class ResourceState(
        val availableRamMb: Long,
        val batteryPercent: Int,
        val charging: Boolean,
        val thermalLevel: Int
    )

    fun canRunLargeModel(state: ResourceState): Boolean {
        return state.availableRamMb >= 4096 &&
            (state.charging || state.batteryPercent > 25) &&
            state.thermalLevel < 3
    }

    fun recommendedModelTier(state: ResourceState): ModelTier {
        return when {
            canRunLargeModel(state) -> ModelTier.LARGE
            state.availableRamMb >= 2500 -> ModelTier.MEDIUM
            else -> ModelTier.SMALL
        }
    }

    enum class ModelTier {
        LARGE,
        MEDIUM,
        SMALL
    }
}
