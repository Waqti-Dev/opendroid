package com.opendroid.ai.ui.screens

import android.net.Uri
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.hilt.navigation.compose.hiltViewModel
import com.opendroid.ai.core.llm.OnDeviceModelRegistry
import com.opendroid.ai.data.db.entities.ModelEntity
import com.opendroid.ai.data.db.entities.ModelStatus
import com.opendroid.ai.ui.theme.AccentCyan
import com.opendroid.ai.ui.theme.CardBackground
import com.opendroid.ai.ui.theme.DarkBackground
import com.opendroid.ai.ui.theme.TextPrimary
import com.opendroid.ai.ui.theme.TextSecondary
import com.opendroid.ai.ui.viewmodel.SettingsViewModel

@Composable
fun ModelsScreen(
    viewModel: SettingsViewModel = hiltViewModel(),
    onNavigateBack: () -> Unit
) {
    val models by viewModel.allModels.collectAsState()
    val config by viewModel.llmConfig.collectAsState()
    val importStatus by viewModel.localImportStatus.collectAsState()
    val picker = rememberLauncherForActivityResult(ActivityResultContracts.OpenDocument()) { uri: Uri? ->
        uri?.let(viewModel::importCustomLocalModel)
    }
    val installed = models.filter { OnDeviceModelRegistry.isCustomId(it.id) }

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("Models", color = TextPrimary) },
                navigationIcon = {
                    IconButton(onClick = onNavigateBack) {
                        Icon(Icons.Default.ArrowBack, contentDescription = "Back", tint = TextPrimary)
                    }
                }
            )
        },
        containerColor = DarkBackground
    ) { padding ->
        LazyColumn(
            modifier = Modifier.fillMaxSize().padding(padding).padding(horizontal = 16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            item {
                Spacer(Modifier.height(4.dp))
                Text("INSTALLED MODELS", color = AccentCyan, fontSize = 12.sp)
                Text(
                    "Import a GGUF from Android storage. The app copies it to private storage, validates it with the native inspector, and keeps only metadata in preferences.",
                    color = TextSecondary,
                    fontSize = 12.sp,
                    modifier = Modifier.padding(top = 6.dp)
                )
                Spacer(Modifier.height(10.dp))
                Button(
                    onClick = { picker.launch(arrayOf("*.gguf")) },
                    modifier = Modifier.fillMaxWidth(),
                    colors = ButtonDefaults.buttonColors(containerColor = AccentCyan)
                ) { Text("Import GGUF", color = DarkBackground) }
                importStatus?.let {
                    Text(it, color = if (it.contains("failed", true) || it.contains("unavailable", true)) Color.Red else AccentCyan, fontSize = 12.sp, modifier = Modifier.padding(top = 8.dp))
                }
            }

            if (installed.isEmpty()) {
                item { Text("No imported GGUF models", color = TextSecondary, modifier = Modifier.padding(vertical = 24.dp)) }
            } else {
                items(installed, key = { it.id }) { entity ->
                    ModelCard(
                        entity = entity,
                        active = config.activeProvider == "Local GGUF" && config.activeModel == entity.id,
                        metadata = viewModel.modelRepository.localGgufMetadata(entity.id),
                        onActivate = { viewModel.activateLocalGguf(entity.id) },
                        onDelete = { viewModel.deleteModel(entity.id) }
                    )
                }
            }
        }
    }
}

@Composable
private fun ModelCard(
    entity: ModelEntity,
    active: Boolean,
    metadata: com.opendroid.ai.core.runtime.gguf.StoredGgufMetadata?,
    onActivate: () -> Unit,
    onDelete: () -> Unit
) {
    Card(colors = CardDefaults.cardColors(containerColor = CardBackground), modifier = Modifier.fillMaxWidth()) {
        Column(Modifier.padding(14.dp)) {
            Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                Column(Modifier.weight(1f)) {
                    Text(entity.name, color = TextPrimary, fontSize = 15.sp)
                    Text(
                        "GGUF · ${metadata?.quantization ?: "metadata pending"} · ${metadata?.architecture ?: "unknown architecture"}",
                        color = TextSecondary,
                        fontSize = 12.sp,
                        modifier = Modifier.padding(top = 4.dp)
                    )
                    Text(
                        "${formatModelBytes(entity.size)} · ${if (entity.status == ModelStatus.READY) "Validated" else entity.status.name}",
                        color = if (entity.status == ModelStatus.READY) AccentCyan else TextSecondary,
                        fontSize = 11.sp,
                        modifier = Modifier.padding(top = 4.dp)
                    )
                }
                if (active) Icon(Icons.Default.Check, contentDescription = "Active", tint = AccentCyan)
            }
            Spacer(Modifier.height(10.dp))
            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                Button(
                    onClick = onActivate,
                    enabled = entity.status == ModelStatus.READY && !active,
                    modifier = Modifier.weight(1f),
                    colors = ButtonDefaults.buttonColors(containerColor = AccentCyan)
                ) { Text(if (active) "Active for Chat" else "Use for Chat", color = DarkBackground, fontSize = 12.sp) }
                IconButton(onClick = onDelete) {
                    Icon(Icons.Default.Delete, contentDescription = "Delete model", tint = Color.Red)
                }
            }
        }
    }
}

private fun formatModelBytes(bytes: Long): String = when {
    bytes >= 1024L * 1024L * 1024L -> "%.1f GB".format(bytes / (1024.0 * 1024.0 * 1024.0))
    bytes >= 1024L * 1024L -> "%.1f MB".format(bytes / (1024.0 * 1024.0))
    else -> "$bytes B"
}
