# Native map floor labels, October25

Implementation/offline verification complete. Installed/hash-verified in all six.
FFII deployment succeeded after the user's deploy reply; prior running-process
deferral remains in the history receipts. Gameplay/listening remains pending.

The reported FFII quicksave is correctly read from the game's displayed location
text. Latest installed log:78972.484/78975.734 displays Pandaemonium–1F; after loading,
78982.015 navigation announces13F, and78982.515 confirms actual map314. The native
map master has Floor13 for that map, but its entrance points have MSGARA_NAME_01
(1F). This is a label fallback error, not evidence of a wrong loaded map or save.

## Data flow and evidence

`system/save_load.cpp::read_from_controller` reads the rendered area/floor text.
Native KeyInput SaveContentController.GetLocationTextBySaveData (FFII RVA8e45c0)
resolves SaveSlotData.CurrentArea/CurrentLocation through MessageManager. Native
FieldController.MapSetup (2824d0, PointIn handling284ae7..284c7d) copies the
entrance's title_id/title_type into current title state and CurrentLocation;
AreaOnly/SilenceAreaOnly clear that location, while MapOnly variants clear area.
GetMapTitleID27a9b0 reads mapTitleMessageId. Names/fields resolved from matching
dump; no code/ABI offsets added to production. Original bounded extractor:
`tools/re/map_location_native.py`. Proprietary disassembly and matching binary/
dump hashes stay outside Git at `../tools/map_floor_20261025/ff2/`.

`field/map_names.cpp::resolve_map_name_by_id` serves current-map/map-key names,
exit destinations and remote floor labels. Existing precedence is authored room
override, explicit master MapTitle, then a newly extracted native entrance floor,
then legacy numeric Floor fallback. `map_exit_name` retains its existing cache;
runtime language-switch cache behavior is outside this fix. No new hooks/settings.

`tools/map_entry_floors.py` reads outside-Git master/object extracts and stores
only factual map/message IDs in each existing map_titles.json under @entryFloors.
Every PointIn must have the same nonempty title_id and type1/4 (area plus map name,
including silent variant); the message must be a numeric above/below-ground floor.
Conflicts, blank entrances, area-only/map-only names and explicit master titles are
excluded. Authored room labels retain precedence, including Statue Room/Roundworm.
All labels resolve through the game's own current-language messages. No arithmetic
offset and no speculative reinterpretation of internal map IDs or Floor values.

Pandaemonium IDs314/317/318/319 are1F/2F/3F/4F;320 and321 are both5F;
322/323/324 are6F/7F/8F;315/316 are9F/10F. The5F side room disproves a blanket
minus12 conversion. All11 maps' entrances agree, including type4 silent entries.

## Ownership, bounds and safe extension

Only offline original extraction and embedded owned strings are added. Existing
owner-thread master/message calls and workers keep their current lifetime/phase
contracts; no new game-thread scan, I/O, queue, worker pointer or synchronization.
One resource-object lookup plus map-key lookup/message resolution is added only
when authored/master titles are absent. Static entries are bounded by the finite
map roster. Routes, baked connectivity/geometry, selected identities and progression
conditions stay untouched. Adding another floor source requires native semantics
and unanimous entrance evidence; never infer a displayed floor from numbering.

## Verification and remaining gameplay check

Evidence receipts are retained at build_check/map_floor_20261025/. Data audit
compares all six regenerated tables and exercises ambiguous/empty/area-only/title
exclusions, with explicit11-map Pandaemonium expectations. Production resolver
coverage passes4944 cases (412 active keys across12 locales), including current,
map-key, exit/cache and precedence paths. All430 generated keys have official
messages in all12 native tables.385 English floor labels change: II149, III35,
V24, VI177; authored overrides remain. I/IV have no active change under this rule.
Only the six floor-title resources differ from the prior scanned payload; all
authored keys and every other data/sound resource match, including routing bakes.

Full canonical build/all checks pass (7719 native names/0 missing;131 used/339
defined actions/0 undefined). Isolated47-export smoke passes;49 data+18 sound
resources match source. Final10902016-byte DLL SHA256
8e47ebf353835ccb715241973848462bc2e06f885ae099b73debf516d32a1155
matches the smoke copy and exact22-file Defender stage, no threats/hash changes,
current enabled definitions1.461.186.0. All six games match all22 scanned files;
144 protected files are unchanged against fresh deploy_preff2.json. Recovery,
partial/final delivery and installed_payloads_verified.json are retained. Four unsigned
DLLs/no signing certificate and protected exclusion-query limits remain.
Native game/audible/frame latency remains untested. No protection changes, game
probe, tool install, delegation, commit/push or release action.

Investigation/fixture corrections are recorded in investigation_limits.json;
the early fake-SDK pointer/signature errors and JSON fixture isolation mistake
were corrected. Production creates a fresh resource value. A bounded Ghidra
decompile produced no output; matching-PE disassembly settled the native data flow.

When progression permits: load that quicksave and use the map key; expect
Pandaemonium1F. Pathfinder exits/remote floors should use the same native labels,
including the5F side room. Wrong13F, a renamed physical route or selection change
is a failure. Gameplay/listening remains user-side; no game launch/probe authorized.
