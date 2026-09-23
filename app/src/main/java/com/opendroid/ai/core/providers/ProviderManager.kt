package com.opendroid.ai.core.providers

/**
 * Manages available model providers and selects an active provider.
 * This is the base layer for local/cloud failover.
 */
class ProviderManager {

    private val providers = mutableListOf<Provider>()

    fun register(provider: Provider) {
        providers.removeAll { it.id == provider.id }
        providers.add(provider)
    }

    fun getProviders(): List<Provider> = providers.toList()

    fun selectAvailable(): Provider? {
        return providers.firstOrNull { it.isAvailable() }
    }
}
