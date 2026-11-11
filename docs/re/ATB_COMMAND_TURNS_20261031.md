# Character-ready turn speech and ATB command ownership

Delivery clarification, November2: the owner still hears Desert Sahagin's turn
in FFIV. This correction is source/focused Closed and has not reached installed
IV DLL a6d9b4ce19dfb6adb33fb0051851814a62b4fcd0848c041c0ef23625b466998f.
The current isolated FF1 cardinal-passage candidate also retains older battle
party/command owners, whose hashes differ from this qualified main source. Do
not install that FF1 candidate into IV and claim it contains the ATB correction.
The owner-prioritized next delivery after Levistone must include and qualify
these exact command-ready/party-only changes. No enemy schedule is required.
Source/focused, installed and naturally confirmed states remain distinct in
MAP_AUDIT_FOLLOWUPS_20261028.md; no new game launch/probe is authorized.

Current isolated IV delivery preparation, November2: four production owners
(command/party/state.cpp and battle_internal.h) are copied byte-for-byte from
the Closed source; utility/public battle/dedup/message dependencies already
match. Three existing fixture sources carry the ATB regressions. The stage's
older unrelated choice owner does not implement main's later choice-open query;
its baseline choice tests are preserved while only those unrelated new lifecycle
assertions are omitted in the staged battle-focus fixture. All ATB/party/equipment
cases pass, current IV/V/VI native hashes/typed ready fields match, production
turn design unchanged. Failed7-assertion mismatch and adapted source are retained
in sibling Class-Delivery/build_check/ff4_party_turn_20261102/. Canonical/smoke/
exact scan/recovery/closed-IV installation are running/pending, not claimed done.

Final IV-only candidate is ready November2 at12:24UTC:712 frozen inputs/170fresh
reports/8317names0missing/47exports/67resources/12languages; isolated smoke and
exact22-file scan pass. DLL11531776B SHA6170394f8218e38aa387e2e891ee21633f0e55462bbee59c044201b0069bfc56.
The first full run exposed a second missed consumer, command_commit_check,
which still equated SetCommandData with ATB readiness. Its setup now invokes
the actual ready entry; all commit/cancel/synchronous handoff/native exception
assertions remain and pass across all six games. Production source unchanged.
Final source/failed reports/current checks are retained in the _final delivery
record. The deployment guard sees IV running; owner save/closure is requested.
This is checked/scanned, not installed or naturally gameplay-confirmed yet.

Owner scope: automatically identify the character when their command menu
becomes usable; T speaks party characters only, including during an overlapping
enemy action. Applicable ATB adapters are IV/V/VI. Shared character-ready and
party-reader paths are also checked for I/II/III. No future-turn schedule is
read or exposed; the earlier FFIII turn-order investigation stays Closed.

## Demonstrated failures

The native action observer stored every executing unit's name, including
enemies. T preferred that execution name over the current command owner.
The active-status row also preferred an executing party member over a different
character whose command menu owned input. The actual party/rendered-reader
fixture reproduces21 overlap assertions across IV/V/VI.

Automatic speech's existing session guard remembered the last character ID.
Accepting a command did not end that guard, so consecutive ready turns from the
same character could be silent. A same-character native re-entry also returned
before restoring command/menu ownership. The previous command fixture always
returned true from its index-guard stub and missed this. With the actual guard
semantics,12 assertions reproduce these two failures across all six games.

Before source, executables, reports and exit codes are retained under
build_check/atb_turn_fix_20261031/. The baseline pair intentionally exits1:
38.019s,311685120bytes peak, children drained, unchanged aggregate1 GB cap.

## Native phase and implementation

ATB SetCommandData prepares the character data. The actual KeyInput
BattleInfomationController.CommandSelectInit entry enables command controls.
Its typed commandSelectController child roots the OwnedCharacterData target.
All three exact current GameAssembly/dump hashes match the retained external
command_commit_20261029 primary archive. Each ready body is unique; the typed
child is at offset0x40 in IV/V/VI. The native-contract check emits only factual
RVA/field/hash metadata, never recovered bodies inside Git.

One ATB-only hook observes that native entry after its original finishes. It
reads the rooted child/actor, restores command ownership and announces the ready
character. I/II/III retain their existing SetCommandData producer. The opening
roster and saved Interrupt speech on character turn preference are preserved;
manual T still intentionally interrupts. Data preparation is silent in ATB.
The ready entry then reads the current command focus after the character.
Native initialization may have set that cursor before admission; the existing
command-event guard prevents a later unchanged cursor callback from repeating
it. Command lists are bounded to64 slots before counting the active page.
An inactive previous-page slot cannot become an initial command announcement.

The accepted-command callback resets the existing character-ID guard before
the original can synchronously admit another actor. Cancel/re-entry within the
same episode keeps its existing local guard. Ownership restoration is independent
from that guard; an empty name cannot consume it. No text-content deduplication
is added. The existing menu/cursor/target cancellation generations remain.

T prefers the admitted rooted command actor through Command/Magic/Items/Target
contexts. A temporary hidden party row may still repeat that already-admitted
character name when the rooted actor matches the saved input identity. The saved
pointer itself is never dereferenced. With no command context, only an executing
party actor qualifies; enemy actions and stale focus markers cannot become turns.
H gives a visible command-owner row priority over another executing party member.
Other party statistics and target/number-row behavior remain unchanged.

The party-information observer now stores execution names only for native
BattlePlayerData. Enemy action, ability, damage and condition producers retain
their own action data; they are not removed or rerouted through turn speech.
Battle lifecycle clears the preserved actor and action context as before.
One unnamed action-active bit preserves H's prior no-active-character result
during a non-party action when no command context owns input. No enemy name,
identity or scheduling information is retained by that party-information state.

## Ownership, cost and verification

All new observations remain on the native callback owner. The added hook runs
once per native command-input entry, with bounded rooted field/class/liveness
reads and the existing character-name formatting/speech route. No polling,
scene scan, schedule enumeration, background native pointer, wait or I/O is added.
Manual party views retain their existing16-owner/nine-row bounds. The enemy
observer no longer performs the unnecessary party-information name read.

The native contract check passes at157908992bytes/1.816s under the same serial
budget. Focused command/party/equipment/action checks all pass current source;
the12 automatic/re-entry and21 overlap failures are zero. New cases cover
actual input entry versus preparation, rooted owner
versus last prepared controller, consecutive same-character commands, cancel
re-entry, enemy/party overlap, hidden rows and lifecycle retirement.
Source/offline/native-frame/audible/gameplay/delivery claims remain separate.
No game was launched or probed. Combined canonical/security/all-six delivery is
pending in the single scope ledger.

Final focused checkpoint:15 frozen inputs, three current-source executables,
their reports, reused unchanged-message regression and native/resource receipts
are in build_check/atb_turn_fix_20261031/qualified_checkpoint/. The final three
checks use321232896bytes/73.661s; the last command-visibility regression uses
285810688bytes/14.945s. All pass, children drained,1 GB unchanged. Source/focused
is Closed; combined delivery and natural confirmation are pending.

After the combined qualified installation, check a normal IV/V/VI battle when
progression permits: hear the character when commands become usable, press T
while an enemy acts, then open Magic/Items/targeting and press T again. The same
commander should be named. On a later ready turn from that character, hear a new
turn announcement. Canceling within that input episode repeats the command
focus without an extra turn line. With no ready menu during an enemy-only
action, T says No active character. Preserve the opening roster and both saved
interrupt-setting behaviors. These are natural-play checks, not a request to
launch/probe a game during engineering.

November2 FFIV-only delivery is complete after explicit owner save/closure and
fresh guard: DLL6170394f8218e38aa387e2e891ee21633f0e55462bbee59c044201b0069bfc56,
22 installed hashes match;144protected files and other five games unchanged.
Final712-input canonical/smoke/scan/install evidence is in sibling
Class-Delivery/build_check/ff4_party_turn_20261102_final/. Dependent fixtures use
the actual command-ready entry; production remains the qualified design above.
Installation8.931s/115068928B/exit0/drained under hard2GB/8.33%CPU. Natural
gameplay/audio/frame confirmation and remaining V/VI combined delivery are open.
