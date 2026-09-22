package com.opendroid.ai.coding

/**
 * Executes coding related tool requests from the agent layer.
 *
 * Forked by Ahmed Badr
 */
class CodingToolExecutor(
    private val fileTools: FileTools,
    private val termuxBridge: TermuxBridge
) {
    fun execute(tool: String, args: Map<String, String>): ToolResult {
        return when (tool) {
            "CREATE_FILE" -> {
                val path = args["path"] ?: return ToolResult(false, "Missing path")
                val content = args["content"] ?: ""
                ToolResult(fileTools.createFile(path, content), "create file")
            }

            "READ_FILE" -> {
                val path = args["path"] ?: return ToolResult(false, "Missing path")
                ToolResult(true, fileTools.readFile(path) ?: "File not found")
            }

            "RUN_COMMAND" -> {
                val command = args["command"] ?: return ToolResult(false, "Missing command")
                termuxBridge.execute(command)
            }

            else -> ToolResult(false, "Unknown coding tool: $tool")
        }
    }
}

data class ToolResult(
    val success: Boolean,
    val message: String
)
