# Imperial Camp visual masks and NPC visibility

Active finite checkpoint in the human-requested six-game map/reachability audit.
No runtime interpreter redesign, new hook, native read or collision/route-policy
change. Exact visual SysCalls in FFVI's offline initializer generator only.
Estimate20–40minutes plus combined release checks. Status: source/data/targeted
offline **Closed**; canonical/applicable delivery pending. Before8 failures→0.
FFIII0a511191 is installed; this FFVI correction is a separate later candidate.

## Exact source/native cause

Map_30410/master350 dog21/script249 has an unreachable authored home in the
initial geometry survey. The initializer's scenario81=1,82=1,90=0 path instead
calls its native Hide21 routine. Projection stopped earlier on visual mask
commands, so it could not establish that visibility fact. Early/late flag
samples are initializer witnesses, not claims about valid whole-story states.

Complete matching native source/provenance stays outside Git in
../tools/npc_geometry_context_20261029/ff6_entry_native/. Current cctor2C85A0
registers these exact method-metadata globals and names:

| Command | Metadata global RVA | Native handler RVA |
| --- | --- | --- |
| Imperial Camp mask present | 25E28D8 | ShowNearMaskImperialCamp2BFE90 |
| Imperial Camp mask absent | 25E7090 | HideNearMaskImperialCamp29CED0 |
| Imperial Camp mask expand | 25E7CC0 | ExpandNearMaskImperialCamp298750 |
| Imperial Camp mask reduce | 25E49B0 | ReduceNearMaskImperialCamp2B0910 |

All four use IMapAccessor slot84 ScalingAlphaMask. FieldController313750/3136A0
dispatch to renderer B2D700/B2D870 and ForegroundAlphaMask88CDF0/88CD00.
ChangeMaskScale88C4D0 writes material texture/scale/center parameters;
UpdateScroll88CEE0 uses camera coordinates/time/easing for that rendering.
Complete bodies do not write flags, entity groups, actor positions or collision.
The four exact Japanese registration strings, global addresses and matching
method addresses are verified in next_audit_mask_bindings.json under
build_check/invincible_preference_20261029/. No broad name/prefix inference.

## Correction and acceptance

`tools/bake_map_state.py::instruction` emits the existing no-op for only these
four exact FFVI calls. Other games and unknown calls retain their current
semantics. Return-value calculations unsupported by the existing model remain
unknown. This is an offline authored-data correction, not execution of native
scripts or game-state mutation. Existing projection instruction/state limits,
worker ownership, copied flags and lifetime remain unchanged; no new runtime
framework, allocation, pointer, polling, setting or translation is introduced.

`tools/mask_projection_check.cpp` reads real maps/state and calls the production
projector/resolver. Both mask branches and the dog visible/Hide21 branches are
checked. Before: all4 projections are unknown and both Hide21 cases remain
active (8 failed assertions). After must establish the exact visibility and
ordinary NPC eligibility. Exact generated-program delta, all-six unchanged
terrain/metadata and existing visibility/collection checks are required before
closure. Preserve source/targeted offline versus later canonical/deployment and
unconfirmed actual-game trajectory/timing evidence separately.


## Accepted data and consumer boundary

Final projected_geometry.txt passes both mask/visibility branches and the
unknown-map-local/current-versus-unloaded-floor cases. Even when overall group
projection remains unknown, both branches retain the guaranteed Hide21 fact and
exclude that NPC. With the native visible-stage flags/local mode known, actual
projected obstacles/transitions admit talk approaches at the home46,11 and the
scene's44,8/45,7 poses. The earlier empty-transition geometry output is diagnostic,
not final route evidence; the accepted check uses the production state/relay/
obstacle projection. NPC congestion is deliberately excluded to isolate geometry.
The scene sc_e_0111 moves dog21; its continuation sc_e_0111_1 sets82 and hides it.
Thus a coarse unknown/conservative authored-home survey did not prove bad terrain
or an incorrect NPC layer. No invented counter or authored position was added.

Exact delta.json proves only Map_30410 code indices4/8 change unknown→no-op;
every other record/field and all six terrain/object bakes are identical. Other
five state files are byte-identical. All-six event tracking2454 roots/2523 unique
identities passes with0 failures. Runtime opcode interpreter/capture/native
ownership is unchanged; additional existing bounded projection remains on its
owned worker. Whole-game story trajectories and audible/frame timing are not
inferred from these offline flag witnesses. No game probe or FFVI deployment.
The generator/visibility component is Closed; continue the remaining audit with
its existing design. Later combined release must include this exact delta chain
instead of assuming every mapstate file is unchanged.
