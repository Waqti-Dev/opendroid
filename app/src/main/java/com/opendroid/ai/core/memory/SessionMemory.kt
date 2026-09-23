package com.opendroid.ai.core.memory

/**
 * Keeps task state so an agent can resume work after provider changes.
 */
data class SessionMemory(
    val taskId: String,
    var currentStep: Int = 0,
    var summary: String = "",
    val changedFiles: MutableList<String> = mutableListOf(),
    var lastError: String? = null
)
