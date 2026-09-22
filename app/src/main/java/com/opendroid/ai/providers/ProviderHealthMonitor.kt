package com.opendroid.ai.providers

/**
 * Tracks provider reliability for automatic routing.
 * Forked by Ahmed Badr
 */

class ProviderHealthMonitor {
    private val failures = mutableMapOf<String, Int>()

    fun recordFailure(providerId: String) {
        failures[providerId] = (failures[providerId] ?: 0) + 1
    }

    fun recordSuccess(providerId: String) {
        failures[providerId] = 0
    }

    fun isHealthy(providerId: String): Boolean {
        return (failures[providerId] ?: 0) < 3
    }
}
