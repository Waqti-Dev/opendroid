package com.opendroid.ai.core.providers

/**
 * Common contract for every model provider.
 * Local models and cloud APIs implement this interface.
 */
interface Provider {
    val id: String
    val displayName: String

    suspend fun isAvailable(): Boolean

    suspend fun generate(prompt: String): String
}
