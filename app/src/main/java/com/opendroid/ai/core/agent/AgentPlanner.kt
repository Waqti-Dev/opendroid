package com.opendroid.ai.core.agent

/**
 * Converts a user request into an executable plan.
 * This is the planning layer between the agent loop and tool execution.
 */
class AgentPlanner {

    fun createPlan(request: String): AgentPlan {
        val steps = mutableListOf<String>()

        when {
            request.contains("app", ignoreCase = true) -> {
                steps += "Analyze requirements"
                steps += "Create project structure"
                steps += "Generate source files"
                steps += "Build project"
                steps += "Run validation"
            }

            request.contains("code", ignoreCase = true) -> {
                steps += "Inspect existing code"
                steps += "Plan modification"
                steps += "Apply changes"
                steps += "Run tests"
            }

            else -> {
                steps += "Understand request"
                steps += "Select tools/models"
                steps += "Execute task"
                steps += "Store experience"
            }
        }

        return AgentPlan(request, steps)
    }
}

data class AgentPlan(
    val request: String,
    val steps: List<String>
)
