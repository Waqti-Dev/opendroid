package com.opendroid.ai.orchestrator

import org.junit.Assert.assertEquals
import org.junit.Test

class SupervisorAgentTest {

    @Test
    fun codingTaskWithHighConfidenceStaysLocal() {
        val engine = object : SupervisorDecision {
            override fun decide(task: AgentTaskState): SupervisorAction {
                return if (task.confidence >= 0.8f)
                    SupervisorAction.CONTINUE_LOCAL
                else
                    SupervisorAction.REQUEST_CLOUD_REVIEW
            }
        }

        val agent = SupervisorAgent(engine)

        val result = agent.evaluate(
            AgentTaskState(
                taskId = "test-1",
                category = TaskCategory.CODING,
                confidence = 0.9f
            )
        )

        assertEquals(SupervisorAction.CONTINUE_LOCAL, result)
    }
}
