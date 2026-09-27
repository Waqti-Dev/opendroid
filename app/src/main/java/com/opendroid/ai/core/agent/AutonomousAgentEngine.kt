package com.opendroid.ai.core.agent

import com.opendroid.ai.core.providers.ProviderManager
import com.opendroid.ai.core.tools.ExecutionResult
import com.opendroid.ai.core.tools.ToolExecutionLayer
import com.opendroid.ai.core.tools.ToolPermissionManager
import com.opendroid.ai.core.tools.ToolRequest
import kotlinx.coroutines.ensureActive
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.jsonObject
import kotlinx.serialization.json.jsonPrimitive
import kotlinx.serialization.json.contentOrNull
import java.util.UUID
import kotlin.coroutines.coroutineContext

/**
 * Bounded coding-agent loop for the MVP. It is intentionally independent of a
 * concrete cloud/local provider and resumes through [AgentCheckpointStore].
 */
class AutonomousAgentEngine(
    private val providerManager: ProviderManager,
    private val toolExecutionLayer: ToolExecutionLayer,
    private val permissionManager: ToolPermissionManager,
    private val checkpointStore: AgentCheckpointStore = InMemoryAgentCheckpointStore(),
    private val maxSteps: Int = DEFAULT_MAX_STEPS
) {
    private val json = Json { ignoreUnknownKeys = true; isLenient = true }

    init {
        require(maxSteps in 1..100) { "maxSteps must be between 1 and 100." }
    }

    suspend fun execute(prompt: String, taskId: String = UUID.randomUUID().toString()): AgentExecutionResult {
        var checkpoint = checkpointStore.load(taskId)
            ?: AgentCheckpoint(taskId = taskId, prompt = prompt)
        val observations = mutableListOf<String>()
        var selectedProviderId: String? = null

        return try {
            for (step in 1..maxSteps) {
                coroutineContext.ensureActive()
                val generation = providerManager.generateWithFailover(
                    buildAgentPrompt(prompt, checkpoint, observations)
                )
                selectedProviderId = generation.provider.id
                val toolCall = parseToolCall(generation.response)

                if (toolCall == null) {
                    checkpoint = checkpoint.copy(
                        currentStep = step,
                        completedSteps = checkpoint.completedSteps + "Model response",
                        contextSummary = generation.response.take(MAX_CONTEXT_CHARS),
                        status = AgentCheckpoint.Status.COMPLETED,
                        lastError = null
                    )
                    checkpointStore.save(checkpoint)
                    return AgentExecutionResult(taskId, generation.response, selectedProviderId, checkpoint)
                }

                val request = toToolRequest(toolCall)
                    ?: return fail(checkpoint, "Unsupported tool: ${toolCall.name}")
                if (!permissionManager.canExecute(request)) {
                    return fail(checkpoint, "Permission denied for tool: ${toolCall.name}")
                }

                checkpoint = checkpoint.copy(
                    currentStep = step,
                    completedSteps = checkpoint.completedSteps + "Plan tool execution: ${toolCall.name}",
                    status = AgentCheckpoint.Status.RUNNING,
                    lastError = null
                )
                checkpointStore.save(checkpoint)

                val execution = toolExecutionLayer.execute(request)
                val observation = when (execution) {
                    is ExecutionResult.Success -> "${toolCall.name} succeeded:\n${execution.output}"
                    is ExecutionResult.Failure -> "${toolCall.name} failed:\n${execution.reason}"
                }.take(MAX_CONTEXT_CHARS)
                observations += observation

                val modified = if (request is ToolRequest.WriteFile || request is ToolRequest.CreateFile || request is ToolRequest.PatchFile) {
                    checkpoint.modifiedFiles + toolPath(request)
                } else checkpoint.modifiedFiles
                checkpoint = checkpoint.copy(
                    currentStep = step,
                    completedSteps = checkpoint.completedSteps + "Observe ${toolCall.name}",
                    modifiedFiles = modified.distinct(),
                    contextSummary = observation,
                    status = AgentCheckpoint.Status.RUNNING,
                    lastError = if (execution is ExecutionResult.Failure) execution.reason else null
                )
                checkpointStore.save(checkpoint)
            }
            fail(checkpoint, "Maximum agent steps reached ($maxSteps).")
        } catch (e: kotlinx.coroutines.CancellationException) {
            val cancelled = checkpoint.copy(status = AgentCheckpoint.Status.WAITING, lastError = "Cancelled")
            checkpointStore.save(cancelled)
            throw e
        } catch (e: Exception) {
            fail(checkpoint, e.message ?: "Unknown agent error")
        }
    }

    private fun buildAgentPrompt(prompt: String, checkpoint: AgentCheckpoint, observations: List<String>): String =
        """
        You are a bounded coding agent working on a local Android project.
        User task:
        $prompt

        Return ONLY JSON for a tool call, or a normal final answer when done.
        Supported calls:
        {"toolCall":{"name":"ReadFile","arguments":{"path":"..."}}}
        {"toolCall":{"name":"WriteFile","arguments":{"path":"...","content":"..."}}}
        {"toolCall":{"name":"CreateFile","arguments":{"path":"...","content":"..."}}}
        {"toolCall":{"name":"PatchFile","arguments":{"path":"...","oldText":"...","newText":"..."}}}
        {"toolCall":{"name":"ListFiles","arguments":{"path":".","recursive":"false"}}}
        {"toolCall":{"name":"SearchFiles","arguments":{"query":"...","path":"."}}}
        {"toolCall":{"name":"RunCommand","arguments":{"command":"..."}}}

        This run is bounded to $maxSteps steps. Never request deletion, remote git push,
        credentials, or commands outside the project workspace.

        Previous checkpoint:
        ${checkpoint.contextSummary}
        Observations:
        ${observations.takeLast(8).joinToString("\n---\n")}
        """.trimIndent()

    private data class ParsedToolCall(val name: String, val arguments: Map<String, String>)

    private fun parseToolCall(response: String): ParsedToolCall? {
        val root = runCatching { json.parseToJsonElement(response.trim()).jsonObject }.getOrNull() ?: return null
        val call = root["toolCall"]?.jsonObject ?: return null
        val name = call["name"]?.jsonPrimitive?.contentOrNull ?: return null
        val args = call["arguments"]?.jsonObject
            ?.mapValues { it.value.jsonPrimitive.contentOrNull ?: "" } ?: emptyMap()
        return ParsedToolCall(name, args)
    }

    private fun toToolRequest(call: ParsedToolCall): ToolRequest? = when (call.name) {
        "ReadFile" -> ToolRequest.ReadFile(call.arguments["path"].orEmpty())
        "WriteFile" -> ToolRequest.WriteFile(call.arguments["path"].orEmpty(), call.arguments["content"].orEmpty())
        "CreateFile" -> ToolRequest.CreateFile(call.arguments["path"].orEmpty(), call.arguments["content"].orEmpty())
        "PatchFile" -> ToolRequest.PatchFile(call.arguments["path"].orEmpty(), call.arguments["oldText"].orEmpty(), call.arguments["newText"].orEmpty())
        "DeleteFile" -> ToolRequest.DeleteFile(call.arguments["path"].orEmpty())
        "ListFiles" -> ToolRequest.ListFiles(call.arguments["path"].ifBlank { "." }, call.arguments["recursive"].toBoolean())
        "SearchFiles" -> ToolRequest.SearchFiles(call.arguments["query"].orEmpty(), call.arguments["path"].ifBlank { "." })
        "RunCommand" -> ToolRequest.RunCommand(call.arguments["command"].orEmpty())
        else -> null
    }

    private fun toolPath(request: ToolRequest): String = when (request) {
        is ToolRequest.WriteFile -> request.path
        is ToolRequest.CreateFile -> request.path
        is ToolRequest.PatchFile -> request.path
        else -> ""
    }

    private suspend fun fail(checkpoint: AgentCheckpoint, error: String): AgentExecutionResult {
        val failed = checkpoint.copy(lastError = error, status = AgentCheckpoint.Status.FAILED)
        checkpointStore.save(failed)
        return AgentExecutionResult(checkpoint.taskId, "", null, failed)
    }

    private companion object {
        const val DEFAULT_MAX_STEPS = 20
        const val MAX_CONTEXT_CHARS = 4_000
    }
}

data class AgentExecutionResult(
    val taskId: String,
    val output: String,
    val provider: String?,
    val checkpoint: AgentCheckpoint
)
