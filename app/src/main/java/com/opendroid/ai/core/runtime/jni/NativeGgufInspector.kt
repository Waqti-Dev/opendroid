package com.opendroid.ai.core.runtime.jni

import kotlinx.serialization.Serializable
import kotlinx.serialization.json.Json

@Serializable
data class NativeGgufInspection(
    val ok: Boolean,
    val inferenceSupported: Boolean = false,
    val version: Int? = null,
    val tensorCount: Long? = null,
    val metadataCount: Long? = null,
    val architecture: String? = null,
    val contextLength: Long? = null,
    val embeddingLength: Long? = null,
    val layers: Long? = null,
    val vocabSize: Long? = null,
    val error: String? = null
)

object NativeGgufInspector {
    private val json = Json { ignoreUnknownKeys = true }

    init {
        System.loadLibrary("waqti_runtime")
    }

    @JvmStatic
    fun inspectFile(path: String): NativeGgufInspection =
        json.decodeFromString(nativeInspectFile(path))

    @JvmStatic
    fun inspect(bytes: ByteArray): NativeGgufInspection =
        json.decodeFromString(nativeInspect(bytes))

    private external fun nativeInspectFile(path: String): String
    private external fun nativeInspect(input: ByteArray): String
}
