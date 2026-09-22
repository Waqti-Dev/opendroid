package com.opendroid.ai.providers

/**
 * Central registry for AI providers.
 * Keeps provider metadata separated from execution logic.
 * Forked by Ahmed Badr
 */

data class ProviderInfo(
    val id: String,
    val name: String,
    val endpoint: String,
    val priority: Int,
    val enabled: Boolean = true
)

class ProviderRegistry {
    private val providers = mutableListOf<ProviderInfo>()

    fun register(provider: ProviderInfo) {
        providers.removeAll { it.id == provider.id }
        providers.add(provider)
    }

    fun getAvailable(): List<ProviderInfo> =
        providers.filter { it.enabled }.sortedBy { it.priority }
}
