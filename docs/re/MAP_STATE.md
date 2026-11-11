# Persistent navigation state, October 15

November4 [Pyramid of Moore](FF5_PYRAMID_20261104.md):the existing bounded
initializer VM now includes the exact returning needle helper before the
original tail,source-qualified physical doors/coffin enemies,and the two native
timed-pit poses. Six record bodies change;all metadata/other records remain.
One appended source-qualified quicksand landing retains trap semantics instead
of ordinary travel. No native writes,new VM opcode/work budget or terrain edit.
Current412/413/414/419/422/425 recovery is guide-owned through actual native
controls/returns,including the flagless corridor. Source/focused verification
and the final combined delivery are recorded separately in that topic.

November3 [Narshe party locks](NARSHE_PARTY_LOCKS_20261103.md): source/focused/documentation Closed.
Six doors require active Locke, with reachable fronts and manual unlocking;
opened receipts remain passable without him. Reader18/six-door93 and current
FFI/FFIII key regressions pass. No native writes/clone/hook or new puzzle recipe.
Combined delivery/gameplay pending; other43 Narshe interactions remain required.

November2 native activation correction is in source/integration qualification.
Every source-matched PuppetCore.Ready calls its actor's virtual Show(0) before
the new body executes. The navigation compiler now includes that activation
before explicit Hide/Show; an opaque final body clears an old hidden receipt
and remains uncertain. This fixes the concrete Catapult book lifecycle, which
previously stayed projected hidden after its SetPos-only reveal. All six exact
producer fingerprints are in puppet_ready_native_20261101.json. The private
projection still runs only on owned data/workers, with the existing instruction,
branch/actor budgets and no new native hook/read. Absolute spatial inference
retains its reviewed game-specific bounds; Ready activation is a separate
producer from SetPos itself. Focused regeneration changes only initializer
puppet effects at existing PCs/slots and preserves authored locks/event guards.
Basic source/Catapult/all-six state checks pass; affected route/guide consumers
are running before current-source evidence is closed. Details and prior-source
recovery: [Catapult dependency](FF5_CATAPULT_20261101.md).

October28 [all-six audit](MAP_EVENT_AUDIT_20261028.md) corrects the observed-group
integration: SetEntities observations are additive positive evidence, never a
complete inventory. Capture copies the bounded observed set; existing lazy worker
projection merges it with initializer certainty, visibility and position facts.
All-six real native-cache witnesses7 before failures→0; no eager owner projection.
See that record for rectangular counters, complete source/capture limits and
the separate per-leg vehicle applicability follow-up.

October26 [world event review](WORLD_EVENTS_20261026.md) copies FFIII's exact
all-active-member Mini predicate into an optional navigation scalar. A reviewed
SysCall becomes private entry-guard projection, never native interpreter execution
or flag writes. Known false hides Tozus; unknown preserves it. Scalar presence/
value participates in route signatures. Other games and optional filter defaults
stay unchanged; all-six regressions and final install hashes pass.

October25 [cross-floor follow-up](NPC_FLOORS_SHOP_LANDING_20261025.md) supersedes
the historical eight-unmapped limitation below: those were explicit flag_id0
objects, and zero is a valid first saved bit. The generator now includes all eight,
with1894 real mappings (249/235/415/396/279/320) and zero unmapped. Missing native
bits still stay unknown. The reported Tozus ordinary chest retains its remote
state separately from the new loose-pickup Treasure label. Partial initializer
projection retains only facts common to completed bounded branches; divergent
actors/unsupported operations remain conservative. Native provenance and current
tests are linked in that follow-up.

October23 [story-state extension](FF6_STORY_NAVIGATION.md) adds owner-scoped area/map
locals, additive native groups and bounded synchronous actor visibility/position
facts. Current live observations still own actor positions; pressure-plate
departure explicitly projects released gates without changing native state or
moving NPCs to baked homes. Metadata IDs such as PointIn remain separate. Native
physical Event actors can be hidden/shown too, as Phoenix's two rock controls
demonstrate. Tests now compare projected actor coordinates, not stale home tiles.

October23 [Castle Fynn](CASTLE_FYNN_20261023.md) generalizes only synchronous
initializer Hide/Show projection to all six games, based on matching native
SetPuppet/ExecPuppet/ShowHideCommon evidence. Absolute SetPos remains FFVI-only.
The hidden passage's persistent flag56 now hides actor22 on projected remote
floors just as the native initializer does. Unknown/asynchronous effects remain
conservative; no game script runs and no native state is written.

The user's FFII log shows opened native chests on Deist B1 losing their state prefix
when browsed from B2. B2's native chests then say Opened while the remote B1 chest
says only Treasure Chest. That is an unknown-state result, not evidence of the game
forgetting an opened chest. The preserved log is
`build_check/chest_state_2026_10_15/ff2_latest.log`.

## Native storage and the faulty boundary

All six matching dumps declare `Last.Interpreter.DataStorage`: persistent scenario
and treasure `Flags` references at 0x10/0x18, global int array at 0x20. Its nested
`Flags` class holds `uint[] bits` at 0x10 and logical `length` at 0x18. The shared
singleton survives floor changes. Recovered `ClearMapLocal` clears the map-local
array and selections; `ClearAreaLocal` clears area/map locals and selections.
Neither clears the scenario/treasure banks. The dump also declares separate
scenario/treasure arrays in DataStorage.Records for save state.

The constructor allocates `ceil(bitLength / 8)` **uint elements**. For 1024 flags
that is 128 words, even though `Flags.Get` indexes `bits[index >> 5]` and extracts
`index & 31`. Therefore only the first 32 words contain the 1024 logical flags.
The mod incorrectly required the array length to equal 32 and silently discarded
the real 128-word banks. Both treasure and story flags were affected. The current
floor's native `FieldTresureBox.isOpen` reader bypasses this path, which explains
the local/remote discrepancy. Observing the current entity group likewise does
not prove that other-floor story flags were captured successfully.

`map_state.cpp::copy_flags` now accepts complete backing arrays of 32 through 128
words with exactly 1024 logical bits, and still reads only 32 words. Missing,
short, oversized or wrong-logical-length banks remain unknown. No native writes,
new polling, native calls on workers, extra copy volume or game-thread waits were
added. Remote projection takes a fresh owned snapshot on map/group rebuild;
native chest entities need not remain loaded for this state to be available.

## Matching evidence and checks

Sol read the installed PE files as data; no game was launched or probed.
`tools/re/mapstate_storage.py` exports ten bounded named functions per game:
DataStorage constructor/getter/GetFlag/local clearing, and Flags constructors/Get/
Import/Initialize. It asserts the two 128-word allocations, 1024 logical lengths
and uint/32-bit indexing in every game. Full instructions, native RVAs, matching
DLL/dump SHA256 hashes and manifests are retained in the ignored investigation
directory's `ffN_storage_native.txt` and `native_provenance.json`.

`tools/mapstate_native_check.cpp` includes the actual production native reader;
only reflection/getter/native memory boundaries are fake. The 128-word native
layout reproduces eight failures before the correction, including zero resolved
chests in every game. The corrected reader preserves all 1024 scenario/treasure
bits, signed globals, boundary bits, minimal backing arrays, unavailable/malformed
state and immutable copies after native storage changes. Its worker consumes
only owned flags and real baked data, with zero native API calls.

| Game | Real chest mappings through native snapshot | Unmapped, still unknown |
| --- | ---: | ---: |
| FF1 | 248 | 1 |
| FF2 | 235 | 0 |
| FF3 | 414 | 1 |
| FF4 | 395 | 1 |
| FF5 | 275 | 4 |
| FF6 | 319 | 1 |

Both unopened and opened results are checked for all 1,886 mapped baked chests.
The eight unmapped chest objects retain their conservative unknown state; the
fix does not fabricate mappings or reveal contents. The canonical build now
runs this regression in addition to the existing initializer, cross-floor
projection, scanner, formatter and route checks. Earlier synthetic-Flags tests
proved those consumers, but missed this native allocation boundary.

The corrected fake-native capture measured 0.262 ms aggregate / 0.004 ms peak for
100 snapshots. These observations exclude real native reflection/cache cost,
game scheduling, whole remote projection and audible response. No runtime
performance claim follows from them. Full gameplay confirmation remains user-side:
browse an already-opened Deist chest from another floor under Current map, then
repeat the tracked Conversation route with real story flags available.

Canonical build and isolated smoke pass: 6,736 name checks, zero missing, zero
undefined action/query references, all 47 exports. All six closed games received
and hash-verified DLL SHA256
`A29CADC0C78AB6D51500876F57A1A7894167482B8E397C9528E55F7BA85D8A0D`,
9,529,344 bytes. Settings, Prism and waypoints retain their previous hashes and
timestamps; SDL3 and the eight pinned HRTF payload files match source. Recovery
and before/deploy receipts are `build_check/chest_state_recovery_2026_10_15` and
`build_check/chest_state_{before,deploy}_2026_10_15.json`. Build/smoke/deploy logs,
the failed-before regression and final reports remain in the investigation directory.
Sol implemented, reviewed and verified this repair alone. No game launch/control/
probe, dependency installation, new commit/push or publication was performed.
# Mt. Kolts explicit layer fact, November4

The existing owned initializer projection accepts an optional seventh field on
opcode10 for a proven layer0–2. Existing six-field records retain the native
home layer. The single initial consumer is FFVI Map_30360's exact40/not41 boss
placement;its compiler qualifier pins native SetPos23,32,2 and the initializer
fingerprint. Optional layer is part of EntityState equality/intersection and
effective-object projection. Invalid layers fail closed;no native reads,VM work
budget or asynchronous processing changed. See
[the source/route contract](MT_KOLTS_VARGAS_20261104.md).

# Loaded Great Sea Trench switch state, November 5

An optional `current_code` program applies only when the copied current map
matches the record's native map IDs. The one authored consumer is the Great
Sea Trench switch room. Its reload initializer can reset partial presses or
open the chest when all three receipts are set. In the loaded room, the copied
receipts preserve the selected switches and the actual chest and door state.
Both programs use the same bounded interpreter on owned worker data. No new
game-thread reads or work were added. [Trench evidence](FF5_GREAT_SEA_TRENCH_20261105.md).
## FFV Western Valley flag operands

Three branches in the Western Valley initializer use indices 417–419
with the first scenario flag category. FFV's native flag reader and
writer add the segment offset and bounds-check the absolute bit. The
offline authoring step stores the equivalent bank and relative index for
these three rows. The projection keeps the original instruction budget
and native reload semantics.
[Evidence](FF5_STORY_CONTACTS_20261105.md)
