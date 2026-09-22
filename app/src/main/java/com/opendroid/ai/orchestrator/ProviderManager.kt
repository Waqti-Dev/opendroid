package com.opendroid.ai.orchestrator

/**
 * Hybrid AI provider manager for Waqti Agent.
 *
 * Local models execute whenever possible.
 * Cloud providers are available for supervision, review and escalation.
 *
 * Forked by Ahmed Badr
 */
class ProviderManager {

    private val providers = mutableListOf<AIProvider>()

    fun register(provider: AIProvider) {
        providers.removeAll { it.id == provider.id }
        providers.add(provider)
    }

    fun availableProviders(): List<AIProvider> =
        providers
            .filter { it.isAvailable() }
            .sortedByDescending { it.healthScore }

    fun selectProvider(
        requireCloud: Boolean = false,
        preferLocal: Boolean = true
    ): AIProvider? {
        val available = availableProviders()

        if (requireCloud) {
            return available.firstOrNull { it.type == ProviderType.CLOUD }
        }

        if (preferLocal) {
            return available.firstOrNull { it.type == ProviderType.LOCAL }
                ?: available.firstOrNull()
        }

        return available.firstOrNull()
    }

    fun fallback(current: AIProvider): AIProvider? {
        return availableProviders()
            .firstOrNull { it.id != current.id }
    }
}

interface AIProvider {
    val id: String
    val type: ProviderType
    val healthScore: Int

    fun isAvailable(): Boolean
}

enum class ProviderType {
    LOCAL,
    CLOUD
}
