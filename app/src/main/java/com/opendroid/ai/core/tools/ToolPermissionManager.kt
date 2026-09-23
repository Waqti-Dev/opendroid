package com.opendroid.ai.core.tools

/**
 * Controls whether an agent action is allowed before execution.
 *
 * This is the safety gate between planning and real device actions.
 */
class ToolPermissionManager {

    fun canExecute(request: ToolRequest): Boolean {
        return when (request) {
            is ToolRequest.ReadFile -> true
            is ToolRequest.WriteFile -> true
            is ToolRequest.RunCommand -> false
        }
    }
}
