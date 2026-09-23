package com.opendroid.ai.core.llm

class SmartModelSelector {
    fun select(task: ModelTask, available: List<LocalModelProfile>): LocalModelProfile? =
        available.filter { it.supportedTasks.contains(task.type) }
            .filter { it.contextWindow >= task.requiredContext }
            .maxByOrNull { score(it, task) }

    fun select(task: String): String {
        val normalized = task.lowercase()
        return when {
            normalized.contains("build") || normalized.contains("android") ||
                normalized.contains("kotlin") || normalized.contains("code") -> "Qwen15B"
            normalized.contains("explain") || normalized.contains("simple") ||
                normalized.length < 120 -> "Qwen3B"
            else -> "Qwen7B"
        }
    }

    private fun score(model: LocalModelProfile, task: ModelTask): Int {
        var score = 0
        if (model.isCodingModel && task.type == TaskType.CODING) score += 50
        score += model.contextWindow / 1024
        score -= model.memoryRequiredMb / 512
        return score
    }
}

data class ModelTask(val type: TaskType, val requiredContext: Int = 4096)
enum class TaskType { CODING, CHAT, SIMPLE }
data class LocalModelProfile(
    val name: String,
    val memoryRequiredMb: Int,
    val contextWindow: Int,
    val isCodingModel: Boolean,
    val supportedTasks: Set<TaskType>
)
