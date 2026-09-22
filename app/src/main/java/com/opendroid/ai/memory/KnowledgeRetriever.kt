package com.opendroid.ai.memory

/**
 * Retrieves useful memories before an agent task starts.
 * This is the bridge between Waqti Agent and long-term experience.
 * Forked by Ahmed Badr
 */
class KnowledgeRetriever(
    private val repository: MemoryRepository
) {

    fun retrieveContext(task: String): String {
        val memories = repository.search(task)

        return memories
            .take(5)
            .joinToString("\n") {
                "${it.title}: ${it.content}"
            }
    }
}
