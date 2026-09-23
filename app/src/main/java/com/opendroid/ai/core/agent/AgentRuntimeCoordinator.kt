package com.opendroid.ai.core.agent

import com.opendroid.ai.core.tools.ToolManager
import com.opendroid.ai.core.tools.ToolPermissionManager

/**
 * Coordinates planning, permissions and tool execution.
 * This is the integration point between the Agent loop and action layer.
 */
class AgentRuntimeCoordinator(
    private val planner: AgentPlanner,
    private val toolManager: ToolManager,
    private val permissionManager: ToolPermissionManager
) {
    fun createExecutionPlan(request: String): List<String> {
        return planner.createPlan(request)
    }

    fun canExecute(toolName: String): Boolean {
        return permissionManager.isAllowed(toolName)
    }
}
