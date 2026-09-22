package com.opendroid.ai.orchestrator

/**
 * Restores interrupted agent tasks without restarting from zero.
 *
 * Forked by Ahmed Badr
 */
class CheckpointResumeEngine(
    private val checkpointManager: CheckpointManager
) {

    fun save(state: AgentCheckpoint) {
        checkpointManager.save(state)
    }

    fun resume(taskId: String): AgentCheckpoint? {
        return checkpointManager.load(taskId)
    }

    fun canResume(taskId: String): Boolean {
        return checkpointManager.load(taskId) != null
    }
}

data class AgentCheckpoint(
    val taskId: String,
    val currentStep: String,
    val completedSteps: List<String>,
    val contextSummary: String,
    val lastProvider: String?
)
