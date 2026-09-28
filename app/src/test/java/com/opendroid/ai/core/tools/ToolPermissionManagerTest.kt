package com.opendroid.ai.core.tools

import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class ToolPermissionManagerTest {

    @Test
    fun allowsReadOnlyProjectCommands() {
        assertTrue(ToolPermissionManager.isSafeCommand("pwd"))
        assertTrue(ToolPermissionManager.isSafeCommand("ls -la"))
        assertTrue(ToolPermissionManager.isSafeCommand("./gradlew test"))
        assertTrue(ToolPermissionManager.isSafeCommand("git status"))
    }

    @Test
    fun rejectsShellCompositionAndDestructiveCommands() {
        assertFalse(ToolPermissionManager.isSafeCommand("ls && pwd"))
        assertFalse(ToolPermissionManager.isSafeCommand("git push"))
        assertFalse(ToolPermissionManager.isSafeCommand("rm file.txt"))
        assertFalse(ToolPermissionManager.isSafeCommand("sudo ls"))
    }

    @Test
    fun rejectsCommandExecutionEscapeHatches() {
        assertFalse(ToolPermissionManager.isSafeCommand("python -c \"print(1)\""))
        assertFalse(ToolPermissionManager.isSafeCommand("node -e \"console.log(1)\""))
        assertFalse(ToolPermissionManager.isSafeCommand("git -C /tmp status"))
        assertFalse(ToolPermissionManager.isSafeCommand("cat ../secret.txt"))
        assertFalse(ToolPermissionManager.isSafeCommand("cat /data/local/tmp/secret.txt"))
    }
}
