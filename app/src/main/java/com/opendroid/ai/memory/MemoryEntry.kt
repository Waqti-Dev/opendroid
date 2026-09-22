package com.opendroid.ai.memory

/**
 * Long term memory item for Waqti Agent.
 * Stores experience instead of changing model weights.
 * Forked by Ahmed Badr
 */
data class MemoryEntry(
    val id: String,
    val category: MemoryCategory,
    val title: String,
    val content: String,
    val tags: List<String> = emptyList(),
    val createdAt: Long = System.currentTimeMillis()
)

enum class MemoryCategory {
    PROJECT,
    SOLUTION,
    USER_PREFERENCE,
    KNOWLEDGE
}
