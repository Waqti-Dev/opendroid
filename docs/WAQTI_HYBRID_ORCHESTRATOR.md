# Waqti Agent / OpenDroid

## Hybrid AI Orchestrator Architecture

Forked by Ahmed Badr

Based on OpenDroid Contributors' work.

## Goal

Create a mobile AI agent that intelligently combines:

- Local models for speed, privacy and cost control.
- Cloud models for advanced reasoning and review.
- Persistent memory for project knowledge.

## Model Roles

### Local

- Qwen 3B: lightweight tasks and conversation.
- Qwen 7B: general assistant tasks.
- Qwen 15B: primary coding execution model.

### Cloud

Cloud providers act as supervisors:

- Architecture review.
- Complex debugging.
- Security review.
- Large context reasoning.

## Provider Failover

Tasks keep checkpoints:

```
Task
 |
Checkpoint
 |
Provider A
 |
Failure
 |
Provider B
 |
Continue
```

The agent should never restart a long task from zero after provider switching.

## Memory Layers

```
Project Memory
Knowledge Memory
Task Checkpoints
User Preferences
```

Large model training is not performed on-device. Instead, the system improves through retrieval, indexing and memory.

## Future Modules

```
ai/orchestrator/
 ├── ProviderManager
 ├── FailoverManager
 ├── CheckpointManager
 ├── TokenBudgetManager
 └── ContextCompressor
```

```
ai/agents/
 ├── CodingAgent
 ├── BusinessAgent
 └── ResearchAgent
```
