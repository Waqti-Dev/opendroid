package com.opendroid.ai.core.agent

data class AgentCheckpoint(
    val taskId: String,
    val prompt: String,
    val currentStep: Int = 0,
    val completedSteps: List<String> = emptyList(),
    val modifiedFiles: List<String> = emptyList(),
    val lastError: String? = null,
    val contextSummary: String = "",
    val status: Status = Status.RUNNING
) {
    enum class Status { RUNNING, WAITING, COMPLETED, FAILED }
}

interface AgentCheckpointStore {
    suspend fun load(taskId: String): AgentCheckpoint?
    suspend fun save(checkpoint: AgentCheckpoint)
}

class InMemoryAgentCheckpointStore : AgentCheckpointStore {
    private val values = mutableMapOf<String, AgentCheckpoint>()
    override suspend fun load(taskId: String): AgentCheckpoint? = values[taskId]
    override suspend fun save(checkpoint: AgentCheckpoint) {
        values[checkpoint.taskId] = checkpoint
    }
}
