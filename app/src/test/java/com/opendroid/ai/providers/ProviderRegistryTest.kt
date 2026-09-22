package com.opendroid.ai.providers

import org.junit.Assert.assertEquals
import org.junit.Test

class ProviderRegistryTest {
    @Test
    fun providersAreSortedByPriority() {
        val registry = ProviderRegistry()
        registry.register(ProviderInfo("cloud", "Cloud", "url", 2))
        registry.register(ProviderInfo("local", "Local", "local", 1))

        assertEquals("local", registry.getAvailable().first().id)
    }
}
