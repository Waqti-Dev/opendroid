package com.opendroid.ai.core.agent

import com.opendroid.ai.core.llm.HybridRouterV2

/**
 * Coordinates task execution through local and cloud aware routing.
 */
class HybridAgentLoop(
    private val router: HybridRouterV2
) {
    suspend fun execute(task: AgentTask): AgentResult {
        val route = router.selectRoute(task)
        return AgentResult(
            task = task,
            route = route
        )
    }
}

data class AgentTask(
    val prompt: String,
    val requiresCoding: Boolean = false
)

data class AgentResult(
    val task: AgentTask,
    val route: String
)
