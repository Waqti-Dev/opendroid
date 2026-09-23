package com.opendroid.ai.core.providers

class SmartProviderRouter(private val manager: ProviderManager) {
    suspend fun route(): Provider? = manager.selectAvailable()
}
