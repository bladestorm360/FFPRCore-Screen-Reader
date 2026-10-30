# Invincible ladder and semicolon Elder label

Source/offline/FFIII delivery Closed in75de3594. All-six/all12-language NPC,
semicolon parser, exact two ladder-label delta, combined build/smoke/native
names/resources/12 locales/22-file Defender scan and installed hashes pass.
144 protected/other5 installations unchanged; no game launched/probed. Exact
receipt: build_check/ff3_oct29_followups/completion.json. Gameplay/audio pending.

Human-scoped FFIII follow-ups during the Torna audit. Estimate10–20minutes plus
combined build/release checks. No general naming refactor or map routing change.
Sol only; no game launch/control/probe. Installed FFIII remains ad0d80c1 until
the combined candidate is checked/scanned/deployed. Status source implemented,
new labels/source checks pending (~65%).

Invincible's two real ladder triggers are Map_20301_1/editor50/native1/script334
sc_ship_0007 and Map_20351_1/editor50/native1/script331/sc_ship_0004. Maps143/157
share the first asset;342 uses the floating-continent form. Both only play sound,
fade and return through their map transfer; the first chooses the appropriate
surface variant. They contain no story dialogue or encounter. The label is
Invincible ladder, in all12 supported languages. Full source hashes and exact
native actor identity are pinned in world_event_reviews.json by the original
review_invincible_ladder.py; actual scripts stay outside Git. No fixed landing
point or replacement map transition is invented for this dynamic vehicle return.
Existing event publication and routing keep their ownership.

The reported “;老人” comes from Falgabard Map_20310 (map144), native actor27,
raw7;老人2. Its existing translation key already has all11 non-Japanese languages.
The translator strips suffix2 and prefix7 before trying that full key; neither
老人 nor ;老人 is a dictionary key. Thus the full authored translation is lost.
The minimal correction tries the exact existing key before decoration parsing,
after the preserved story-name mask/Japanese policy. The shared NPC parser also
recognizes ASCII/full-width semicolon after a numeric developer prefix, retaining
that number as ordering metadata. Japanese stays Japanese and loses the same
developer separator. Existing contiguous NPC suffixes remain centrally owned.

The actual all12-language live/copied-label checks fail before this correction;
the source7;老人2 case is now explicit in npc_label_audit_check.cpp. Semicolon
parser cases are added to npc_names_check.cpp. Existing all-six exhaustive label,
stable ordering and unrevealed-name-mask checks remain required. The earlier
audit verified decoration/numbering but did not certify every translation's
lookup success; this regression must not be called previously covered.

No new native read, hook, timer, speech event or content deduplication is added.
Exact lookup reuses the immutable language catalog and existing label pipeline.
Native/game-frame and audible confirmation remain separate from offline checks.
