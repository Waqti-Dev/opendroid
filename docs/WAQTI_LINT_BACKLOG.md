# Waqti Lint Backlog

## Baseline

Command run before migration planning:

```text
./gradlew :app:lint --no-daemon
```

Result: **FAIL** — 40 errors and 1 hint. The first reported finding is:

```text
app/src/main/java/com/opendroid/ai/actions/SocialActions.kt:52
DefaultLocale
```

The project also has 15 findings filtered by `app/lint-baseline.xml`, and the lint task reported one baseline entry no longer present. This backlog intentionally does not attempt to fix all lint during migration.

## Blocking vs non-blocking for Phase 0

| Finding group | Phase 0 status | Action |
|---|---|---|
| Existing production lint findings | Non-blocking for audit | Record; fix only when touching the file or before release |
| `DefaultLocale` in `SocialActions.kt` | Non-blocking for repository audit | Do not mix into migration checkpoint |
| Baseline drift | Review before release | Shrink baseline only when the underlying issue is actually fixed |
| Findings in files being rewritten for Waqti UI | Blocking for that milestone | Resolve as part of the rewrite and test the new file |
| New lint in Local GGUF/JNI bridge | Blocking for that milestone | Fix before declaring the milestone verified |
| Release-critical/security lint | Blocking before release | Fix and verify with release lint |

## Policy

- Do not regenerate the baseline to hide new findings.
- Do not change unrelated dependencies to suppress lint.
- Keep existing lint debt separate from the test/Maven infrastructure issue.
- Run targeted lint on changed modules/files where practical.
- Run full lint before a Waqti release, not as a prerequisite for this Phase 0 audit.
