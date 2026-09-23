package com.opendroid.ai.core.tools

import java.io.File

/**
 * Basic local implementation of tool execution.
 * This is intentionally minimal and should be wrapped with permissions
 * before exposing powerful device actions.
 */
class DefaultToolExecutor : ToolExecutionLayer {

    override suspend fun execute(request: ToolRequest): ExecutionResult {
        return try {
            when (request) {
                is ToolRequest.ReadFile -> {
                    ExecutionResult.Success(File(request.path).readText())
                }

                is ToolRequest.WriteFile -> {
                    File(request.path).writeText(request.content)
                    ExecutionResult.Success("File written: ${request.path}")
                }

                is ToolRequest.RunCommand -> {
                    ExecutionResult.Success("Command queued: ${request.command}")
                }
            }
        } catch (e: Exception) {
            ExecutionResult.Failure(e.message ?: "Unknown error")
        }
    }
}
