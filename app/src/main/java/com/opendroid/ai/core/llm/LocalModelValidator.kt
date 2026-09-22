package com.opendroid.ai.core.llm

import java.io.File

/**
 * Validates locally imported LLM artifacts before registration.
 * Keeps model import safe and prevents broken entries.
 */
class LocalModelValidator {

    fun validate(file: File): ValidationResult {
        if (!file.exists()) {
            return ValidationResult(false, "Model file does not exist")
        }

        if (!file.isFile) {
            return ValidationResult(false, "Path is not a file")
        }

        if (file.length() <= 0) {
            return ValidationResult(false, "Model file is empty")
        }

        val supported = file.name.endsWith(".gguf", true) ||
                file.name.endsWith(".bin", true) ||
                file.name.endsWith(".tflite", true)

        if (!supported) {
            return ValidationResult(false, "Unsupported model format")
        }

        return ValidationResult(true, "Model is valid")
    }
}

data class ValidationResult(
    val valid: Boolean,
    val message: String
)
