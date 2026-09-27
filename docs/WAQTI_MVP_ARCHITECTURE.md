# Waqti MVP Architecture

## Scope

This document describes the Android MVP track on `waqti-mvp-sprint`. The recovered local-Qwen runtime remains a separate correctness track; the unavailable exact GGUF blocks numerical parity but does not block the cloud-backed MVP.

## Current architecture map

| Area | Status | Evidence / reuse decision |
|---|---|---|
| Android shell | PRESENT | `MainActivity`, Compose navigation, Hilt entry point |
| Chat UI | PRESENT | `ChatScreen`, `ChatViewModel`, session switching, retry/stop affordances |
| Session persistence | PRESENT | Room conversation/session DAOs and `ConversationRepository` |
| Settings/secrets | PRESENT | `SettingsRepository`, provider credential store, Keystore-backed storage |
| Cloud providers | PRESENT | Existing provider catalog, factory, OpenAI-compatible and vendor providers |
| Agent planning | PRESENT | `AgentLoop`, plans, approval policy, action dispatcher |
| Local Qwen boundary | PRESENT / BLOCKED | Local model registry and runtime artifacts exist; exact GGUF/parity unavailable |
| Generic coding-agent loop | PARTIAL | `AutonomousAgentEngine` exists but originally handled one tool call only |
| Project file tools | PARTIAL | Workspace-safe read/write existed; list/search/create/delete/patch were missing |
| Terminal tool | PARTIAL | Request type existed but executor intentionally rejected all commands |
| Git inspection | PARTIAL | Existing app has Git-related utilities, but the coding-agent tool boundary needed explicit read-only commands |
| Tool safety | PARTIAL | Workspace canonical-path check existed; command allowlist and destructive classification needed tightening |
| Tests | PRESENT / EXPANDING | Broad existing unit suite; MVP milestone adds multi-step, sandbox, and command-policy coverage |

## Target MVP flow

`Chat UI → Agent controller → ProviderManager → cloud provider (AUTO fallback) → structured tool call → permission policy → workspace/terminal tool → observation → next bounded step → persisted chat/checkpoint`.

The controller is provider-agnostic and uses the existing `Provider`/`ProviderManager` abstraction. Local Qwen can later implement the same provider contract without changing the controller.

## Safety boundaries

- File paths are canonicalized beneath the selected workspace root.
- Commands are limited to a development allowlist and reject shell metacharacters and destructive tokens.
- Delete and other destructive operations remain denied by the default permission policy.
- Agent execution has a finite step limit and cooperatively observes coroutine cancellation.
- Credentials are supplied through existing secure settings; no keys are stored in prompts, logs, or source.

## Deliberate non-goals in this milestone

Full Qwen numerical parity, JNI changes, Android native runtime changes, unrestricted shell access, automatic remote Git push, and device/emulator claims remain out of scope or blocked until separately verified.
