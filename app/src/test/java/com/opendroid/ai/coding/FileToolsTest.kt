package com.opendroid.ai.coding

import org.junit.Assert.assertEquals
import org.junit.Test
import java.io.File
import kotlin.io.path.createTempDirectory

class FileToolsTest {

    @Test
    fun createAndReadFile() {
        val root = createTempDirectory().toFile()
        val tools = FileTools()

        tools.createFile(root, "test.txt", "hello")

        assertEquals("hello", tools.readFile(root, "test.txt"))
    }

    @Test(expected = IllegalArgumentException::class)
    fun blocksUnsafePath() {
        val root = createTempDirectory().toFile()
        FileTools().createFile(root, "../escape.txt", "bad")
    }
}
