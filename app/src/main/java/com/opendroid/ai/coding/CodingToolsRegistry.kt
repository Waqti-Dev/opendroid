package com.opendroid.ai.coding

/**
 * Registry for tools exposed to the coding agent.
 * Forked by Ahmed Badr
 */
class CodingToolsRegistry(
    private val fileTools: FileTools = FileTools()
) {

    fun availableTools(): List<String> = listOf(
        "CREATE_FILE",
        "READ_FILE",
        "UPDATE_FILE",
        "LIST_FILES"
    )

    fun files(): FileTools = fileTools
}
