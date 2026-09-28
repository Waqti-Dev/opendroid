# Waqti Open-Source Credits and Ownership Map

## Audit status

This is the Phase 0 ownership map. It is not a final generated third-party license bundle. Before a public Waqti release, generate and review dependency notices from the resolved Gradle graph and preserve each dependency's license text.

## Project code

| Component | Origin | License evidence | Can copy/modify? | Attribution/NOTICE action |
|---|---|---|---|---|
| Selected Android foundation | OpenDroid repository | Root `LICENSE`: Apache License 2.0; Copyright 2026 OpenDroid Contributors | Yes, subject to Apache conditions | Keep `LICENSE`; retain relevant notices; mark modified files where required |
| Waqti bounded agent/tool additions | Waqti development commits on OpenDroid branch | Root project Apache 2.0 unless a file states otherwise | Yes, subject to same project terms | Preserve history and identify Waqti modifications |
| GGUF C++ reference files | Waqti reference branch; files carry `SPDX-License-Identifier: Apache-2.0` | Per-file SPDX headers | Yes, subject to Apache 2.0 | Keep SPDX headers; document reference/validation status |
| OpenDroid logo/screenshots/video | `assets/`, explicitly described as OpenDroid branding | Asset README points to project Apache license, but trademark/third-party provenance still needs review | Do not copy into Waqti user-facing product by default | Remove/defer; preserve only historical/legal references if needed |
| OpenDroid name/trademark | README, package, services, assets | Apache 2.0 does not grant trademark permission | No blanket reuse as Waqti branding | Retain only in historical attribution and migration docs |

## Direct dependencies declared in Gradle

The current project declares AndroidX, Jetpack Compose, Room, WorkManager, DataStore, Hilt, OkHttp/Retrofit, Shizuku, Kotlin Serialization, Coil, Lottie, Google ML Kit GenAI, LiteRT-LM, JUnit, Robolectric, AndroidX testing, and related artifacts.

Most declared libraries are commonly distributed under Apache 2.0 or permissive licenses, but **this document does not make a final license claim for every artifact**. Version-specific license files must be collected from the resolved dependency graph, including transitive dependencies.

Required pre-release action:

1. Resolve `debugRuntimeClasspath`, `releaseRuntimeClasspath`, and test configurations.
2. Generate a machine-readable dependency inventory with group, artifact, version, repository, license, and license URL.
3. Copy the actual license texts into `THIRD_PARTY_LICENSES/` or an equivalent documented bundle.
4. Review Shizuku, LiteRT/Google components, ML Kit, model/tokenizer code, and any downloaded model licenses separately.
5. Add a Waqti About → Credits & Licenses screen backed by the same reviewed inventory.

## Model and asset licensing gate

A GGUF file is not automatically licensed like the runtime that loads it. For every supported model, Waqti must record:

- model name and publisher;
- exact source URL;
- model license and usage restrictions;
- SHA-256;
- tokenizer/license relationship;
- whether redistribution is permitted.

The required Qwen GGUF is currently unavailable locally and has not been copied into GitHub.

## Required notices for Waqti

The new repository should contain:

- `LICENSE` — retained Apache 2.0 project license with accurate Waqti modification attribution;
- `NOTICE` — project and retained upstream attribution where applicable;
- `THIRD_PARTY_LICENSES/` — generated dependency license texts;
- `docs/WAQTI_OPEN_SOURCE_CREDITS.md` — human-readable ownership map.

Do not claim that a UI credits page alone satisfies source redistribution conditions.
