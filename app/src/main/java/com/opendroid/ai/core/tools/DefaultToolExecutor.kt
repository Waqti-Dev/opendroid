package com.opendroid.ai.core.tools

import java.io.File
import java.util.concurrent.TimeUnit

class DefaultToolExecutor(
    private val workspaceRoot: File = File(".").canonicalFile,
    private val commandTimeoutSeconds: Long = 30L
) : ToolExecutionLayer {

    override suspend fun execute(request: ToolRequest): ExecutionResult = try {
        when (request) {
            is ToolRequest.ReadFile -> {
                val file = resolveInsideWorkspace(request.path)
                require(file.isFile) { "Not a regular file: ${request.path}" }
                ExecutionResult.Success(file.readText())
            }
            is ToolRequest.WriteFile -> {
                val file = resolveInsideWorkspace(request.path)
                file.parentFile?.mkdirs()
                file.writeText(request.content)
                ExecutionResult.Success("File written: ${request.path}")
            }
            is ToolRequest.CreateFile -> {
                val file = resolveInsideWorkspace(request.path)
                require(!file.exists()) { "File already exists: ${request.path}" }
                file.parentFile?.mkdirs()
                file.writeText(request.content)
                ExecutionResult.Success("File created: ${request.path}")
            }
            is ToolRequest.PatchFile -> {
                val file = resolveInsideWorkspace(request.path)
                require(file.isFile) { "Not a regular file: ${request.path}" }
                val original = file.readText()
                require(original.contains(request.oldText)) { "Patch context not found: ${request.path}" }
                file.writeText(original.replace(request.oldText, request.newText))
                ExecutionResult.Success("File patched: ${request.path}")
            }
            is ToolRequest.DeleteFile -> ExecutionResult.Failure("Destructive file deletion requires explicit confirmation.")
            is ToolRequest.ListFiles -> {
                val directory = resolveInsideWorkspace(request.path)
                require(directory.isDirectory) { "Not a directory: ${request.path}" }
                val files = if (request.recursive) directory.walkTopDown() else directory.listFiles()?.asSequence() ?: emptySequence()
                val output = files
                    .filter { it != directory }
                    .take(200)
                    .joinToString("\n") { it.relativeTo(workspaceRoot).path }
                ExecutionResult.Success(output)
            }
            is ToolRequest.SearchFiles -> {
                require(request.query.isNotBlank()) { "Search query cannot be empty." }
                val directory = resolveInsideWorkspace(request.path)
                require(directory.isDirectory) { "Not a directory: ${request.path}" }
                val output = directory.walkTopDown()
                    .filter { it.isFile && it.length() <= MAX_SEARCH_FILE_BYTES }
                    .mapNotNull { file ->
                        runCatching { if (file.readText().contains(request.query, ignoreCase = true)) file.relativeTo(workspaceRoot).path else null }.getOrNull()
                    }
                    .take(100)
                    .joinToString("\n")
                ExecutionResult.Success(output)
            }
            is ToolRequest.RunCommand -> runCommand(request.command)
        }
    } catch (e: Exception) {
        ExecutionResult.Failure(e.message ?: "Unknown tool execution error")
    }

    private fun runCommand(command: String): ExecutionResult {
        require(ToolPermissionManager.isSafeCommand(command)) { "Command is not allowed by the MVP safety policy." }
        val process = ProcessBuilder("sh", "-c", command)
            .directory(workspaceRoot)
            .redirectErrorStream(false)
            .start()
        val completed = process.waitFor(commandTimeoutSeconds, TimeUnit.SECONDS)
        if (!completed) {
            process.destroyForcibly()
            return ExecutionResult.Failure("Command timed out after ${commandTimeoutSeconds}s.")
        }
        val stdout = process.inputStream.bufferedReader().readText().take(MAX_COMMAND_OUTPUT_CHARS)
        val stderr = process.errorStream.bufferedReader().readText().take(MAX_COMMAND_OUTPUT_CHARS)
        val output = buildString {
            append("exitCode=").append(process.exitValue()).append('\n')
            if (stdout.isNotBlank()) append(stdout)
            if (stderr.isNotBlank()) append("\nstderr:\n").append(stderr)
        }.trim()
        return if (process.exitValue() == 0) ExecutionResult.Success(output)
        else ExecutionResult.Failure(output)
    }

    private fun resolveInsideWorkspace(path: String): File {
        require(path.isNotBlank()) { "File path cannot be empty." }
        val root = workspaceRoot.canonicalFile
        val target = File(root, path).canonicalFile
        require(target.path == root.path || target.path.startsWith(root.path + File.separator)) {
            "Path escapes workspace: $path"
        }
        return target
    }

    private companion object {
        const val MAX_SEARCH_FILE_BYTES = 1_000_000L
        const val MAX_COMMAND_OUTPUT_CHARS = 20_000
    }
}
