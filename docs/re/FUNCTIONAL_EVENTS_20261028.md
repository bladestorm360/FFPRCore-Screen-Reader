# FFIII functional event labels

Current broader scope: [six-game audit](MAP_EVENT_AUDIT_20261028.md) adds980 exact
functional reviews with full source/continuation hashes and12 locales, including
three Falgabard entry occurrences. Original21 FFIII entrance/barrier decisions
remain pinned; narrative Saronia remains Scene in source and its separate Events
destination is Saronia Airspace. FFIII final installed build9aff4280; other five
games' installs unchanged. Remaining ambiguous contexts are explicitly listed,
not inferred functional merely because their scripts lack dialogue.

Human explicitly requested Tozus entry, ship barriers and closed Living Woods
labels based on what the scripts actually do. Source complete, Closed October28;
estimate20-40minutes,
Sol only. Vehicle state, battle effects, returns and Saronia source are Closed.
This checkpoint changes reviewed labels/localization only, preserving native
scripts, story state, route validity, scene spoilers and bakes. After acceptance,
deploy FFIII before the separate all-six bake/event/reachability audit.

Reviewed full native scripts, outside Git in parent tools/mapdump/ff3_scripts.jsonl:

| Occurrence | Native script | Observed behavior | Label |
| --- | --- | --- | --- |
| Map1, Tozus event803 | 353/sc_e_0342 | Checks Mini, changes map45/point1; no dialogue/scene | Tozus entrance |
| Map1, closed woods event69 | 297/sc_e_0265 | Moves/spins player and companion back; no dialogue/travel | Closed forest entrance |
| Map4,12 barrier events for three aircraft families | 229/sc_e_0207,375/sc_e_0207_1,376/sc_e_0207_2 | Shake, barrier effect and repel aircraft; no dialogue/travel | Airship barrier |
| Map4, Doga approach events896/898/899 | 386/sc_e_0355,389/sc_e_0358,388/sc_e_0357 | Repels foot/Invincible/Enterprise approach; no dialogue/travel | Passage barrier |

Nepto Dragon obstruction298/sc_e_0266 starts a boss encounter and remains Scene
trigger. Saronia's narrative120/sc_e_0074 also remains a scene; its separately
published progression event keeps Castle Saronia (Airship). No future story
outcome or raw developer tag is spoken. Exact occurrence IDs and whole-script
hashes will pin these decisions in the existing reviewed-label generator.

Implementation:17 new exact occurrence/full-script-hash records in
tools/world_event_reviews.json, consumed by the existing reviewed identity path.
Only FFIII's catalog/audit was regenerated with audit_event_labels.build(3);
other games' labels/bakes are untouched at this checkpoint. Shared raw names
retain existing context-key disambiguation where prototypes differ. The four
functional labels are localized in all12 languages through
tools/modtext_functional_events.py; Tozus uses the game's official place spelling.
No new hooks, native calls, settings or runtime scan is added. Label lookups keep
the existing bounded dictionary/worker formatting path; native timing unchanged
by this data-only change, with real audible timing still unmeasured.

tools/functional_events_check.py verifies all17 actual source occurrences,
whole-script hashes, behavior class, effective raw/context labels and all12
translations, preserving Nepto's encounter and Saronia's narrative Scene labels.
It passes and is part of build.bat. Final canonical8321/0 names,142/349/0 wiring
and smoke47 pass. All67 embedded resources,136 indexed feature registrations,
all12 locales and six unchanged bakes verify. Final candidate11096064bytes SHA256
960a0c64da4755f17af7b0f4d6a38b6b231e8743a88ca32c3edded43b9583699.
Evidence:build_check/ff3_completion_20261028/. Combined FFIII scan/installation
completed: FFIII only installed/hash-verified;22 exact payload hashes match,
144 protected files unchanged and other five games' payloads unchanged. Defender
reports no threats; exclusion coverage unknown (OS denied query), no signing
certificate. No game launched. Source/delivery Closed; gameplay pending.
The separately authorized six-game audit may now start.
