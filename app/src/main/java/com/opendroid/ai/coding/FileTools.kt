package com.opendroid.ai.coding

import java.io.File

/**
 * Basic filesystem tools for Waqti Agent / OpenDroid coding mode.
 * Forked by Ahmed Badr
 */
class FileTools {

    fun createFile(root: File, path: String, content: String): File {
        val target = safeResolve(root, path)
        target.parentFile?.mkdirs()
        target.writeText(content)
        return target
    }

    fun readFile(root: File, path: String): String {
        return safeResolve(root, path).readText()
    }

    fun updateFile(root: File, path: String, content: String) {
        safeResolve(root, path).writeText(content)
    }

    fun listFiles(root: File): List<String> {
        return root.walk()
            .filter { it.isFile }
            .map { it.relativeTo(root).path }
            .toList()
    }

    private fun safeResolve(root: File, path: String): File {
        val file = File(root, path).canonicalFile
        require(file.path.startsWith(root.canonicalPath)) {
            "Unsafe path outside workspace"
        }
        return file
    }
}
