package com.opendroid.ai.orchestrator

/**
 * Saves execution state so tasks can continue after provider switching.
 * Forked by Ahmed Badr
 */
class CheckpointManager {

    private var checkpoint: AgentCheckpoint? = null

    fun save(state: AgentCheckpoint) {
        checkpoint = state
    }

    fun restore(): AgentCheckpoint? = checkpoint
}

data class AgentCheckpoint(
    val taskId: String,
    val currentStep: Int,
    val completedSteps: List<String>,
    val contextSummary: String
)
