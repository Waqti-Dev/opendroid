package com.opendroid.ai.coding.model

/**
 * Chooses the suitable model for a task.
 * Cloud providers remain the first testing target; this router prepares
 * the same logic for future local inference.
 */
class ModelRouter {

    fun select(task: TaskType): LocalModelProfile {
        return when (task) {
            TaskType.SIMPLE_CHAT -> LocalModelProfile.QWEN_3B
            TaskType.CODING -> LocalModelProfile.QWEN_7B
            TaskType.DEBUGGING -> LocalModelProfile.QWEN_15B
            TaskType.ARCHITECTURE -> LocalModelProfile.QWEN_15B
        }
    }
}

enum class TaskType {
    SIMPLE_CHAT,
    CODING,
    DEBUGGING,
    ARCHITECTURE
}
