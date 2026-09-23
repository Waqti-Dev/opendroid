package com.opendroid.ai.core.llm

/**
 * Coordinates local model execution with cloud fallback decisions.
 * Keeps routing decisions separate from model runtimes.
 */
class HybridRouterV2(
    private val resourceMonitor: ModelResourceMonitor,
    private val smartSelector: SmartModelSelector
) {
    fun selectRoute(task: String): RouteDecision {
        val model = smartSelector.select(task)
        return if (resourceMonitor.canRun(model)) {
            RouteDecision.Local(model)
        } else {
            RouteDecision.CloudFallback(model)
        }
    }
}

sealed interface RouteDecision {
    data class Local(val model: String) : RouteDecision
    data class CloudFallback(val preferredLocalModel: String) : RouteDecision
}
