package com.opendroid.ai.orchestrator

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class CheckpointResumeEngineTest {

    @Test
    fun checkpointKeepsTaskState() {
        val checkpoint = AgentCheckpoint(
            taskId = "task-1",
            currentStep = "build",
            completedSteps = listOf("plan", "code"),
            contextSummary = "android project",
            lastProvider = "qwen-15b"
        )

        assertEquals("build", checkpoint.currentStep)
        assertTrue(checkpoint.completedSteps.contains("code"))
    }
}
