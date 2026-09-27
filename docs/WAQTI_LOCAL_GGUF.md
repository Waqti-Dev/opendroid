# Waqti Local GGUF Audit

## Current status

**Local GGUF is not yet a working product path.** The current app has model abstractions and LiteRT-LM support, plus a recovered native GGUF reference implementation, but the exact Qwen test model is unavailable and no real local GGUF → Agent vertical slice has been demonstrated.

Required test model:

```text
qwen2.5-0.5b-instruct-q4_k_m.gguf
SHA-256: 74a4da8c9fdbcd15bd1f6d01d621410d31c6fc00986f5eb687824e7b93d7a9db
```

Status of exact file: **UNAVAILABLE**. Do not substitute another model.

## Existing current-app support

The current OpenDroid-based app has:

- `LocalModelImporter` and model format classification including `GGUF`;
- `RuntimeSelector` that recognizes `gguf` as a llama-cpp runtime;
- `ModelRuntime` abstraction;
- model registry/repository/download/storage abstractions;
- LiteRT-LM provider and catalog;
- an explicit current error path saying GGUF is not supported by the LiteRT import flow.

This is useful scaffolding, but it is not proof that Android GGUF inference is integrated.

## Recovered reference runtime classification

| Area | Classification | Readiness |
|---|---|---|
| GGUF header/metadata reader | Directly reusable after API isolation | Reference-tested; needs Android error/reporting tests |
| mmap/file abstraction | Reusable with Android storage/URI adaptation | Needs scoped-storage and SAF validation |
| tensor directory/dequantization | Reusable candidate | Needs device performance and malformed-file tests |
| F32/F16/Q4_K/Q5_0/Q6_K/Q8_0 paths | Candidate runtime components | Quantization parity and coverage must be proven per type |
| embedding/RMSNorm/RoPE/GQA/attention/SwiGLU/residual/LM head | Reference implementation | Needs exact-model numerical parity and device testing |
| KV cache/sequence | Reference implementation | Needs multi-turn and cancellation tests |
| generation | Preliminary/reference | Not production-ready until exact GGUF parity is verified |
| JNI bridge | Needs Android adaptation | Must define stable Kotlin error/result API and lifecycle |
| C++ probes/tests | Keep in reference/test area | Do not ship all probes in production APK |

## Target user flow

```text
/storage/emulated/0/Download/models/*.gguf
        ↓
Discover
        ↓
Select Model
        ↓
Validate magic/version/metadata/file bounds
        ↓
Read and show metadata
        ↓
Load through isolated runtime
        ↓
Run inference smoke test
        ↓
Mark Ready
```

## Hard gates

1. Exact file SHA-256 must match the required value for the Qwen parity track.
2. Python ↔ C++ parity must be measured on the exact file.
3. JNI/native failures must become safe Kotlin errors, not process crashes.
4. Model load must be cancellable and must not block the main thread.
5. The Agent must consume an LLM interface, not GGUF-specific classes.
6. A real vertical slice must prove `User → Local GGUF → tool call → observation → Local GGUF → final response`.
7. No production-readiness claim may be based only on recovered reference tests.

## First Local GGUF milestone after migration

Build only the discovery/validation/metadata path first, with tests for:

- valid GGUF header;
- invalid magic/version;
- truncated file;
- unsupported tensor type;
- metadata extraction;
- directory permission failure;
- exact SHA verification;
- cancellation and cleanup.

Inference integration comes only after the repository migration and model availability gate.
