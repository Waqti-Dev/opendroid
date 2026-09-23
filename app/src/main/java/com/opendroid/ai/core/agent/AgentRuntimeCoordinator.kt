package com.opendroid.ai.core.agent

import com.opendroid.ai.core.tools.ToolManager
import com.opendroid.ai.core.tools.ToolPermissionManager
import com.opendroid.ai.core.tools.ToolRequest

class AgentRuntimeCoordinator(
    private val planner: AgentPlanner,
    private val toolManager: ToolManager,
    private val permissionManager: ToolPermissionManager
) {
    fun createExecutionPlan(request: String): AgentPlan = planner.createPlan(request)

    fun canExecute(toolName: String): Boolean {
        return when (toolName) {
            "ReadFile" -> permissionManager.canExecute(ToolRequest.ReadFile(""))
            "WriteFile" -> permissionManager.canExecute(ToolRequest.WriteFile("", ""))
            "RunCommand" -> permissionManager.canExecute(ToolRequest.RunCommand(""))
            else -> false
        }
    }
}
