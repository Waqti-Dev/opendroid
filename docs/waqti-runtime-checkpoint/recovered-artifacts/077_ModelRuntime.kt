package com.opendroid.ai.core.runtime

/**
 * Waqti Agent / Open Droid
 * Forked by Ahmed Badr
 *
 * Unified interface for local and cloud language model runtimes.
 */
interface ModelRuntime {

    suspend fun generate(prompt: String): String

    suspend fun stream(
        prompt: String,
        onToken: (String) -> Unit
    )

    fun isAvailable(): Boolean

    fun unload()
}
