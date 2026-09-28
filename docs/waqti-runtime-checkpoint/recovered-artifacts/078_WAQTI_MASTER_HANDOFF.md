# WAQTI_MASTER_HANDOFF.md

> **Project handoff document — Waqti Agent / OpenDroid**
>
> Last updated: **2026-09-23**
> Repository: **Waqti-Dev/opendroid**
> Base project: **yashab-cyber/opendroid**
> Default branch: **main**
> Current known head: **8d3d0ed16b8b8e0c79b70428a9254e1b5e5dfa96**

---

## 0. READ THIS FIRST — OPERATING CONTRACT

You are taking over an **active software project**, not a brainstorming exercise.

The user expects real engineering work inside the repository.

### Mandatory behavior

1. Inspect the actual repository before proposing architectural changes.
2. Implement real Kotlin/Android code.
3. Build and test after meaningful changes.
4. Never claim a feature works unless it was actually verified.
5. Do not create documentation-only progress when implementation is requested.
6. Preserve existing working behavior unless there is a clear reason to change it.
7. Prefer small, reviewable commits with meaningful commit messages.
8. When something cannot be tested on the current hardware/environment, say exactly what was and was not verified.
9. Do not silently replace the project's architecture with a toy implementation.
10. The long-term goal is a genuinely useful **local + cloud coding/automation agent on Android**, not a chatbot skin.

### User's development style

The user is building this primarily from an Android phone and wants the AI collaborator to do the difficult engineering work rather than repeatedly asking for basic setup steps.

When a task is actionable, **inspect → implement → test → report**.

---

# 1. PROJECT IDENTITY

## Name

**Waqti Agent**

The current Android codebase is a fork/custom evolution of:

**OpenDroid** — an open-source autonomous AI agent for Android.

## Repository

**Waqti-Dev/opendroid**

GitHub:
https://github.com/Waqti-Dev/opendroid

The repository is public and the user has push/admin access.

## Base repository

The project originated from:

**yashab-cyber/opendroid**

Do not assume the fork is still identical to upstream. Waqti-specific engineering has already been added.

---

# 2. PRODUCT VISION

Waqti is intended to become a **real autonomous Android AI agent**.

The target is not simply:

> user asks → model answers

The target is:

> user gives goal → agent reasons/plans → selects tools → executes actions → observes results → verifies → recovers/replans → reports completion.

Long-term, Waqti should support:

- Local LLM inference on Android
- Cloud LLM providers
- Provider/model failover
- Tool calling
- File/workspace operations
- Coding workflows
- Terminal/shell workflows where safely available
- Git/GitHub workflows
- Persistent task state/checkpoints
- Device automation
- Screen understanding
- Memory
- Voice
- Vision
- Long-running autonomous tasks
- Strong workspace and permission boundaries

The project should eventually be able to act as a practical **mobile coding agent**, not merely a general assistant.

---

# 3. CURRENT REPOSITORY STATE

## Important recent commits

Recent engineering work includes:

- `3a95b5fbe1eb47e88a1bceeabf8e13d682718261`
  - Add autonomous agent checkpoint contract
- `932915c8d5b3916a324706477275bee689353ca0`
  - Add end-to-end autonomous agent engine
- `ca0b325eb17da95297e8d64864628df5817dcce0`
  - Add provider generation failover
- `ac83586f1e1b4af33c51ecba37170bd410e6f6c9`
  - Use provider failover in agent engine
- `a8f237120223c70ed071f97ea74f48ecea7ff6b2`
  - Test autonomous agent execution and failover
- `a793e68001379ee49d4b5356346517ccc285daf8`
  - Bridge production LLM providers into agent failover
- `41f6f60270fc9f27ab46ecc14afaebe9fa47d029`
  - Sandbox coding file tools to workspace
- `8d3d0ed16b8b8e0c79b70428a9254e1b5e5dfa96`
  - Use existing coroutines test primitive

Current known HEAD at handoff:

`8d3d0ed16b8b8e0c79b70428a9254e1b5e5dfa96`

---

# 4. WHAT HAS ALREADY BEEN IMPLEMENTED

The current Debug implementation is materially beyond the original OpenDroid baseline.

Known Waqti work includes:

## Agent engine

- AutonomousAgentEngine
- Agent checkpoint contract/state
- Execution flow
- Provider failover
- Scenario/unit tests
- Workspace-constrained coding/file tools
- Path-traversal protection
- Permission-flow groundwork
- Resource/model routing groundwork

## Provider layer

Production LLM providers have been bridged into the agent's failover path.

The system is intended to continue with another provider/model when a generation attempt fails, subject to provider availability/configuration.

## Coding safety

Coding/file operations have been sandboxed to a configured workspace.

Path traversal must not escape the authorized workspace.

Do not weaken this boundary just to make a demo work.

## Model routing / registry

The current design includes routing/registry/resource-monitoring groundwork for local model families including:

- Qwen 15B
- Qwen 7B
- Qwen 3B

These should be treated as routing/catalog targets unless the code proves a model is actually loaded and generating on-device.

---

# 5. VERIFIED BUILD / TEST STATUS

The latest known engineering checkpoint reported:

- `assembleDebug`: passed
- Unit tests: passed
- API 26 instrumented tests: passed
- API 36 instrumented tests: passed
- Debug APK produced
- Approximate APK size at that checkpoint: ~71 MB

Known limitation:

- Lint still reports pre-existing UI findings.

Do not state that lint is clean unless it is actually rerun and confirmed clean.

---

# 6. IMPORTANT CURRENT LIMITATIONS

These are critical.

## 6.1 Local GGUF/llama.cpp Android runtime is NOT YET CONNECTED to the Kotlin agent

There is local llama.cpp/Snapdragon infrastructure outside the Android Kotlin app, but the current Waqti Kotlin agent has not yet been proven to use that runtime end-to-end.

Do not claim:

> “Waqti is already running a local Qwen model inside the Android agent”

unless the integration is implemented and tested.

## 6.2 Agent loop is currently limited

The known current implementation has a **single LLM → tool cycle** rather than a mature unrestricted multi-step autonomous loop.

The next engineering stages should turn this into a robust iterative loop with:

- observe
- reason
- tool selection
- execute
- tool result
- verification
- next action
- checkpoint
- recovery/replan
- termination

## 6.3 Model routing is not equivalent to inference

The presence of Qwen 3B/7B/15B routing/registry code does not mean all three models are installed or executable on the phone.

---

# 7. LOCAL MOBILE RUNTIME ENVIRONMENT

The user has a local Snapdragon-oriented llama.cpp runtime prepared on the Android/Termux side.

Known paths:

`~/llama-snapdragon/bin/llama-server`

`~/llama-snapdragon/bin/llama-cli`

`~/llama-snapdragon/lib/libggml-htp-v73.so`

Known local model files/configuration from the latest environment inspection:

- Qwen2.5 0.5B
- Qwen 3B Official — ~2.10 GB
- Qwen Coder 7B — ~4.68 GB
- Qwen Coder 14B — ~8.99 GB

At the last known inspection:

- no llama process was running
- no Waqti process was running
- nothing was listening on port 8080

Therefore, before relying on the runtime, verify the actual current state.

---

# 8. USER'S CURRENT DEVELOPMENT HARDWARE

Current development is primarily performed from an Android phone.

Known phone:

- Redmi Turbo 4 Pro
- Snapdragon 8s Gen 4
- 12 GB RAM
- 256 GB storage
- Android 16 / API 36
- HyperOS

The user does **not currently own the planned desktop PC**.

A future PC is conditional on demonstrating meaningful progress/success with Waqti and sustained effort from the phone.

Do not assume a desktop GPU is currently available.

---

# 9. FUTURE LOCAL-AI PC TARGET

This is future planning, not current hardware.

Discussed possible future configuration:

- NVIDIA RTX 5080 AORUS Master ICE — 16 GB VRAM
- AMD Ryzen 9 9950X3D
- 64 GB RAM preferred over 32 GB for local-AI experimentation

The user understands that CPU/GPU/RAM hybrid inference can run models larger than VRAM through CPU/RAM offloading, but that RAM is not equivalent to VRAM in bandwidth.

This future machine is relevant to the long-term local-model strategy, but **must not be treated as present infrastructure**.

---

# 10. EXISTING OPEN-DROID BASELINE

The upstream-derived Android app already contains a broad autonomous-assistant architecture.

The README describes:

### Core

- AgentLoop
- PlanManager
- IntentClassifier
- VisionEngine
- LLM provider system
- Memory system
- Security
- Foreground service
- Voice

### Device actions

The baseline advertises actions around:

- Wi-Fi
- Bluetooth
- brightness
- flashlight
- DND
- volume
- screenshots
- WhatsApp
- Telegram
- calls
- SMS
- email drafts
- alarms
- timers
- reminders
- calendar
- notes
- navigation
- media
- camera
- smart-home functionality

Do not assume every advertised feature is fully working on the user's exact Android/HyperOS device. Verify before depending on it.

---

# 11. BASE ARCHITECTURE

The original project uses a Kotlin Android architecture with major areas including:

```
com.opendroid.ai
├── accessibility/
├── actions/
├── core/
│   ├── agent/
│   ├── llm/
│   ├── memory/
│   ├── security/
│   ├── service/
│   └── voice/
├── data/
│   ├── db/
│   ├── models/
│   └── repository/
├── di/
└── ui/
```

Dependency injection is based on Dagger-Hilt.

UI is Jetpack Compose.

Room/DataStore are used by the baseline data layer.

Do not rewrite the entire project into a different framework without an explicit reason.

---

# 12. LLM PROVIDER STRATEGY

The original project supports a broad provider ecosystem, including:

- Google Gemini
- Anthropic Claude
- OpenAI
- Groq
- DeepSeek
- Mistral
- OpenRouter
- Together AI
- Cohere
- GitHub Copilot
- Ollama
- Custom OpenAI-compatible endpoints

The Waqti direction is:

### Local-first where practical

Local inference should be preferred when:

- the model fits
- latency is acceptable
- privacy matters
- the task does not require cloud-only capabilities

### Cloud fallback

Cloud providers remain valuable for:

- large reasoning tasks
- models too large for the phone
- difficult coding tasks
- vision/multimodal tasks
- reliability/fallback

The provider abstraction should make local and cloud models interchangeable at the agent level.

---

# 13. LOCAL INFERENCE ROADMAP

The local inference architecture should eventually support:

```
User
  ↓
Waqti Agent
  ↓
Model Router
  ├── Local Android runtime
  │     └── llama.cpp / Snapdragon backend
  │
  └── Cloud providers
        ├── Provider A
        ├── Provider B
        └── Provider C
```

The router should consider:

- task type
- model capability
- model availability
- RAM
- thermal state
- latency
- battery
- context requirements
- connectivity
- provider health
- fallback order

The user specifically wants local coding/agent capability, so local inference is a major product goal rather than a cosmetic setting.

---

# 14. MODEL STRATEGY

Known mobile model targets include:

### Small

- Qwen 3B

Use cases:
- simple commands
- classification
- routing
- lightweight tool selection

### Medium

- Qwen Coder 7B

Use cases:
- normal coding
- file edits
- shell/tool reasoning
- general agent work

### Larger mobile target

- Qwen Coder 14B

Use cases:
- harder coding tasks
- stronger local reasoning where thermals/RAM permit

### Future

Potentially larger models can be handled using:

- quantization
- CPU/GPU offloading
- hybrid inference
- MoE models
- cloud fallback

Do not optimize solely for parameter count. Measure actual task quality, tokens/sec, first-token latency, memory usage, thermal behavior and reliability.

---

# 15. AGENT LOOP — TARGET DESIGN

The current single-cycle behavior needs to evolve toward:

```
REQUEST
  ↓
CONTEXT ASSEMBLY
  ↓
MODEL DECISION
  ↓
TOOL CALL?
  ├── NO → FINAL RESPONSE
  │
  └── YES
       ↓
    VALIDATE TOOL
       ↓
    EXECUTE
       ↓
    CAPTURE RESULT
       ↓
    VERIFY
       ↓
    CHECKPOINT
       ↓
    CONTINUE / REPLAN / STOP
```

Required properties:

- bounded iteration count
- cancellation
- timeout handling
- tool error handling
- model failure handling
- provider failover
- checkpoint persistence
- resumability
- explicit terminal states
- no infinite loops
- no accidental repeated destructive actions

---

# 16. CODING AGENT REQUIREMENTS

Waqti should eventually be able to receive requests such as:

> “Inspect this project, find the bug, fix it, run the tests and tell me what changed.”

Expected behavior:

1. Inspect workspace
2. Understand project structure
3. Identify relevant files
4. Read source
5. Form a hypothesis
6. Make a minimal patch
7. Run relevant tests/build
8. Read failures
9. Iterate
10. Verify final state
11. Summarize changed files and verification

For coding operations:

- Workspace boundaries are mandatory.
- Avoid arbitrary filesystem access outside authorized roots.
- Avoid destructive operations unless explicitly authorized.
- Prefer patches/minimal changes.
- Always verify compilation/tests after modifications.

---

# 17. SECURITY REQUIREMENTS

Security is part of the product architecture.

Maintain:

- workspace path restrictions
- path traversal protection
- permission checks
- secure provider credential storage
- Android permission boundaries
- safe tool invocation
- explicit handling of destructive actions

Never add a shortcut such as:

> allow arbitrary absolute paths

just because it makes development easier.

Any such change must be deliberate, reviewed and justified.

---

# 18. DEBUG BUILD GOAL

The Debug build should become the primary development artifact.

Every meaningful milestone should ideally provide:

- source changes
- successful build
- relevant tests
- APK
- exact commit
- short verification report

The user wants to understand:

> “What did this Debug version actually add compared with normal OpenDroid?”

Therefore every milestone should clearly distinguish:

### Existing OpenDroid capability

from

### Waqti-specific capability

and

### Still unfinished capability.

---

# 19. NEXT PRIORITY ORDER

Unless a newer explicit user instruction overrides this, prioritize engineering roughly in this order:

## Priority 1 — Local LLM integration

Connect the existing Snapdragon/llama.cpp runtime to the Android Kotlin agent.

Required outcome:

- start/manage local inference
- send prompts
- receive streamed/generated output
- detect runtime failures
- expose model health
- integrate with the existing provider abstraction

Do not stop at a fake/mock provider.

## Priority 2 — Real iterative agent loop

Upgrade the single LLM→tool cycle into a bounded multi-step loop.

## Priority 3 — Tool execution + verification

Ensure:

- tools return structured results
- model sees tool results
- failures are actionable
- verification can happen after modifications

## Priority 4 — Checkpoint/resume

Make long-running tasks resumable.

## Priority 5 — Coding workflow

Build a reliable:

```
inspect → edit → build/test → diagnose → patch → verify
```

pipeline.

## Priority 6 — Model routing

Use resource-aware local model selection.

## Priority 7 — UX

Expose:

- current model
- local/cloud state
- task progress
- tool execution
- checkpoints
- failures
- cancellation

without turning the UI into a debugging console.

---

# 20. TESTING STANDARD

For each change:

### Minimum

- compile affected module
- run relevant unit tests

### For agent behavior

- scenario tests
- tool success
- tool failure
- provider failure
- fallback
- cancellation
- workspace boundary

### For Android behavior

- API 26 where applicable
- API 36 where applicable
- real-device validation when hardware-specific behavior matters

### For local inference

Test on the actual phone/runtime when possible.

A desktop emulator test is not enough to claim Snapdragon acceleration works.

---

# 21. CURRENT KNOWN LOCAL TOOLCHAIN

Known local runtime artifacts:

```
~/llama-snapdragon/bin/llama-server
~/llama-snapdragon/bin/llama-cli
~/llama-snapdragon/lib/libggml-htp-v73.so
```

Known models:

```
Qwen2.5 0.5B
Qwen 3B Official
Qwen Coder 7B
Qwen Coder 14B
```

The local runtime should be inspected before making assumptions about:

- ports
- running processes
- model paths
- backend support
- available shared libraries
- current performance

---

# 22. IMPORTANT DISTINCTION: MOBILE AGENT REPO VS EXPERIMENTAL RUNTIME

The user previously had local directories such as:

```
~/waqti-agent
~/waqti-agent-repo
```

These were not Git repositories at the time of inspection.

They contained/related to experimental agent material including:

- PROJECT_CONTEXT.md
- app_v2.py
- coding_agent.py
- research_*
- task_memory.py
- waqti_start.py

Do not assume these directories are the authoritative source of the Kotlin Android repository.

The authoritative GitHub Android repository for this handoff is:

**Waqti-Dev/opendroid**

---

# 23. DO NOT CONFUSE THESE PROJECT LAYERS

There are currently several related layers:

### Layer A — Android Waqti/OpenDroid Kotlin app

Repository:
`Waqti-Dev/opendroid`

This is the production-target Android codebase.

### Layer B — Snapdragon llama.cpp runtime

Local runtime used for experimenting with on-device inference.

### Layer C — Experimental Python/Termux agent material

Files such as:

- `app_v2.py`
- `coding_agent.py`
- `task_memory.py`
- `waqti_start.py`

These may contain useful ideas/prototypes but should not automatically replace the Kotlin architecture.

---

# 24. USER EXPECTATIONS FROM THE AI ENGINEER

The user wants an engineering partner.

Preferred response pattern:

### Before work

- inspect relevant files
- identify current behavior
- identify exact gap

### During work

- implement actual changes
- keep architecture coherent
- test continuously

### After work

Report:

1. What changed
2. Files changed
3. Why it changed
4. Tests/builds run
5. Results
6. Known limitations
7. Commit SHA
8. Next concrete engineering step

Avoid long generic tutorials when the repository can simply be inspected and modified.

---

# 25. CLAIM DISCIPLINE

Use these distinctions:

### VERIFIED

The repository/runtime was actually inspected or the test was actually run.

### IMPLEMENTED

Code exists, but runtime verification may still be pending.

### PLANNED

Not implemented.

### HYPOTHESIS

An idea that has not been validated.

Never turn:

> “we should support X”

into:

> “X is supported.”

Never turn:

> “the model should run locally”

into:

> “local inference works.”

---

# 26. DO NOT REGRESS THESE FEATURES

Do not accidentally remove:

- provider failover
- workspace restrictions
- path traversal protection
- checkpoints
- existing tests
- existing OpenDroid device automation
- existing provider abstractions
- existing security mechanisms

When modifying shared infrastructure, run regression tests.

---

# 27. SUCCESS DEFINITION

Waqti should eventually demonstrate this end-to-end:

### Example

User:

> “Open the project, inspect the failing test, fix the code, run the tests, and tell me what happened.”

Waqti:

1. Understands the request.
2. Selects an appropriate local/cloud model.
3. Creates a bounded task.
4. Inspects the workspace.
5. Reads relevant source/test files.
6. Diagnoses the issue.
7. Edits the correct files.
8. Runs tests.
9. Reads failures.
10. Corrects the implementation if needed.
11. Re-runs tests.
12. Verifies success.
13. Saves checkpoint/state.
14. Gives a concise final report.

That is the target product behavior.

---

# 28. IMMEDIATE HANDOFF TASK

The next engineer/AI should **not** start by rewriting the app.

First:

1. Inspect current HEAD.
2. Locate the Waqti agent engine.
3. Locate provider interfaces and failover implementation.
4. Locate model registry/resource monitoring.
5. Locate workspace-constrained coding tools.
6. Locate checkpoint implementation.
7. Locate existing tests.
8. Verify current Debug build status.
9. Inspect how an LLM request currently enters the agent.
10. Design the smallest real integration point for the Snapdragon llama.cpp runtime.
11. Implement the local provider.
12. Build and test.
13. Only then continue toward the iterative tool loop.

---

# 29. HANDOFF PROMPT FOR A NEW AI

Copy the following as the first instruction after providing this file:

> You are taking over the Waqti Agent / OpenDroid Android project.
>
> Read `WAQTI_MASTER_HANDOFF.md` completely before making changes.
>
> The authoritative repository is `Waqti-Dev/opendroid`.
>
> This is an active engineering project. Do not respond with generic suggestions when the repository can be inspected and modified.
>
> Your workflow is:
>
> **inspect → understand → implement → build → test → verify → commit → report**
>
> Never claim something works without verification.
>
> Preserve workspace security, path traversal protection, provider failover, checkpoints, and existing regression tests.
>
> The most important unfinished capability is connecting the existing Snapdragon/llama.cpp local runtime to the Kotlin agent as a real provider, followed by converting the current single LLM→tool cycle into a robust bounded multi-step agent loop.
>
> Before coding, inspect the actual current HEAD and relevant files. Do not assume the handoff document perfectly reflects changes made after its timestamp.
>
> At the end of every meaningful milestone, report:
>
> - implementation summary
> - exact files changed
> - tests/build commands
> - results
> - limitations
> - commit SHA
> - next engineering step
>
> Do real work. Do not manufacture progress.

---

# 30. CHANGE LOG FOR THIS HANDOFF

### 2026-09-23

This handoff was generated from the current project context and the GitHub repository state.

Known repository HEAD:
`8d3d0ed16b8b8e0c79b70428a9254e1b5e5dfa96`

Known milestone:
- autonomous agent engine exists
- provider failover exists
- coding tools are workspace constrained
- checkpoint contract exists
- tests/build have passed at the latest reported checkpoint
- local llama.cpp runtime exists externally
- Kotlin-to-local-llama integration remains unfinished
- multi-step agent loop remains unfinished

**This file is a living document. Update it after major architectural milestones.**
