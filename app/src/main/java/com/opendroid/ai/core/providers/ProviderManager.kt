package com.opendroid.ai.core.providers

class ProviderManager {
    private val providers = mutableListOf<Provider>()

    fun register(provider: Provider) {
        providers.removeAll { it.id == provider.id }
        providers.add(provider)
    }

    fun getProviders(): List<Provider> = providers.toList()

    suspend fun selectAvailable(): Provider? {
        for (provider in providers) {
            if (provider.isAvailable()) return provider
        }
        return null
    }

    /**
     * Executes a request with ordered provider failover.
     * A failed provider does not abort the task; the next available provider gets
     * the same prompt without restarting the surrounding agent state.
     */
    suspend fun generateWithFailover(prompt: String): ProviderGenerationResult {
        var lastError: Throwable? = null
        for (provider in providers) {
            if (!provider.isAvailable()) continue
            try {
                return ProviderGenerationResult(provider, provider.generate(prompt))
            } catch (error: Throwable) {
                lastError = error
            }
        }
        throw IllegalStateException(
            "All registered providers failed.",
            lastError
        )
    }
}

data class ProviderGenerationResult(
    val provider: Provider,
    val response: String
)
