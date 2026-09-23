package com.opendroid.ai.core.tools

/**
 * Foundation layer for agent tool execution.
 *
 * The agent should not directly execute actions. Every action goes through
 * this layer so permissions, logging and safety checks can be added later.
 */
interface ToolExecutionLayer {
    suspend fun execute(request: ToolRequest): ExecutionResult
}

sealed class ToolRequest {
    data class ReadFile(val path: String) : ToolRequest()
    data class WriteFile(val path: String, val content: String) : ToolRequest()
    data class RunCommand(val command: String) : ToolRequest()
}

sealed class ExecutionResult {
    data class Success(val output: String) : ExecutionResult()
    data class Failure(val reason: String) : ExecutionResult()
}
