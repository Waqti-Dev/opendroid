package com.opendroid.ai.core.tools

import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertTrue
import org.junit.Test
import java.nio.file.Files

class DefaultToolExecutorTest {
    @Test
    fun traversal_isRejected_andWorkspaceOperationsWork() = runBlocking {
        val root = Files.createTempDirectory("waqti-tools").toFile()
        try {
            val executor = DefaultToolExecutor(root)
            val escaped = executor.execute(ToolRequest.ReadFile("../outside.txt"))
            assertTrue(escaped is ExecutionResult.Failure)

            assertTrue(executor.execute(ToolRequest.CreateFile("src/Test.kt", "class Test")).isSuccess())
            assertTrue(executor.execute(ToolRequest.PatchFile("src/Test.kt", "class Test", "class Patched")).isSuccess())
            assertTrue(executor.execute(ToolRequest.SearchFiles("Patched")).isSuccess())
            assertTrue(executor.execute(ToolRequest.ListFiles(".", recursive = true)).isSuccess())
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun safeCommand_runsInsideWorkspace_andDangerousCommandsAreBlocked() = runBlocking {
        val root = Files.createTempDirectory("waqti-command").toFile()
        try {
            val executor = DefaultToolExecutor(root)
            val safe = executor.execute(ToolRequest.RunCommand("pwd"))
            assertTrue(safe is ExecutionResult.Success)
            assertTrue((safe as ExecutionResult.Success).output.contains(root.canonicalPath))

            val dangerous = executor.execute(ToolRequest.RunCommand("rm -rf ."))
            assertTrue(dangerous is ExecutionResult.Failure)
        } finally {
            root.deleteRecursively()
        }
    }

    private fun ExecutionResult.isSuccess(): Boolean = this is ExecutionResult.Success
}
