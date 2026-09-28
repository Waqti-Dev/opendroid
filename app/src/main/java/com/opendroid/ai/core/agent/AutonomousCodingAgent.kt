package com.opendroid.ai.core.agent

import android.content.Context
import com.opendroid.ai.core.providers.LLMProviderAdapter
import com.opendroid.ai.core.providers.ProviderManager
import com.opendroid.ai.core.llm.LLMProviderFactory
import com.opendroid.ai.core.storage.StorageWorkspaceProvider
import com.opendroid.ai.core.tools.DefaultToolExecutor
import com.opendroid.ai.core.tools.ToolPermissionManager
import javax.inject.Inject
import javax.inject.Singleton

/**
 * Android entry point for Waqti's bounded coding-agent MVP.
 *
 * The existing chat/LLM stack remains unchanged. Callers explicitly opt into
 * the coding agent, which then runs the provider -> tool -> observation loop.
 */
@Singleton
class AutonomousCodingAgent @Inject constructor(
    private val providerFactory: LLMProviderFactory,
    private val applicationContext: Context
) {
    suspend fun execute(prompt: String, taskId: String): AgentExecutionResult {
        val providerManager = ProviderManager().apply {
            register(LLMProviderAdapter(providerFactory.getActiveProvider()))
        }
        val workspace = StorageWorkspaceProvider.getDefaultWorkspaceDir(applicationContext)
        val executor = DefaultToolExecutor(workspaceRoot = workspace)
        val engine = AutonomousAgentEngine(
            providerManager = providerManager,
            toolExecutionLayer = executor,
            permissionManager = ToolPermissionManager()
        )
        return engine.execute(prompt = prompt, taskId = taskId)
    }
}
