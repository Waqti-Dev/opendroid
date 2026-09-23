package com.opendroid.ai.core.agent

import com.opendroid.ai.core.llm.HybridRouterV2

/**
 * Agent loop foundation with memory hooks.
 * The real memory implementation can be injected later without changing flow.
 */
class MemoryAwareAgentLoop(
    private val router: HybridRouterV2,
    private val memory: AgentMemory
) {
    suspend fun run(task: String): String {
        val context = memory.retrieve(task)
        val result = router.route(task, context)
        memory.store(task, result)
        return result
    }
}

interface AgentMemory {
    suspend fun retrieve(task: String): String?
    suspend fun store(task: String, result: String)
}
