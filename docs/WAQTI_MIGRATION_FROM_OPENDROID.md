# Waqti Migration From OpenDroid

## Status

**Phase:** 0 — audit, ownership review, and safety checkpoint.

**No migration has started.** The current OpenDroid repository remains the reference/safety source. The new repository target is `Waqti-Dev/waqti`; it was checked and is currently unavailable/not found, so creation is a separate next operation after this audit checkpoint.

## CURRENT

- Repository: `Waqti-Dev/opendroid`
- Branch: `waqti-mvp-sprint`
- HEAD: `4ccf0b56095ca8eda4764a5c1f7e91574123779d`
- `origin`: `https://github.com/Waqti-Dev/opendroid.git`
- `upstream`: `https://github.com/yashab-cyber/opendroid.git`
- `main`: `b89a255b37be33096a1ba67a3cff40efcd919b82`, unchanged
- `waqti-qwen-reference-runtime`: `0ca348d`, historical/reference branch, untouched
- Working tree before audit: clean

## TARGET

A separate `Waqti-Dev/waqti` repository containing a Waqti product, with a clear migration commit and preserved relevant Waqti history. Development should move there only after the new remote is verified.

## FILES / AREAS AFFECTED (planned, not yet changed)

- Root Gradle/settings identity and application configuration
- Kotlin namespace and Android application ID, after a separate impact plan
- Android Manifest component names and permissions
- Compose UI, navigation, theme, onboarding, settings, and visible strings
- Agent/LLM/tool boundaries
- GGUF discovery/validation/JNI/native runtime integration
- Tests for retained contracts
- README, release docs, credits, NOTICE/third-party license records
- OpenDroid branding assets and user-facing strings

## WHAT IS INHERITED

The project is demonstrably OpenDroid-based: its license identifies OpenDroid Contributors, its namespace/application identity is OpenDroid, its README and assets are OpenDroid-branded, and most Android Kotlin packages are `com.opendroid.ai`.

Potentially reusable portions are the Android foundation, selected agent/provider/storage/security primitives, and targeted tests. The Waqti bounded agent/tool work and recovered GGUF reference work are distinct Waqti additions and must remain clearly identified in history.

## WHAT WILL BE MODIFIED

- Product identity and UI will be rewritten for Waqti.
- Provider routing will be reduced to Local GGUF first, LiteRT optional, Cloud optional.
- Model import/storage contracts will be redesigned around validated `.gguf` discovery.
- Retained code will move behind Waqti-owned boundaries rather than preserving OpenDroid assumptions.

## WHAT WILL BE REMOVED OR DEFERRED

OpenDroid branding/assets, unrelated social/finance/communication automation, broad device-control permissions, and LiteRT-only assumptions that reject GGUF will not be copied into the first Waqti vertical slice. Any later retention requires a separate product and permission review.

## OWNERSHIP / LEGAL GATE

The repository contains an Apache License 2.0 file with explicit requirements to:

- distribute a copy of the license;
- mark modified files with prominent notices;
- retain copyright, patent, trademark, and attribution notices that pertain to retained derivative files;
- preserve NOTICE contents if a NOTICE file exists.

No repository-level NOTICE file was found in the audited tree. C++ reference files carry `SPDX-License-Identifier: Apache-2.0`. The Waqti repository must keep the license and add Waqti modification notices where required; it must not imply that the OpenDroid trademark/branding was transferred.

Third-party dependency license claims in the current `LicenseScreen.kt` are not sufficient as an automated legal inventory. A generated dependency notice/license report is required before release.

## RISK

| Risk | Level | Control |
|---|---|---|
| Package/application ID migration breaks manifest, JNI, tests, providers, deep links | High | separate impact map, compile, install, and instrumentation checks |
| Copying OpenDroid UI/branding makes Waqti a cosmetic fork | High | rewrite UI shell and remove user-facing identity before release |
| License/asset provenance is incomplete | High | retain Apache license, audit headers/assets/dependencies, add NOTICE/credits |
| GGUF reference is mistaken for production inference | High | exact model SHA gate, parity tests, device smoke test, no readiness claim before evidence |
| Broad privileged permissions leak into first MVP | High | start with local file-agent slice; add permissions only with feature justification |
| History/remote migration loses current progress | High | pushed checkpoint, new repo verification, no reset/clean/force push |
| Existing lint/Maven issues obscure migration failures | Medium | track separately; run targeted tests only |

## ROLLBACK PLAN

1. Stop at the current pushed commit/branch if any migration conflict appears.
2. Keep `Waqti-Dev/opendroid` and `waqti-qwen-reference-runtime` intact.
3. Keep each Waqti milestone as a commit and push it before the next milestone.
4. Repoint local work to the last verified Waqti commit or clone the new repository again; do not use `reset --hard`, `clean`, or force push.
5. If the new repository creation or remote verification is ambiguous, stop before pushing code.

## Planned migration order

1. Phase 0 audit, license map, and checkpoint.
2. Create empty `Waqti-Dev/waqti` only after verifying availability.
3. Migrate history safely with an explicit migration commit.
4. Waqti UI/branding shell.
5. Model/runtime abstraction.
6. GGUF discovery and validation.
7. Native runtime integration.
8. Agent ↔ Local GGUF vertical slice.
9. Targeted verification and device testing.
10. Credits, notices, release documentation, and handoff.
