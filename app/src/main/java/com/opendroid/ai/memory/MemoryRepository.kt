package com.opendroid.ai.memory

import java.util.concurrent.CopyOnWriteArrayList

/**
 * In-memory foundation for Waqti Agent memory layer.
 * Will be replaced by Room/vector storage while keeping the same interface.
 * Forked by Ahmed Badr
 */
class MemoryRepository {

    private val entries = CopyOnWriteArrayList<MemoryEntry>()

    fun save(entry: MemoryEntry) {
        entries.removeIf { it.id == entry.id }
        entries.add(entry)
    }

    fun getAll(): List<MemoryEntry> = entries.toList()

    fun search(query: String): List<MemoryEntry> {
        val normalized = query.lowercase()
        return entries.filter {
            it.title.lowercase().contains(normalized) ||
                it.content.lowercase().contains(normalized) ||
                it.tags.any { tag -> tag.lowercase().contains(normalized) }
        }
    }
}
