package com.opendroid.ai.memory

/**
 * Scores stored experiences before they are reused by Waqti Agent.
 * Higher confidence memories are preferred during retrieval.
 * Forked by Ahmed Badr
 */
class MemoryScoring {

    fun score(
        successCount: Int,
        failureCount: Int,
        lastUsedTimestamp: Long
    ): Double {
        val total = successCount + failureCount
        if (total == 0) return 0.0

        val successRate = successCount.toDouble() / total.toDouble()
        val failurePenalty = failureCount * 0.05

        return (successRate - failurePenalty).coerceIn(0.0, 1.0)
    }
}
