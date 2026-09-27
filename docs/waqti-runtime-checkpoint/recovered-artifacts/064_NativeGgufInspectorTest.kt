package com.opendroid.ai.core.runtime.jni

import org.junit.Assert.assertTrue
import org.junit.Test

class NativeGgufInspectorTest {
    @Test
    fun rejectsInvalidMagicThroughJni() {
        val result = NativeGgufInspector.inspect(byteArrayOf(0, 0, 0, 0))
        assertTrue(result.json.contains("\"ok\":false"))
    }

    @Test
    fun parsesMinimalMetadataThroughJni() {
        val bytes = FixtureBuilder.validQwen2Header()
        val result = NativeGgufInspector.inspect(bytes)
        assertTrue(result.ok)
        assertTrue(result.json.contains("\"architecture\":\"qwen2\""))
        assertTrue(result.json.contains("\"inferenceSupported\":false"))
    }

    private object FixtureBuilder {
        fun validQwen2Header(): ByteArray {
            val out = ArrayList<Byte>()
            u32(out, 0x46554747L); u32(out, 3); u64(out, 0); u64(out, 1)
            string(out, "general.architecture"); u32(out, 8); string(out, "qwen2")
            return out.toByteArray()
        }

        private fun u32(out: MutableList<Byte>, value: Long) {
            repeat(4) { out += ((value shr (8 * it)) and 0xff).toByte() }
        }

        private fun u64(out: MutableList<Byte>, value: Long) {
            repeat(8) { out += ((value ushr (8 * it)) and 0xff).toByte() }
        }

        private fun string(out: MutableList<Byte>, value: String) {
            u64(out, value.toByteArray(Charsets.UTF_8).size.toLong())
            value.toByteArray(Charsets.UTF_8).forEach(out::add)
        }
    }
}
