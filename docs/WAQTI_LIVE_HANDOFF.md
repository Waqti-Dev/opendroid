# WAQTI LIVE HANDOFF

- Last Updated (UTC): 2026-09-27T15:07:40Z
- Branch: `waqti-qwen-reference-runtime`
- Golden correction commit: `e908749` (`fix: correct independent qwen swiglu golden`)
- Remote commit: `b86b7190c1eb3e4500ad9b183cadc0d0316330e8` (verified on `origin/waqti-qwen-reference-runtime`)
- Previous known-good base: `b89a255b37be33096a1ba67a3cff40efcd919b82` (`main`)

## Current Phase

Exact GGUF verification; model-level numerical verification remains blocked.

## Current Checkpoint

- Local preservation snapshot: `/home/ubuntu/WAQTI_CHECKPOINT_SNAPSHOT_20260927_1454/`
- Source manifest entries: 79
- Copied artifacts: 79
- Snapshot manifest SHA-256: `adac3a218eb15ec0ff96bffaf355a25c28f35c9f24f302bf1ec293e4788cfe05`
- This branch contains native C++ files under `app/src/main/cpp/` and an exact artifact archive under `docs/waqti-runtime-checkpoint/recovered-artifacts/`.

## Verified in Current Recovery Environment

- Source manifest resolves to 79/79 present files.
- Copied artifacts match their source SHA-256 and size.
- Direct `g++` manual tests passed for the recovered native test executables.
- C++ source syntax checks passed with `-Wall -Wextra -Wpedantic`.
- Original recovered directories remained unchanged during snapshot verification.
- Golden Python AST syntax check: PASS.
- Model-independent SwiGLU expression unit check: PASS.
- `git diff --check`: PASS.

## Golden Correction

- File: `docs/waqti-runtime-checkpoint/recovered-artifacts/010_waqti_multitoken_golden.py`
- Old: `sigmoid(gate) * up`
- New: `gate * sigmoid(gate) * up` (`SiLU(gate) * up`)
- Golden status: CORRECTED / UNVERIFIED.

## Exact GGUF Verification

- Filename: `qwen2.5-0.5b-instruct-q4_k_m.gguf`
- Required SHA-256: `74a4da8c9fdbcd15bd1f6d01d621410d31c6fc00986f5eb687824e7b93d7a9db`
- Search result: `EXACT GGUF UNAVAILABLE`.
- Search scope: `/home/ubuntu`, `/tmp`, `/mnt`, `/workspace`, `/workspaces`, `/opt`, `/srv`, Downloads, uploads, recovered task, recovered project, and preservation snapshot.
- Numerical parity: BLOCKED until the exact file is found and its SHA-256 matches.

## Unverified / Not Claimed

- The corrected golden has not been executed against the exact model.
- CTest 8/8 has not been rerun in this environment.
- Generation source is present but not integrated/tested in CMake.
- Full recomputation versus cached incremental equivalence is unverified.
- JNI, Android inference, device validation, and performance are unverified.

## Blocked

- Numerical verification: BLOCKED — exact GGUF unavailable.
- CTest reproduction: BLOCKED — `cmake`/`ctest` unavailable in the current sandbox.

## Generation / KV / Android

- Generation: PRESENT / UNVERIFIED
- KV cache: PRESENT / structural manual tests only; full equivalence UNVERIFIED
- JNI: not integrated for this runtime checkpoint
- Android inference: unverified

## Tests Recorded

- Manual direct-compiler tests: 9 native test programs plus benchmark executed; all returned success.
- Syntax checks: native runtime source files passed.
- Golden AST syntax: PASS.
- SwiGLU expression unit check: PASS.
- `git diff --check`: PASS.
- CTest: not run; do not infer CTest status from manual tests.

## Exact Next Action

Obtain/verify the exact Qwen GGUF SHA-256 before executing independent Python↔C++ parity.
