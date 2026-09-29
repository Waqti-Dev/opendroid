# WAQTI LIVE HANDOFF

- Last Updated (UTC): 2026-09-27T17:58:45Z
- Branch: `waqti-mvp-sprint`
- Current committed base: `3b0bb335450b2ed209a212ee2ef9284223c0e15f`
- Previous Qwen branch: `waqti-qwen-reference-runtime`
- Main remains unchanged at `b89a255b37be33096a1ba67a3cff40efcd919b82`.

## Current phase

MVP integration/build verification. Android SDK is installed locally, debug assembly passes, and the approved test-fixture-only fix is verified. Full unit-test evidence remains recorded separately.

## What was run

1. Installed the minimum SDK required by the current project: `platforms;android-36`, `build-tools;36.0.0`, and `platform-tools` under `/home/ubuntu/Android/Sdk`. Local `local.properties` points to this SDK and is untracked.
2. Ran `./gradlew :app:assembleDebug --no-daemon`: **PASS** after fixing two nullable argument compile errors in `AutonomousAgentEngine.kt`.
3. Ran `./gradlew :app:testDebugUnitTest --no-daemon` exactly once after build setup: **FAIL**, 515 tests completed, 5 failed, duration 2h19m34s.

## Full-suite failures

Four tests fail through the same Robolectric dependency resolver path: `ResourceCleanupTest`, `FinanceActionsTest`, `MediaActionsTest`, and `ToggleBluetoothActionTest`. The common artifact is `org.robolectric:android-all-instrumented:15-robolectric-13954326-i7`. The common exception is a SHA-512 mismatch for its POM. The local `.pom.sha512` sidecar begins with the JAR checksum and is concatenated with the POM checksum; Robolectric treats the first 128 characters as the POM checksum. A direct HTTP 200 fetch from Maven Central succeeded, and the remote/local POM body matches at SHA-512 `e16eb42ba12b823ede906bec317f73688c58d05ebc7f6f4c39ff555c08926515aac59dbddddd888b87c85a022d756d6aa1520cc0276e2adee2caf946ad79e659`. Classification: **external Maven/Robolectric cache metadata issue**.

The MVP failure was `AutonomousAgentEngineTest.createUserFile_throughToolCall`, expected `COMPLETED` but got `FAILED`. The provider fixture returned the same `WriteFile` JSON forever. The approved fix makes the fake return `WriteFile` once and a final assistant response on the next call. The targeted test passed in 33 seconds and the complete `AutonomousAgentEngineTest` class passed in 20 seconds. Production agent code was not changed for this fix.

Full evidence and possible fixes are in `docs/WAQTI_FAILURE_ROOT_CAUSE_REPORT.md`.

## Current file state

The source changes after the committed MVP milestone are the already-verified nullable fix in `AutonomousAgentEngine.kt` that made `assembleDebug` pass and the approved test-only FakeProvider sequencing fix. No Robolectric dependency, cache, Qwen source, golden, KV, JNI, or Android native runtime has been changed.

## Exact next command

The targeted fixture verification already passed:

```text
./gradlew :app:testDebugUnitTest --tests '*AutonomousAgentEngineTest.createUserFile_throughToolCall'
```

The related agent test class also passed:

```text
./gradlew :app:testDebugUnitTest --tests 'com.opendroid.ai.core.agent.AutonomousAgentEngineTest'
```

For Maven, use a preserved isolated cache or carefully repair only the affected checksum sidecar, then run one representative Robolectric test. Do not delete the existing cache and do not rerun the 515-test suite yet.

## Remaining status

- Android SDK: PASS.
- Debug compilation/assemble: PASS.
- Unit tests: Full suite FAIL due to four external Maven/Robolectric failures; MVP targeted tests PASS.
- Targeted MVP retest: PASS.
- Related `AutonomousAgentEngineTest` class: PASS.
- Lint: FAIL — 40 errors and 1 hint; first failure is pre-existing `DefaultLocale` at `app/src/main/java/com/opendroid/ai/actions/SocialActions.kt:52`. No unrelated lint fixes were applied.
- APK: PASS — `app/build/outputs/apk/debug/app-debug.apk`, 73,854,374 bytes, produced by `assembleDebug`.
- Device smoke test: NOT RUN.
- Exact Qwen GGUF: UNAVAILABLE.
- Qwen parity: BLOCKED.

## Phase 0 Migration Audit — 2026-09-27

The requested independent-product migration audit is complete. No migration or broad source-code change has started.

- Current repository: `Waqti-Dev/opendroid`
- Current branch: `waqti-mvp-sprint`
- Working tree at audit start: clean
- Current product identity: `com.opendroid.ai` / `com.opendroid.aiagent`; OpenDroid appears in package names, Android components, UI, strings, assets, docs, and README.
- Tracked inventory: 283 main Kotlin files, 65 unit tests, 6 instrumentation tests, 38 native C/C++ files, 13 assets, and 87 docs.
- Existing root license: Apache License 2.0, Copyright 2026 OpenDroid Contributors.
- No repository-level `NOTICE` or `THIRD_PARTY_LICENSES` file was found; dependency license inventory is required before release.
- Native recovered GGUF files carry `SPDX-License-Identifier: Apache-2.0` and remain reference/research code until exact-model parity and device validation gates pass.
- Existing app is LiteRT-oriented; it has GGUF format/runtime selection scaffolding but explicitly rejects GGUF in the current LiteRT import flow. Local GGUF is not yet a production path.
- `Waqti-Dev/waqti`: checked with GitHub CLI and not found. No new repository was created before recording this audit checkpoint.
- `Waqti-Dev/waqti`: created as a new **private, empty** repository at https://github.com/Waqti-Dev/waqti after the audit checkpoint. No code has been pushed to it yet.

Required audit documents now present:

- `docs/WAQTI_ARCHITECTURE.md`
- `docs/WAQTI_MIGRATION_FROM_OPENDROID.md`
- `docs/WAQTI_OPEN_SOURCE_CREDITS.md`
- `docs/WAQTI_LOCAL_GGUF.md`
- `docs/WAQTI_LINT_BACKLOG.md`

Migration rule: keep `opendroid/main` and `waqti-qwen-reference-runtime` unchanged; create and verify `Waqti-Dev/waqti` before pushing migrated code. Do not perform package/application-ID changes until their impact map and rollback checkpoint are approved by the next milestone evidence.

## Repository verification

- New repository: `Waqti-Dev/waqti`
- Visibility: private
- Default branch: not initialized yet (empty repository)
- Old `opendroid/main`: unchanged
- Old `waqti-qwen-reference-runtime`: untouched
- Current old-repository audit checkpoint: `a117c2a843a745e52e61d92b8f5b2ad552355d50`


## Model Management + GGUF Import Checkpoint — 2026-09-29

- **Branch:** `waqti-mvp-v1`
- **Commit:** `ba8ea5a`
- **Architecture audit:** the app uses Jetpack Compose, a single `MainDashboard` tab shell, `SettingsViewModel`, Room `ModelDao`/`ModelRepository`, `SettingsRepository` for provider/model config, Hilt provider injection, `LLMProviderFactory`, and the native `waqti_runtime` JNI bridge.

### Verified in source

- Settings now has a real `Models` entry routed to `ModelsScreen`.
- `ModelsScreen` uses Android `OpenDocument` with a `.gguf` filter.
- Import streams the selected `content://` URI into app-managed private model storage; the model bytes are not stored in Kotlin memory or preferences.
- `ModelRepository` validates the copied file using `NativeGgufInspector.inspectFile`, requires `ok && inferenceSupported`, registers a Room model row, and persists the active GGUF path plus native metadata.
- Stored metadata includes architecture, quantization label, context length, layer count, and vocabulary size.
- The user can see installed custom models, native metadata, validation status, set one as the Local GGUF Chat model, and delete it.
- Local GGUF provider resolves the persisted active file and returns the persisted model ID; it does not silently switch to a cloud provider on native failure.

### Verification

- `./gradlew assembleDebug`: **PASS**
- `./gradlew testDebugUnitTest lintDebug`: **PASS**; lint reports one hint plus 33 findings covered by the existing baseline.
- `./gradlew assembleRelease -PallowUnsignedRelease=true`: **PASS**
- Native CTest: **8/8 PASS**
- ASAN CTest: **8/8 PASS**
- UBSAN CTest: **8/8 PASS**
- Real Android device execution: **NOT VERIFIED** — `adb` is unavailable in the sandbox and no authorized Android device was available.

### Artifacts and limitations

- Debug APK: `app/build/outputs/apk/debug/app-debug.apk`
- Unsigned release APK: `app/build/outputs/apk/release/app-release-unsigned.apk`
- The full Settings → Models → Import GGUF → validate → install → select → Chat flow is implemented and build-verified, but the end-to-end Android device flow and generation through the installed APK remain **NOT VERIFIED**.
