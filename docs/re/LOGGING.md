# Shared diagnostics

Implementation status, October21: **complete** for diagnostic hardening and result-name
provenance; canonical build and isolated smoke checks pass. The missing-session cause
and reported wrong name remain unconfirmed/open. No game was launched or probed.

## Evidence and defects

The explicitly authorized workspace-root
`Tester Logs/Timorensinck/ffpr_screen_reader.1.log` is31,729 bytes. Its header identifies
FF1, PID17188, October20 at18:18:21 and V0.9.3 compiled October16 at13:16:32. The
timestamp range281388.515–281390.328 covers about1.813 seconds and ends with startup
audio-cue loading. No battle-result, speech-output or periodic timing-summary records
follow. The user says it should cover a full session and could not reproduce the
name issue locally. Neither a wrong supplied file nor a logger failure is established.
The size does not show the4MiB rotation threshold. A numbered suffix can represent a
previous run or an earlier size part; it cannot identify a session by itself.

The port preserved central submitted-speech text logging, result phase/row counts,
hook/lookup errors, navigation timings, scene/state transitions and automatic timing
diagnostics. `speech::drain_backend` logs text immediately before calling Prism.
The source does not support a blanket claim that runtime diagnostics were omitted.

Independent defects found and corrected:

- File failures were ignored, failed handles were not recovered, and successful
  short writes discarded their suffixes.
- There was no independent periodic logger health or per-part session identity.
- Truncated printf records could disappear entirely. Long wide CJK text could also
  exceed the UTF-8 conversion buffer and disappear.
- Result logs counted rows without retaining their contents, native name-field
  identity or accepted-buffer generation. Successful mod commands lacked a central trace.

These are real logging defects, but do not establish why this session stopped logging
or why a tester heard Character09. See [the open name investigation](RESULTS.md#result-name-provenance-october-21).

## Ownership, bounds and recovery

`src/core/log.cpp` owns a2048-entry queue. Producers use bounded formatting and a
non-waiting lock attempt; contention/full capacity counts a diagnostic loss and
returns. The below-normal writer owns all file I/O, rotation and recovery. Game
callbacks never wait for disk, worker completion, backend RPCs or flushing. Numeric
timing snapshots are formatted by the writer. Diagnostic hardening changes neither
spoken content nor speech cancellation/deduplication.

The writer wakes on an event or ten-second timeout and also checks health while
draining sustained traffic. Health identifies PID, monotonic session start, part,
uptime, the atomically published native input frame, successful writes, cumulative
queue losses, file failures/lost bytes and last file error. Advancing health with an
unchanged frame means the logger is alive while that input producer is not advancing;
it does not establish that the entire game is stalled.

New files retain UTF-8, the4MiB threshold and five older files. Startup/size parts
include session identity; size rotation increments the part index. Recovery opens
in append mode and seeks the end, preserving prior bytes. Short-write suffixes get
up to four attempts; failure closes the handle and records lost bytes. Health time
retries opening. Persistent storage failure can prevent its own diagnostic from
reaching disk; counters become visible on recovery. Process termination can lose
queued tail records. Explicit draining shutdown is for the isolated host only;
DllMain does not wait. Existing file-sharing and rotation policy are retained.

Long diagnostic text keeps a valid UTF-8 prefix with an explicit truncation marker.
The intermediate wide conversion buffer fits before the bounded output limit.
There is no added user control, hotkey, dependency or localization string; internal
diagnostic labels are English in every game.

## Producer coverage

| Producer | Meaning |
| --- | --- |
| Boot/installers/hooks/refs | Host, game, build, installer counts, hook and native lookup failures. |
| `actions::run` | Accepted discrete mod action or existing unavailable-action error; no physical-key polling trace. |
| `events::fire`, `menu_state::set_active` | Lifecycle event kind/game and changed menu flags. Event enum is in `core.h`; bulk resets retain existing behavior, so this is not a full state dump. |
| `battle_result.cpp` | Native phase, pending owner/generation, first hidden-owner or missing-total wait, invalid-pending cancellation and populated counts. |
| `ui::review::log_capture` | Owned capture/domain/generations/phase/flags, source-name sample and integer identities, headline, totals, notifications, detail/change rows. |
| Review publication | Confirms an owner accepted that generation. A captured generation without publication may have been cancelled. |
| `speech::drain_backend` | Existing say text plus submission generation, interruption, kind, feature and Prism result; stop result too. SDK success does not prove audible delivery. |
| Existing timing/navigation/audio/input | Native/mod/original slow spans, input gaps, queue/data age, routes/cancellation and owner/device failures; subsystem records remain authoritative. |
| Logger | Independent file/session/queue/frame health and size-part identity. |

Review diagnostics run on the existing computation worker using owned text. Content
logging permits128 rows and16,384 UTF-16 units per capture, at most512 per row, with
an aggregate-limit record. Name provenance separately permits128 rows and128 raw
UTF-16 units per visible name. The native owner copies through the existing bounded
`il2cpp::read_string` from `Text.m_Text`; markup remains for comparison with assembled
text. Pointer values are integer identity tokens and must never be dereferenced by
workers. Native phase values:1 Points,2 Status,3 Items,4 Learning,5 Ability,6 FF2 Weapon;
minus one is another review producer. No per-frame scan is added. Existing phase
generation, visibility, owner and list bounds remain. Same-page replacements retain
their original name provenance rather than claim a fresh native read.

## Safe extension and verification

Trace source observation, accepted buffer and actual submission separately. Do not
repair an unexplained name by stripping digits, suppressing speech strings or using
character IDs as alternative names. Extend the known producer's bounded `NameSource`
snapshot if another layout needs evidence. Preserve queue/text/work bounds, native
affinity and integer-only identities. This subsystem integrates with the existing
review worker, speech queue and automatic timing services.

`tools/log_worker_check.cpp` compiles the real logger with a controllable file boundary:
blocked disk/full queue, producer progress/loss accounting, idle periodic frame health,
long narrow/formatted-wide/CJK text, valid UTF-8, short writes, disk failure, append
recovery and size-part identity. Its timeout allows the intentional ten-second health
interval and is not a gameplay performance target. `battle_results_check` covers real
result/review code with fake ready views, custom names and source provenance. Existing
speech/fake-Prism checks verify queue/output behavior at those boundaries.

No comparable modded/unmodded gameplay, native callback or audible-response measurement
has been taken. Source bounds and isolated checks do not certify game responsiveness.
Final build/smoke evidence follows. Remaining nonvisual steps are in
[the prioritized test procedure](../TESTING.md#results-names-and-log-continuity-october-21).

## Final offline receipt, October21

Canonical `build.bat` passes all checks:6,864 native names/zero missing;119 used
wiring names/323 defined/zero undefined;47 exports. Logger/results/speech/fake-Prism
and the remaining canonical regressions pass. Embedded soundscape/dependency checks
pass and README/TXT match. Existing unreachable-code/test strcpy warnings remain;
there are no build errors. `smoketest.bat` loads the DLL in an isolated host, resolves
all47 proxy/real exports, forwards HidD_GetHidGuid and exits normally; logger emits
652 bytes. No game was launched.

Local diagnostic DLL: `out/hid.dll`,9,719,808 bytes, SHA256
`CED3B991C8D7B307B929A3C44B874F509C87F2BE2BD9BC5B6F2E1E3768926C12`.
Focused receipts and the canonical log are preserved in
`build_check/name_logging_2026_10_21/`. This task built locally; it did not deploy,
commit/push or replace the published release. Existing dirty documentation is
preserved. Sol owned research, code, review and verification; no delegation occurred.

## Delivery follow-up

The combined navigation/logging build was subsequently deployed and hash-verified
in all six closed games at the user's request. See
[the navigation delivery receipt](NAVIGATION_GROUPS.md#verification-and-safe-extension).
The user then requested release 0.9.4; its final package/source/publication evidence
is recorded in [the release history](../release.md). The original name report remains
open; improved diagnostics do not establish that the intermittent bug is fixed.
