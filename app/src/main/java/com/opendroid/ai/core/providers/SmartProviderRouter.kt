package com.opendroid.ai.core.providers

/**
 * Basic routing logic between local and cloud providers.
 * More scoring rules will be added with device metrics and token limits.
 */
class SmartProviderRouter(
    private val manager: ProviderManager
) {

    fun route(): Provider? {
        return manager.selectAvailable()
    }
}
