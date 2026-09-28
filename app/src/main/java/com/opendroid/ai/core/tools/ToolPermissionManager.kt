package com.opendroid.ai.core.tools

/** Default MVP policy: read/write and safe development inspection are automatic. */
class ToolPermissionManager {
    fun canExecute(request: ToolRequest): Boolean = when (request) {
        is ToolRequest.ReadFile,
        is ToolRequest.WriteFile,
        is ToolRequest.CreateFile,
        is ToolRequest.PatchFile,
        is ToolRequest.ListFiles,
        is ToolRequest.SearchFiles -> true
        is ToolRequest.DeleteFile -> false
        is ToolRequest.RunCommand -> isSafeCommand(request.command)
    }

    companion object {
        private val allowedFirstTokens = setOf(
            "./gradlew", "gradle", "git", "ls", "pwd", "find", "grep", "rg",
            "python", "python3", "node", "npm", "cat", "head", "tail"
        )

        private val forbiddenFragments = listOf(
            "git push", "git commit", "git reset", "git clean", "git checkout",
            "rm ", "rm\\t", "sudo", "chmod", "chown", "dd ", "mkfs", "> /", ">/",
            " -c ", " -e ", " --eval", " -C ", " --work-tree", " --git-dir"
        )

        fun isSafeCommand(command: String): Boolean {
            val normalized = command.trim()
            if (normalized.isBlank() || normalized.length > 500) return false
            if (normalized.any { it in ";|&<>\`" } || normalized.contains("\u0024(")) return false

            val first = normalized.substringBefore(' ').substringBefore('\t')
            if (first !in allowedFirstTokens) return false

            // Keep command arguments relative to the agent workspace.
            // Absolute paths and ../ traversal are rejected at the policy boundary.
            val tokens = normalized.split(Regex("\\s+"))
            if (tokens.any { token ->
                    token == ".." || token.startsWith("../") || token.contains("/../") ||
                    token.startsWith("/") || token.startsWith("~")
                }) return false

            return forbiddenFragments.none { normalized.contains(it, ignoreCase = true) }
        }
    }
}
