# WAQTI FAILURE ROOT-CAUSE REPORT

Evidence collected after the single full unit-test run. The approved test-fixture-only fix has now been applied and verified.

## A) MAVEN / ROBOLECTRIC FAILURES

**Affected tests:** `ResourceCleanupTest`, `FinanceActionsTest`, `MediaActionsTest`, and `ToggleBluetoothActionTest`.

**Common exception:**

```text
java.lang.AssertionError: Failed to fetch maven artifact
org.robolectric:android-all-instrumented:15-robolectric-13954326-i7

Caused by:
java.lang.AssertionError: SHA-512 mismatch for POM file
expected SHA-512=5fc64ffccf4788154162686b3966ddd63abea8326542c465e2a7f0129fc92770d0e1e8ddcfabd7f1500e491ec02e3b4104b9e51b61fccd9cf08006c9dc02987c
actual SHA-512=bb7f3766ae422c3f622fe7389448e80d1b17036b7adc2288542699eec20190f3f666781700ee8eeff493d9a4e075203729cf404d0e975796098db221ffd17bef
```

**Repository / URL:** Robolectric's Maven dependency resolver requests Maven Central. The verified POM URL is:

`https://repo1.maven.org/maven2/org/robolectric/android-all-instrumented/15-robolectric-13954326-i7/android-all-instrumented-15-robolectric-13954326-i7.pom`

**HTTP/network evidence:** The URL returned HTTP 200, content type `text/xml`, size 1391 bytes. A fresh direct download had SHA-512:

`e16eb42ba12b823ede906bec317f73688c58d05ebc7f6f4c39ff555c08926515aac59dbddddd888b87c85a022d756d6aa1520cc0276e2adee2caf946ad79e659`

The local POM had the same SHA-512 and the same 1391-byte size. Therefore the network request itself was successful and the POM body is valid.

**Local checksum evidence:** The local checksum sidecar at:

`~/.m2/repository/org/robolectric/android-all-instrumented/15-robolectric-13954326-i7/android-all-instrumented-15-robolectric-13954326-i7.pom.sha512`

contains a concatenated value beginning with the JAR checksum, followed immediately by the POM checksum, rather than a clean POM checksum entry. Robolectric interpreted the first 128 characters as the POM expected checksum, causing the mismatch. The local JAR checksum itself begins with the same value reported as the expected POM checksum.

**Common root cause:** **EXTERNAL TEST INFRASTRUCTURE ISSUE — malformed or cross-associated local Maven checksum metadata for the shared Robolectric `android-all-instrumented` artifact.** All four tests fail before their test bodies execute, while Robolectric tries to create its Android sandbox. No Waqti application assertion is reached.

**Classification:** Infrastructure / dependency cache metadata / Robolectric resolver. No Gradle dependency version was changed and no cache was deleted.

## B) MVP FAILURE

**Test:** `AutonomousAgentEngineTest.createUserFile_throughToolCall`

**File / lines:** `app/src/test/java/com/opendroid/ai/core/agent/AutonomousAgentEngineTest.kt`, test starts at line 17; first failing assertion is line 27 in the generated report.

**Exception:**

```text
java.lang.AssertionError: expected:<COMPLETED> but was:<FAILED>
```

**Expected:** The test's `FakeProvider` returns a `WriteFile` tool call, the executor records it, and the engine reaches `AgentCheckpoint.Status.COMPLETED`.

**Actual:** `FakeProvider` returns the same `WriteFile` JSON on every generation call. `AutonomousAgentEngine` correctly executes the tool and then requests the next bounded step so it can receive a final assistant response. Because the fake never returns a final response, the loop reaches its maximum of 20 steps and returns `FAILED` with `Maximum agent steps reached (20)`.

**First failing component:** Test fixture / mock-provider contract, not `CreateFile` or workspace validation. The test actually requests `WriteFile`, not `CreateFile`; `RecordingExecutor` returns success, but its success is not a terminal agent response.

**Failure chain:**

```text
test
→ FakeProvider returns WriteFile JSON
→ parseToolCall succeeds
→ toToolRequest returns ToolRequest.WriteFile
→ ToolPermissionManager allows WriteFile
→ RecordingExecutor returns ExecutionResult.Success("ok")
→ engine asks provider for the next step
→ FakeProvider returns the same WriteFile JSON 20 times
→ maxSteps reached
→ checkpoint FAILED
→ assertion expected COMPLETED fails
```

**Classification:** MOCK / TEST FIXTURE BUG. The engine's multi-step behavior is consistent with its contract: tool execution is an intermediate state and completion requires a non-tool response. No workspace or file-path failure occurred.

## C) APPLIED FIX

### Sequenced FakeProvider — APPLIED

`FakeProvider` now returns the `WriteFile` tool call once, then returns a normal final response on the next generation. This models the production protocol and preserves the tool-dispatch and completion assertions.

**Risk:** Low. Only the test double changed; production code and security boundaries were not changed.

The higher-risk alternatives—marking every successful tool execution as terminal or adding a new terminal protocol field—were not applied.

### Option 2 — Change the engine to mark any successful write as COMPLETED

This would make the current fixture pass without a second model turn.

**Risk:** High. It would break the intended multi-step loop, prevent the model from summarizing tool results, and make intermediate tool completion indistinguishable from final agent completion. Not recommended.

### Option 3 — Add an explicit terminal/non-terminal field to the tool protocol

The provider could declare whether a tool call is the final action.

**Risk:** Medium/high. It expands the protocol and production/UI integration without evidence that the MVP needs it. Not justified for this failure.

### Maven remediation options

1. Inspect/rebuild only the affected local `.pom.sha512` metadata after preserving the current cache, then run one representative Robolectric test.
2. Use a clean isolated Maven cache for one representative test, without deleting the existing cache.
3. Investigate Robolectric resolver/version behavior only if the isolated-cache test still reproduces the mismatch.

No Maven remediation was applied in this checkpoint; it remains a separate external infrastructure issue.

## D) RECOMMENDED NEXT ACTION

The smallest test-only fix is complete. Verification was run with:

```text
./gradlew :app:testDebugUnitTest --tests '*AutonomousAgentEngineTest.createUserFile_throughToolCall'
```

The targeted test passed in 33 seconds, and the complete `AutonomousAgentEngineTest` class passed in 20 seconds. The 515-test suite was not rerun. Maven/Robolectric remains separate and should be investigated only with one representative test and a preserved isolated cache.
