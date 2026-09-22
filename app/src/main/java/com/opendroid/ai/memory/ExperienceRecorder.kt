package com.opendroid.ai.memory

/**
 * Converts completed tasks into reusable experiences.
 * This creates memory for retrieval instead of retraining the LLM.
 * Forked by Ahmed Badr
 */
class ExperienceRecorder {

    fun recordSolution(
        title: String,
        solution: String,
        tags: List<String>
    ): MemoryEntry {
        return MemoryEntry(
            id = generateId(),
            category = MemoryCategory.SOLUTION,
            title = title,
            content = solution,
            tags = tags
        )
    }

    private fun generateId(): String =
        "mem_${System.currentTimeMillis()}"
}
