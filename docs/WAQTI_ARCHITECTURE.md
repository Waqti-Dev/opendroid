# Waqti Architecture Audit and Target

## Audit scope

This is a **Phase 0 audit**, not a migration. No package, application ID, UI, native code, or dependency was changed during this audit.

Repository audited: `Waqti-Dev/opendroid`
Audited branch: `waqti-mvp-sprint`
Audited commit: `4ccf0b56095ca8eda4764a5c1f7e91574123779d`

Tracked inventory at audit time:

| Area | Count | Initial classification |
|---|---:|---|
| Kotlin source under `app/src/main/java` | 283 | Mixed: KEEP/ADAPT/REWRITE/REMOVE |
| Unit tests | 65 | KEEP selectively / ADAPT namespaces |
| Instrumentation tests | 6 | ADAPT selectively |
| Native C/C++ files | 38 | GGUF subset ADAPT; probes/tests selective |
| Android resources | 25 tracked files in the counted resource set | ADAPT/REWRITE |
| Assets | 13 | OpenDroid branding: REMOVE or legal review |
| Documentation | 87 | KEEP selectively; rewrite product docs |

## Current architecture

```text
OpenDroid Android app
├── Compose UI and navigation
├── OpenDroid application/services/accessibility components
├── Agent and action orchestration
├── 12-provider/cloud-heavy LLM layer
├── LiteRT-LM and Android AI Core on-device path
├── Room/DataStore repositories
├── Android Keystore/security and permission policy
├── social/automation/device actions
└── recovered C++ GGUF reference runtime + JNI inspector
```

Current namespace/application identity:

- Kotlin namespace: `com.opendroid.ai`
- application ID: `com.opendroid.aiagent`
- application class: `OpenDroidApp`
- visible app identity and many service/component names: OpenDroid

## Target Waqti architecture

```text
Waqti
├── UI / Waqti navigation / first-run flow
├── Agent
│   ├── bounded loop
│   ├── checkpoints and cancellation
│   ├── tool dispatch and observations
│   └── final response protocol
├── LLM abstraction
│   ├── LocalGGUF (first-class)
│   ├── LiteRT (optional)
│   └── Cloud (optional)
├── Tools
│   ├── workspace files
│   ├── bounded terminal
│   ├── search
│   └── Git/GitHub later, with explicit permissions
├── Memory / session state
├── Runtime
│   └── validated GGUF/native implementation
└── Android platform adapters
```

The Agent must depend on a stable LLM abstraction, not on GGUF or LiteRT implementation details.

## Classification by component

| Component | Current origin/evidence | Waqti disposition | Reason/risk |
|---|---|---|---|
| Gradle Android foundation | OpenDroid-based project history | ADAPT | Retain only after identity and dependency audit |
| Compose/theme/navigation | OpenDroid UI | REWRITE | Waqti must have its own UI, not a rename |
| Onboarding/startup | OpenDroid-specific | REWRITE | New Local GGUF-first flow required |
| Bounded agent loop and checkpoints | Waqti MVP additions on top of foundation | KEEP/ADAPT | Already targeted-tested; isolate from UI |
| Workspace-safe file tools | Waqti MVP additions | KEEP/ADAPT | Preserve canonical-root and permission boundaries |
| Bounded terminal policy | Waqti MVP additions | KEEP/ADAPT | Preserve allowlist and destructive-command rejection |
| Provider/LLM interfaces | Mixed OpenDroid and Waqti additions | ADAPT | Keep abstraction; remove unnecessary provider assumptions |
| Cloud providers | OpenDroid feature set | ADAPT selectively | Optional; not required for Local GGUF |
| LiteRT provider/catalog | OpenDroid on-device path | ADAPT optional | Must not block GGUF; retain only useful abstraction/tests |
| GGUF parser/reference C++ | Recovered Waqti reference branch | ADAPT | Requires JNI bridge, Android adaptation, and more validation |
| Native JNI inspector | Waqti reference implementation | ADAPT | Metadata inspection is not full production inference |
| Qwen transformer/generation reference | Waqti research/reference branch | ADAPT with gate | Numerical parity and exact GGUF remain unverified/unavailable |
| Accessibility/device automation | OpenDroid product scope | REMOVE initially or ADAPT later | High-risk permissions; not needed for Local GGUF file-agent MVP |
| SMS/calls/social/finance actions | OpenDroid product scope | REMOVE from first Waqti slice | Broad permissions and unrelated product surface |
| Memory/security/storage primitives | OpenDroid foundation | ADAPT selectively | Useful after data contracts and legal review |
| Room/DataStore | OpenDroid foundation | ADAPT | Keep only required Waqti state/model registry |
| OpenDroid assets/screenshots/video | OpenDroid branding | REMOVE | Not Waqti identity; asset rights/attribution need separate review |
| Documentation and website | OpenDroid product docs | REWRITE selectively | Preserve historical/legal material only |
| Tests | Mixed | KEEP/ADAPT | Retain tests for retained contracts; do not bulk-copy unrelated features |

## Required target boundaries

1. `Agent` knows only the LLM/tool contracts.
2. `LocalGGUF` owns file validation, metadata, native loading, and inference lifecycle.
3. `Tools` are permissioned and workspace-scoped.
4. UI never assumes Cloud API credentials exist.
5. Local GGUF discovery starts from `*.gguf` in the user model directory and requires validation before load.
6. Production readiness is not claimed from reference tests alone.

## Migration gate

No broad code migration should begin until the audit/dependency/license reports are committed and the new repository is created without touching `opendroid/main` or `waqti-qwen-reference-runtime`.
