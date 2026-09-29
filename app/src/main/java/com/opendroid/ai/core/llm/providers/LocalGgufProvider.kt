package com.opendroid.ai.core.llm.providers

import android.content.Context
import com.opendroid.ai.core.llm.LLMProvider
import com.opendroid.ai.core.llm.LLMRequest
import com.opendroid.ai.core.llm.LLMResponse
import com.opendroid.ai.core.runtime.gguf.LocalGgufModelStore
import com.opendroid.ai.core.runtime.jni.NativeGgufGenerator
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flow
import kotlinx.coroutines.withContext
import javax.inject.Inject
import kotlin.time.Duration.Companion.nanoseconds

class LocalGgufProvider @Inject constructor(
    context: Context
) : LLMProvider {
    private val store = LocalGgufModelStore(context.applicationContext)

    override val name: String = "Local GGUF"
    override val availableModels: List<String>
        get() = listOfNotNull(store.activeModelId())
            .ifEmpty { listOf("qwen2.5-0.5b-instruct-q4_k_m") }

    override suspend fun complete(request: LLMRequest): LLMResponse = withContext(Dispatchers.Default) {
        val modelFile = store.activeFile()
            ?: throw IllegalStateException("No valid GGUF model has been imported.")
        val activeModelId = store.activeModelId()
            ?: throw IllegalStateException("No active GGUF model has been selected.")
        val prompt = request.messages.lastOrNull()?.text?.trim().orEmpty()
        if (prompt.isEmpty()) throw IllegalArgumentException("Local GGUF requires a non-empty prompt.")
        val maxTokens = request.maxTokens.coerceIn(1, 2048)
        val started = System.nanoTime()
        val result = NativeGgufGenerator.generate(modelFile.absolutePath, prompt, maxTokens)
        if (!result.ok) throw IllegalStateException(result.error ?: "Native GGUF generation failed.")
        LLMResponse(
            content = result.generatedText,
            tokensUsed = result.promptTokenIds.size + result.generatedTokenCount,
            model = activeModelId,
            provider = name,
            latencyMs = (System.nanoTime() - started).nanoseconds.inWholeMilliseconds
        )
    }

    override fun streamComplete(request: LLMRequest): Flow<String> = flow {
        emit(complete(request).content)
    }

    override suspend fun isAvailable(): Boolean = store.activeFile() != null
}
