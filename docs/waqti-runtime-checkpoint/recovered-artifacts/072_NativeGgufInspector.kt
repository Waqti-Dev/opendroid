package com.opendroid.ai.core.runtime.jni

/**
 * JNI boundary for the limited GGUF metadata inspector.
 * This API deliberately does not load weights or generate tokens.
 */
internal object NativeGgufInspector {
    init {
        System.loadLibrary("waqti_runtime")
    }

    data class Result(
        val ok: Boolean,
        val json: String,
        val error: String? = null,
    )

    fun inspect(bytes: ByteArray): Result {
        val json = nativeInspect(bytes)
        return if (json.startsWith("{\"ok\":true")) {
            Result(ok = true, json = json)
        } else {
            Result(ok = false, json = json, error = json)
        }
    }

    private external fun nativeInspect(bytes: ByteArray): String
}
