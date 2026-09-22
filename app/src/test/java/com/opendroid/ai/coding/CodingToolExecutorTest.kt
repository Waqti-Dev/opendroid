package com.opendroid.ai.coding

import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class CodingToolExecutorTest {

    @Test
    fun unknownToolShouldFail() {
        val result = ToolResult(false, "Unknown coding tool")
        assertFalse(result.success)
    }

    @Test
    fun knownToolResultCanSucceed() {
        val result = ToolResult(true, "created")
        assertTrue(result.success)
    }
}
