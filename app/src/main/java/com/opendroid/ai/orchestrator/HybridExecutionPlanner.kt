package com.opendroid.ai.orchestrator

/**
 * Decides execution strategy between local and cloud models.
 * Forked by Ahmed Badr
 */
class HybridExecutionPlanner {

    enum class Strategy {
        LOCAL_ONLY,
        LOCAL_WITH_CLOUD_REVIEW,
        CLOUD_ASSISTED
    }

    fun decide(
        complexity: Int,
        localConfidence: Float,
        internetAvailable: Boolean
    ): Strategy {
        if (!internetAvailable) return Strategy.LOCAL_ONLY

        return when {
            complexity >= 8 -> Strategy.CLOUD_ASSISTED
            localConfidence < 0.65f -> Strategy.LOCAL_WITH_CLOUD_REVIEW
            else -> Strategy.LOCAL_ONLY
        }
    }
}
