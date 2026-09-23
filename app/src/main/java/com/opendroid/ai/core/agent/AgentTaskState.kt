package com.opendroid.ai.core.agent

/**
 * Lifecycle state of an agent task.
 */
enum class AgentTaskState {
    CREATED,
    RUNNING,
    WAITING_FOR_PROVIDER,
    FAILED,
    COMPLETED
}
