package com.opendroid.ai.orchestrator

/**
 * Keeps task continuity when a cloud provider reaches limits or fails.
 */
class FailoverManager(
    private val providers: List<String>
) {
    fun nextProvider(current: String?): String? {
        val index = providers.indexOf(current)
        return providers.getOrNull(index + 1)
            ?: providers.firstOrNull()
    }
}
