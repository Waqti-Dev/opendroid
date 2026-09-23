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
}
