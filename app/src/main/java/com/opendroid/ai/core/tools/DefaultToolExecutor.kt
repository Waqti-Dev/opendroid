package com.opendroid.ai.core.tools

import java.io.File

class DefaultToolExecutor(
    private val workspaceRoot: File = File(".").canonicalFile
) : ToolExecutionLayer {

    override suspend fun execute(request: ToolRequest): ExecutionResult {
        return try {
            when (request) {
                is ToolRequest.ReadFile -> {
                    val file = resolveInsideWorkspace(request.path)
                    ExecutionResult.Success(file.readText())
                }
                is ToolRequest.WriteFile -> {
                    val file = resolveInsideWorkspace(request.path)
                    file.parentFile?.mkdirs()
                    file.writeText(request.content)
                    ExecutionResult.Success("File written: " + request.path)
                }
                is ToolRequest.RunCommand -> {
                    ExecutionResult.Failure(
                        "Command execution is not enabled by DefaultToolExecutor: " + request.command
                    )
                }
            }
        } catch (e: Exception) {
            ExecutionResult.Failure(e.message ?: "Unknown tool execution error")
        }
    }

    private fun resolveInsideWorkspace(path: String): File {
        require(path.isNotBlank()) { "File path cannot be empty." }
        val root = workspaceRoot.canonicalFile
        val target = File(root, path).canonicalFile
        require(
            target.path == root.path ||
                target.path.startsWith(root.path + File.separator)
        ) {
            "Path escapes workspace: " + path
        }
        return target
    }
}
