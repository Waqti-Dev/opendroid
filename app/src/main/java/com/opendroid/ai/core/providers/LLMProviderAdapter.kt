package com.opendroid.ai.core.providers

import com.opendroid.ai.core.llm.LLMProvider
import com.opendroid.ai.core.llm.LLMRequest
import com.opendroid.ai.data.models.ChatMessage

/**
 * Bridges the production LLMProvider stack into the autonomous ProviderManager.
 */
class LLMProviderAdapter(
    private val delegate: LLMProvider,
    private val model: String? = null
) : Provider {

    override val id: String = "llm:" + delegate.name
    override val displayName: String = delegate.name

    override suspend fun isAvailable(): Boolean = delegate.isAvailable()

    override suspend fun generate(prompt: String): String {
        val request = LLMRequest(
            systemPrompt = "You are an autonomous coding agent for Android.",
            messages = listOf(
                ChatMessage(
                    id = "agent-prompt",
                    text = prompt,
                    sender = ChatMessage.Sender.USER
                )
            ),
            model = model
        )
        return delegate.complete(request).content
    }
}
