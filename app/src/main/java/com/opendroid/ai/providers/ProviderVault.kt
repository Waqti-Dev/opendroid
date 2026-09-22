package com.opendroid.ai.providers

/**
 * Secure provider credential abstraction for Waqti Agent.
 *
 * API keys should never be hardcoded inside the APK.
 * This layer prepares encrypted storage and provider synchronization.
 * Forked by Ahmed Badr
 */
class ProviderVault {

    private val providers = mutableMapOf<String, String>()

    fun saveProviderKey(provider: String, encryptedKey: String) {
        providers[provider] = encryptedKey
    }

    fun getProviderKey(provider: String): String? {
        return providers[provider]
    }

    fun removeProvider(provider: String) {
        providers.remove(provider)
    }

    fun hasProvider(provider: String): Boolean {
        return providers.containsKey(provider)
    }
}
