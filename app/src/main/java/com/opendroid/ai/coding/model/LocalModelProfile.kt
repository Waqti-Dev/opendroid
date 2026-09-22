package com.opendroid.ai.coding.model

/**
 * Local model profiles for Waqti Agent / OpenDroid.
 *
 * 0.5B is intentionally excluded because it is not suitable for the
 * Arabic coding assistant workflow.
 */
enum class LocalModelProfile(
    val modelName: String,
    val purpose: String,
    val recommended: Boolean
) {
    QWEN_3B(
        modelName = "3B",
        purpose = "Fast assistant, simple coding tasks, Arabic interaction",
        recommended = true
    ),
    QWEN_7B(
        modelName = "7B",
        purpose = "Primary coding agent model",
        recommended = true
    ),
    QWEN_15B(
        modelName = "15B",
        purpose = "Heavy reasoning, debugging and architecture tasks",
        recommended = true
    )
}
