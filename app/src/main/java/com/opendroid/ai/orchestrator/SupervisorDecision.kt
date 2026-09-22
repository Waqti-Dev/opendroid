package com.opendroid.ai.orchestrator

/**
 * Decides whether local execution is enough or cloud review is needed.
 */
class SupervisorDecision {

    fun shouldEscalate(
        complexity: Int,
        localConfidence: Float,
        hasInternet: Boolean
    ): Boolean {
        if (!hasInternet) return false
        return complexity >= 8 || localConfidence < 0.65f
    }
}
