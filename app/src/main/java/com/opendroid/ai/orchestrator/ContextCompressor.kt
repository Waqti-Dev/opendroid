package com.opendroid.ai.orchestrator

/**
 * Reduces the amount of project context sent to remote providers.
 * Keeps local execution efficient and lowers cloud token usage.
 */
class ContextCompressor {

    fun compress(files: List<ProjectFile>, query: String): CompressedContext {
        val relevant = files.filter {
            it.name.contains(query, ignoreCase = true) ||
                    it.content.contains(query, ignoreCase = true)
        }.take(20)

        return CompressedContext(
            summary = "Selected ${relevant.size} relevant files for task",
            files = relevant
        )
    }
}

data class ProjectFile(
    val name: String,
    val content: String
)

data class CompressedContext(
    val summary: String,
    val files: List<ProjectFile>
)
