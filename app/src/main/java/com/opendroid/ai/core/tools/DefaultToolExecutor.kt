package com.opendroid.ai.core.tools

import java.io.File

/**
 * Basic local implementation of tool execution.
 * This is intentionally minimal and should be wrapped with permissions
 * before exposing powerful device actions.
 */
class DefaultToolExecutor : ToolExecutionLayer {

    override suspend fun execute(request: ToolRequest): ToolResult {
        return try {
            when (request) {
                is ToolRequest.ReadFile -> {
                    ToolResult.Success(File(request.path).readText())
                }

                is ToolRequest.WriteFile -> {
                    File(request.path).writeText(request.content)
                    ToolResult.Success("File written: ${request.path}")
                }

                is ToolRequest.RunCommand -> {
                    ToolResult.Success("Command queued: ${request.command}")
                }
            }
        } catch (e: Exception) {
            ToolResult.Failure(e.message ?: "Unknown error")
        }
    }
}
