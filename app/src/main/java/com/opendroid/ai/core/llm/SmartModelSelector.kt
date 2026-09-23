package com.opendroid.ai.core.llm

/**
 * Selects the most suitable local model for a task based on requirements.
 * Keeps model selection separate from execution/runtime.
 */
class SmartModelSelector {

    fun select(task: ModelTask, available: List<LocalModelProfile>): LocalModelProfile? {
        return available
            .filter { it.supportedTasks.contains(task.type) }
            .filter { it.maxContext >= task.requiredContext }
            .maxByOrNull { score(it, task) }
    }

    private fun score(model: LocalModelProfile, task: ModelTask): Int {
        var score = 0
        if (model.isCodingModel && task.type == TaskType.CODING) score += 50
        score += model.contextWindow / 1024
        score -= model.memoryRequiredMb / 512
        return score
    }
}


data class ModelTask(
    val type: TaskType,
    val requiredContext: Int = 4096
)

enum class TaskType {
    CODING,
    CHAT,
    SIMPLE
}

data class LocalModelProfile(
    val name: String,
    val memoryRequiredMb: Int,
    val contextWindow: Int,
    val isCodingModel: Boolean,
    val supportedTasks: Set<TaskType>
)
