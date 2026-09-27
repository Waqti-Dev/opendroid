package com.opendroid.ai.core.agent

import com.opendroid.ai.core.providers.Provider
import com.opendroid.ai.core.providers.ProviderManager
import com.opendroid.ai.core.tools.ExecutionResult
import com.opendroid.ai.core.tools.ToolExecutionLayer
import com.opendroid.ai.core.tools.ToolPermissionManager
import com.opendroid.ai.core.tools.ToolRequest
import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class AutonomousAgentEngineTest {

    @Test
    fun createUserFile_throughToolCall() = runBlocking {
        val provider = FakeProvider(
            response = """{"toolCall":{"name":"WriteFile","arguments":{"path":"User.kt","content":"data class User(val name: String)"}}}""",
            finalResponse = "User.kt created successfully"
        )
        val manager = ProviderManager().apply { register(provider) }
        val executor = RecordingExecutor()
        val engine = AutonomousAgentEngine(manager, executor, ToolPermissionManager())

        val result = engine.execute("Create User.kt")

        assertEquals(AgentCheckpoint.Status.COMPLETED, result.checkpoint.status)
        assertEquals("User.kt", (executor.lastRequest as ToolRequest.WriteFile).path)
    }

    @Test
    fun deniedCommand_isRecordedAsFailure() = runBlocking {
        val manager = ProviderManager().apply {
            register(FakeProvider("""{"toolCall":{"name":"RunCommand","arguments":{"command":"rm -rf ."}}}"""))
        }
        val engine = AutonomousAgentEngine(manager, RecordingExecutor(), ToolPermissionManager())

        val result = engine.execute("Run tests")

        assertEquals(AgentCheckpoint.Status.FAILED, result.checkpoint.status)
        assertTrue(result.checkpoint.lastError!!.contains("Permission denied"))
    }

    @Test
    fun toolObservations_areFedBack_untilFinalResponse() = runBlocking {
        val provider = SequencedProvider(
            """{"toolCall":{"name":"ReadFile","arguments":{"path":"README.md"}}}""",
            "Final answer after observation"
        )
        val manager = ProviderManager().apply { register(provider) }
        val engine = AutonomousAgentEngine(manager, RecordingExecutor(), ToolPermissionManager(), maxSteps = 3)

        val result = engine.execute("Inspect README")

        assertEquals(AgentCheckpoint.Status.COMPLETED, result.checkpoint.status)
        assertEquals("Final answer after observation", result.output)
        assertEquals(2, provider.prompts.size)
        assertTrue(provider.prompts[1].contains("ReadFile succeeded"))
    }

    @Test
    fun providerFailure_failsOverToBackupProvider() = runBlocking {
        val manager = ProviderManager().apply {
            register(FakeProvider(throwOnGenerate = true))
            register(FakeProvider("Recovered response", idValue = "backup"))
        }
        val engine = AutonomousAgentEngine(manager, RecordingExecutor(), ToolPermissionManager())

        val result = engine.execute("Explain this code")

        assertEquals(AgentCheckpoint.Status.COMPLETED, result.checkpoint.status)
        assertEquals("backup", result.provider)
        assertEquals("Recovered response", result.output)
    }

    private class FakeProvider(
        private val response: String = "",
        private val idValue: String = "primary",
        private val throwOnGenerate: Boolean = false,
        private val finalResponse: String? = null
    ) : Provider {
        override val id: String = idValue
        override val displayName: String = idValue
        override suspend fun isAvailable(): Boolean = true
        private var callCount = 0
        override suspend fun generate(prompt: String): String {
            if (throwOnGenerate) throw IllegalStateException("primary failed")
            return if (callCount++ == 0) response else finalResponse ?: response
        }
    }

    private class SequencedProvider(private vararg val responses: String) : Provider {
        override val id: String = "sequenced"
        override val displayName: String = id
        val prompts = mutableListOf<String>()
        private var index = 0
        override suspend fun isAvailable(): Boolean = true
        override suspend fun generate(prompt: String): String {
            prompts += prompt
            return responses.getOrElse(index++) { responses.last() }
        }
    }

    private class RecordingExecutor : ToolExecutionLayer {
        var lastRequest: ToolRequest? = null
        override suspend fun execute(request: ToolRequest): ExecutionResult {
            lastRequest = request
            return ExecutionResult.Success("ok")
        }
    }
}
