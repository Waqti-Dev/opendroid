package com.opendroid.ai.core.models

/**
 * Waqti Agent / Open Droid
 * Forked by Ahmed Badr
 */
data class ModelInfo(
    val id: String,
    val name: String,
    val sizeGb: Float,
    val contextSize: Int,
    val local: Boolean,
    val available: Boolean = false
)
