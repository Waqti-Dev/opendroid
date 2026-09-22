package com.opendroid.ai.coding

/**
 * Bridge foundation for developer mode command execution.
 *
 * Future versions will integrate with Termux:API and Android shell
 * execution providers for build/test workflows.
 *
 * Forked by Ahmed Badr
 */
class TermuxBridge {

    data class CommandResult(
        val exitCode: Int,
        val output: String,
        val error: String
    )

    fun validateCommand(command: String): Boolean {
        return command.isNotBlank()
    }

    fun createPendingCommand(command: String): String {
        require(validateCommand(command)) { "Command cannot be empty" }
        return command.trim()
    }
}
