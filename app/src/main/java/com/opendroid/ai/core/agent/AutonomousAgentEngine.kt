package com.opendroid.ai.core.agent

import com.opendroid.ai.core.providers.ProviderManager
import com.opendroid.ai.core.tools.ExecutionResult
import com.opendroid.ai.core.tools.ToolExecutionLayer
import com.opendroid.ai.core.tools.ToolPermissionManager
import com.opendroid.ai.core.tools.ToolRequest
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.jsonObject
import kotlinx.serialization.json.jsonPrimitive
import kotlinx.serialization.json.contentOrNull
import java.util.UUID

class AutonomousAgentEngine(
    private val providerManager: ProviderManager,
    private val toolExecutionLayer: ToolExecutionLayer,
    private val permissionManager: ToolPermissionManager,
    private val checkpointStore: AgentCheckpointStore = InMemoryAgentCheckpointStore()
) {
    private val json = Json { ignoreUnknownKeys = true; isLenient = true }

    suspend fun execute(prompt: String, taskId: String = UUID.randomUUID().toString()): AgentExecutionResult {
        var checkpoint = checkpointStore.load(taskId)
            ?: AgentCheckpoint(taskId = taskId, prompt = prompt)

        return try {
            val available = providerManager.selectAvailable()
                ?: return fail(checkpoint, "No available AI provider.")

            checkpoint = checkpoint.copy(
                currentStep = 1,
                contextSummary = "Selected provider: ${available.displayName}"
            )
            checkpointStore.save(checkpoint)

            val generation = providerManager.generateWithFailover(
                buildAgentPrompt(prompt, checkpoint)
            )
            val provider = generation.provider
            val firstResponse = generation.response
            val toolCall = parseToolCall(firstResponse)

            if (toolCall == null) {
                checkpoint = checkpoint.copy(
                    currentStep = 2,
                    completedSteps = checkpoint.completedSteps + "Model response",
                    contextSummary = firstResponse.take(1000),
                    status = AgentCheckpoint.Status.COMPLETED
                )
                checkpointStore.save(checkpoint)
                return AgentExecutionResult(taskId, firstResponse, provider.id, checkpoint)
            }

            val request = toToolRequest(toolCall)
                ?: return fail(checkpoint, "Unsupported tool: ${toolCall.name}")

            if (!permissionManager.canExecute(request)) {
                return fail(checkpoint, "Permission denied for tool: ${toolCall.name}")
            }

            checkpoint = checkpoint.copy(
                currentStep = 2,
                completedSteps = checkpoint.completedSteps + "Plan tool execution"
            )
            checkpointStore.save(checkpoint)

            val execution = toolExecutionLayer.execute(request)
            if (execution is ExecutionResult.Failure) {
                return fail(checkpoint, execution.reason)
            }

            val output = (execution as ExecutionResult.Success).output
            val modified = if (request is ToolRequest.WriteFile) {
                checkpoint.modifiedFiles + request.path
            } else checkpoint.modifiedFiles

            checkpoint = checkpoint.copy(
                currentStep = 3,
                completedSteps = checkpoint.completedSteps + "Execute ${toolCall.name}",
                modifiedFiles = modified,
                contextSummary = output.take(1000),
                status = AgentCheckpoint.Status.COMPLETED
            )
            checkpointStore.save(checkpoint)

            AgentExecutionResult(taskId, output, provider.id, checkpoint)
        } catch (e: Exception) {
            fail(checkpoint, e.message ?: "Unknown agent error")
        }
    }

    private fun buildAgentPrompt(prompt: String, checkpoint: AgentCheckpoint): String =
        """
        You are a coding agent working on a local Android project.
        User task:
        $prompt

        If a file operation is required, return ONLY JSON:
        {"toolCall":{"name":"ReadFile","arguments":{"path":"..."}}}
        {"toolCall":{"name":"WriteFile","arguments":{"path":"...","content":"..."}}}
        {"toolCall":{"name":"RunCommand","arguments":{"command":"..."}}}
        Otherwise return a normal final answer.

        Previous checkpoint:
        ${checkpoint.contextSummary}
        """.trimIndent()

    private data class ParsedToolCall(val name: String, val arguments: Map<String, String>)

    private fun parseToolCall(response: String): ParsedToolCall? {
        val root = runCatching { json.parseToJsonElement(response.trim()).jsonObject }.getOrNull()
            ?: return null
        val call = root["toolCall"]?.jsonObject ?: return null
        val name = call["name"]?.jsonPrimitive?.contentOrNull ?: return null
        val args = call["arguments"]?.jsonObject
            ?.mapValues { it.value.jsonPrimitive.contentOrNull ?: "" } ?: emptyMap()
        return ParsedToolCall(name, args)
    }

    private fun toToolRequest(call: ParsedToolCall): ToolRequest? = when (call.name) {
        "ReadFile" -> ToolRequest.ReadFile(call.arguments["path"].orEmpty())
        "WriteFile" -> ToolRequest.WriteFile(call.arguments["path"].orEmpty(), call.arguments["content"].orEmpty())
        "RunCommand" -> ToolRequest.RunCommand(call.arguments["command"].orEmpty())
        else -> null
    }

    private suspend fun fail(checkpoint: AgentCheckpoint, error: String): AgentExecutionResult {
        val failed = checkpoint.copy(lastError = error, status = AgentCheckpoint.Status.FAILED)
        checkpointStore.save(failed)
        return AgentExecutionResult(checkpoint.taskId, "", null, failed)
    }
}

data class AgentExecutionResult(
    val taskId: String,
    val output: String,
    val provider: String?,
    val checkpoint: AgentCheckpoint
)
