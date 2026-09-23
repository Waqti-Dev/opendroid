package com.opendroid.ai.core.agent

import com.opendroid.ai.core.llm.HybridRouterV2
import com.opendroid.ai.core.llm.RouteDecision

class HybridAgentLoop(private val router: HybridRouterV2) {
    suspend fun execute(task: AgentTask): AgentResult {
        val route = router.selectRoute(task.prompt)
        return AgentResult(task = task, route = route)
    }
}

data class AgentTask(val prompt: String, val requiresCoding: Boolean = false)

data class AgentResult(val task: AgentTask, val route: RouteDecision)
