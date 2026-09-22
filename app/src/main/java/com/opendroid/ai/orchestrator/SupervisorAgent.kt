package com.opendroid.ai.orchestrator

/**
 * Coordinates local and cloud reasoning decisions.
 *
 * The supervisor does not replace the coding model. It decides when
 * additional review, escalation, or recovery is required.
 */
class SupervisorAgent(
    private val decisionEngine: SupervisorDecision
) {

    fun evaluate(task: AgentTaskState): SupervisorAction {
        return decisionEngine.decide(task)
    }
}


data class AgentTaskState(
    val taskId: String,
    val category: TaskCategory,
    val confidence: Float,
    val requiresExternalKnowledge: Boolean = false,
    val hasRepeatedFailure: Boolean = false
)


enum class TaskCategory {
    CHAT,
    CODING,
    DEBUGGING,
    ARCHITECTURE,
    RESEARCH
}


enum class SupervisorAction {
    CONTINUE_LOCAL,
    REQUEST_CLOUD_REVIEW,
    SWITCH_PROVIDER,
    SAVE_CHECKPOINT
}
