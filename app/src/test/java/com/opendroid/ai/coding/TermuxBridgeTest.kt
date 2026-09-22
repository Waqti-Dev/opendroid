package com.opendroid.ai.coding

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class TermuxBridgeTest {

    @Test
    fun validCommand_isAccepted() {
        val bridge = TermuxBridge()
        assertTrue(bridge.validateCommand("ls"))
    }

    @Test
    fun commandIsTrimmed() {
        val bridge = TermuxBridge()
        assertEquals("git status", bridge.createPendingCommand("  git status  "))
    }
}
