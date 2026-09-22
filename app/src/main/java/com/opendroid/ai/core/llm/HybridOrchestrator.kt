package com.opendroid.ai.core.llm

import com.opendroid.ai.data.models.ChatMessage

/**
 * Hybrid orchestration foundation for Waqti Agent.
 *
 * Local models (Qwen 15B etc.) remain the execution engine while cloud
 * providers act as supervisors/reviewers when required.
 *
 * Forked by Ahmed Badr
 */
class HybridOrchestrator(
    private val providers: List<LLMProvider>
) {

    suspend fun selectExecutionMode(
        task: String,
        hasInternet: Boolean,
        localConfidence: Float
    ): ExecutionMode {
        return when {
            !hasInternet -> ExecutionMode.LOCAL
            localConfidence >= 0.85f -> ExecutionMode.LOCAL_WITH_OPTIONAL_REVIEW
            task.length > 500 || localConfidence < 0.5f -> ExecutionMode.CLOUD_SUPERVISED
            else -> ExecutionMode.LOCAL_WITH_OPTIONAL_REVIEW
        }
    }

    suspend fun findAvailableCloudProvider(): LLMProvider? {
        return providers.firstOrNull { it.isAvailable() }
    }

    suspend fun reviewIfNeeded(
        mode: ExecutionMode,
        messages: List<ChatMessage>
    ): LLMProvider? {
        if (mode != ExecutionMode.CLOUD_SUPERVISED) return null
        return findAvailableCloudProvider()
    }
}

enum class ExecutionMode {
    LOCAL,
    LOCAL_WITH_OPTIONAL_REVIEW,
    CLOUD_SUPERVISED
}
