# FFIV map/story puzzle review

## November 10 Troya Castle treasury

In `Map_20111_6` (Troia Castle B1, map 102), all five contacts share
`sc_s_20111_2`. Native gates determine the accessible contact at each stage.
The southern contact opens the first gate. The next three contacts open
the other three gates in sequence. The last door's far-side contact is
behind the fourth gate and retires when the sequence finishes.

| Guide step | Contact entity | Bottom cell | Native receipt |
| --- | --- | --- | --- |
| 1 | 41 | (7,10) | 274 |
| 2 | 43 | (2,6) | 275 |
| 3 | 45 | (5,6) | 276 |
| 4 | 44 | (8,6) | 279 |

The receipts are scenario bank 2 bits 18, 19, 20 and 23. In a live run the
next contact remains visible after the gate opens. The room initializer
can hide that contact when an already-open gate is restored. The entrance
switch remains available until completion and can run the next branch of
the same script. The existing navigation worker tries at most two exact
contacts in one native capture, preferring the numbered contact and
avoiding unrelated active step events. If the selected switch is already
underfoot, the guide asks the player to step off and back on. Bit 279 also
appears in the jail passage, so the guide checks each ordered stage receipt.

B1 entry announces availability, and Shift+G starts or repeats guidance.
The controller check exercises entry, movement receipts, resume, fallback
speech and completion. The production route check uses the native stairs,
the three logged switch poses and the reloaded room state with the native
gates still in place. Fallback instructions cover all 12 locales.

## November 2 review

November2 at18:16UTC: the initial dialogue-owned key-item reader and deferred
generic-cursor suppression were installed in FFIV payload011f0dc2. Full canonical/
smoke/scan and guarded22-file installation pass,144protected/otherfive unchanged.
Receipt: sibling Class-Delivery/build_check/ff4_party_keys_20261102/completion.json.
Natural Sand Ruby opening/I/Autodetail confirmation remains separate. The final
all-six main guide installation is still part of the unfinished original audit.

The single scope ledger is MAP_AUDIT_FOLLOWUPS_20261028.md. Sol only; no game
launch, attach or runtime probe. Existing Mist, map/scene/gate and static switch
proofs remain preserved. This topic records the new finite puzzle slice;
canonical build, security, deployment and natural play remain separate.

All112 broad/animation candidates have explicit source-hashed semantic decisions
in puzzle_semantic_review_20261101.json. An independent item-selector census adds
18 exact caller decisions, including the Agart well absent from the broad name
discovery. Ordinary NPC quest exchanges, optional storage/performance, automatic
story/vehicle transitions and passive switch-door counterparts are distinguished
from unique environmental mechanics. Same-named0156/0157 in185 are Scarmiglione
encounters, not the Baron key doors in24. Native implementations stay outside Git.

## Native guide families and retirement

Six additional static families contain ten exact operations. Existing thirteen
FFIV recipes are preserved; shared totals are86 operations/52 recipes. Authoring:
tools/author_ff4_puzzle_guides.py; four original instructions/five official item
terms in12 languages: tools/modtext_ff4_puzzles.py. Native bank2 receipts use
the existing shared index256+native bit; never treat them as bank1 flags.

| Family | Actual operation and progress |
| --- | --- |
| Baron keys |24/66 item68→271, then24/67 item68→272. Native31 proves Yang/key acquisition. Later33 retires both old controls; completion outranks an unopened optional shop bit. |
| Magnetic cave |100/22 gives Twin Harp69→34;201/5 first encounter231 and automatic harp chain→97;201/4 second encounter232→36; animated crystal201/3 awards66→98. |
| Agart well |109/3 item72, requires43. Native109→world1→109 cinematic ends at0045_2 receipt44, not proximity or initial item use. |
| Babel key |220/28 item75→273 after47. Message-only broken-door26 is a different state. Later48 retires the key event. |
| Dwarven passage |125/17 guard opens the passage→278. Contact event24 owns the same operation; it is not an additional ordered puzzle. |
| Sealed entrance |268/6 item74→58. Necklace award56 alone does not spawn the control: initializer requires57. Event7 shares the physical operation. |

Magnetic guidance first obtains the harp, then instructs removal of metal
equipment while exploring. The initial encounter is not the later boss variant.
Its native automatic chain goes201→100→201, stops the magnetic effect and only
then sets97. Guidance then requests metal equipment again, routes to boss4 and
finally the real animated crystal. Battle36 alone never finishes the guide.
Manual equipment, item choices, battles and cinematics stay native; the mod does
not change equipment or submit selections. World fetch/return legs explicitly
permit the existing vehicle-aware world router. Target-specific fixture journeys
separately qualify the ally room and the full cave from its native entrance;
the earlier shared mounted/world gate evidence remains distinct from that fixture.

## Native item-menu integration

FFIV uses SelectFieldContentManager for these item choices, separate from the
ordinary ItemWindowController. The pre-change dedicated adapter was FFIII-only.
src/games/ff4/event_item_select.cpp adds two scoped postfix hooks: ready KeyInput
SelectContent(int,WithinRangeType) and manager Close. Exact body/ABI evidence is
tools/re/ff4_event_item_contract.py: DLL bb2f4c9db44c8ee9b065696aeafb77561a1709492130f045433e6d215abc42ed,
dump011bc480d50bba3784b001844f854eae9abbe7a0050cb91c6e82146dffacc916;
native828460/704C70 must remain unique, non-forwarded bodies. Typed fields are
the base contentDataList, manager controller, and row NameMessageId/DescriptionMessageId.

The native original runs first. Observation verifies the active manager/controller,
copies one ready row from at most512 entries and limits both identifiers to128
characters. No scan, input, write to game state or worker-owned native pointer is
added. One native selection speaks once, including equal-text repeated events;
no text deduplication is introduced. Autodetail retains its independent preference;
I repeats owned current details or No description. Invalid/hidden/replaced owners,
Close, map changes and title clear state/details. Generic cursor work queued before
the ready postfix rechecks this exact owner before speaking, avoiding a late second
row announcement while retaining ordinary generic fallback.

The active static guide permits owned instruction repetition in native FFIV item
and equipment menus. That path reads no native progress, performs no routing and
preserves cancellation. Shared GameState uses the same EventItemSelectState owner
for event-UI readiness. Do not broaden these hooks to another game without its
exact ABI, ownership, source hashes and producer checks. FFIII's established reader
and its local legacy guards remain unchanged.

November2 owner initial-selection follow-up is source/focused Closed. The
installed delivery branch has no FFIV adapter at all; the correctly name-resolved
main adapter is unchanged. Exact native KeyInput Open828350 forces initial
SelectInit828690, which reads native cursor30/index18 and dispatches SelectContent
through slot9/offset1C0. No arrow movement or new native hook is required. The
older proof incorrectly labeled Touch8091D0 as KeyInput; production class lookup
was correct. Four unique body hashes and this initial-ready dispatch now pass
the corrected contract. Prior proof is retained in the original checkpoint and
build_check/damcyan_20261102/ff4_initial/prior_contract_touch_label.json.
Actual adapter16cases plus preserved FFIII suite pass, including initial row,
equal-text events, I, Autodetail, invalid/hidden/replaced owners and retirement.
35.919s/116199424B/exit0/drained under2GB; native proof1.824s/115802112B. Exact
inputs/reports are bound in ff4_initial/completion.json. Next qualified FFIV
delivery must port the adapter and generic queued-cursor owner suppression;
installation and natural Sand Ruby confirmation remain pending.

## Verification and remaining limits

Initial shared guide check caught a stale expected-count assertion; preserved
ff4_shared_initial_report.txt. The corrected shared/controller/static checks pass86
operation identities,18 prior new-control entrance witnesses and12 Citadel resumes.
Receipt ff4_guides_retry_budget.json:100.880s/490766336B, exit0/drained.
The first journey run passed all magnetic stages, Baron, Agart and Babel; the
Sealed test omitted mandatory57. That native initializer fact corrects its fixture
and guide prerequisite. Two new Dwarven cases were also compiled before their
resource regeneration; preserve the failed report/receipt rather than claim they
passed. Current reader/native-contract/full-journey qualification remains active.
These resource-throttled source fixtures do not prove actual frame/audible latency.

Final source/focused checkpoint is Closed. Full qualification passes23 episodes/
33 production route legs, including both source-authored post-harp proxy12,15
and post-victory leader12,14 positions. The early fixture incorrectly reused the
boss's occupied12,13 cell; it is corrected without changing production routing.
The game always captures its actual current player on a new request. Receipt
ff4_pose_qualify_budget.json:38.290s/432295936B, exit0/drained. The initial21-case
report remains preserved separately rather than replacing that evidence silently.

Native contract/source/reader/controller chain passes with15 FFIV item-owner
cases (including100 equal-text native selections), the preserved FFIII adapter
check, deferred generic-owner handoff and shared86-operation/52-recipe checks.
Receipt ff4_native_qualify_budget.json:68.993s/456949760B, exit0/drained. Final
exact automatic harp-chain, authoring,12-language,112-candidate/18-item-caller
source hashes and8330 reference names pass: ff4_final_contracts_budget.json,
10.086s/261992448B, exit0/drained. All limits remain aggregate1 GB/serial.
Current source/data/reports are frozen in ff4_guides_checkpoint/; original
failures and prior Closed checkpoints remain intact. Main DLL/canonical/smoke/
security/all-six delivery and natural confirmation are still pending. Next finite
system is FFV semantic/unique mechanics coverage, then remaining FFVI families.
