package com.opendroid.ai.core.runtime.gguf

import android.content.Context
import androidx.core.content.edit
import java.io.File

class LocalGgufModelStore(context: Context) {
    private val preferences = context.getSharedPreferences(PREFERENCES, Context.MODE_PRIVATE)

    fun setActive(modelId: String, path: String) {
        preferences.edit {
            putString(KEY_MODEL_ID, modelId)
            putString(KEY_PATH, path)
        }
    }

    fun activeModelId(): String? = preferences.getString(KEY_MODEL_ID, null)

    fun activeFile(): File? = preferences.getString(KEY_PATH, null)
        ?.let(::File)
        ?.takeIf { it.isFile && it.length() > 0L }

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
    }
}
