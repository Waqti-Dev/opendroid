package com.opendroid.ai.core.runtime.device

import android.app.ActivityManager
import android.content.Context
import android.content.pm.PackageManager
import android.os.Build
import android.os.PowerManager

/** Runtime capability snapshot; values are observed from this device, not inferred from marketing specs. */
data class WaqtiDeviceProfile(
    val supportedAbis: List<String>,
    val isArm64: Boolean,
    val totalRamBytes: Long,
    val availableRamBytes: Long,
    val thermalStatus: Int?,
    val apiLevel: Int,
    val hasVulkanApi: Boolean,
    val recommendedBackend: Backend,
    val recommendedThreads: Int,
    val recommendedContextTokens: Int,
) {
    enum class Backend { CPU_NEON, VULKAN_EXPERIMENTAL, NPU_PROBE_REQUIRED }

    fun shouldThrottle(): Boolean = thermalStatus != null && thermalStatus >= PowerManager.THERMAL_STATUS_MODERATE

    companion object {
        fun collect(context: Context): WaqtiDeviceProfile {
            val activityManager = context.getSystemService(ActivityManager::class.java)
            val memory = ActivityManager.MemoryInfo().also(activityManager::getMemoryInfo)
            val abis = Build.SUPPORTED_ABIS.toList()
            val arm64 = abis.any { it == "arm64-v8a" }
            val power = context.getSystemService(PowerManager::class.java)
            val thermal = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) power?.currentThermalStatus else null
            // Vulkan must be probed separately through the Android package manager/instance creation.
            val vulkan = context.packageManager.hasSystemFeature(PackageManager.FEATURE_VULKAN_HARDWARE_LEVEL)
            val cores = Runtime.getRuntime().availableProcessors().coerceAtLeast(1)
            val threads = if (arm64) cores.coerceAtMost(6).coerceAtLeast(2) else cores.coerceAtMost(4)
            val contextTokens = when {
                memory.availMem < 2L * 1024 * 1024 * 1024 -> 2048
                memory.availMem < 4L * 1024 * 1024 * 1024 -> 4096
                else -> 8192
            }
            return WaqtiDeviceProfile(
                supportedAbis = abis,
                isArm64 = arm64,
                totalRamBytes = memory.totalMem,
                availableRamBytes = memory.availMem,
                thermalStatus = thermal,
                apiLevel = Build.VERSION.SDK_INT,
                hasVulkanApi = vulkan,
                recommendedBackend = Backend.CPU_NEON,
                recommendedThreads = threads,
                recommendedContextTokens = contextTokens,
            )
        }
    }
}

class RuntimeThermalPolicy {
    enum class Mode { FULL, REDUCED, PAUSED }

    fun mode(status: Int?): Mode = when {
        status == null || status <= PowerManager.THERMAL_STATUS_LIGHT -> Mode.FULL
        status <= PowerManager.THERMAL_STATUS_SEVERE -> Mode.REDUCED
        else -> Mode.PAUSED
    }
}
