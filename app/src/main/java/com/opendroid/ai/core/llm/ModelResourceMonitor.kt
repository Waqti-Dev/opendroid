package com.opendroid.ai.core.llm

import java.io.File

/**
 * Monitors coarse device resources before selecting or loading local models.
 * The agent uses available system RAM as a routing guard; the inference
 * runtime still performs its own allocation checks before loading a model.
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

    /**
     * Routing guard for the model names produced by SmartModelSelector.
     * /proc/meminfo is available on Android and avoids coupling this policy
     * layer to an Activity/Context.
     */
    fun canRun(model: String): Boolean {
        val availableMb = readAvailableRamMb()
        return when {
            model.contains("15B", ignoreCase = true) -> availableMb >= 10_240
            model.contains("7B", ignoreCase = true) -> availableMb >= 6_144
            else -> availableMb >= 3_072
        }
    }

    private fun readAvailableRamMb(): Long {
        return runCatching {
            File("/proc/meminfo").useLines { lines ->
                lines.firstOrNull { it.startsWith("MemAvailable:") }
                    ?.split(Regex("\\s+"))
                    ?.getOrNull(1)
                    ?.toLongOrNull()
                    ?.div(1024)
                    ?: 0L
            }
        }.getOrDefault(0L)
    }

    enum class ModelTier {
        LARGE,
        MEDIUM,
        SMALL
    }
}
