package com.opendroid.ai.core.llm

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

    fun route(task: String, context: String?): String {
        return when (val decision = selectRoute(task)) {
            is RouteDecision.Local -> decision.model
            is RouteDecision.CloudFallback -> "cloud:" + decision.preferredLocalModel
        }
    }
}

sealed interface RouteDecision {
    data class Local(val model: String) : RouteDecision
    data class CloudFallback(val preferredLocalModel: String) : RouteDecision
}
