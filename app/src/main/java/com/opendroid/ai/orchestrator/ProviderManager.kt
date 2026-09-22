package com.opendroid.ai.orchestrator

/**
 * Manages available AI providers for Waqti Agent.
 *
 * Cloud providers act as supervisors while local models handle execution.
 * Forked by Ahmed Badr
 */
class ProviderManager {

    private val providers = mutableListOf<AIProvider>()

    fun register(provider: AIProvider) {
        providers.add(provider)
    }

    fun availableProviders(): List<AIProvider> =
        providers.filter { it.isAvailable() }

    fun nextProvider(failedProvider: AIProvider?): AIProvider? {
        return availableProviders()
            .firstOrNull { it != failedProvider }
    }
}

interface AIProvider {
    val name: String
    fun isAvailable(): Boolean
}
