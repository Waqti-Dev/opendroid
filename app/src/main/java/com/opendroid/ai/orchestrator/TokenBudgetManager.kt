package com.opendroid.ai.orchestrator

/**
 * Controls when cloud usage is necessary.
 * Local models should handle work first to reduce token usage.
 * Forked by Ahmed Badr
 */
class TokenBudgetManager {

    fun shouldUseCloud(
        complexity: Int,
        contextTokens: Int,
        localConfidence: Float
    ): Boolean {
        return complexity > 7 || contextTokens > 12000 || localConfidence < 0.6f
    }
}
