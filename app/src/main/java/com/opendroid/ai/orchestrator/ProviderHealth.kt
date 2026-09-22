package com.opendroid.ai.orchestrator

/**
 * Tracks provider availability for automatic failover.
 * Forked by Ahmed Badr
 */
data class ProviderHealth(
    val providerId: String,
    var failures: Int = 0,
    var successes: Int = 0,
    var available: Boolean = true
) {
    fun recordSuccess() {
        successes++
        failures = 0
        available = true
    }

    fun recordFailure() {
        failures++
        if (failures >= 3) {
            available = false
        }
    }
}
