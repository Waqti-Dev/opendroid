package com.opendroid.ai.core.tools

/**
 * Central registry for agent tools.
 * Keeps tool discovery separate from model reasoning.
 */
class ToolManager {
    private val tools = mutableMapOf<String, AgentTool>()

    fun register(tool: AgentTool) {
        tools[tool.name] = tool
    }

    fun get(name: String): AgentTool? = tools[name]

    fun availableTools(): List<String> = tools.keys.toList()
}

interface AgentTool {
    val name: String
    val description: String
    suspend fun execute(input: String): ToolResult
}

data class ToolResult(
    val success: Boolean,
    val output: String,
    val error: String? = null
)
