package com.opendroid.ai.providers

/**
 * Secure provider configuration foundation for Waqti Agent.
 *
 * API keys are never hardcoded in the app source.
 * They are loaded from encrypted local storage or user-provided secrets.
 *
 * Forked by Ahmed Badr
 */

enum class ProviderType {
    OPENAI,
    GEMINI,
    CLAUDE,
    DEEPSEEK,
    GROQ,
    OPENROUTER
}

 data class ProviderConfig(
    val provider: ProviderType,
    val enabled: Boolean = false,
    val baseUrl: String? = null,
    val apiKeyAlias: String? = null
)

class ProviderConfigManager {

    private val providers = mutableMapOf<ProviderType, ProviderConfig>()

    fun register(config: ProviderConfig) {
        providers[config.provider] = config
    }

    fun get(provider: ProviderType): ProviderConfig? {
        return providers[provider]
    }

    fun enabledProviders(): List<ProviderConfig> {
        return providers.values.filter { it.enabled }
    }
}
