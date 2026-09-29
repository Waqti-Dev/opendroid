package com.opendroid.ai.core.runtime.gguf

import android.content.Context
import androidx.core.content.edit
import com.opendroid.ai.core.runtime.jni.NativeGgufInspection
import java.io.File

class LocalGgufModelStore(context: Context) {
    private val preferences = context.getSharedPreferences(PREFERENCES, Context.MODE_PRIVATE)

    fun setActive(modelId: String, path: String, inspection: NativeGgufInspection) {
        preferences.edit {
            putString(KEY_MODEL_ID, modelId)
            putString(KEY_PATH, path)
            putString(key(modelId, "architecture"), inspection.architecture)
            putString(key(modelId, "quantization"), inspection.quantization)
            putLong(key(modelId, "context"), inspection.contextLength ?: 0L)
            putLong(key(modelId, "layers"), inspection.layers ?: 0L)
            putLong(key(modelId, "vocab"), inspection.vocabSize ?: 0L)
        }
    }

    fun activeModelId(): String? = preferences.getString(KEY_MODEL_ID, null)

    fun activeFile(): File? = preferences.getString(KEY_PATH, null)
        ?.let(::File)
        ?.takeIf { it.isFile && it.length() > 0L }

    fun metadataFor(modelId: String): StoredGgufMetadata? {
        val architecture = preferences.getString(key(modelId, "architecture"), null) ?: return null
        return StoredGgufMetadata(
            architecture = architecture,
            quantization = preferences.getString(key(modelId, "quantization"), null),
            contextLength = preferences.getLong(key(modelId, "context"), 0L).takeIf { it > 0L },
            layers = preferences.getLong(key(modelId, "layers"), 0L).takeIf { it > 0L },
            vocabSize = preferences.getLong(key(modelId, "vocab"), 0L).takeIf { it > 0L }
        )
    }

    fun clearModel(modelId: String) {
        val isActive = preferences.getString(KEY_MODEL_ID, null) == modelId
        preferences.edit {
            remove(key(modelId, "architecture"))
            remove(key(modelId, "quantization"))
            remove(key(modelId, "context"))
            remove(key(modelId, "layers"))
            remove(key(modelId, "vocab"))
            if (isActive) {
                remove(KEY_MODEL_ID)
                remove(KEY_PATH)
            }
        }
    }

    fun clear() {
        preferences.edit {
            remove(KEY_MODEL_ID)
            remove(KEY_PATH)
        }
    }

    private companion object {
        const val PREFERENCES = "local_gguf_model"
        const val KEY_MODEL_ID = "active_model_id"
        const val KEY_PATH = "active_path"

        fun key(modelId: String, field: String): String = "model_${modelId}_$field"
    }
}

data class StoredGgufMetadata(
    val architecture: String,
    val quantization: String?,
    val contextLength: Long?,
    val layers: Long?,
    val vocabSize: Long?
)
