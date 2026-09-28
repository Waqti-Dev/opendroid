package com.opendroid.ai.core.tools

/**
 * Boundary for coding-agent tools. Implementations must enforce workspace and
 * permission policy; the agent controller never touches the filesystem directly.
 */
interface ToolExecutionLayer {
    suspend fun execute(request: ToolRequest): ExecutionResult
}

sealed class ToolRequest {
    data class ReadFile(val path: String) : ToolRequest()
    data class WriteFile(val path: String, val content: String) : ToolRequest()
    data class CreateFile(val path: String, val content: String) : ToolRequest()
    data class PatchFile(val path: String, val oldText: String, val newText: String) : ToolRequest()
    data class DeleteFile(val path: String) : ToolRequest()
    data class ListFiles(val path: String = ".", val recursive: Boolean = false) : ToolRequest()
    data class SearchFiles(val query: String, val path: String = ".") : ToolRequest()
    data class RunCommand(val command: String) : ToolRequest()
}

sealed class ExecutionResult {
    data class Success(val output: String) : ExecutionResult()
    data class Failure(val reason: String) : ExecutionResult()
}
