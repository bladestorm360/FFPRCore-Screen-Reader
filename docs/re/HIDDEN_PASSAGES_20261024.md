# Concealing objects for hidden-passage interactions

Current outcome: implemented and offline verified. The combined tested/scanned
payload is installed/hash-verified in all six games after FFII closed. Player
confirmation remains pending. [Current delivery and evidence](COMBINED_MAINTENANCE_20261024.md).

Implementation/offline verification complete. The user requests the
actual concealing object (for example Clock or Fireplace), using scripts,
dialogue/context or art evidence rather than Hidden passage as its object name.
The standing pathfinding-identification approval applies; no game probe.

## Reviewed occurrences and source chain

The all-six catalog audit finds generic Hidden passage object labels only in
FFII and FFVI. Exact native map/entity/type/raw identities distinguish the
shared FFII identifier; a global replacement would choose the wrong context.

| Game/map | Native object | Name | Offline evidence |
|---|---|---|---|
| FFII21, Map_20031_1 | entity36, type5, editor38 | Wall near bed | Paul's clue E0128_00_021_d_02 identifies the wall by his bed; object script249/sc_e_0216 and exact room placement |
| FFII43, Map_20041_7 | entity22, type5, editor59 | Wall | script87/sc_e_0063, E0063_00_043_a_02 describes the gap/secret door in the wall |
| FFVI3, Map_20010 | entity132, type5, editor215 | Rock | script118/sc_e_0089, E0089_00_003_a_02 identifies the stone that is twisted |

Dialogue/script bodies remain outside Git in ../tools/mapdump/ffN_scripts.jsonl
and ffN_master/story_mes_en.txt. Repository records contain factual locators and
classifications only. Existing FFII118's separate Josef rock switch remains Rock.
FFVI's Zozo Clock already uses the actual object name and withholds its solution.
Fireplace and other established physical names remain unchanged. The audit does
not rename automatic dog/cutscene helpers or hidden-passage treasure chests:
their current categories and type-only treasure labels remain appropriate.

## Implementation and verification

Three exact records in tools/event_identity_reviews.json feed the established
reviewed_identity resolver/generator. Regenerated only FFII/VI eventlabels and
their audit records. FFII's ambiguous global raw name is neutral, with explicit
map21/43 native context keys supplying the two physical labels. The same keys
serve live interaction cues, Objects lists and copied remote/route labels.
No native C++ naming branch or raw routing/selection identity changes.

Wall near bed is localized in all12 languages and retained in the maintained
modtext_object_followup recipe. Existing Wall/Rock/Clock/Fireplace translations
remain authoritative. Native cue checks exercise the actual label resolver,
context differences, Josef's unchanged Rock and Clock without its solution.
The full actionable-identity check passes all six source/key tables with zero
unresolved or mismatched entries; final embedded/native cue verification passes.

This changes labels only: no collision, baked grids, script eligibility, story
flags, route behavior, controls, settings or new native work. Player-side names/
listening remain unconfirmed. Source backups/review deltas are in ignored
build_check/hidden_passages_20261024/; the combined payload/evidence will be
recorded under build_check/stat_effects_20261024/.
