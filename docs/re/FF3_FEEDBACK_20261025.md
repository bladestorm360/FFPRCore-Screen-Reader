# MP details, world transitions, landing and undead healing, October25

[Subsequent loading-context correction](LANDING_LOAD_20261025.md): the user
confirmed an unwanted Can land during save construction. A visible preloaded
guide alone is insufficient; readiness/fade guards now seed that state silently.
The build/delivery evidence below describes the prior combined payload.

Status: implementation complete and offline verified. Canonical build, all checks,
isolated smoke and exact-payload Defender scan pass. All six games are installed
and hash-verified after the user closed III. Final deployment receipts are
recorded below;
gameplay listening remains unconfirmed. Sol performed the work without delegation,
game launch/control/probes, dependency installation or publication.

| Feature | Implementation / offline status | Gameplay evidence |
|---|---|---|
| FF1/FFIII MP-only I details | Complete; actual-source overview, target and battle queries pass | User confirms the prior build restored I; new payload needs listening |
| FFIII world transition identity and Alt+Backslash | Complete; real1535-row bake replay and both keymaps pass | User's own log confirms the existing grouping reduces this state to8 |
| All-six native landing speech | Complete; exact native producer review and actual-source detours pass | Cid's first airship near Kazus needs confirmation |
| All-six HP damage/recovery polarity | Complete; rendered94/nominal9999 regression passes all ABIs | Cure against Skeleton needs confirmation |

## MP detail owns only the optional tail

The preceding [I ownership fix](INN_COUNTER_MP_20261025.md) restored the key;
the user confirmed it works. Its manual payload still repeated the ordinary
character row. `ui/character_row.cpp::details` now returns only the visible
current MP counts for levels1..8. It retains native phase/cursor ownership,
positive MainGame Menu5 and visible-row checks. Overview job/TNL ordering and
the short HP/status focus read are unchanged. Item/spell targets use the same
service; closing them still restores item/spell description ownership.

`battle_party.cpp::BattleParty.ChargeDetails` likewise returns only the selected
visible charge cells, using the focused ally during targeting or the active
actor during command selection. Ordinary H/party/target rows retain their
existing vitals and command-column policy. Scalar-MP games retain their layout.
The existing Autodetail producer speaks the ordinary row once, then its charge
tail once. No text comparison or new deduplication was introduced.

The `party_esper_check` actual-source fixture checks FF1 and FFIII, both
Autodetail settings, native IsFocus=false overview selection, item/spell targets,
manual overview I and the real registered battle command/ally-target query.
Details begin with Level1 MP and end with Level8 MP; they exclude character
name, HP, command and the deliberately unrelated scalar MP999 field. Native
reads remain on their current UI owner; copied strings use the speech service.
Charge observation is bounded by the eight displayed levels, with no new poll.

## Why the world list contained1535 events

The user's FFIII log records1535 Event rows on world map1 / `Map_10010`.
Native map data and reviewed script families explain the count:

| Native purpose | Active rows in the reported early state |
|---|---:|
| Enterprise world-border transition, script341 / sc_e_0336 |508|
| Nautilus world-border transition, script345 / sc_e_0338 |508|
| Invincible world-border transition, script349 / sc_e_0340 |508|
| Lake entry, script390 |7|
| Tozus entry353, passage379, world-circuit380 |3|
| Current conditional rock/airship scene in ev_e_0024 |1|
| Total |1535|

These1524 repeated border tiles are real travel scripts, not animation events.
Their native ChangeMap branches lead to world maps2/4, with vehicle/scenario
conditions. The world map's three conditional AnimEntity records have no action
or script and are already excluded by existing scanner policy. Other conditional
event variants are state-dependent;1535 is not a universal count for all progress.

`tools/world_transition_reviews.json` records three exact raw-name/script/group
families,508 copies each and canonical JSON script hashes. The identity generator
and audit require that evidence, rather than treating arbitrary script-bearing
entities as transitions. Only these three FFIII labels change from Scene trigger
to the new12-language World map transition label. The other five games' generated
label resources are unchanged. Native Event activation/category, IDs, vehicle
requirements, route geometry, unlocks and all other scene labels remain intact.
This is a label correction, not a new world travel route or a vehicle bypass.

The existing EventGroupFilter already collapses the reported state to8 logical
events, as the user's subsequent own-log entries also show. Its default binding
is now **Alt+Backslash** on the field, in both keymaps, using the same registered
action/remapping identity and F8 Navigation toggle. Saved custom remaps and the
persistent default-off preference are preserved. The previous default was
Ctrl+Shift+P. Grouping remains independent of the exit filter and soundscape's
physical grouping. The real-bake `navigation_groups_check` verifies1535 ungrouped
rows,8 grouped rows and unchanged raw physical audio registry; existing explicit
target retention and nearest-reachable representative behavior still apply.

## Landing speech observes the actual guide producers

FFIII FieldController.UpdateStateSwitchLandable calls MapUIManager.ShowLandingGuide.
That function activates the guide and directly calls LandingGuideController's
SwitchLandable leaf, bypassing MapUIManager.SwitchLandable, the old mod hook.
The leaf shares a native address with AirshipEventGuideController's mountain
indicator, so hooking it would capture an unrelated UI purpose. Matching native
functions were reviewed across all six games; all names/signatures pass the
build's installed-dump verification.

`fieldnav/vehicle_landing.cpp` now observes both MapUIManager.ShowLandingGuide
and SwitchLandable after their originals. One shared state edge speaks Can land
when a currently active guide becomes landable. It validates the owner class
and current guide hierarchy. A second callback with the same native state adds
no event. This retains the established state guard, not text deduplication.
The guide's visible native boolean is authoritative; stale movement-speech
on-foot state during boarding or a boarded save no longer suppresses the line.
Native HideLandingGuide, MapChanging and BattleEntering reset the edge. Updates
from another hidden guide cannot clear the active guide's state.

The cached guide pointer is only a borrowed identity token; each observation
obtains and validates the current native-owned guide before comparison. No
worker dereferences it. There is one field/visibility observation per native
callback and no collection, scene search, terrain calculation, timer, wait,
input action or new worker queue. The shared last-announcement timestamp lets
FF6 heading/altitude retain its existing0.5-second Can land interruption priority;
FF6's redundant SwitchLandable timestamp hook is removed.

`vehicle_landing_check` exercises the actual production detours in all six game
modes with a deliberately stale on-foot cache: initial Show without Switch,
repeated Show/Switch, false-to-true changes, another hidden owner, hide/reshow,
context reset, original-call counts and owner-thread observation. Zero failures.
The canonical build now includes this check. Actual game-frame cost, queue age
and first audible response remain unmeasured; fake SDK checks do not certify them.

FFIII native locators: FieldController.UpdateStateSwitchLandable296900,
MapUIManager.ShowLandingGuide31D240, HideLandingGuide319650,
SwitchLandable31D460, landingGuide field1E8; folded leaf3BB350 is avoided.
The original mod extractor `tools/re/map_location_native.py --landing-guidance`
archives all-six functions/provenance outside Git in
`../tools/landing_guidance_20261025/ffN`. No game source is stored here.

## Cure against undead follows rendered HP polarity

The user's own FFIII log records Skeleton: Recovered94HP followed by Skeleton:KO.
`battle_message.cpp::damage_view` previously let nominal HitType.Recovery4
override the innermost rendered isRecovery=false. A recovery-class function can
produce damage against undead, so its nominal type does not establish healing.

The inner ViewDamageEntity.CreateDamageValue recovery flag controls the actual
digit recovery colour. HP feedback now follows that flag alone. Rendered digits
still supply the number; internal nominal amounts are not substituted. Misses,
explicit MPHit5/MPRecovery6 and the established three-part MP absorption producer
retain their separate resource logic. KO and native phase ordering are unchanged.
No spell-name whitelist, enemy taxonomy, HP-delta inference or hidden parameter
read is added. Existing damage/recovery translations cover all supported locales.

`battle_feedback_check` enters the actual production detours using every game's
ABI, renders94 while passing nominal9999, and checks: Recovery4 with false gives
94damage; Recovery4 with true heals94HP; ordinary type with true also heals;
Miss,94MP damage and recovered94MP retain correct wording. Existing Parasite,
Swap, status-removal, visible-stat and rendered-feedback regressions also pass.
The change adds no native observation, work queue, scan or scheduling stage.

FFIII native locators: BattleBasicFunction.CreateDamageView94B6D0,
BattleUtility.CreateDamageView6EE220/6EE000 and
ViewDamageEntity.CreateDamageValue419700; recovery-only SetColor419EB0.
All-six functions/provenance are outside Git in
`../tools/hp_polarity_20261025/ffN`, generated by the original mod extractor
`tools/re/battle_feedback_native.py --hp-polarity`. FF2's extra SE argument,
FF5's MissType and FF6's renderer offset ABI are retained.

## Verification and delivery

Evidence/recovery directory: `build_check/ff3_feedback_20261025/`.
Canonical build and every offline check pass:7843 native names,0 missing;
133 used/341 defined wiring names,0 undefined. The isolated smoke host resolves
and forwards47 HID exports and exits normally. All49 embedded data and18 sound
resources match current source. Final DLL:10,916,864 bytes, SHA256
ab54b0b0b99100c191d998a8e89df0172d4bf826e84ce9bb82d425d3376c8389.
Defender reports no threats across the exact22-file staged payload, hashes
unchanged. `verification.json` records source hashes and copied check/build/smoke
reports; `security_final_predeploy` retains scan/manifest/signature evidence.
The mod DLL is unsigned; no signing certificate is installed. Protected exclusion
inspection is unavailable; no protection or exclusion setting was changed.
No claim of universal scanner acceptance or game runtime performance is made.

Deployment uses only deploy.bat, which refuses a running game's replacement.
Final deployment receipts and installed manifest will record delivered games and
protected-file preservation. Listening checks after installation, in priority:

1. Cure an undead enemy naturally: damage amount followed by KO if defeated;
   ordinary healing must still say recovered HP. Failure: recovered HP on harm.
2. On the first FFIII airship, move off/on landable ground near Kazus: Can land
   on entry, no repetition while remaining landable. Failure: silence or chatter.
3. With Autodetail off, highlight a character then I: only current MP levels.
   With it on, focus gives the short row once followed by MP once. Repeat FF1
   and battle targets when available. Failure: repeated name/vitals or silence.
4. On the world map, Alt+Backslash toggles event grouping. The reported early
   state becomes8 groups; World map transition replaces generic Scene trigger
   for the reviewed border families. Later-state counts may legitimately differ.

These tests depend on the relevant gameplay state and remain user-side. No
uncertainty here blocks unrelated implementation or requires a runtime probe.

Initial delivery: deploy.bat installed the exact scanned22-file manifest into I/II/IV/V/VI;
all110 installed file hashes match. Protected files in those games are unchanged
against deploy_finalbefore.json. FFIII's running-process guard refused replacement
and its prior DLL hash/size/timestamp are unchanged. deploy_partial.json and
installed_payloads_verified.json retain the evidence; completion.json records the
implementation/offline outcome separately from the pending III deployment and
unconfirmed gameplay. No game was closed or controlled by the agent.

Final delivery after the user closed FFIII: deploy.bat ff3 succeeded. All22 scanned
payload files now match in every game (132 hashes);144 protected files are unchanged
against the post-close deploy_preclosed.json baseline. The prior partial receipt
and installed_payloads_partial_verified.json remain for history. Final evidence:
deploy_after.json, installed_payloads_verified.json and completion.json. The same
tested/scanned DLL and source hashes were verified before deployment; no rebuild,
game launch, runtime probe, dependency install or new delegation was required.
